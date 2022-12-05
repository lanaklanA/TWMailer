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
// #include <thread>

///////////////////////////////////////////////////////////////////////////////
// Own Headers Includes
#include "header/function.h"

int create_socket = -1;
int new_socket = -1;
int abortRequested = 0;

int main(int argc, char **argv) {
   int reuseValue = 1, port;
   std::string spoolPath;
   
   struct sockaddr_in address, cliaddress;
   socklen_t addrlen;

   //Validate programm execution
   if (argc != 3) {
        std::cout << "Command usage: " << argv[0] << " [port] [mail-spool-directoryname]" << std::endl;
        exit(EXIT_FAILURE);
   } 
   
   if (signal(SIGINT, signalHandler)  == SIG_ERR) {
      perror("signal can not be registered");
      return EXIT_FAILURE;
   }

   //Create socket
   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("Socket error"); // errno set by socket()
      return EXIT_FAILURE;
   }

   //Set socketoption for address
   if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &reuseValue, sizeof(reuseValue)) == -1) {
      perror("set socket options - reuseAddr");
      return EXIT_FAILURE;
   }

   //Set socketoption for port
   if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEPORT, &reuseValue, sizeof(reuseValue)) == -1) {
      perror("set socket options - reusePort");
      return EXIT_FAILURE;
   }

   //Init connection data
   port = atoi(argv[1]);
   spoolPath = argv[2];
   memset(&address, 0, sizeof(address));
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(port);

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
      clientCommunication(new_socket, spoolPath); // returnValue can be ignored
      
      // std::thread bread (clientCommunication, new_socket); // returnValue can be ignored
      // bread.detach();
      new_socket = -1;
   }

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
