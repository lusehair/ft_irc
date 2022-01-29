
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



