#include <strings.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include<algorithm>
#include<vector>
#include<iterator>
#include <fstream>
#include <locale>
#include <sstream>
#include <pthread.h> /* POSIX threads */
#include <sys/wait.h> /* waitpid() */

#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> /* for close() */

#define PORT 8000
#define N_LINES 5
const char* server;

struct thread_data
{
	int thread_id;
};

int open_connection() {

  int i;
	int client_socket;
  struct sockaddr_in server_addr ;
  struct sockaddr_in client_addr ;
	int status; //estado da chamada


  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

	if(inet_aton(server, &server_addr.sin_addr) == 0)
	{
    	std::cout << "ERROR inet_aton() failed" << std::endl;
    	exit(1);
    }

	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket == -1)
	{
		perror("ERROR socket not created.\n");
		exit(1);
	}

	bzero((char *) &client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = PORT;

    status = bind(client_socket, (struct sockaddr *)&client_addr.sin_addr, sizeof(client_addr));
	if(status != 0)
	{
    	std::cout << "ERROR on bind()" << std::endl;
    }

	status = connect( client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr) );
	if (status == -1)
	{
		std::cout << "ERROR on server_addr connect()" << std::endl;
		exit(1);
	}
  return client_socket;
}

int send_file(int client_socket, int size, int pack_size, char file_name[]) {

  //enviar informações sobre o arquivo+tamanho do pacote
  char buffer[15];
  int bytes_sent;
	int i;
  std::string send_aux;
  send_aux += file_name;
  send_aux += " ";
  sprintf(buffer,"%d",size);
  send_aux += buffer;
  send_aux += " ";
  sprintf(buffer,"%d",pack_size);
  send_aux += buffer;

  char send_info[send_aux.length() + 1];

  for(i = 0; i < send_aux.length(); i++){
  	send_info[i] = send_aux.at(i);
  }
  send_info[i] = '\0';
  do
  { 
     	bytes_sent = send(client_socket, send_info, send_aux.length(), 0);
  	if (bytes_sent == 0)	{
  		std::cout << "ERROR connection closed." << std::endl;
  	} else if (bytes_sent == -1) {
  		std::cout << "ERROR sendto() failed" << bytes_sent << std::endl;
  	}

  } while(bytes_sent == -1);

  //open file
  std::ifstream in_file(file_name);

  //file aux
  std::string the_chosen_one = "";
  char reader;
    //read from file
  while(in_file >> std::noskipws >> reader)
  {
  	the_chosen_one += reader;
  }

  char send_msg[pack_size];
    bzero(send_msg, pack_size);

  int count;
  int pontr = 0; //ponteiro dentro do char[]

  while(pontr != size)
  {
  	strcpy(send_msg, "");

  	if((size-pontr) >= pack_size)
  	{
  		for(int i = pontr; i < pack_size + pontr; i++)
  		{
  			send_msg[i%pack_size] = the_chosen_one.at(i);
  		}
  		pontr += pack_size;
  	}
  	else
  	{
  		pack_size = size-pontr;
  		for(i = pontr; i < pack_size + pontr; i++)
  		{
  			if(pontr == 0) {
  				send_msg[i] = the_chosen_one.at(i);
  			} else {
  				send_msg[i%pontr] = the_chosen_one.at(i);
  			}
  		}
  		pontr = size;
  	}

  	do
  	{
       	bytes_sent = send(client_socket, send_msg, pack_size , 0);
  		if (bytes_sent == 0)	{
  			std::cout << "ERROR connection closed." << std::endl;
  		} else if (bytes_sent == -1) {
  			std::cout << "ERROR sendto() failed" << bytes_sent << std::endl;
  		}

  	} while(bytes_sent == -1);
  }




}


int main(int argc, char const *argv[]) {

  if(argc != 2){
    std::cout << "ERROR server IP missing" << std::endl;
    exit(1);
  }

  int client_socket;
  server = argv[1];
  struct thread_data *thread_args;
  pthread_t threadID; /* Thread ID from pthread_create()*/
  //read action file and create threads to handle it
  char size_[15];
  char pack_size_[15];
  std::string file_name_;
  std::string exec = "";
  char exec_reader;
  int status;
  int i;
  //read from file that has the execution order "exec_cs"
  std::ifstream exec_file("exec_client");
  while(exec_file >> std::noskipws >> exec_reader)
  {
    exec += exec_reader;
  }

  std::istringstream exec_stream(exec);

	int j;


  for (size_t i = 0; i < 4; i++)
  {
      exec_stream >> file_name_;
      exec_stream >> size_;
      exec_stream >> pack_size_;
      char file_name[file_name_.length()];
      for(j = 0; j < file_name_.length(); j++){
        file_name[j] = file_name_.at(j);
      }
      file_name[j] = '\0';

      int size = atoi(size_);
      int pack_size = atoi(pack_size_);


      client_socket = open_connection();
      send_file(client_socket, size, pack_size, file_name);
			std::cout << "enviei" << file_name << '\n';
			close(client_socket);
}


  return 0;
}
