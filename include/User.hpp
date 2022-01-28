/**
 * @file User.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

// 





# ifndef USER_HPP
# define USER_HPP 

#include <string> // For string storage (username)
#include <list> // For channel container 
#include <irc.hpp> 






class User
{
    private:  
    
        // Use for test and debuging 
        User::User() : _username("Default" + rand()), _nickname("LocalDefault"), _isOperator(false)
        {
            
        }

        User::User(std::string nickname, std::string username, int fd) : _nickname(nick), _username(username), fd(fd)
        {
        }

        // Parser for set username and nickname from the first client request


        //                 
        bool _isOperator;
        std::list<const irc::channel* own_chan> 

        ~User()
        {
            
        }



    public: 

    std::string _username; 
    std::string _nickname;
    int fd; 


}







#endif 