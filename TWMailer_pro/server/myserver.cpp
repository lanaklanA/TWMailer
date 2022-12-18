/////////////////////////////////////////////////////////////////////////////
// C++/11 Headers Includes
#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

///////////////////////////////////////////////////////////////////////////////
// C Headers Includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ldap.h>
// #include <thread>

///////////////////////////////////////////////////////////////////////////////
// Own Headers Includes
#include "header/function.h"
#include "header/ldap.h"

LDAP *ldapHandle;
int create_socket = -1;
int new_socket = -1;
int abortRequested = 0;

///////////////////////////////////////////////////////////////////////////////
//
// USAGE:         ./bin/server [port] [mail-spool-directoryname] 
// USAGE EXAMPLE: ./bin/server 6543 database
//
// The client connects to the server and communicates through a stream socket connection in a
// proprietary plain-text format delimited by new-line or “dot + new-line
//
// After executing this programm, it will open a connection and listen to a 
// given port. When an handshake has been established, the server react of 
// incoming requestes, handles them and answer with an response 
//   

int main(int argc, char **argv) {
   int reuseValue = 1;
      
   struct sockaddr_in address, cliaddress;
   socklen_t addrlen;

   // Validate programm execution
   if (argc != 3) {
        std::cout << "Command usage: " << argv[0] << " [port] [mail-spool-directoryname]" << std::endl;
        exit(EXIT_FAILURE);
   } 
   
   // Register signalhandler
   if (signal(SIGINT, signalHandler)  == SIG_ERR) {
      std::cerr << "signal can not be registered" << std::endl;
      return EXIT_FAILURE;
   }

   // Create socket
   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      std::cerr << "Socket error" << std::endl;// errno set by socket()
      return EXIT_FAILURE;
   }

   // Set socketoption for address
   if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &reuseValue, sizeof(reuseValue)) == -1) {
      std::cerr << "set socket options - reuseAddr" << std::endl;
      return EXIT_FAILURE;
   }

   // Set socketoption for port
   if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEPORT, &reuseValue, sizeof(reuseValue)) == -1) {
      std::cerr << "set socket options - reusePort" << std::endl;
      return EXIT_FAILURE;
   }

   //Init connection data
   memset(&address, 0, sizeof(address));
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(atoi(argv[1]));

   //Bind data to socket
   if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
      perror("bind error");
      return EXIT_FAILURE;
   }

   //Listen at incoming connection
   if (listen(create_socket, 5) == -1) {
      perror("listen error");
      return EXIT_FAILURE;
   }

   while (!abortRequested) {
      printf("Waiting for connections...\n");

      addrlen = sizeof(struct sockaddr_in);
      if ((new_socket = accept(create_socket, (struct sockaddr *)&cliaddress, &addrlen)) == -1) {
         if (abortRequested)  perror("accept error after aborted");
         else                 perror("accept error");
         break;
      }

      printf("Client connected from %s:%d...\n", inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));

      clientCommunication(new_socket, argv[2]); // returnValue can be ignored
      
      // std::thread bread (clientCommunication, new_socket); // returnValue can be ignored
      // bread.detach();
      new_socket = -1;
   }

   //Error Handling
   if (create_socket != -1) {
      if (shutdown(create_socket, SHUT_RDWR) == -1) {
         perror("shutdown create_socket");
      }
      if (close(create_socket) == -1) {
         perror("close create_socket");
      }
      create_socket = -1;
   }

   return EXIT_SUCCESS;
}
