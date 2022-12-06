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