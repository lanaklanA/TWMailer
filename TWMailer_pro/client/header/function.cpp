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
#include "function.h"

///////////////////////////////////////////////////////////////////////////////
// Catch userinput
std::string get_usr_input(std::string content,  int maxLength) {
   std::string input;

   std::cout << content << std::endl;
   std::getline(std::cin, input);
         
   while(input == "" || input == "." || input == "/" || input == "./" || input.length() > (long unsigned int)maxLength) {
      std::cerr << "Please use a Minimum of 1 and a Maximum of " << maxLength << " chars!" << std::endl;
      std::cout << content << std::endl;
      std::getline(std::cin, input);
   }
   return input;
}

std::string get_usr_input_typed(std::string content, std::string type) {
      std::string input;
      std::string message;

      std::cout << content << std::endl;

      if(type == "number") {
            std::getline(std::cin, input);

            while(!is_number(input) && type == "number") {
               std::cerr << "Only Numericals are allowed" << std::endl;
               std::cout << "Enter the Message Nummer:" << std::endl;
               std::getline(std::cin, input);
            }
      }

      if(type == "message") {
         while (getline(std::cin, input) && type == "message") {
            if (input == ".") break;
               message += "\n" + input;
         }
         input = message;
      }
      return input;
}


///////////////////////////////////////////////////////////////////////////////
// Client commands
std::string c_login(struct credential *loggedUser) { 
   std::string name = get_usr_input("Enter Username:", 8);
   std::string pwd = get_usr_input("Enter Password:", 256);
   loggedUser->username = name; 
   loggedUser->password = pwd;

   return "[login]\n" + name + "\n" + pwd + "\n";
}

std::string c_send(struct credential loggedUser) {
   // std::string sender = get_usr_input("Enter the Sender:", 8);
   std::string sender = loggedUser.username;
   std::string receiver = get_usr_input("Enter the Receiver:", 8);
   std::string subject = get_usr_input("Enter the Subject:", 80);
   std::string message = get_usr_input_typed("Enter the Message Content:", "message");

   return "[send]\n" + sender + "\n" + receiver + "\n" + subject + message + "\n";
}

std::string c_list(struct credential loggedUser) {
   // std::string username = get_usr_input("Enter the Username:", 8);
   std::string username = loggedUser.username;

   return "[list]:" + username + ":";
}

std::string c_read(struct credential loggedUser) {
   // std::string username = get_usr_input("Enter the Username:", 8);
   std::string username = loggedUser.username;
   std::string message_number = get_usr_input_typed("Enter the Message Nummer:", "number");

   return "[read]\n" + username + "\n" + message_number + "\n";
}

std::string c_del(struct credential loggedUser) {
   // std::string username = get_usr_input("Enter the Username:", 8);
   std::string username = loggedUser.username;
   std::string message_number = get_usr_input_typed("Enter the Message Nummer:", "number");

   return "[del]\n" + username + "\n" + message_number + "\n";
}

///////////////////////////////////////////////////////////////////////////////
// Helper
bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool is_auth(struct credential user) {
   return (user.username != "" || user.password != "");
}

