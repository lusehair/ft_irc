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
}

// function to split on \n, check if there is one and delete the command line from _recv after each call to the corresponding command function

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

std::string irc::Server::reply(const User * input_user ,  const char * code, std::string message) const 
{
    std::string reply = ":" + input_user->_nickname + "!" + input_user->_username + "@" + "localhost " + code + " " + input_user->_nickname + " :" + message; 
    // reply += code;
    // reply+= " ";
    // reply += input_user->_nickname; 
    // reply += " : " + message; 
    //std::cout << "__ INTO the reply |" << reply << std::endl;
    return(reply); 

}


std::string irc::Server::head(const User *input_user)
{
    std::string ret =  ":" + input_user->_nickname + "!" + input_user->_username + "@" + "localhost ";
    return (ret);
}

// void    irc::Server::send_header(User * input_user) const
// {
//     //std::string line = "Hello from server " + input_user->_nickname + "\r\n"; 
//     //std::string retline; 
//     //std::ifstream head ("src/head_message"); 
//     //int user_socket = input_user->_own_socket; 
//     //int i = 1;
//     //if(head.is_open())
//     //{
//       //  while(getline (head, line))
//         //{
//           std::string retline = reply(input_user, "001", "Hello from lusehair server\r\n");
//           input_user->_pending_data._send.append(retline);
//                                                     //   send(input_user->_own_socket, retline.c_str(), retline.size(), 0);  
//           //i++;
//           //line.clear(); 
//         //}
//     //}
    
// }

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
 * 1. If the users is already register (in unnamed users map, find by input_fd)
 * 2. If we have no argument (no argument after "PASS" command)
 * 3. If the pass given by the user doesn't match with the server password.
 * 
 * @param input_fd (int)
 * @sa RFC 2812 (3.1.1)
 * 
 */



std::string * irc::Server::cmd_pass(const int input_fd, const std::string command_line, User * input_user)
{
    // loop in all users to see if the socket is already registered
    if(input_user != NULL)
    {
        LOG_PASSTWICE(_raw_start_time, input_fd);
        input_user->_pending_data._send.append(ERR_ALREADYREGISTRED(input_user, input_user->_nickname));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
                                        // send(input_fd, ERR_ALREADYREGISTRED, strlen(ERR_ALREADYREGISTRED), 0);
        // LOG PASS [NICk] : Try to set pass again 
        return &input_user->_pending_data._recv;
    }

    unnamed_users_iterator_t current_unnamed_user = _unnamed_users.insert(std::make_pair(input_fd, pending_socket())).first;
    _opened_sockets.insert(input_fd);

    if(command_line.length() < strlen(PASS) + 2)
    {
       // No need to send a msg 
        // LOG_NOPARAM(_raw_start_time, input_fd, command_line);
        // current_unnamed_user->second._pending_data._send.append(ERR_NEEDMOREPARAMS(input_user, ));
        // _pending_sends.insert(std::make_pair(current_unnamed_user->first, &(current_unnamed_user->second._pending_data._send)));
                                        // send(input_fd, ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return &current_unnamed_user->second._pending_data._recv;
    }   
    
    std::string clean_pass = command_line.substr(strlen(PASS) + 1); 
    if(clean_pass.size() != _passlength)
    {
            LOG_PASSFAILED(_raw_start_time, input_fd);
            _unnamed_users.erase(current_unnamed_user);
            _opened_sockets.erase(current_unnamed_user->first);
            FD_CLR(input_fd, &_client_sockets);
            close(input_fd);
            return NULL; 
    }
    int *hash_pass = pass_hash(clean_pass);
    
    for(unsigned long i = 0; i < clean_pass.size(); i++)
    {
        if(hash_pass[i] != _password[i])
        {
            delete (hash_pass);
            LOG_PASSFAILED(_raw_start_time, input_fd);
            _unnamed_users.erase(current_unnamed_user);
            _opened_sockets.erase(current_unnamed_user->first);
            FD_CLR(input_fd, &_client_sockets);
            close(input_fd);
            return NULL;
        }
        // LOG PASS [SOCKET] pass succesfull 
    }
    LOG_PASSSUCCESS(_raw_start_time, input_fd); 
    current_unnamed_user->second.pass_check = true;
    // _unnamed_users.insert(make_pair(input_fd, "")); 
    delete(hash_pass);
    return &current_unnamed_user->second._pending_data._recv;
}

std::string *
irc::Server::user_create(unnamed_users_iterator_t valid_unnamed_user)
{
    User * new_user = new User(valid_unnamed_user->second.nickname, valid_unnamed_user->second.username, valid_unnamed_user->first);

    new_user->_pending_data._recv = valid_unnamed_user->second._pending_data._recv;
    new_user->_pending_data._send = valid_unnamed_user->second._pending_data._send;
    _connected_users.insert(std::make_pair(new_user->_nickname, new_user));
    _unnamed_users.erase(valid_unnamed_user->first);

    new_user->_pending_data._send.append(reply(new_user, "001", "Hello from irc server\r\n"));

    if (_pending_sends.insert(std::make_pair(new_user->_own_socket, &(new_user->_pending_data._send))).second != true) {
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
std::string * irc::Server::cmd_nick(const int input_fd, const std::string command_line, User * input_user)
{
    std::string nick = command_line.substr(strlen(NICK) + 1, command_line.length() - strlen(NICK) + 1); // segfault?

    if (input_user != NULL)
    {   
        if(_connected_users.find(nick) != _connected_users.end())
        {
            LOG_NICKTAKEN(_raw_start_time,input_user->_nickname, nick);
            input_user->_pending_data._send.append(ERR_NICKNAMEINUSE(input_user, input_user->_nickname, nick));
            _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        }
        else // if (input_user->_own_socket == input_fd)  ?? to verify ??
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
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.insert(std::make_pair(input_fd, pending_socket())).first;
        
        if(_connected_users.find(nick) != _connected_users.end())
        {
            std::string ret = ": 433 * " + nick + " :Nickname is already in use\r\n"; 
            current_unnamed_user->second._pending_data._send.append(ret); 
            _pending_sends.insert(std::make_pair(input_fd, &(current_unnamed_user->second._pending_data._send)));
            return &current_unnamed_user->second._pending_data._recv;
        }

        if (current_unnamed_user->second.pass_check != true) {
            _unnamed_users.erase(current_unnamed_user);
            _opened_sockets.erase(current_unnamed_user->first);
            _pending_sends.erase(current_unnamed_user->first);
            FD_CLR(input_fd, &_client_sockets);
            close(input_fd);
            return NULL;
        }

        current_unnamed_user->second.nickname = nick;
        LOG_NICKREGISTER(_raw_start_time, nick);

        if (!current_unnamed_user->second.username.empty()) {
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
 * @param input_fd (int)
 * @sa RFC 2812 (3.1.3)
 * 
 */
std::string * irc::Server::cmd_user(const int input_fd, const std::string command_line, User * input_user)
{
    if(input_user != NULL)
    {
        LOG_USERTAKEN(_raw_start_time, input_user->_nickname, input_user->_username);
        input_user->_pending_data._send.append(ERR_ALREADYREGISTRED(input_user, input_user->_nickname)); // error code format the packet
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    std::map<int, pending_socket>::iterator current_unnamed_user = _unnamed_users.find(input_fd);

    if (current_unnamed_user->second.pass_check != true) {
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

        // no need to send msg 
        LOG_NOPARAM(_raw_start_time, input_fd, command_line);
        // current_unnamed_user->second._pending_data._send.append(ERR_NEEDMOREPARAMS());
        // _pending_sends.insert(std::make_pair(current_unnamed_user->first, &(current_unnamed_user->second._pending_data._send)));
        return &current_unnamed_user->second._pending_data._recv; 
    }

    std::size_t start = strlen(USER) + 1; //username
    std::size_t end = command_line.find(' ', start); // username end
    
    current_unnamed_user->second.username = command_line.substr(start, end - start);
    if (!current_unnamed_user->second.nickname.empty()) {
        return (user_create(current_unnamed_user));
    }
    return (&current_unnamed_user->second._pending_data._recv);
}

std::string *    irc::Server::cmd_ping(const int input_fd, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        LOG_PONGNOREGISTERUSER(_raw_start_time, input_fd);
        send(input_fd, ERR_NOTREGISTERED, strlen(ERR_NOTREGISTERED), 0); 
    }
    std::string ret = head(input_user) + "PONG :" + command_line.substr(command_line.find(" ") + 1, command_line.size() - command_line.find(" ") + 1) + "\r\n";  
    //std::cout << "_____THE PING MSG IS :" << ret << "\n";
    LOG_PONGUSERPING(_raw_start_time, input_user->_nickname);
    input_user->_pending_data._send.append(ret);
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
    return &input_user->_pending_data._recv;
} 

// void    irc::Server::cmd_kill(const int input_fd, const std::string command_line, User * input_user)
// {
//     size_t end = command_line.find(" ", strlen(KILL) + 2); 
//     std::string killed_user = command_line.substr(strlen(KILL) + 2, end - strlen(KILL) + 2);
//     std::string reason = command_line.substr(end + 1, command_line.size() - end);     
    
//     if(input_user == NULL)
//     {
//         LOG_KILLNOREGISTER(_raw_start_time, input_fd);
//         send(input_fd, ERR_NOPRIVILEGES, strlen(ERR_NOPRIVILEGES), 0);
//         return ; 
//     }

//     else if(!input_user->_isOperator)
//     {
//         LOG_KILLWITHOUTPRIV(_raw_start_time, input_user->_nickname);
//         send(input_fd, ERR_NOPRIVILEGES, strlen(ERR_NOPRIVILEGES), 0);
//         return ; 
//     }

//     else if(std::count(command_line.begin(), command_line.end(), ' ') < 2 && command_line.find_last_of(' ') < command_line.size())
//     {
        
//         LOG_NOPARAM(_raw_start_time, input_fd, command_line);
//         send(input_fd, ERR_NEEDMOREPARAMS, strlen(ERR_NOPRIVILEGES), 0);
//         return ;
//     }

    

//     std::map<std::string , User * >::iterator user_it = _connected_users.find(killed_user);
//     if(user_it == _connected_users.end())
//     {
//         LOG_KILLUKNOWNTARGET(_raw_start_time, input_user->_nickname, killed_user);
//         send(input_fd, ERR_NOSUCHNICK, strlen(ERR_NOSUCHNICK), 0);
//     }

//     else if(input_user->_isOperator)
//     {
//         LOG_KILLWITHPRIV(_raw_start_time, input_user->_nickname, killed_user);
//         // maybe send another msg to killed user 
//         send(input_fd, reason.c_str(), reason.size(), 0);
//         // close & delete user method()
//         disconnect_user(user_it);
//     }
// }



// void    irc::Server::cmd_quit(const int input_fd, const std::string command_line, User * input_user)
// {
//     if(input_user == NULL)
//     {
//         return ;
//     }
//     else if(command_line.size() == strlen(QUIT) + 1)
//     {
//         // send message at all user in the chan "user input_user->_nickame has left"
//         disconnect_user(input_user); 
//         return ;
//     }
    
//     std::string quit_message = command_line.substr(strlen(QUIT) + 1, command_line.size() - strlen(QUIT) + 1); 
//     // send at all users in the chan "user input_user->nickname has left : [quit_message]"
// }


// void    irc::Server::cmd_list(const int input_fd, const std::string command_line, User * input_user)
// {
//     if(input_user == NULL)
//     {
//         send(input_fd, ERR_NOTREGISTERED, strlen(ERR_NOTREGISTERED), 0);
//         return ; 
//     }
    
//     send(input_fd, RPL_LISTSTART, strlen(RPL_LISTSTART), 0);
//     if(command_line.find("#") == std::string::npos)
//     {
//         std::string ret_list; 
//         for(std::set<void *>::iterator running_channel_iterator = _running_channels.begin(); running_channel_iterator != running_channel_iterator.end() ; running_channel_iterator++)
//         {
//            ret_list = running_channel_iterator->_name; 
//            ret_list+= " "; 
//            ret_list+= running_channel_iterator->_members_count;

//            if(!running_channel_iterator->_topic.empty)
//            {
//                ret_list+= " "; 
//                ret_list+= running_channel_iterator->_topic;
//            }
//            ret_list+= '\n';
//            send(input_fd, ret_list.c_str(), ret_list.size(), 0);
//            ret_list.clear();
//         }
//         return; 

//     }
    
//     else
//     {
//         size_t pos = 0;  
//         size_t running_channel_iteratorerate;
//         std::string requiere_chan; 
//         size_t end; 
//         std::set<std::string>::iterator running_channel_iterator;
//         std::string ret_list;

//         while((running_channel_iteratorerate = command_line.find_first_of("#", pos)) != std::string::npos)
//         {
//             end = command_line.find_first_of(" ", running_channel_iteratorerate); 
//             requiere_chan = command_line.substr(running_channel_iteratorerate, end - running_channel_iteratorerate); 
//             running_channel_iterator = _running_channels.find(requiere_chan); 
            
//             if(running_channel_iterator != _running_channels.end())
//             {
//                 ret_list = running_channel_iterator->_name; 
//                 ret_list+= " "; 
//                 ret_list+= running_channel_iterator->_members_count;

//                 if(!running_channel_iterator->_topic.empty)
//                 {
//                     ret_list+= " "; 
//                     ret_list+= chan->it_topic;
//                 }

//                 ret_list+= '\n';
//                 send(input_fd, ret_list.c_str(), ret_list.size(), 0);
//                 ret_list.clear();
//             }

//             pos = running_channel_iteratorerate + 1; 
//             requiere_chan.clear(); 
//         }
//     }
//     send(input_fd, RPL_LISTEND, strlen(RPL_LISTEND), 0);
// }


// void    irc::Server::cmd_kick(const int input_fd, const std::string command_line, User * input_user)
// {
//     if (input_user == NULL)
//     {
//         LOG_KICKNOREGISTER(_raw_start_time, input_fd);
//         send(input_fd, ERR_CHANOPRIVSNEEDED, strlen(ERR_CHANOPRIVSNEEDED), 0); 
//         return ;
//     }
    

//     else if (std::count(command_line.begin(), command_line.end(), ' ') < 2 && command_line.find_last_of(' ') < command_line.size())
//     {
        
//         LOG_NOPARAM(_raw_start_time, input_fd, command_line);
//         send(input_fd, ERR_NEEDMOREPARAMS, strlen(ERR_NOPRIVILEGES), 0);
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
//         send(input_fd, ERR_NOSUCHCHANNEL, strlen(ERR_NOSUCHCHANNEL), 0); 
//         return ;
//     }
//     else if(/* the target_user is not in the chan*/)
//     {
//         LOG_KICKNOTONTHECHAN(_raw_start_time, input_user->_nickname, user_target, channel_target); 
//         send(input_fd, ERR_NOTONCHANNEL, strlen(ERR_NOTONCHANNEL), 0); 
//         return ;
//     }
//     else if(/* the user doesn't have the op access to kick someone*/)
//     {
//         LOG_KICKWITHOUTOP(_raw_start_time, input_user->_nickname, channel_target); 
//         send(input_fd, ERR_CHANOPRIVSNEEDED, strlen(ERR_CHANOPRIVSNEEDED), 0); 
//         return ;
//     }
//     else 
//     {
//         // KICK THE USER 
//     }
// }

void irc::Server::send_names(User * input_user, Channel * channel_target)
{
    std::string ret = head(input_user) + " 353 " + input_user->_nickname + " = #" + channel_target->getName()+ " :" + input_user->_nickname; 
    std::map<User*, const bool>::iterator members_it; 
    std::string notify = head(input_user) + " JOIN :#" + channel_target->getName() + "\r\n";
    puts("into the send names");
    for(members_it =  channel_target->_members.begin(); members_it !=  channel_target->_members.end() ; members_it++)
    {
        if(members_it->first->_own_socket != input_user->_own_socket)
        {
            members_it->first->_pending_data._send.append(notify); 
            puts("into the for");
            _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
            if(members_it->second == false)
            {
                ret.append(" " + members_it->first->_nickname);
            }
            else 
            {
                ret.append(" @" + members_it->first->_nickname);
            }
        }
    }
    ret.append("\r\n");
    input_user->_pending_data._send.append(ret + RPL_ENDOFNAMES(input_user, input_user->_nickname, channel_target->getName())); 
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
    std::cout << "___ the ret for new user is : |" << ret; 
}


std::string *    irc::Server::cmd_join(const int input_fd, const std::string command_line, User * input_user)
{
    (void)input_fd;
    if(input_user == NULL)
    {
        // instead kill the connection
        return NULL;
    }

    size_t next_comma, next_space, next_hashtag = 0;
    std::string channel_name;
    running_channels_iterator_t running_channels_iterator;

    while ((next_hashtag = command_line.find('#', next_hashtag)) != command_line.npos) {
        next_space = command_line.find(' ', next_hashtag);
        next_comma = command_line.find(',', next_hashtag);
        channel_name = command_line.substr(next_hashtag + 1, std::min(std::min(next_space, next_comma), command_line.length()) - next_hashtag + 1);
        running_channels_iterator = _running_channels.find(channel_name);
        if (running_channels_iterator != _running_channels.end()) {
            running_channels_iterator->second->add_user(input_user);
        } else {
            running_channels_iterator = _running_channels.insert(std::make_pair(channel_name, new irc::Channel(*this, input_user, channel_name))).first;
        }
        input_user->make_current(running_channels_iterator->second);

        ++next_hashtag;
        // channel joined msg
        send_names(input_user, running_channels_iterator->second);
        // send_names
    }
    return &input_user->_pending_data._recv;
}



std::string *    irc::Server::cmd_privmsg(const int input_fd, const std::string command_line, User *input_user)
{
    if(input_user == NULL)
        return &_unnamed_users.find(input_fd)->second._pending_data._recv;

    size_t start = command_line.find(" ") + 1;
    size_t end = command_line.find(":") - 1;
    std::string sender = input_user->_nickname; 
    std::string reciever = command_line.substr(start, end - start);
    std::string ret = head(input_user) + command_line + "\r\n"; 
    std::cout << "___THIS IS THE RET |" << ret; 
    if(command_line.find("#") != std::string::npos)
    {
        privmsg_hashtag_case(ret, input_user);
        return &input_user->_pending_data._recv;
    }
    std::cout << "___ the recieve : |" << reciever << "|\n";
    std::map<std::string , User * >::iterator user_it = _connected_users.find(reciever);
    
    if(user_it == _connected_users.end())
    {
        ret = ERR_NOSUCHNICK(input_user, reciever);
        puts("should be not here");
        input_user->_pending_data._send.append(ret);
        _pending_sends.insert(std::make_pair(input_fd, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;         
    }
    // Change the send to happen 
    //send(user_it->_own_socket, ret-c_str(), ret.size(), 0); 
    user_it->second->_pending_data._send.append(ret);
    _pending_sends.insert(std::make_pair(user_it->second->_own_socket, &(user_it->second->_pending_data._send)));
    //try_sending_data();
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
        puts("still here");
        ERR_NOSUCHCHANNEL(input_user, chan); // add the response to the user
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
    return ; 
}


