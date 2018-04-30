#include <iostream>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <errno.h> /* perror ()*/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> /* close() */

#include <pthread.h> /* POSIX threads */
#include <sys/wait.h> /* waitpid() */

#define TAM_PACK 128
#define PORT 8000

using namespace std;

struct sockaddr_in server_addr; /* Local address */
struct sockaddr_in client_addr; /* Client address */

int server;

struct thread_data
{
	int thread_id;
};

int create_TCP_server (int num_clients)
{

	int status;
    bzero((char *) &server_addr, sizeof(server_addr));

	//define endereço destino
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == -1)
	{
		perror("SERVER_ERROR socket().\n");
		exit(1);
	}

	status = bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(status == -1){
		perror("SERVER_ERROR bind()");
		exit(1);
	}

	status = listen(server, num_clients);
	if(status == -1){
		perror("SERVER_ERROR listen()");
		exit(1);
	}
}


int recv_file(int in_socket)
{
	//recv file name /pack size /file size
	char pack_size_[15];
	char size_[15];
	string file_name_;

	int i;
	int bytes_recv = 1;
	char recv_inf[TAM_PACK];
    bzero(recv_inf, TAM_PACK);

	//coletar informações sobre o arquivo
	do {
			bytes_recv = recv(in_socket, recv_inf, TAM_PACK, 0);
			if(bytes_recv > 0) {
				cout << "recv_inf successful: " << bytes_recv << endl;
			} else if (bytes_recv == 0){
				perror("SERVER_ERROR connection closed on recv_inf");
				exit(1);
			} else {
				perror("SERVER_ERROR recv() on recv_inf");
			}

	} while(bytes_recv == -1);

	string recv_aux = recv_inf;
	cout << recv_aux << endl;

	istringstream recv_stream(recv_aux);
	recv_stream >> file_name_;
	recv_stream >> size_;
	recv_stream >> pack_size_;

	char file_name[file_name_.length()];

	for(i = 0; i < file_name_.length(); i++){
		file_name[i] = file_name_.at(i);
	}
	file_name[i] = '\0';

	int size = atoi(size_);
	int pack_size = atoi(pack_size_);

	ofstream out_file(file_name);
	char recv_buf[pack_size];

	int pontr = 0;

    bzero(recv_buf, pack_size);

    while(pontr != size){

		do {
			bytes_recv = recv(in_socket, recv_buf, pack_size, 0);
			if(bytes_recv > 0) {
				for(i = 0; i < bytes_recv; i++) {
					out_file << recv_buf[i];
				}
				pontr += bytes_recv;
	        	strcpy( recv_buf, "" );
			} else if (bytes_recv == 0){
				perror("SERVER_ERROR connection closed");
				return 0;
			} else {
				perror("SERVER_ERROR recv()");
			}

		} while(bytes_recv == -1);
    }
    return 1;
}

void *handle_client(void *thread_args)
{
	int in_socket;

	pthread_detach(pthread_self()); // Guarantees that thread resources are deallocated upon return

	// Extract socket file descriptor from argument
	in_socket = ((struct thread_data *) thread_args)->thread_id;
	free(thread_args); // Deallocate memory for argument
	if(recv_file(in_socket)) {
		cout << "client " << in_socket << " finished successfuly" << endl;
	} else {
		cout << "client " << in_socket << " returned 0 [connection closed]" << endl;
	}
}

int main(int argc, char* argv[])
{
	typedef basic_stringstream<char> stringstream;

	int status;
	int in_socket; //incoming client
	int num_clients = 20;
	unsigned int clntLen;
	struct thread_data *thread_args;
	pthread_t threadID; /* Thread ID from pthread_create()*/

	status = create_TCP_server(num_clients);
	if(status != 0) {
		perror("SERVER_ERROR create_TCP_server()");
	} else {
		cout << "waiting for connection..." << endl;
	}
	clntLen = sizeof(client_addr);



    for (;;)
    { // Run forever

		in_socket = accept(server, (struct sockaddr *)&client_addr, &clntLen);
		if (((thread_args = (struct thread_data *) malloc(sizeof(struct thread_data)))) == NULL)
			perror("ERROR threads()");
		thread_args -> thread_id = in_socket;
		if (pthread_create (&threadID, NULL, handle_client, (void *) thread_args) != 0)
			perror("ERROR pthread_create()");

	}

	status = close(server);
	if(status == -1) {
		perror("Falha no close() do server");
	}

}
