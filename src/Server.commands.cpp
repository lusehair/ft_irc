#include "Server.hpp"
#include "log.hpp"

void
irc::Server::init_commands_map( void )
{
    (irc::Server::_commands).insert(std::make_pair(PASS, &irc::Server::cmd_pass));
    (irc::Server::_commands).insert(std::make_pair(NICK, &irc::Server::cmd_nick));
    (irc::Server::_commands).insert(std::make_pair(USER, &irc::Server::cmd_user));
}

// function to split on \n, check if there is one and delete the command line from pending_recv after each call to the corresponding command function

template <>
void irc::Server::cmd_caller<std::map<int, pending_socket>::iterator>(std::map<int, pending_socket>::iterator unnamed_user_iterator)
{
    std::string input_command(_main_buffer); 
    std::string command_name = input_command.substr(0, input_command.find(" "));
    // std::string ret;
    // raw_command.copy(ret.c_str(), end); 

    if (input_command.size() - command_name.size() + 1 > 0)
    {
        std::map<const std::string, command_function>::iterator it = (irc::Server::_commands).find(command_name); 
        if(it == (irc::Server::_commands).end())
        {
            // LOG CMD : [FD] sent bad request : _main_buffer 
        // handle non-existing commands
            return; 
        }
        else 
        {
            (this->*(it->second))(input_socket, /* parsed string */, NULL);
        }
    }
    else
    {
        // ERR_NEEDMOREPARAMS
    }
}

template <>
void irc::Server::cmd_caller<irc::User *>(User * input_user)
{
    std::string input_command(_main_buffer); 
    std::string command_name = input_command.substr(0, input_command.find(" "));
    // std::string ret;
    // raw_command.copy(ret.c_str(), end);

    if (input_command.size() - command_name.size() + 1 > 0)
    {
        std::map<const std::string, command_function>::iterator it = (irc::Server::_commands).find(command_name); 
        if(it == (irc::Server::_commands).end())
        {
            // LOG CMD : [FD] sent bad request : _main_buffer 
        // handle non-existing commands
            return; 
        }
        else 
        {
            (this->*(it->second))(input_user->_own_socket, /* parsed string */, input_user);
        }
    }
    else
    {
        // ERR_NEEDMOREPARAMS
    }
}

void    irc::Server::send_header(const User * input_user) const
{
    std::string line = "Hello from server " + input_user->_nickname + '\n'; 
    std::ifstream head ("src/head_message"); 
    int user_socket = input_user->_own_socket; 

    if(head.is_open())
    {
        while(getline (head, line))
        {
          send(user_socket, line.c_str(), line.size(), 0);  
          line.clear(); 
        }
    }
   send(user_socket, line.c_str(), line.size(), 0);
}


size_t    blank_arguments(std::string input_line, const char * cmd)
{
    (void) input_line;
    (void )cmd;

    for(size_t i = strlen(cmd) + 1 ; input_line.size(); i++)
    {
        if(input_line[i] != ' ' )
        {
              //std::cout << "INTO THE FOR|" << input_line[i] << '|';
              return i;
        }
          
    }
    //puts("why is not here");
    return 0;
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
 * 1. If the users is already register (in unnamed users map, find by input_fd)
 * 2. If we have no argument (no argument after "PASS" command)
 * 3. If the pass given by the user doesn't match with the server password.
 * 
 * @param input_fd (int)
 * @sa RFC 2812 (3.1.1)
 * 
 */



void irc::Server::cmd_pass(const int input_fd, const std::string command_line, User * input_user)
{
    std::map<int, pending_socket>::iterator unnamed_it = _unnamed_users.find(input_fd); 

    // loop in all users to see if the socket is already registered
    if(input_user != NULL)
    {
        LOG_PASSTWICE(_raw_start_time, input_fd);
        send(input_fd, ERR_ALREADYREGISTRED, strlen(ERR_ALREADYREGISTRED), 0);
        // LOG PASS [NICk] : Try to set pass again 
        return ;
    }

    if(!blank_arguments(command_line, PASS))
    {
        LOG_NOPARAM(_raw_start_time, input_fd, command_line);
        send(input_fd, ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return ;
    }   
    
    std::string clean_pass = command_line.substr(strlen(PASS) + 1); 
    int *hash_pass = pass_hash(clean_pass); 
    for(unsigned long i = 0; i < clean_pass.size(); i++)
    {
        if(hash_pass[i] != _password[i])
        {
            delete (hash_pass);
            LOG_PASSFAILED(_raw_start_time, input_fd); 
            _unnamed_users.erase(input_fd);
            _opened_sockets.erase(input_fd);
            FD_CLR(input_fd, &_client_sockets);
            close(input_fd);
            return ;
        }
        _unnamed_users[input_fd].pass_check = true;
        // LOG PASS [SOCKET] pass succesfull 
        LOG_PASSSUCCESS(_raw_start_time, input_fd); 
    }
    

    // _unnamed_users.insert(make_pair(input_fd, "")); 
    // delete(hash_pass); 
}

/**
 * @brief Command NICK from IRC Protocol 
 * 
 * 
 * We have 4 cases 
 * 1. If User want changes his nickname to an another (same FD)
 * 2. If the User have the same nickname to another, kick the two user 
 * 3. If it's a new user, we put the nickname in unnamed_users map. 
 * 4. If the nickname is not in the block_list
 * the void* is cast into a input_fd (int)
 * 
 * @param void* input_socket 
 *@sa RFC 2812 (3.1.2)
 */
void irc::Server::cmd_nick(const int input_fd, const std::string command_line, User * input_user)
{
    if(!blank_arguments(command_line, NICK))
    {
        // err arg
        return;
    }
    std::string nick = command_line.substr(strlen(NICK) + 1); // segfault?
    if (input_user != NULL)
    {
        if(input_user->_own_socket == input_fd) 
        {
            User * tmp = input_user;
            _connected_users.erase(input_user->_nickname); 
            // LOG NICK [tmp->nick] change his nickname for nick
            LOG_NICKCHANGE(_raw_start_time, tmp->_nickname, nick);
            tmp->_nickname = nick;
            _connected_users.insert(std::make_pair(nick, tmp));
            // REPLY TO CLIENT??
        }
        else 
        {
            LOG_NICKTAKEN(_raw_start_time,input_user->_nickname, nick); 
        }
        return ;
    }

    // LOG NICK [FD] : Connected to the server has [NICKNAME] nickname
    LOG_NICKREGISTER(_raw_start_time, nick);

    std::map<int, pending_socket>::iterator unnamed_it = _unnamed_users.find(input_fd);
    unnamed_it->second.nick_name = nick;
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
void irc::Server::cmd_user(const int input_fd, const std::string command_line, User * input_user)
{

   
    std::size_t start = blank_arguments(command_line, USER); // username start
    std::size_t end = command_line.find(' ', start); // username end
    std::size_t nb_of_space = std::count(command_line.begin(), command_line.end(), ' ');

    if((nb_of_space < 4 || command_line.find(':') == std::string::npos) || !blank_arguments(command_line, USER))
    {
        // LOG USER ERROR [Nick] : Bad request no paramater
        LOG_NOPARAM(_raw_start_time, input_fd, command_line);
        send(input_fd, ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return ; 
    }

    std::string username = command_line.substr(5, end - start);
    std::map<int, pending_socket>::iterator unnamed_it = _unnamed_users.find(input_fd);

    if(input_user != NULL || (unnamed_it != _unnamed_users.end()))
    {
            // LOG USER ERROR [Nick] : [USERNAME] is already taken
            LOG_USERTAKEN(_raw_start_time, unnamed_it->second.nick_name, username); 
            send(input_fd, ERR_ALREADYREGISTRED, strlen(ERR_ALREADYREGISTRED), 0); 
            return ;
    }
    
    else if (unnamed_it->second.nick_name.size() != 0)
    {
            User * new_user = new User(unnamed_it->second.nick_name, username, unnamed_it->first); 
            _connected_users.insert(std::make_pair(unnamed_it->second.nick_name, new_user)); 
            _unnamed_users.erase(input_fd); 
            send_header(new_user);
            // LOG USER : [NICKNAME] is connected to the server 
            LOG_USERCONNECTED(_raw_start_time, unnamed_it->second.nick_name);
    }
    
}

void    irc::Server::cmd_pong(const int input_fd, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        send(input_fd, ERR_ALREADYREGISTRED, strlen(ERR_ALREADYREGISTRED), 0); 

    }
    std::string ret = command_line; 
    ret.replace(1,1,"O"); 
    send(input_fd, ret.c_str(), ret.size(),0);
}


void    irc::Server::cmd_kick(const int input_fd, const std::string command_line, User * input_user)
{
    if(input_user == NULL || !input_user->_isOperator)
    {
        send(input_fd, ERR_NOPRIVILEGES, strlen(ERR_NOPRIVILEGES), 0);
    }

}

