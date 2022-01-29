# ifndef LOG_HPP
# define LOG_HPP 

#include <ctime>

#define IRC_START 1001
#define ERRGETADDRINFO 1002
#define LISTPOTENTIALADDR 1003 // Do special method for iteration
#define TRYBIDINGSOCKET 1004
#define INVALIDSOCKET 1005
#define OPTSETFAIL 1006
#define BINDFAIL 1007
#define BINDSUCCESS 1008
#define LISTENFAILED 1009
#define NOBIND 1010
#define CLOSESOCKET 1011 // with fd in param 
#define SELECTERR 1012 //with fd
#define TIMEOUT 1013 //with fd
#define CONNECTACCEPT 1014
#define CONNECTFAILED 1015
#define 


class log 
{
    public: 
        log::log(t_time time, int ERROR, void *data)
        {

        }

    private 

    const std::string IRC_START = "IRC SERVER START\n"; 
    const std::string ERRGETADDRINFO = "cannot get a listen port\n";
    const std::string LISTPOTENTIALADDR = "find these adresses : \n"; //after 
    const std::string TRYBIDINGSOCKET = "Try to bind : "; // after
    const std::string INVALIDSOCKET = "cannot bind : invalid socket : "; // after 
    const std::string OPTSETFAIL = "cannot set option for the socket : ";  //after 
    const std::string BINDFAIL = "cannot bind on: ";  //after
    const std::string BINDSUCCESS = " bind with succesful\n"; //before 
    const std::string LISTENFAILED = "cannot listen the socket :";  //after 
    const std::string NOBIND = "cannot bind and connect: "; //after 
    const std::string CLOSESOCKET = " has been close\n"; //before 
    const std::string SELECTERR = "cannot get the request from: "; //after  
    const std::string TIMEOUT = " is connected to se server\n"; //before 
    const std::string CONNECTFAILED = " cannot being conencted to the server\n"; // before  
    




}








# endif 