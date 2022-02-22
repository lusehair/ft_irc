#include "Server.hpp"

/**
 * @brief Hash Password 
 * 
 * This method take a string as parameter and transform inch character as int value transform by arthimetical operations.
 * the method return an array of int with encrypted password. 
 * 
 * @param input_pass (char*) 
 * @return int* 
 */
int *    irc::Server::pass_hash(std::string input_pass)
{
    int len = input_pass.size();
    int * ret = new int[len];
    for (int i = 0; i < len; i++)
    {
        ret[i] = (input_pass[i] * len) - (input_pass[i] - i); 
    }
    return (ret); 
}

/**
 * @brief Command PASS from IRC Protocol 
 * 
 * We have 3 cases : 
 * 1. If the users is already register (in unnamed users map, find by input_socket)
 * 2. If we have no argument (no argument after "PASS" command)
 * 3. If the pass given by the user doesn't match with the server password.
 * 
 * @param input_socket (int)
 * @sa RFC 2812 (3.1.1)
 * 
 */
std::string * irc::Server::cmd_pass(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user != NULL)
    {
        LOG_PASSTWICE(_raw_start_time, input_socket);
        input_user->_pending_data._send.append(ERR_ALREADYREGISTRED(input_user->_nickname));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    unnamed_users_iterator_t current_unnamed_user = _unnamed_users.insert(std::make_pair(input_socket, pending_socket())).first;
    //_opened_sockets.insert(input_socket);

    if (std::count(command_line.begin(), command_line.end(), ' ') < 1)
    {
        current_unnamed_user->second._pending_data._send.append(ERR_NEEDMOREPARAMS("*", PASS));
        _pending_sends.insert(std::make_pair(input_socket, &(current_unnamed_user->second._pending_data._send)));
        return &current_unnamed_user->second._pending_data._recv;
    }

    std::string clean_pass = command_line.substr(strlen(PASS) + 1); 
    if(clean_pass.size() != _passlength)
    {
            LOG_PASSFAILED(_raw_start_time, input_socket);
            _unnamed_users.erase(current_unnamed_user);
            _opened_sockets.erase(current_unnamed_user->first);
            FD_CLR(input_socket, &_client_sockets);
            close(input_socket);
            return NULL; 
    }
    
    int *hash_pass = pass_hash(clean_pass);
    for(unsigned long i = 0; i < clean_pass.size(); i++)
    {
        if(hash_pass[i] != _password[i])
        {
            delete (hash_pass);
            LOG_PASSFAILED(_raw_start_time, input_socket);
            _unnamed_users.erase(current_unnamed_user);
            _opened_sockets.erase(current_unnamed_user->first);
            FD_CLR(input_socket, &_client_sockets);
            close(input_socket);
            return NULL;
        }
    }

    LOG_PASSSUCCESS(_raw_start_time, input_socket);
    
    current_unnamed_user->second.pass_check = true;
    delete(hash_pass);
    
    return &current_unnamed_user->second._pending_data._recv;
}
