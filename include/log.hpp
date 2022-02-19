# ifndef LOG_HPP
# define LOG_HPP 

#include <ctime>
#include <iostream>
#include <fstream>
#include "Server.hpp"

// SERVER.CPP LOGS 
#define LOG_IRC_START(TIME) _log_file << log_time(TIME) << "s: IRC SERVER START\n"
#define LOG_ERRGETADDRINFO(TIME, PORT_NUMBER) _log_file << log_time(TIME) << "s: couldn't recieve info from port: " << PORT_NUMBER << '\n'
#define LOG_LISTPOTENTIALADDR(TIME, PORT_NUMBER) _log_file << log_time(TIME) << "s: recieve info from port: " << PORT_NUMBER << '\n' 
#define LOG_TRYBIDINGSOCKET(TIME, LISTEN_SOCKET) _log_file << log_time(TIME) << "s: Try to bind Socket number: " << LISTEN_SOCKET << '\n'
#define LOG_INVALIDSOCKET(TIME, LISTEN_SOCKET) _log_file << log_time(TIME) << "s: " << LISTEN_SOCKET << " is a invalid socket\n"
#define LOG_OPTSETFAIL(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: cannot set option on socket " << LISTEN_SOCKET << '\n'
#define LOG_CLOSEFROMFAIL(TIME, LISTEN_SOCKET) _log_file << log_time(TIME) << "s: " << LISTEN_SOCKET << ": listening socket has been closed\n";
#define LOG_BINDFAIL(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: cannot bind on  " << LISTEN_SOCKET << '\n' 
#define LOG_BINDSUCCESS(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: server has bind " << LISTEN_SOCKET << " with success\n"
#define LOG_LISTENFAILED(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: server cannot listen on " << LISTEN_SOCKET << '\n'
#define LOG_TRYLISTENING(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: server try to listen on " << LISTEN_SOCKET << '\n'
#define LOG_LISTENSUCCESS(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: server listening on " << LISTEN_SOCKET << " with success !\n"
#define LOG_NOMOREBIND(TIME) _log_file << log_time(TIME) << "s: Server can't start : cannot bind on anny socket\n" 
#define LOG_CLOSESOCKET(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: socket number " << LISTEN_SOCKET << "  has been closed\n" 
#define LOG_SELECTERR(TIME) _log_file << log_time(TIME) << "s: An error was occured on listening socket (select)\n"
#define LOG_TIMEOUT _log_file << "Nothing received in last " << _time_before_timeout.tv_sec << " seconds\n"
#define LOG_CONNECTACCEPT(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: connected to socket number " << LISTEN_SOCKET << " with success !\n" 
#define LOG_CONNECTFAILED(TIME, LISTEN_SOCKET) _log_file  << log_time(TIME) << "s: couldn't connext at " << LISTEN_SOCKET << '\n' 
#define LOG_CLOSINGFD(TIME, FD) _log_file << log_time(TIME) <<  "s: The socket " << FD << " was close\n"
#define LOG_CANNOTTRANSLATE(TIME) _log_file << log_time(TIME) << "s: could not translate the socket address\n";
#define LOG_SHOWIP(TIME, IPBUFFER) _log_file << log_time(TIME) << "s: catch the IP address :" << IPBUFFER << "\n"; 

// SERVER.CPP CLOSING LOGS 
#define LOG_CLOSEUSER(TIME, NICKNAME) _log_file << log_time(TIME) << "s: User: " << NICKNAME << " deletion\n";
#define LOG_CLOSEMAPUSER(TIME) _log_file << log_time(TIME) << "s: The map of users has been emptied\n";
#define LOG_CLOSEFDSOCKET(TIME) _log_file << log_time(TIME) << "s: The fd_set of client sockets has been emptied\n";
#define LOG_CLOSEOPENSOCKET(TIME) _log_file << log_time(TIME) << "s: The set of opened sockets has been emptied\n";
#define LOG_CLOSEPENDINGSOCKET(TIME) _log_file << log_time(TIME) << "s: The map of pending sockets has been emptied\n";
#define LOG_CLOSEPASS(TIME) _log_file << log_time(TIME) << "s: The password has been freed\n";


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
#define LOG_PONGNOREGISTERUSER(TIME, FD) _log_file << log_time(TIME) << "s" << ": Socket number" << FD << ": Try to ping the server without connexion\n"
#define LOG_PONGUSERPING(TIME, NICK) _log_file << log_time(TIME) << "s" << ": User" << NICK << ": has ping the server\n"
#define LOG_KILLNOREGISTER(TIME, FD) _log_file << log_time(TIME) << "s" << ": Socket number" << FD << ": Try to kill someone without connexion\n"
#define LOG_KILLWITHOUTPRIV(TIME, NICK) _log_file << log_time(TIME) << "s" << ": User" << NICK << ": Try to kill someone whitout privilege\n" 
#define LOG_KILLWITHPRIV(TIME, NICK, TARGET) _log_file << log_time(TIME) << "s" << ": User" << NICK << ": Kill " << TARGET << '\n' 
#define LOG_KILLUKNOWNTARGET(TIME, NICK, TARGET) _log_file << log_time(TIME) << "s" << ": User" << NICK << ": try to kill an uknown user:  " << TARGET << '\n' 
#define LOG_KICKNOREGISTER(TIME, FD) _log_file << log_time(TIME) << "s" << ": Socket number" << FD << ": Try to kick someone without registration\n"
#define LOG_KICKUKNOWNCHAN(TIME, USER, CHAN) _log_file << log_time(TIME) << "s" << "User: " << USER << "try to kick someone in a uknown channel: " << CHAN << '\n'
#define LOG_KICKNOTONTHECHAN(TIME, USER,TARGET, CHAN) _log_file << log_time(TIME) << "s" << "User: " << USER << " try to kick " << TARGET << " is not in the channel: " << CHAN << '\n'
#define LOG_KICKWITHOUTOP(TIME, USER, CHAN) _log_file << log_time(TIME) << "s" << " User: " << USER << "try to kick someone whithout operator access in the channel: " << CHAN << '\n' 


// SERVER.COMMAND CHANNEL 

#define LOG_CREATECHAN(TIME, USER, CHAN) _log_file << log_time(TIME) << "s User: " << USER << " Create and join channel: " << CHAN << '\n' 
#define LOG_JOINCHAN(TIME, USER, CHAN) _log_file << log_time(TIME) << "s User: " << USER << " Join channel: " << CHAN << '\n' 
#define LOG_LEFTCHAN(TIME, USER, CHAN) _log_file << log_time(TIME) << "s User: " << USER << " left: " << CHAN << '\n' 
#define LOG_SENDMSGTOCHAN(TIME, USER, CHAN, MSG) _log_file << log_time(TIME) << "s User: " << USER << " send: " << CHAN << " to the channel: " << MSG << '\n'; 
#define LOG_CHANCONSTRUCT(CHAN) std::cout << " the channel constructor was called for chan: " << CHAN << '\n'; 
#define LOG_CHANDESTRUCT(CHAN) std::cout  << " the channel destructor of " << CHAN << " was called\n"; 

// COUT SENT / RECIEVE 

#define LOG_


long    log_time(time_t input_time);  


# endif 