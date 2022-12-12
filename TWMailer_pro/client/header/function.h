#include <string>

#ifndef FUNCTION_H_
#define FUNCTION_H_
#define BUF 2048

///////////////////////////////////////////////////////////////////////////////
// Catch userinput
std::string get_usr_input(std::string content,  int maxLength);
std::string get_usr_input_typed(std::string content, std::string type);

///////////////////////////////////////////////////////////////////////////////
// Server commands
std::string c_del();
std::string c_read();
std::string c_list();
std::string c_send();

///////////////////////////////////////////////////////////////////////////////
// Helper
bool is_number(const std::string& s);

#endif //FUNCTION_H_