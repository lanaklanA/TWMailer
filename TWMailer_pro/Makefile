#############################################################################################
# Makefile
#############################################################################################
# G++ is part of GCC (GNU compiler collection) and is a compiler best suited for C++
# Compiler Flags: https://linux.die.net/man/1/g++
#############################################################################################
# -g: produces debugging information (for gdb)
# -Wall: enables all the warnings
# -Wextra: further warnings
# -O: Optimizer turned on
# -std: c++ 17 is the latest stable version c++2a is the upcoming version
# -I: Add the directory dir to the list of directories to be searched for header files.
# -c: says not to run the linker
# -pthread: Add support for multithreading using the POSIX threads library. This option sets 
#           flags for both the preprocessor and linker. It does not affect the thread safety 
#           of object code produced by the compiler or that of libraries supplied with it. 
#           These are HP-UX specific flags.
#############################################################################################


all: myclient myserver

myclient: client/myclient.cpp
	g++ -ggdb -std=c++11 -pthread -Wall -o ./bin/client client/myclient.cpp client/header/*.cpp

myserver: server/myserver.cpp 
	g++ -ggdb -std=c++11 -pthread -Wall -o ./bin/server server/myserver.cpp server/header/*.cpp

clean:
	rm -f bin/client *.o
	rm -f bin/server *.o









# clean:
# 	rm -f myclient *.o
# 	rm -f myserver *.o

# ./obj/myclient.o: client/myclient.cpp
# 	${CC} ${CFLAGS} -o obj/myclient.o client/myclient.cpp -c

# ./obj/myserver.o: server/myserver.cpp
# 	${CC} ${CFLAGS} -o obj/myserver.o server/myserver.cpp -c 

# ./bin/server: ./obj/myserver.o
# 	${CC} ${CFLAGS} -o bin/server obj/myserver.o

# ./bin/client: ./obj/myclient.o
# 	${CC} ${CFLAGS} -o bin/client obj/myclient.o