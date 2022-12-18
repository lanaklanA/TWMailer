#include <string>

#ifndef FUNCTION_H_
#define FUNCTION_H_
#define BUF 2048


struct credential {
    std::string username;
    std::string password;
};
///////////////////////////////////////////////////////////////////////////////
// Catch userinput
std::string get_usr_input(std::string content,  int maxLength);
std::string get_usr_input_typed(std::string content, std::string type);

///////////////////////////////////////////////////////////////////////////////
// Server commands
std::string c_login(struct credential *loggedUser);
std::string c_del  (struct credential loggedUser);
std::string c_read (struct credential loggedUser);
std::string c_list (struct credential loggedUser);
std::string c_send (struct credential loggedUser);

///////////////////////////////////////////////////////////////////////////////
// Helper
bool is_number(const std::string& s);
bool is_auth(struct credential user);

#endif //FUNCTION_H_