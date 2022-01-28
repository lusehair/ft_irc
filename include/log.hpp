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
    const std::string LISTPOTENTIALADDR = "find these adresses : \n"; 
    const std::string TRYBIDINGSOCKET = "Try to bind : "; 
    const std::string INVALIDSOCKET = "cannot bind : invalid socket : "; 
    const std::string OPTSETFAIL = "cannot set option for the socket : "; 
    const std::string BINDFAIL = "cannot bind on: "; 
    const std::string BINDSUCCESS = " bind with succesful\n"; 
    const std::string LISTENFAILED = "cannot listen the socket :"; 
    const std::string NOBIND = "cannot bind and connect: "; 
    const std::string CLOSESOCKET = " has been close\n"; 
    const std::string SELECTERR = "cannot get the request from: "; 
    const std::string TIMEOUT = ""



}








# endif 