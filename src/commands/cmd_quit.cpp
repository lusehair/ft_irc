#include "Server.hpp"

std::string *
irc::Server::cmd_quit(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.find(input_socket);
        _unnamed_users.erase(current_unnamed_user);
        _opened_sockets.erase(current_unnamed_user->first);
        _pending_sends.erase(current_unnamed_user->first);
        FD_CLR(current_unnamed_user->first, &_client_sockets);
        close(current_unnamed_user->first);
    }
    else if (!input_user->_already_dead)
    {
        size_t reason_begin = command_line.find(':');
        std::string reason;
        if (reason_begin != command_line.npos)
        {
            reason = command_line.substr(reason_begin - 1);
        }
        else
        {
            reason = + " :" + input_user->_nickname + "\r\n";
        }
        quit_all_chan(input_user, reason);
        close(input_user->_own_socket);
        _opened_sockets.erase(input_user->_own_socket);
        _pending_sends.erase(input_user->_own_socket);
        FD_CLR(input_user->_own_socket, &_client_sockets);
        _connected_users.erase(input_user->_nickname);
        std::vector<User *>::iterator check_if_killed = _to_kill_users.begin();
        while (check_if_killed != _to_kill_users.end())
        {
            if (*check_if_killed != input_user)
            {
                ++check_if_killed;
                continue ;
            }
            _to_kill_users.erase(check_if_killed);
            break ;
        }
        delete input_user;
    }
    return NULL;
}
