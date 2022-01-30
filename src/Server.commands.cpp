
#include "Server.hpp"


/**
 * @brief Command NICK from IRC Protocol 
 * 
 * 
 * We have 4 cases 
 * 1. If User want changes his nickname to an another (same FD)
 * 2. If the User have the same nickname to another, kick the two user 
 * 3. If it's a new user, we put the nickname in unnamed_users map. 
 * 4. If the nickname is not in the block_list
 * the void* is cast into a fd (int)
 * 
 * @param void* sender 
 *@sa RFC 2812 (3.1.2)
 */
void irc::Server::cmd_nick(void *sender)
{
    const int fd = *(reinterpret_cast<int*>(sender)); 
    
    std::string nick; 
    std::string tmp(_main_buffer);
    std::size_t found = tmp.find("NICK") + 4; 

    tmp.copy((char*)nick.c_str(), tmp.size() - found , found);
    std::map<std::string, irc::User>::iterator connected_it = _connected_users.find(nick); 
    std::map<int, std::string>::iterator unnamed_it;
    
    // need to check if nickname is ban 

    if (connected_it != _connected_users.end())
    {
        if(connected_it->second._own_socket == fd) 
        {
            User* tmp = connected_it->second; 
            _connected_users.erase(connected_it->second._nickname); 
            tmp->_nickname = nick; 
            _connected_users.insert(std::make_pair(nick, tmp));
            // REPLY TO CLIENT 
        }
        else 
        {
            // SENT ERR MSG IF qui dit que le nickname est deja pris
        }
    }
    else if ((unnamed_it = _unnamed_users.find(fd)) == _unnamed_users.end())
    {
        unnamed_it->second = nick; 
    }
}


/**
 * @brief command USER from IRC Protocl
 * 
 * We have 2 cases : 
 * 1. If the USER command doesn't have username on his request 
 * 2. User is already register 
 * 
 * @param input_fd (int)
 * @sa RFC 2812 (3.1.3)
 * 
 */
void irc::Sevrer::cmd_user(void *input_fd)
{
    const int fd = *(reinterpret_cast<int*>(sender)); 
    std::string username; 
    std::string tmp(_main_buffer);
    std::size_t start = tmp.find("USER") + 5; 
    std::size_t nb_of_space = std::count(tmp.begin(), tmp.end(), ' ');
    if(nb_of_space == 4)
    {
        send(fd, 461, sizeof(int), MSG_DONTWAIT)
        return ; 
    }
    tmp.copy((char*)user.c_str(), start , end - start);
    std::map<int, std::string>::iterator unnamed_it; 


    if(connected_it != _unnamed_users.end())
    {
        std::map<int, std::string>::iterator check_even_connected_it = _connected_users.find(connected_it->second); 
        if(check_even_conneceted_it != _connected_users.end())
        {
            send(fd, 462, sizeof(int), MSG_DONTWAIT)
            return ;
        }
        else if(connected_it->second.size() != 0)
        {
            _connected_users.insert(User(_connected_it->second, username, fd));
            _unnamed_users.erase(fd);
        }
    }
}