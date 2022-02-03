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
        User::User(char *input) : _isOperator(false), _username(set_nick(input), _nickname(set_username(input)))
        {
        }

        bool _isOperator;
        std::list<const irc::set _own_chan> 

    public: 
        std::string _username; 
        std::string _nickname;
        int _own_socket;
}






#endif 