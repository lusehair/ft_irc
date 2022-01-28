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

        // Parser for set username and nickname from the first client request 
        std::string User::set_nick(char *raw_input)
        {
            int i; 
            std::string ret; 
            std::string tmp(raw_input);
            size_t separate = tmp.find_first_of('@'); 
            i = separate - 1;
            while(tmp.compare("<"))
                i--;
            i++;
            tmp.copy((char*)ret.c_str(), (separate -1) - i, i); 
            return (ret); 
        }

        std::string User::set_username(char * raw_input)
        {
            int i; 
            std::string ret; 
            std::string tmp(raw_input); 
            size_t separate = tmp.find_first_of('@'); 
            i = separate + 1; 
            while(tmp.compare(">"))
                i++; 
            i--; 
            tmp.copy((char*)ret.c_str(), (i - 1) - separate, i);
            return (ret); 
        }


        //                 
        bool _isOperator;
        std::list<const irc::channel* own_chan> 



    public: 

    std::string _username; 
    std::string _nickname;
    int 


}







#endif 