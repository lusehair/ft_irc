#include "Server.hpp"

/**
 * @brief Cmd caller select the good command from request of the user
 * 
 * @tparam  
 * @param unnamed_user_iterator 
 */
template <>
void irc::Server::cmd_caller<std::map<int, irc::Server::pending_socket>::iterator>(std::map<int, pending_socket>::iterator unnamed_user_iterator)
{
    std::string * received_data = &(unnamed_user_iterator->second._pending_data._recv);
    size_t  endl_pos;
    size_t  last_endl_pos = 0;
    
    while ((endl_pos = received_data->find("\r\n", last_endl_pos)) != received_data->npos)
    {
        std::string command_line = received_data->substr(last_endl_pos, endl_pos - last_endl_pos);
        std::cout << "|" << command_line << "|\n";
        size_t command_name_end = command_line.find(" ");
        
        if (command_name_end == command_line.npos || (command_name_end != command_line.npos && !(command_name_end < endl_pos)))
        {
            command_name_end = endl_pos;
        }
        
        std::string command_name = command_line.substr(0, command_name_end);
        std::map<const std::string, command_function>::iterator it = (irc::Server::_commands).find(command_name);
        
        if (it != irc::Server::_commands.end())
        {
            if ((received_data = (this->*(it->second))(unnamed_user_iterator->first, command_line, NULL)) == NULL) {
                return ;
            }
        }
        
        last_endl_pos = endl_pos + 2;
    }
    received_data->erase(0, last_endl_pos);
}

/**
 * @brief Cmd caller select the good command from request of the user
 * 
 * @tparam  
 * @param input_user 
 */
template <>
void irc::Server::cmd_caller<irc::User *>(User * input_user)
{
    std::string * received_data = &(input_user->_pending_data._recv);
    size_t  endl_pos;
    size_t  last_endl_pos = 0;
    
    while ((endl_pos = received_data->find("\r\n", last_endl_pos)) != received_data->npos)
    {
        std::string command_line = received_data->substr(last_endl_pos, endl_pos - last_endl_pos);
        size_t command_name_end = command_line.find(" ");
        
        if (command_name_end == command_line.npos || (command_name_end != command_line.npos && !(command_name_end < endl_pos)))
        {
            command_name_end = endl_pos;
        } 
        
        std::string command_name = command_line.substr(0, command_name_end);
        std::map<const std::string, command_function>::iterator it = (irc::Server::_commands).find(command_name);
        
        if (it != irc::Server::_commands.end())
        {
            if ((received_data = (this->*(it->second))(input_user->_own_socket, command_line, input_user)) == NULL) {
                return ;
            }
        }
        last_endl_pos = endl_pos + 2;
    }
    received_data->erase(0, last_endl_pos);
}
