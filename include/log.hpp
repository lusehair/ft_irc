# ifndef LOG_HPP
# define LOG_HPP 

#include <ctime>
#include "Server.hpp"

// SERVER.CPP LOGS 
#define IRC_START "IRC SERVER START\n"
#define ERRGETADDRINFO "cannot get a listen port\n"
#define LISTPOTENTIALADDR "find these adresses : \n" // Do special method for iteration
#define TRYBIDINGSOCKET  "Try to bind : " // after
#define INVALIDSOCKET  "cannot bind : invalid socket : " // after 
#define OPTSETFAIL "cannot set option for the socket : "  //after 
#define BINDFAIL "cannot bind on: "  //after
#define BINDSUCCESS " bind with succesful\n"; //before 
#define LISTENFAILED "cannot listen the socket :"  //after 
#define NOBIND "cannot bind and connect: " //after 
#define CLOSESOCKET " has been close\n" //before 
#define SELECTERR "cannot get the request from: " //after  
#define TIMEOUT " is connected to se server\n" //before 
#define CONNECTACCEPT 1014
#define CONNECTFAILED " cannot being conencted to the server\n"; // before  

// SERVER.COMMAND LOGS 

#define NOPARAM(FD, PARAM) std::cout << FD << ": No paramater or parameter not valid :" << PARAM << '\n'

#define PASSTWICE(FD) std::cout << FD << ": Try to set pass twice\n" 
#define PASSSUCCESS(FD) std::cout << FD ": Password set succefully\n"
#define PASSFAILED(FD) std::cout << FD <<  ": Bad Password, close connection\n" 

#define NICKCHANGE(OLDNICK, NICK)  sst#OLDNICK Change his nickname as #NICK \n"
#define NICKTAKEN(NICK, NICKTAKEN) "#NICK Nickname already taken  : #NICKTAKEN \n"
#define NICKREGISTER(NICK) "connected to the server with #NICK \n" 

#define USERTAKEN(NICK, USERNAME) "#NICK Try to set a username already set : #USERNAME \n"
#define USERCONNECTED "#NICK Is fully connected ! \n"


class log_cmd
{
    public: 
        log_cmd(t_time time, const char *message, ); 
}; 


# endif 