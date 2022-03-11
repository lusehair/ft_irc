#include "Server.hpp"

/**
 * @brief KILL command, only use by operator (see command below)
 * 
 * @param input_socket 
 * @param command_line 
 * @param input_user 
 * @return std::string* 
 */
std::string *
irc::Server::cmd_kill(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.find(input_socket);
        current_unnamed_user->second._pending_data._send.append(ERR_NOTREGISTERED);
        _pending_sends.insert(std::make_pair(input_socket, &current_unnamed_user->second._pending_data._send));
        return &current_unnamed_user->second._pending_data._recv;
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    if(!input_user->_isOperator)
    {
        LOG_KILLWITHOUTPRIV(_raw_start_time, input_user->_nickname);
        input_user->_pending_data._send.append(ERR_NOPRIVILEGES(input_user->_nickname));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return &input_user->_pending_data._recv;
    }

    size_t space_pos = command_line.find(" ");
    size_t second_space_pos = command_line.find(" ", space_pos + 1);

    if(std::count(command_line.begin(), command_line.end(), ' ') < 2 || command_line.find(":") == command_line.npos)
    {
        LOG_NOPARAM(_raw_start_time, input_socket, command_line);
        input_user->_pending_data._send.append(ERR_NEEDMOREPARAMS(input_user->_nickname, "KILL"));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return &input_user->_pending_data._recv;
    }

    std::string target = command_line.substr(space_pos + 1, second_space_pos - (space_pos + 1));
    std::string reason = command_line.substr(second_space_pos);     

    connected_users_iterator_t connected_user_iterator = _connected_users.find(target);
    if(connected_user_iterator == _connected_users.end())
    {
        LOG_KILLUKNOWNTARGET(_raw_start_time, input_user->_nickname, target);
        input_user->_pending_data._send.append(ERR_NOSUCHNICK(target));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return &input_user->_pending_data._recv;
    }

    LOG_KILLWITHPRIV(_raw_start_time, input_user->_nickname, target);
    connected_user_iterator->second->_pending_data._send.append(MSG_KILL(input_user, reason));
    _pending_sends.insert(std::make_pair(connected_user_iterator->second->_own_socket, &connected_user_iterator->second->_pending_data._send));
    connected_user_iterator->second->_already_dead = true; 
    _to_kill_users.push_back(connected_user_iterator->second);
    quit_all_chan(connected_user_iterator->second, reason);
    return &input_user->_pending_data._recv;
}
