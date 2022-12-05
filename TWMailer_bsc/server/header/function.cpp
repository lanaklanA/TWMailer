///////////////////////////////////////////////////////////////////////////////
// C++/11 Headers Includes
#include <iostream>
#include <vector>
// #include <thread>
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

///////////////////////////////////////////////////////////////////////////////
// Own Headers Includes
#include "function.h"

///////////////////////////////////////////////////////////////////////////////
// Signal
void signalHandler(int sig) {

   if (sig == SIGINT) {
      printf("abort Requested... "); // ignore error
      abortRequested = 1;

      if (new_socket != -1) {
         if (shutdown(new_socket, SHUT_RDWR) == -1) 
            perror("shutdown new_socket");
         
         if (close(new_socket) == -1) 
            perror("close new_socket");
         
         new_socket = -1;
      }

      if (create_socket != -1)
      {
         if (shutdown(create_socket, SHUT_RDWR) == -1) 
            perror("shutdown create_socket");
         
         if (close(create_socket) == -1) 
            perror("close create_socket");
         
         create_socket = -1;
      }
   }
   else
      exit(sig);

}

///////////////////////////////////////////////////////////////////////////////
// Connection
void *clientCommunication(int current_socket, std::string spoolDir) {
   int size;
   char buffer[BUF];
   struct msg mail;
 
   strcpy(buffer, "Welcome to myserver!\r\nPlease enter your commands...\r\n");
   if (send(current_socket, buffer, strlen(buffer), 0) == -1)   {
      perror("send failed");
      return NULL;
   }
   memset(buffer, 0 ,sizeof(buffer));

   do {
      size = recv(current_socket, buffer, BUF - 1, 0);
      if (size == -1) {
         if (abortRequested)  perror("recv error after aborted");
         else                 perror("recv error");
         break;
      }

      if (size == 0) {
         printf("Client closed remote socket\n"); 
         break;
      }

      if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n') size -= 2;
      else if (buffer[size - 1] == '\n')                        --size;    
      buffer[size] = '\0';
           
      std::string cli_command(buffer);
      std::string command = cli_command.substr(cli_command.find_first_of('[')+1, cli_command.find_first_of(']')-1);

      std::cout << "Message/Command received: " << command << std::endl; // ignore error

      if     (command == "send") s_send(fetch_msg_content(cli_command), current_socket, spoolDir);
      else if(command == "list") s_list(cli_command.substr(7, cli_command.find_last_of(':')-7), current_socket, spoolDir);
      else if(command == "read") s_read_or_del(1, fetch_username_msg_number(cli_command), current_socket, spoolDir);
      else if(command == "del")  s_read_or_del(2, fetch_username_msg_number(cli_command), current_socket, spoolDir);
      else                       send(current_socket, "OK", 3, 0);

   } while (!abortRequested);

   if (current_socket != -1) {
      if (shutdown(current_socket, SHUT_RDWR) == -1) {
         perror("shutdown new_socket");
      }
      if (close(current_socket) == -1) {
         perror("close new_socket");
      }
      current_socket = -1;
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Input parsing
struct msg_u_mn fetch_username_msg_number(std::string buffer) {
   struct msg_u_mn client_msg;
   std::istringstream iss(buffer);
   std::string token, final;

   std::getline(iss, token, '\n');
   std::getline(iss, token, '\n');
   client_msg.username = token;

   std::getline(iss, token, '\n');
   client_msg.message_number = token;

   return client_msg;
}

struct msg fetch_msg_content(std::string buffer) {
   struct msg client_msg;
   std::istringstream iss(buffer);
   std::string token, final;

   std::getline(iss, token, '\n');
   std::getline(iss, token, '\n');
   client_msg.sender = token;

   std::getline(iss, token, '\n');
   client_msg.receiver = token;
   
   std::getline(iss, token, '\n');
   client_msg.subject = token;

   while (std::getline(iss, token, '\n'))
      final += token + "\n";
   
   client_msg.content = final + "\n";

   return client_msg;
}

///////////////////////////////////////////////////////////////////////////////
// Server commands
void s_send(struct msg recv_msg, int current_socket, std::string spoolDir) {
   std::string basePath = "./spoolDir/" + spoolDir;
   std::string dirPath = basePath + "/" + recv_msg.receiver;
   std::string filePath = dirPath + "/" + recv_msg.subject;
   
   mkdir(basePath.c_str(), 0700);
   mkdir(dirPath.c_str(), 0700);
   create_msg_file(filePath, recv_msg);

   dirPath = basePath + "/" + recv_msg.sender;
   filePath = dirPath + "/" + recv_msg.subject;

   mkdir(dirPath.c_str(), 0700);
   create_msg_file(filePath, recv_msg);

   send(current_socket, "OK", 3, 0);
}

void s_list(std::string username, int current_socket, std::string spoolDir) {
   DIR *folder;
   struct dirent *entry;
   int count = 0;
   std::string output, messages, dirPath = "./spoolDir/" + spoolDir + "/" + username;

   if((folder = opendir(dirPath.c_str())) == NULL) {
      std::cerr << "Unable to read directory" << std::endl;
      std::string errorMessage = "ERR: Username does not exits";
      send(current_socket, errorMessage.c_str(), errorMessage.length(), 0);
      return;
   }

   for(int i = 0; i < 2; readdir(folder), i++);

   while((entry = readdir(folder))) {
      count++;
      messages.append("[" + std::to_string(count) + "] " + entry->d_name + "\n");
   }
   output = "[Count of Messages of the User : " + std::to_string(count) + "]\n" + messages;
   if(count == 0) {
      std::string errorMessage = "ERR: No Messages of User found";
      send(current_socket, errorMessage.c_str(), errorMessage.length(), 0);
   } else {
      send(current_socket, output.c_str(), strlen(output.c_str()), 0);
   }
}

void s_read_or_del(int type, struct msg_u_mn recv_msg, int current_socket, std::string spoolDir) {
   DIR *folder;
   int count = 0;
   struct dirent *entry = NULL;
   std::string filePath, tempLine, token, output = "\n[Message-Number: " + recv_msg.message_number + "]\n";
   std::string dirPath = "./spoolDir/" + spoolDir + "/" + recv_msg.username;

   if((folder = opendir(dirPath.c_str())) == NULL) {
      std::cerr << "Unable to read directory" << std::endl;
      std::string errorMessage = "ERR: Username does not exits";
      send(current_socket, errorMessage.c_str(), errorMessage.length(), 0);
      return;
   }
   while((entry = readdir(folder))) count++;

   if((folder = opendir(dirPath.c_str())) == NULL) {
      std::cerr << "Unable to read directory" << std::endl;
      std::string errorMessage = "ERR: Username does not exits";
      send(current_socket, errorMessage.c_str(), errorMessage.length(), 0);
      return;
   }
   if(count -2 < atoi(recv_msg.message_number.c_str())) { 
      std::cerr << "Unable to read file" << std::endl;
      std::string errorMessage = "ERR: Message does not exist";
      send(current_socket, errorMessage.c_str(), errorMessage.length(), 0);
      return;
   }

   for(int i = 0; i < 2 + atoi(recv_msg.message_number.c_str());  i++) entry=readdir(folder);

   filePath = dirPath + "/" + entry->d_name;
   if(type == 1) {
      std::ifstream MyReadFile(filePath);
      for(int i = 0; i < 6; getline(MyReadFile, tempLine), i++);
      std::cout << filePath << std::endl; 
      std::cout << std::endl;
      while (getline(MyReadFile, tempLine)) {
         output += "\n" + tempLine;
         std::cout << tempLine << std::endl;
      }
      std::cout << std::endl;
      send(current_socket, output.c_str(), strlen(output.c_str()), 0);

   } else if(type == 2) {
      std::remove(filePath.c_str());
      send(current_socket, "OK", 3, 0);
   }
}

///////////////////////////////////////////////////////////////////////////////
// Helper
void create_msg_file(std::string filePath, struct msg recv_msg) {
   std::ofstream MyFile(filePath.c_str());
   MyFile << recv_msg.sender   << std::endl << std::endl 
          << recv_msg.receiver << std::endl << std::endl 
          << recv_msg.subject  << std::endl << std::endl 
          << recv_msg.content;
   MyFile.close();
}




