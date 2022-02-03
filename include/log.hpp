# ifndef LOG_HPP
# define LOG_HPP 

#include <ctime>
#include <iostream>
#include <fstream>
#include "Server.hpp"

// SERVER.CPP LOGS 
#define LOG_IRC_START(TIME) _log_file << log_time(TIME) << "s: IRC SERVER START\n"

//#define ERRGETADDRINFO "cannot get a listen port\n"

#define LOG_ERRGETADDRINFO(TIME, PORT_NUMBER) _log_file << log_time(TIME) << "s: couldn't recieve info from port: " << PORT_NUMBER << '\n'

#define LOG_LISTPOTENTIALADDR(TIME, PORT_NUMBER) _log_file << log_time(TIME) << "s: recieve info from port: " << PORT_NUMBER << '\n' // Do special method for iteration 

//#define TRYBIDINGSOCKET  "Try to bind : " // after

#define LOG_TRYBIDINGSOCKET(TIME, LISTEN_SOCKET) _log_file << log_time(TIME) << "s: Try to bind Socket number:" << LISTEN_SOCKET << '\n'

//#define LOG_INVALIDSOCKET  "cannot bind : invalid socket : " // after 

#define LOG_INVALIDSOCKET(TIME, LISTEN_SOCKET) _log_file << log_time(TIME) << "s: " << LISTEN_SOCKET << " is a invalid socket\n"

//#define OPTSETFAIL "cannot set option for the socket : "  //after 

#define LOG_OPTSETFAIL(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: cannot set option on socket " << LISTEN_SOCKET << '\n'

//#define BINDFAIL "cannot bind on: "  //after

#define LOG_BINDFAIL(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: cannot bind on  " << LISTEN_SOCKET << '\n' 


//#define BINDSUCCESS " bind with succesful\n"; //before 


#define LOG_BINDSUCCESS(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: the server has bind " << LISTEN_SOCKET << " with success\n"

//#define LISTENFAILED "cannot listen the socket :"  //after 

#define LOG_LISTENFAILED(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: the server cannot listen on " << LISTEN_SOCKET << '\n'


#define LOG_TRYLISTENING(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: the server try to listen on " << LISTEN_SOCKET << '\n'

#define LOG_LISTENSUCCESS(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: the server listening on " << LISTEN_SOCKET << " with success !\n"



//#define NOMOREBIND "cannot bind and connect: " //after 

#define LOG_NOMOREBIND(TIME) _log_file << log_time(TIME) << "s: Server can't start : cannot bind on anny socket\n" 



//#define CLOSESOCKET " has been close\n" //before 


#define LOG_CLOSESOCKET(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: socket number " << LISTEN_SOCKET << "  has been closed\n" 

//#define SELECTERR "cannot get the request from: " //after  

#define LOG_SELECTERR(TIME) _log_file << log_time(TIME) << "s: An error was occured on listening socket (select)\n"

//#define TIMEOUT " is connected to se server\n" //before 


#define LOG_TIMEOUT _log_file << "Nothing received in last " << _time_before_timeout.tv_sec << " seconds\n"



#define LOG_CONNECTACCEPT(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: connected to socket number " << LISTEN_SOCKET << " with success !\n" 


#define LOG_CONNECTFAILED(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: couldn't connext at " << LISTEN_SOCKET << '\n' 


#define LOG_CLOSINGFD(TIME, FD) _log_file << log_time(TIME) <<  "s: The socket " << FD << " was close\n"
// SERVER.COMMAND LOGS 

#define LOG_NOPARAM(TIME, FD, PARAM) _log_file << log_time(TIME) << FD << "s: No paramater or parameter not valid : " << PARAM << '\n'

#define LOG_PASSTWICE(TIME, FD) _log_file  << log_time(TIME) << "s" << ": Socket number "<< FD << ": Try to set pass twice\n" 
#define LOG_PASSSUCCESS(TIME, FD) _log_file << log_time(TIME) << "s"  << ": Socket number " << FD << ": Password set succefully\n"
#define LOG_PASSFAILED(TIME, FD) _log_file << log_time(TIME) << "s" << ": Socket number " << FD <<  ": Bad Password, close connection\n" 

#define LOG_NICKCHANGE(TIME, OLDNICK, NICK) _log_file << log_time(TIME) << "s" << ": " << OLDNICK << " Change his nickname as #NICK \n"
#define LOG_NICKTAKEN(TIME, NICK, NICKTAKEN) _log_file << log_time(TIME) << "s" << ": " <<  NICK << " Nickname already taken  : " << NICKTAKEN << "\n"
#define LOG_NICKREGISTER(TIME, NICK) _log_file << log_time(TIME) << "s" << ": connected to the server with " << NICK << "\n" 

#define LOG_USERTAKEN(TIME, NICK, USERNAME) _log_file << log_time(TIME) << "s" << ": " << NICK << " Try to set a username already set : " << USERNAME << "\n"
#define LOG_USERCONNECTED(TIME, NICK) _log_file << log_time(TIME) << "s" <<  ": " << NICK << " Is fully connected ! \n"




long    log_time(time_t input_time);  


# endif 