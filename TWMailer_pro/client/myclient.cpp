///////////////////////////////////////////////////////////////////////////////
// C++/11 Headers Includes
#include <iostream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// C Headers Includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// Own Headers Includes
#include "header/function.h"

///////////////////////////////////////////////////////////////////////////////
//
// USAGE:         ./bin/client [ip] [port] 
// USAGE EXAMPLE: ./bin/client localhost 6543
//
// The client connects to the server and communicates through a stream socket connection in a
// proprietary plain-text format delimited by new-line or “dot + new-line
//
// After executing this programm, it will connect to a listening server.
// When the connection is established, the users input gets parsed and send through 
// a sockets to the listing server. After sending, the client waits of a response 
// and represent this the user
//
// The server responds to the following commands:
// 
// send >> client sends a message to the server
// list >> lists all messages of a specific user
// read >> display a specific message of a specific user
// del  >> removes a specific message
// quit >> logout the client
//

int main(int argc, char **argv) {
   int create_socket, size, isQuit = 0;
   char buffer[BUF];

   struct sockaddr_in address;
   std::string cli_input, serialized_input;
   

   //Validate programm execution
   if (argc != 3) {
        std::cout << "Command usage: " << argv[0] << " [ip] [port]" << std::endl;
        exit(EXIT_FAILURE);
   } 

   //Create socket
   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)   {
      perror("Socket error");
      return EXIT_FAILURE;
   }

   //Init connection data
   memset(&address, 0, sizeof(address)); // init storage with 0
   address.sin_family = AF_INET;         // IPv4
   address.sin_port = htons(atoi(argv[2]));
   inet_aton(argv[1], &address.sin_addr);
   
   //Connect to server
   if (connect(create_socket, (struct sockaddr *)&address,sizeof(address)) == -1)   {
      perror("Connect error - no server available");
      return EXIT_FAILURE;
   }
   printf("Connection with server (%s) established\n", inet_ntoa(address.sin_addr));
   fflush(stdout);

   size = recv(create_socket, buffer, BUF - 1, 0);
   if      (size == -1)       perror("recv error");
   else if (size == 0)        printf("Server closed remote socket\n"); // ignore error
   else   {
      buffer[size] = '\0';
      printf("%s", buffer); // ignore error
   }


   struct credential loggedUser;
   //login usr
   while (1) {
 
         std::cout << ">> ";
         std::getline(std::cin, cli_input);   

         //Parsing users input
         if      (cli_input == "login") serialized_input = c_login(&loggedUser);
        
         else    {std::cout << "ERR: Pls log in to continue" << std::endl; continue;}
         fflush(stdout);      

         if ((send(create_socket, serialized_input.c_str(), serialized_input.length(), 0)) == -1) {
            perror("send error");
            break;
         }
     

         size = recv(create_socket, buffer, BUF - 1, 0);
        
      
         //Error handling
         if (size == 0) {
            std::cout << "Server closed remote socket" << std::endl;
            break;
         }
         else if(size < 0) {       
            std::cout << "Reciving Error" <<  std::endl;   
            break;
         }

         //Print received data from server
         buffer[size] = '\0';
         std::cout << buffer << std::endl;
        
         if(strcmp(buffer, "ISOK\0") == 0) break;  
   }

   std::cout << "\nLogged in as: " + loggedUser.username << std::endl;

   do   {
         std::cout << ">> ";
         fflush(stdout);
         std::getline(std::cin, cli_input);      
         fflush(stdout);

         //Parsing users input
         if     (cli_input == "send")  serialized_input = c_send(loggedUser);
         else if(cli_input == "list")  serialized_input = c_list(loggedUser); 
         else if(cli_input == "read")  serialized_input = c_read(loggedUser);
         else if(cli_input == "del")   serialized_input = c_del (loggedUser);
         else if(cli_input == "quit")  break;
         else serialized_input = cli_input + " ";
         fflush(stdout);
         
         if ((send(create_socket, serialized_input.c_str(), serialized_input.length(), 0)) == -1) {
            perror("send error");
            break;
         }
         fflush(stdout);

         size = recv(create_socket, buffer, BUF - 1, 0);
         fflush(stdout);
      
         //Error handling
         if (size == 0) {
            std::cout << "Server closed remote socket" << std::endl;
            break;
         }
         else if(size < 0) {       
            std::cout << "Reciving Error" <<  std::endl;   
            break;
         }
         fflush(stdout);

         //Print received data from server
         buffer[size] = '\0';
         std::cout << buffer << std::endl;
         fflush(stdout);
      
   } while (!isQuit);

   //Error handling
   if (create_socket != -1)   {
      if (shutdown(create_socket, SHUT_RDWR) == -1)      {
         perror("shutdown create_socket"); 
      }
      if (close(create_socket) == -1)      {
         perror("close create_socket");
      }
      create_socket = -1;
   }
   return EXIT_SUCCESS;
}
