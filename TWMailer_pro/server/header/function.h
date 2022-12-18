#include <string>

#ifndef FUNCTION_H_
#define FUNCTION_H_
#define BUF 2048

///////////////////////////////////////////////////////////////////////////////
// Global variable
extern int abortRequested, create_socket, new_socket;

///////////////////////////////////////////////////////////////////////////////
// Structs
struct msg {
   std::string sender, receiver, subject, content;
};

struct msg_u_mn {
   std::string username, message_number;
};

struct credential {
   std::string username, password;
};

struct blacklist {
   std::string attempts, time;
};

///////////////////////////////////////////////////////////////////////////////
// Signal
void signalHandler(int sig);

///////////////////////////////////////////////////////////////////////////////
// Connection
void *clientCommunication(int current_socket, std::string spoolDir);

///////////////////////////////////////////////////////////////////////////////
// Input parsing
struct msg_u_mn fetch_username_msg_number(std::string buffer);
struct msg fetch_msg_content(std::string buffer, struct credential loggedUser);
struct credential fetch_usr_pwd(std::string buffer);



///////////////////////////////////////////////////////////////////////////////
// Server commands
struct credential s_login(struct credential crd, int current_socket);
void s_send(struct msg recv_msg, int current_socket, std::string spoolDir);
void s_list(std::string username, int current_socket, std::string spoolDir);
void s_read_or_del(int type, struct msg_u_mn recv_msg, int current_socket, std::string spoolDir);

///////////////////////////////////////////////////////////////////////////////
// Helper
void create_msg_file(std::string filePath, struct msg recv_msg);
bool is_auth(struct credential user);
struct blacklist fetch_infos(std::string username);
void add_attempt(std::string username);

#endif //FUNCTION_H_