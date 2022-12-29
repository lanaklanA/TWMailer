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
#include <time.h>

///////////////////////////////////////////////////////////////////////////////
// Own Headers Includes
#include "function.h"
#include "ldap.h"

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
   struct credential loggedUser;
 
   strcpy(buffer, "Welcome to TWMailer Pro! By Florian Czachor and Brian Schneider\r\nPlease proceed with the command: LOGIN\r\n\n");
   if (send(current_socket, buffer, strlen(buffer), 0) == -1)   {
      perror("send failed");
      return NULL;
   }
   memset(buffer, 0 ,sizeof(buffer));

   do {
      memset(buffer, 0 ,sizeof(buffer));

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
      std::cout << "Message/Command received: " << cli_command << std::endl; // ignore error
      

      if      (strcasecmp(command.c_str(), "login") == 0 && !is_auth(loggedUser))  loggedUser = s_login(fetch_usr_pwd(cli_command), current_socket);
      else if (strcasecmp(command.c_str(), "send") == 0 && is_auth(loggedUser))    s_send(fetch_msg_content(cli_command, loggedUser), current_socket, spoolDir);
      else if (strcasecmp(command.c_str(), "list") == 0 && is_auth(loggedUser))    s_list(cli_command.substr(7, cli_command.find_last_of(':')-7), current_socket, spoolDir);
      else if (strcasecmp(command.c_str(), "read") == 0 && is_auth(loggedUser))    s_read_or_del(1, fetch_username_msg_number(cli_command), current_socket, spoolDir);
      else if (strcasecmp(command.c_str(), "del") == 0  && is_auth(loggedUser))    s_read_or_del(2, fetch_username_msg_number(cli_command), current_socket, spoolDir);
      else                                                                    send(current_socket, "ERR: Command not found", 23, 0);



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
   fflush(stdout); 
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

struct credential fetch_usr_pwd(std::string buffer) {
   struct credential client_msg;
   std::istringstream iss(buffer);
   std::string token;

   std::getline(iss, token, '\n');
   std::getline(iss, token, '\n');
   client_msg.username = token;

   std::getline(iss, token, '\n');
   client_msg.password = token;

   return client_msg;
}

struct msg fetch_msg_content(std::string buffer, struct credential loggedUser) {
   struct msg client_msg;
   std::istringstream iss(buffer);
   std::string token, final;

   std::getline(iss, token, '\n');
   std::getline(iss, token, '\n');
   client_msg.sender = loggedUser.username;

   std::getline(iss, token, '\n');
   client_msg.receiver = token;
   
   std::getline(iss, token, '\n');
   client_msg.subject = token;

   while (std::getline(iss, token, '\n'))
      final += token + "\n";
   
   client_msg.content = final;

   return client_msg;
}

///////////////////////////////////////////////////////////////////////////////
// Server commands
struct credential s_login(struct credential crd, int current_socket) {
   LDAP *ldapHandle;
   // int counter = 0;
   struct blacklist bl;

   // Initialize Ldap
   ldapHandle = ldap_init();


   // Bind Ldap
   int rc = login_and_bind((char*)crd.username.c_str(), (char*)crd.password.c_str(),  ldapHandle);
   
   bl = fetch_infos(crd.username);

   if(stoi(bl.attempts) >= 3 && ( (time(NULL) - stoi(bl.time)) < 60 ) ) {
      std::cout << "Remaining time: " << time(NULL) - stoi(bl.time) << std::endl;

      send(current_socket, "ERR: ur are banned", 19, 0);
      crd.username = "";
      crd.password = "";
      return crd;
   }

   if (rc != LDAP_SUCCESS) {
      add_attempt(crd.username); 

      std::string errorMessage = "ERR: Invalid credential"; //+ std::string(ldap_err2string(rc));
      send(current_socket, errorMessage.c_str(), 24, 0);

      crd.username = "";
      crd.password = "";
      return crd;
   } else {
      std::string filepath = "./Blacklists/" + crd.username;
      std::remove(filepath.c_str());

      send(current_socket, "ISOK", 5, 0);
   }

   return crd;
}


void s_send(struct msg recv_msg, int current_socket, std::string spoolDir) {
   std::string basePath = "./spoolDir/" + spoolDir;
   std::string dirPath = basePath + "/" + recv_msg.receiver;
   std::string filePath = dirPath + "/" + recv_msg.subject;
   
   mkdir(basePath.c_str(), 0700);
   mkdir(dirPath.c_str(), 0700);
   create_msg_file(dirPath, filePath, recv_msg, current_socket);

   dirPath = basePath + "/" + recv_msg.sender;
   filePath = dirPath + "/" + recv_msg.subject;

   mkdir(dirPath.c_str(), 0700);
   create_msg_file(dirPath, filePath, recv_msg, current_socket);

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
      messages.append("\n[" + std::to_string(count) + "] " + entry->d_name);
   }
   output = "[Count of Messages of the User: " + std::to_string(count) + "]" + messages;
   if(count == 0) {
      std::string errorMessage = "ERR: No messages found from user";
      send(current_socket, errorMessage.c_str(), errorMessage.length(), 0);
      return;
   }
   
   send(current_socket, output.c_str(), strlen(output.c_str()), 0);
}

void s_read_or_del(int type, struct msg_u_mn recv_msg, int current_socket, std::string spoolDir) {
   DIR *folder;
   int count = 0;
   struct dirent *entry = NULL;
   std::string filePath, tempLine, token, output = "OK";
   std::string dirPath = "./spoolDir/" + spoolDir + "/" + recv_msg.username;

   if((folder = opendir(dirPath.c_str())) == NULL) {
      std::cerr << "Unable to read directory" << std::endl;
      std::string errorMessage = "ERR: Username does not exits";
      send(current_socket, errorMessage.c_str(), errorMessage.length() +1, 0);
      return;
   }
   while((entry = readdir(folder))) count++;

   if((folder = opendir(dirPath.c_str())) == NULL) {
      std::cerr << "Unable to read directory" << std::endl;
      std::string errorMessage = "ERR: Username does not exits";
      send(current_socket, errorMessage.c_str(), errorMessage.length() +1, 0);
      return;
   }
   if(count -2 < atoi(recv_msg.message_number.c_str()) || atoi(recv_msg.message_number.c_str()) == 0) {
      std::cerr << "Unable to read file" << std::endl;
      std::string errorMessage = "ERR: Message does not exist";
      send(current_socket, errorMessage.c_str(), errorMessage.length() +1, 0);
      return;
   }

   for(int i = 0; i < 2 + atoi(recv_msg.message_number.c_str());  i++) entry=readdir(folder);

   filePath = dirPath + "/" + entry->d_name;
   if(type == 1) {
      std::ifstream MyReadFile(filePath);
      std::string hans;
   
      getline(MyReadFile, hans);
      output += "\n\nSender: " + hans + "\n";
      getline(MyReadFile, hans);
      getline(MyReadFile, hans);
      output += "Receiver: " + hans + "\n";
      getline(MyReadFile, hans);
      getline(MyReadFile, hans);
      output += "Subject: " + hans + "\n" ;
      getline(MyReadFile, hans);
      output += "Message: ";
     
      while (getline(MyReadFile, tempLine)) {
         output += "\n" + tempLine;
      }
      send(current_socket, output.c_str(), output.length() +1, 0);
   } 
   else if(type == 2) {
      std::remove(filePath.c_str());
      send(current_socket, "OK", 3, 0);
   }
}

///////////////////////////////////////////////////////////////////////////////
// Helper

void create_msg_file(std::string dirPath, std::string filePath, struct msg recv_msg, int current_socket) {
   DIR *folder;
   struct dirent *entry;
   int count;

   if((folder = opendir(dirPath.c_str())) == NULL) {
      std::cerr << "Unable to read directory" << std::endl;
      std::string errorMessage = "ERR: Username does not exits";
      send(current_socket, errorMessage.c_str(), errorMessage.length(), 0);
      return;
   }

   while((entry = readdir(folder))) {
      std::string fileName = entry->d_name;
      if(fileName.find(recv_msg.subject) != std::string::npos) {
         count++;
      }
   }

   if(count > 1) {
      filePath += "(" + std::to_string(count-1) + ")";
   }

   std::ofstream MyFile(filePath.c_str());
   MyFile << recv_msg.sender   << std::endl << std::endl 
          << recv_msg.receiver << std::endl << std::endl 
          << recv_msg.subject  << std::endl << std::endl 
          << recv_msg.content;
   MyFile.close();
}

bool is_auth(struct credential user) {
   return (user.username != "" || user.password != "");
}

struct blacklist fetch_infos(std::string username) {
   DIR *folder;
   struct blacklist bl;

   std::string filePath, tempLine;
   std::string dirPath = "./Blacklists/";

   if((folder = opendir(dirPath.c_str())) == NULL) {
      std::cerr << "Unable to read directory" << std::endl;
      bl.attempts = "0";
      bl.time = "0";
      return bl;
   }

   filePath = dirPath + "/" + username;
   std::ifstream MyReadFile(filePath);

   if(MyReadFile.good()) {
      std::string hans;

      getline(MyReadFile, hans);
      bl.attempts = hans;

      getline(MyReadFile, hans);
      bl.time = hans;
      MyReadFile.close();
   } else {
      std::ofstream MyFile(filePath.c_str());
      MyFile   << 0           << std::endl  
               << time(NULL)  << std::endl;

      bl.attempts = "0";
      bl.time = "0";
     
      MyFile.close();
   }
   return bl;
}

void add_attempt(std::string username) {
   struct blacklist bl;
   std::string filePath = "./Blacklists/" + username;
   bl = fetch_infos(username);


   std::ofstream MyFile(filePath.c_str());
   MyFile << stoi(bl.attempts) +1   << std::endl  
          << time(NULL)            << std::endl;

   MyFile.close();
}



