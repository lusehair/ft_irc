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

#include <string> // For string stoage (username)





class User
{
    private:  
    
        User() : _username("Default" + rand()), _nickname("LocalDefault"), _isOperator(false)
        {
            
        }

        User() : _isOperator(false)
        {

        }

        std::string setValue(char *raw_input)
        {
            std::string tmp = str(raw_input); 
        }
    
        std::string _username; 
        std::string _nickname;
        bool _isOperator;  
        const std::string _connect_format; 



    public: 

    
    
    
    const std::string getUsername(void) const
    {
        return (_username);
    }

    const std::string getNickname(void) const
    {
        return (_nickname);
    }


}







#endif 