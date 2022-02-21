#include "Server.hpp"

/**
 * @brief oper command, use to have operator privilege 
 * 
 * @param input_socket 
 * @param command_line 
 * @param input_user 
 * @return std::string* 
 */
std::string *
irc::Server::cmd_oper(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.find(input_socket);
        current_unnamed_user->second._pending_data._send.append(ERR_NOTREGISTERED);
        _pending_sends.insert(std::make_pair(input_socket, &current_unnamed_user->second._pending_data._recv));
        return &current_unnamed_user->second._pending_data._recv;
    }
    else if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    if (std::count(command_line.begin(), command_line.end(), ' ') < 2)
    {
        input_user->_pending_data._send.append(ERR_NEEDMOREPARAMS(input_user->_nickname, OPER)); 
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    size_t first_space_pos = command_line.find(" ");
    size_t second_space_pos = command_line.find(" ", first_space_pos + 1);
    std::string log = command_line.substr(first_space_pos + 1, second_space_pos - (strlen(OPER) + 1));
    std::string pass = command_line.substr(second_space_pos + 1, command_line.size() - first_space_pos);

    if (log == _oper_log && pass == _oper_pass) 
    {
        input_user->_isOperator = true;
        input_user->_pending_data._send.append(RPL_YOUREOPER(input_user));
    } 
    else
    {
        input_user->_pending_data._send.append(ERR_PASSWDMISMATCH);
    }
    
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
    return (&input_user->_pending_data._recv);
}
