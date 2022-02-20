#include "Server.hpp"
#include "log.hpp"
#include "Channel.hpp"

//Used for the fopen and close the MOTD file (fopen & fcloses)
#include <cstdio>


void
irc::Server::init_commands_map( void )
{
    (irc::Server::_commands).insert(std::make_pair(PASS, &irc::Server::cmd_pass));
    (irc::Server::_commands).insert(std::make_pair(NICK, &irc::Server::cmd_nick));
    (irc::Server::_commands).insert(std::make_pair(USER, &irc::Server::cmd_user));
    (irc::Server::_commands).insert(std::make_pair(PING, &irc::Server::cmd_ping));
    (irc::Server::_commands).insert(std::make_pair(JOIN, &irc::Server::cmd_join));
    (irc::Server::_commands).insert(std::make_pair(PRIVMSG, &irc::Server::cmd_privmsg));
    (irc::Server::_commands).insert(std::make_pair(KILL, &irc::Server::cmd_kill));
    (irc::Server::_commands).insert(std::make_pair(OPER, &irc::Server::cmd_oper));
    (irc::Server::_commands).insert(std::make_pair(PART, &irc::Server::cmd_part));
    (irc::Server::_commands).insert(std::make_pair(MODE, &irc::Server::cmd_mode));
    (irc::Server::_commands).insert(std::make_pair(WHO, &irc::Server::cmd_who));
    (irc::Server::_commands).insert(std::make_pair(QUIT, &irc::Server::cmd_quit));
    (irc::Server::_commands).insert(std::make_pair(LIST, &irc::Server::cmd_list));
    (irc::Server::_commands).insert(std::make_pair(NOTICE, &irc::Server::cmd_notice));
}

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


/**
 * @brief Special Syntax for Welcome message 
 * 
 * @param input_user 
 * @param code 
 * @param message 
 * @return std::string 
 */
std::string irc::Server::reply(const User * input_user ,  const char * code, std::string message) const 
{
    std::string reply = ":" + input_user->_nickname + "!" + input_user->_username + "@" + "localhost " + code + " " + input_user->_nickname + " :" + message; 
    return(reply); 
}


/**
 * @brief Header Generator for the reply to the client 
 * 
 * @param input_user 
 * @return std::string 
 */
std::string irc::Server::head(const User *input_user)
{
    std::string ret =  ":" + input_user->_nickname + "!" + input_user->_username + "@" + "localhost ";
    return (ret);
}


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
    // loop in all users to see if the socket is already registered
    if(input_user != NULL)
    {
        LOG_PASSTWICE(_raw_start_time, input_socket);
        input_user->_pending_data._send.append(ERR_ALREADYREGISTRED(input_user, input_user->_nickname));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }


    unnamed_users_iterator_t current_unnamed_user = _unnamed_users.insert(std::make_pair(input_socket, pending_socket())).first;
    _opened_sockets.insert(input_socket);

    if(command_line.length() < strlen(PASS) + 2)
    {
        LOG_NOPARAM(_raw_start_time, input_socket, command_line);
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


/**
 * @brief Create a new user in user_map and transfert all data from pending map 
 * 
 * @param valid_unnamed_user 
 * @return std::string* 
 */
std::string *
irc::Server::user_create(unnamed_users_iterator_t valid_unnamed_user)
{
    User * new_user = new User(valid_unnamed_user->second.nickname, valid_unnamed_user->second.username, valid_unnamed_user->first);

    new_user->_pending_data._recv = valid_unnamed_user->second._pending_data._recv;
    new_user->_pending_data._send = valid_unnamed_user->second._pending_data._send;
    _connected_users.insert(std::make_pair(new_user->_nickname, new_user));
    _unnamed_users.erase(valid_unnamed_user->first);


    // std::string nick_confirm = ": NICK : | " + (_connected_users.find(new_user->_nickname))->first + " | " + username + " |\r\n";
    new_user->_pending_data._send.append(": NICK :" + new_user->_nickname + "\r\n");
    new_user->_pending_data._send.append(reply(new_user, "001", "Hello from irc server\r\n"));

    if (_pending_sends.insert(std::make_pair(new_user->_own_socket, &(new_user->_pending_data._send))).second != true) 
    {
        _pending_sends.find(new_user->_own_socket)->second = &(valid_unnamed_user->second._pending_data._send);
    }

    LOG_USERCONNECTED(_raw_start_time, valid_unnamed_user->second.nickname);
    return &new_user->_pending_data._recv;
}


/**
 * @brief Command NICK from IRC Protocol 
 * 
 * 
 * We have 4 cases 
 * 1. If User want changes his nickname to an another (same FD)
 * 2. If the User have the same nickname as another, kick the two users
 * 3. If it's a new user, we put the nickname in unnamed_users map.
 * 4. If the nickname is not in the block_list
 * 
 * @param const int input_socket 
 *@sa RFC 2812 (3.1.2)
 */
std::string * irc::Server::cmd_nick(const int input_socket, const std::string command_line, User * input_user)
{
    std::string nick = command_line.substr(strlen(NICK) + 1, command_line.length() - strlen(NICK) + 1);

    if (input_user != NULL)
    {   
        if(_connected_users.find(nick) != _connected_users.end())
        {
            LOG_NICKTAKEN(_raw_start_time,input_user->_nickname, nick);
            input_user->_pending_data._send.append(ERR_NICKNAMEINUSE(input_user, input_user->_nickname, nick));
            _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        }
        
        else
        {
            input_user->_pending_data._send.append(head(input_user) + "NICK :" + nick + "\r\n");
            _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));

            _connected_users.erase(input_user->_nickname);

            LOG_NICKCHANGE(_raw_start_time, input_user->_nickname, nick);

            input_user->_nickname = nick;
            _connected_users.insert(std::make_pair(input_user->_nickname, input_user));
            return &input_user->_pending_data._recv;
        }
       
       return &input_user->_pending_data._recv;
    }
    
    else
    {
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.insert(std::make_pair(input_socket, pending_socket())).first;
        
        if(_connected_users.find(nick) != _connected_users.end())
        {
            std::string ret = ": 433 * " + nick + " :Nickname is already in use\r\n"; 
            current_unnamed_user->second._pending_data._send.append(ret); 
            _pending_sends.insert(std::make_pair(input_socket, &(current_unnamed_user->second._pending_data._send)));
            return &current_unnamed_user->second._pending_data._recv;
        }

        if (current_unnamed_user->second.pass_check != true) 
        {
            _unnamed_users.erase(current_unnamed_user);
            _opened_sockets.erase(current_unnamed_user->first);
            _pending_sends.erase(current_unnamed_user->first);
            FD_CLR(input_socket, &_client_sockets);
            close(input_socket);
            return NULL;
        }

        current_unnamed_user->second.nickname = nick;
        LOG_NICKREGISTER(_raw_start_time, nick);

        if (!current_unnamed_user->second.username.empty())
        {
            return (user_create(current_unnamed_user));
        }
        return &current_unnamed_user->second._pending_data._recv;
    }
}


/**
 * @brief command USER from IRC Protocl
 * 
 * We have 2 cases : 
 * 1. If the USER command doesn't have username on his request  (461)
 * 2. User is already register (462)
 * 
 * @param input_socket (int)
 * @sa RFC 2812 (3.1.3)
 * 
 */
std::string * irc::Server::cmd_user(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user != NULL)
    {
        LOG_USERTAKEN(_raw_start_time, input_user->_nickname, input_user->_username);
        input_user->_pending_data._send.append(ERR_ALREADYREGISTRED(input_user, input_user->_nickname));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }


    std::map<int, pending_socket>::iterator current_unnamed_user = _unnamed_users.find(input_socket);

    if (current_unnamed_user->second.pass_check != true) 
    {
        _unnamed_users.erase(current_unnamed_user);
        _opened_sockets.erase(current_unnamed_user->first);
        _pending_sends.erase(current_unnamed_user->first);
        FD_CLR(current_unnamed_user->first, &_client_sockets);
        close(current_unnamed_user->first);
        return NULL;
    }

    std::size_t nb_of_space = std::count(command_line.begin(), command_line.end(), ' ');
    if((nb_of_space < 4 || command_line.find(':') == std::string::npos))
    {
        LOG_NOPARAM(_raw_start_time, input_socket, command_line);
        return &current_unnamed_user->second._pending_data._recv; 
    }

    std::size_t start = strlen(USER) + 1; 
    std::size_t end = command_line.find(' ', start); 
    
    current_unnamed_user->second.username = command_line.substr(start, end - start);
    if (!current_unnamed_user->second.nickname.empty()) 
    {
        return (user_create(current_unnamed_user));
    }
    return (&current_unnamed_user->second._pending_data._recv);
}



/**
 * @brief Response to ping client (pong)
 * 
 * @param input_socket 
 * @param command_line 
 * @param input_user 
 * @return std::string* 
 */
std::string *    irc::Server::cmd_ping(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        LOG_PONGNOREGISTERUSER(_raw_start_time, input_socket);
        send(input_socket, ERR_NOTREGISTERED, strlen(ERR_NOTREGISTERED), 0); 
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    std::string ret = head(input_user) + "PONG :" + command_line.substr(command_line.find(" ") + 1, command_line.size() - command_line.find(" ") + 1) + "\r\n";  
    LOG_PONGUSERPING(_raw_start_time, input_user->_nickname);
    
    input_user->_pending_data._send.append(ret);
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
    
    return &input_user->_pending_data._recv;
} 

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
        return &_unnamed_users.find(input_socket)->second._pending_data._recv; 
    }

    if(input_user->_already_dead)
    {
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
        input_user->_pending_data._send.append(ERR_PASSWDMISMATCH(input_user));
    }
    
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
    return (&input_user->_pending_data._recv);
}


void
irc::Server::quit_all_chan(User * target, std::string & reason)
{
    std::string killed_notice = MSG_QUIT(target, reason);
    std::list<Channel *>::iterator current_channel = target->_joined_channels.begin();
    if (current_channel != target->_joined_channels.end())
    {
        std::list<Channel *>::iterator tmp = current_channel;
        ++current_channel;
        for(std::map<User *, const bool>::iterator members_it = (*tmp)->_members.begin();
                members_it != (*tmp)->_members.end();
                ++members_it)
        {
            members_it->first->_pending_data._send.append(killed_notice);
            _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
        }
        LOG_LEFTCHAN(_raw_start_time, target->_nickname, "from quitting all chan");
        (*tmp)->kick_user(target);
    }
}


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
        LOG_KILLNOREGISTER(_raw_start_time, input_socket);
        return &_unnamed_users.find(input_socket)->second._pending_data._recv;
    }

    if(input_user->_already_dead)
    {
             return &input_user->_pending_data._recv;
    }

    if(!input_user->_isOperator)
    {
        LOG_KILLWITHOUTPRIV(_raw_start_time, input_user->_nickname);
        input_user->_pending_data._send.append(ERR_NOPRIVILEGES(input_user, input_user->_nickname));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return &input_user->_pending_data._recv;
    }

    size_t space_pos = command_line.find(" ");
    size_t second_space_pos = command_line.find(" ", space_pos + 1);

    if(std::count(command_line.begin(), command_line.end(), ' ') < 2 || command_line.find(":") == command_line.length() - 1)
    {
        LOG_NOPARAM(_raw_start_time, input_socket, command_line);
        input_user->_pending_data._send.append(ERR_NEEDMOREPARAMS(input_user, input_user->_nickname, "KILL"));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return &input_user->_pending_data._recv;
    }

    std::string target = command_line.substr(space_pos + 1, second_space_pos - (strlen(KILL) + 1));
    std::string reason = command_line.substr(second_space_pos);     

    connected_users_iterator_t connected_user_iterator = _connected_users.find(target);
    if(connected_user_iterator == _connected_users.end())
    {
        LOG_KILLUKNOWNTARGET(_raw_start_time, input_user->_nickname, target);
        input_user->_pending_data._send.append(ERR_NOSUCHNICK(input_user, target));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return &input_user->_pending_data._recv;
    }

    LOG_KILLWITHPRIV(_raw_start_time, input_user->_nickname, target);
    connected_user_iterator->second->_pending_data._send.append(MSG_KILL(input_user, reason));
    _pending_sends.insert(std::make_pair(connected_user_iterator->second->_own_socket, &connected_user_iterator->second->_pending_data._send));
    _to_kill_users.push_back(connected_user_iterator->second);
    quit_all_chan(connected_user_iterator->second, reason);
    return &input_user->_pending_data._recv;
}


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
    
    else if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    else
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


std::string *irc::Server::cmd_notice(const int input_socket, const std::string command_line, User *input_user)
{
    if(input_user == NULL) 
    {
        return &_unnamed_users.find(input_socket)->second._pending_data._recv;
    }
    if(input_user->_already_dead)
    {
       return &_unnamed_users.find(input_socket)->second._pending_data._recv; 
    }

    size_t start = command_line.find(" ") + 1;
    size_t end = command_line.find(":") - 1;
    
    std::string sender = input_user->_nickname; 
    std::string reciever = command_line.substr(start, end - start);
    std::string ret = head(input_user) + command_line + "\r\n"; 
    
    std::map<std::string , User * >::iterator user_it = _connected_users.find(reciever);
    if(user_it == _connected_users.end())
    {
        return &input_user->_pending_data._recv;         
    }

    user_it->second->_pending_data._send.append(ret);
    _pending_sends.insert(std::make_pair(user_it->second->_own_socket, &(user_it->second->_pending_data._send)));
    return &input_user->_pending_data._recv;
}



std::string   *irc::Server::cmd_list(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        return &_unnamed_users.find(input_socket)->second._pending_data._recv;
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }


    std::string ret_list; 
    running_channels_iterator_t running_channel_iterator; 
    size_t cases = command_line.find("#");
    if(cases == std::string::npos)
    {
        for(running_channel_iterator = _running_channels.begin(); running_channel_iterator != _running_channels.end() ; running_channel_iterator++)
        {
            ret_list.append(RPL_LIST(input_user, input_user->_nickname, running_channel_iterator->first) + " " + std::to_string(running_channel_iterator->second->_members_count) + " :\r\n");
        }
       
    }
    else
    {
        size_t pos = 0; 
        while((command_line.find_first_of("#", pos)) != std::string::npos)
        {
            size_t start = pos + 2; 
            size_t end = command_line.find(" ", start); 
            std::string require_channel = command_line.substr(start, end - start); 
            if ((running_channel_iterator = _running_channels.find(require_channel)) != _running_channels.end())
            {
                ret_list.append(RPL_LIST(input_user, input_user->_nickname, running_channel_iterator->first) + " " + std::to_string(running_channel_iterator->second->_members_count) + " :\r\n");
            }
            pos = end; 
        }
    }
    ret_list.append(RPL_LISTEND(input_user, input_user->_nickname)); 
  
    input_user->_pending_data._send.append(ret_list);
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
    return &input_user->_pending_data._recv; 
}


// void    irc::Server::cmd_kick(const int input_socket, const std::string command_line, User * input_user)
// {
//     if (input_user == NULL)
//     {
//         LOG_KICKNOREGISTER(_raw_start_time, input_socket);
//         send(input_socket, ERR_CHANOPRIVSNEEDED, strlen(ERR_CHANOPRIVSNEEDED), 0); 
//         return ;
//     }
    

//     else if (std::count(command_line.begin(), command_line.end(), ' ') < 2 && command_line.find_last_of(' ') < command_line.size())
//     {
        
//         LOG_NOPARAM(_raw_start_time, input_socket, command_line);
//         send(input_socket, ERR_NEEDMOREPARAMS, strlen(ERR_NOPRIVILEGES), 0);
//         return ;
//     }
    
//     size_t start = strlen(KICK) + 2; 
//     size_t end = command_line.find(' ',strlen(KICK) + 2); 
//     std::string channel_target = command_line.substr(start, end  - (start -1));    
//     start = end + 1; 
    
//     if(std::count(command_line.begin(), command_line.end(), ' ') > 2)
//     {
//         end = command_line.find(' ',strlen(KICK) + 2);
//     }
    
//     else
//     {
//         end = command_line.size();
//     }
//     std::string user_target = command_line.substr(start, end - start);


//     if(/* the chan doesn't exist */)
//     {
//         LOG_KICKUKNOWNCHAN(_raw_start_time, input_user->_nickname, channel_target);
//         send(input_socket, ERR_NOSUCHCHANNEL, strlen(ERR_NOSUCHCHANNEL), 0); 
//         return ;
//     }
//     else if(/* the target_user is not in the chan*/)
//     {
//         LOG_KICKNOTONTHECHAN(_raw_start_time, input_user->_nickname, user_target, channel_target); 
//         send(input_socket, ERR_NOTONCHANNEL, strlen(ERR_NOTONCHANNEL), 0); 
//         return ;
//     }
//     else if(/* the user doesn't have the op access to kick someone*/)
//     {
//         LOG_KICKWITHOUTOP(_raw_start_time, input_user->_nickname, channel_target); 
//         send(input_socket, ERR_CHANOPRIVSNEEDED, strlen(ERR_CHANOPRIVSNEEDED), 0); 
//         return ;
//     }
//     else 
//     {
//         // KICK THE USER 
//     }
// }


/**
 * @brief this function is use for generate the header message when a new user enter in a channel 
 * 
 * @param input_user 
 * @param channel_target 
 */
void irc::Server::send_names(User * input_user, Channel * channel_target)
{
    std::string ret = head(input_user) + "353 " + input_user->_nickname + " = #" + channel_target->getName()+ " :";
    std::string notify = head(input_user) + "JOIN :#" + channel_target->getName() + "\r\n";

    std::map<User*, const bool>::iterator members_it;
    for(members_it =  channel_target->_members.begin(); members_it !=  channel_target->_members.end() ; members_it++)
    {
        if(members_it->second == false)
        {
            ret.append(" " + members_it->first->_nickname);
        }
        else 
        {
            ret.append(" @" + members_it->first->_nickname);
        }

        members_it->first->_pending_data._send.append(notify);
        _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
    }
    ret.append("\r\n");

    input_user->_pending_data._send.append(ret + RPL_ENDOFNAMES(input_user, input_user->_nickname, channel_target->getName())); 
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
}


/**
 * @brief JOIN command is use to create a join an existing channel 
 * 
 * @param input_socket 
 * @param command_line 
 * @param input_user 
 * @return std::string* 
 */
std::string *    irc::Server::cmd_join(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        // LOG_KILLNOREGISTER(_raw_start_time, input_socket); NOT REGISTERED
        return &_unnamed_users.find(input_socket)->second._pending_data._recv; 
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }


    size_t next_comma, next_space, next_hashtag = 0;
    std::string channel_name;
    running_channels_iterator_t running_channels_iterator;

    while ((next_hashtag = command_line.find('#', next_hashtag)) != command_line.npos) 
    {
        next_space = command_line.find(' ', next_hashtag);
        next_comma = command_line.find(',', next_hashtag);
        channel_name = command_line.substr(next_hashtag + 1, std::min(std::min(next_space, next_comma), command_line.length()) - (next_hashtag + 1));
        running_channels_iterator = _running_channels.find(channel_name);
        if (running_channels_iterator != _running_channels.end())
        {
            LOG_JOINCHAN(_raw_start_time, input_user->_nickname, channel_name); 
            running_channels_iterator->second->add_user(input_user);
        } 
        else 
        {
            running_channels_iterator = _running_channels.insert(std::make_pair(channel_name, new irc::Channel(*this, input_user, channel_name))).first;
            LOG_CREATECHAN(_raw_start_time, input_user->_nickname, channel_name);
        }
        
        input_user->make_current(running_channels_iterator->second);

        ++next_hashtag;
        send_names(input_user, running_channels_iterator->second);
    }
    return &input_user->_pending_data._recv;
}


void
irc::Server::make_user_part(User * input_user, const std::string channel_name, const std::string reason)
{
    std::string channel_part_notice = PART_NOTICE(input_user, channel_name, reason);
    running_channels_iterator_t running_channels_iterator;
    running_channels_iterator = _running_channels.find(channel_name);

    if (running_channels_iterator != _running_channels.end())
    {
        if (running_channels_iterator->second->_members.find(input_user)
                != running_channels_iterator->second->_members.end())
        {
            running_channels_iterator_t tmp = running_channels_iterator;
            ++running_channels_iterator;
            for(std::map<User *, const bool>::iterator members_it = tmp->second->_members.begin();
                    members_it !=  tmp->second->_members.end();
                    ++members_it)
            {
                members_it->first->_pending_data._send.append(channel_part_notice);
                _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
            }
            LOG_LEFTCHAN(_raw_start_time, input_user->_nickname, channel_name);
            tmp->second->kick_user(input_user);
        }
        else
        {
            input_user->_pending_data._send.append(ERR_NOTONCHANNEL(input_user, channel_name));
            _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        }
    }
    else
    {
        input_user->_pending_data._send.append(ERR_NOSUCHCHANNEL(input_user, channel_name));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
    }
}


std::string *    irc::Server::cmd_part(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        return &_unnamed_users.find(input_socket)->second._pending_data._recv; 
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }


    size_t next_comma = 0;
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

    std::string channel_name;
    while ((next_comma = command_line.find(',', next_comma)) < reason_begin)
    {
        channel_name = command_line.substr(command_line.find_last_of(", ", next_comma) + 1, next_comma);
        make_user_part(input_user, channel_name, reason);
        ++next_comma;
    }
    channel_name = command_line.substr(command_line.find_last_of(", ", next_comma) + 2, next_comma);
    make_user_part(input_user, channel_name, reason);
    return &input_user->_pending_data._recv;
}


/**
 * @brief the PRIVMSG or /msg on irssi is, to send a private message to an another user 
 * 
 * @param input_socket 
 * @param command_line 
 * @param input_user 
 * @return std::string* 
 */
std::string *    irc::Server::cmd_privmsg(const int input_socket, const std::string command_line, User *input_user)
{
    if(input_user == NULL) 
    {
        return &_unnamed_users.find(input_socket)->second._pending_data._recv;
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }


    size_t start = command_line.find(" ") + 1;
    size_t end = command_line.find(":") - 1;
    
    std::string sender = input_user->_nickname; 
    std::string reciever = command_line.substr(start, end - start);
    std::string ret = head(input_user) + command_line + "\r\n"; 
    
    if(command_line.find("#") != std::string::npos)
    {
        privmsg_hashtag_case(ret, input_user);
        return &input_user->_pending_data._recv;
    }
    
    std::map<std::string , User * >::iterator user_it = _connected_users.find(reciever);
    if(user_it == _connected_users.end())
    {
        ret = ERR_NOSUCHNICK(input_user, reciever);
        input_user->_pending_data._send.append(ret);
        _pending_sends.insert(std::make_pair(input_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;         
    }

    user_it->second->_pending_data._send.append(ret);
    _pending_sends.insert(std::make_pair(user_it->second->_own_socket, &(user_it->second->_pending_data._send)));
    return &input_user->_pending_data._recv;
}


void irc::Server::privmsg_hashtag_case(std::string command_line, User *input_user)
{
    size_t start = command_line.find("#") + 1;
    size_t end = command_line.find(" ", start); 
    std::string chan = command_line.substr(start, end - start); 
    
    running_channels_iterator_t running_channels_iterator =  _running_channels.find(chan);  
    if(running_channels_iterator == _running_channels.end())
    {
         input_user->_pending_data._send.append(ERR_NOSUCHCHANNEL(input_user, chan));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return ; 
    }
    if(!input_user->if_is_on_chan(running_channels_iterator->second))
    {
         puts("_______cant send message");
         input_user->_pending_data._send.append(ERR_NOTONCHANNEL(input_user, chan));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return ;
    }
    std::map<User*, const bool>::iterator members_it; 
    for(members_it =  running_channels_iterator->second->_members.begin(); members_it !=  running_channels_iterator->second->_members.end() ; members_it++)
    {
        if(members_it->first != input_user)
        {
            members_it->first->_pending_data._send.append(command_line); 
            _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
        }
    }
    LOG_SENDMSGTOCHAN(_raw_start_time, input_user->_nickname, chan, command_line); 
    return ; 
}


// Dirty work, need to check exception and the all cases of WHO cmd 
std::string *irc::Server::cmd_who(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        return &input_user->_pending_data._recv;;
    }

    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }


    size_t start = command_line.find("#"); 
    if(start == std::string::npos) 
    {
        return &input_user->_pending_data._recv;;
    }
    puts("Hello"); 
    std::string ret = head(input_user) + "315 " + input_user->_nickname + " " + input_user->_username + " :End of /WHO list\r\n";
    input_user->_pending_data._send.append(ret);
    
    _pending_sends.insert(std::make_pair(input_socket, &input_user->_pending_data._send));
    return &input_user->_pending_data._recv;

}

std::string   * irc::Server::cmd_mode(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        return &input_user->_pending_data._recv;;
    }
    size_t start = command_line.find("#"); 
    
    // case if the mode request is on the connection 
    // if(start == std::string::npos)
    // {
    //     start = strlen(MODE) + 2;
    //     size_t end = command_line.find(" ", pos);  
    //     std::string nickname = command_line.substr()
    //     return &input_user->_pending_data._recv; 
    // }
    start++;
    size_t end = command_line.find(" ", start);
    std::string channel = command_line.substr(start, end - start);
    running_channels_iterator_t running_channels_iterator =  _running_channels.find(channel);

    if(running_channels_iterator == _running_channels.end())
    {
        ERR_NOSUCHCHANNEL(input_user, channel);
    }

    std::string ret = head(input_user) + "324 " + input_user->_nickname + " #" + channel + " +n \r\n";
    input_user->_pending_data._send.append(ret);
    _pending_sends.insert(std::make_pair(input_socket, &(input_user->_pending_data._send)));
    return &input_user->_pending_data._recv;
}
