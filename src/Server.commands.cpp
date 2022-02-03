#include "Server.hpp"
#include "log.hpp"
void
irc::Server::init_commands_map( void )
{
    (irc::Server::_commands).insert(std::make_pair(PASS, &irc::Server::cmd_pass));
    (irc::Server::_commands).insert(std::make_pair(NICK, &irc::Server::cmd_nick));
    (irc::Server::_commands).insert(std::make_pair(USER, &irc::Server::cmd_user));
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
int *    irc::Server::pass_hash(const char * input_pass)
{
    int len = strlen(input_pass); 
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
 * 1. If the users is already register (in unnamed users map, find by target_socket)
 * 2. If we have no argument (no argument after "PASS" command)
 * 3. If the pass given by the user doesn't match with the server password.
 * 
 * @param input_fd (int)
 * @sa RFC 2812 (3.1.1)
 * 
 */



void irc::Server::cmd_pass(void * input_fd)
{
    const int target_socket = *(reinterpret_cast<int *>(input_fd));
    std::map<int, pending_socket>::iterator unnamed_it = _unnamed_users.find(target_socket); 

    // loop in all users to see if the socket is already registered
    if(unnamed_it == _unnamed_users.end())
    {
        LOG_PASSTWICE(_raw_start_time, target_socket);
        send(target_socket, ERR_ALREADYREGISTRED, sizeof(int), 0);
        // LOG PASS [NICk] : Try to set pass again 
        return ;
    }

    std::string input_pass(_main_buffer);
    // std::string raw_pass; 

    if(input_pass.size() < strlen(PASS) + 2)
    {
        LOG_NOPARAM(_raw_start_time, target_socket, input_pass);
        send(target_socket, ERR_NEEDMOREPARAMS, sizeof(int), 0);
        return ;
    }

    // input_pass.copy((char*)raw_pass.c_str(), input_pass.size() - 6 , 6);
    // int *hash_pass = pass_hash((char*)raw_pass.c_str()); 
    const char *clean_pass = input_pass.substr(strlen(PASS) + 1, input_pass.find('\n') - (strlen(PASS) + 1)).c_str(); 
    int *hash_pass = pass_hash(clean_pass); 

        std::cout << "client string to compare: |" << clean_pass << "|" << "the len of pass" << strlen(clean_pass) << std::endl; 


    for(unsigned long i = 0; i < strlen(clean_pass); i++)
    {
        std::cout << "client hash : " << hash_pass[i] << " the hash pass : " << _password[i] << std::endl; 
        if(hash_pass[i] != _password[i])
        {
            
            puts("here");
            delete(hash_pass);
            LOG_PASSFAILED(_raw_start_time, target_socket); 
            _unnamed_users.erase(target_socket);
            FD_CLR(target_socket, &_client_sockets);
            close(target_socket);
            return;
        }
        _unnamed_users[target_socket].pass_check = true;
        // LOG PASS [SOCKET] pass succesfull 
        LOG_PASSSUCCESS(_raw_start_time, input_fd); 
    }
    
    //print_pass(input_pass); 

    // _unnamed_users.insert(make_pair(target_socket, "")); 
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
 * the void* is cast into a target_socket (int)
 * 
 * @param void* input_socket 
 *@sa RFC 2812 (3.1.2)
 */
void irc::Server::cmd_nick(void *input_socket)
{
    const int target_socket = *(reinterpret_cast<int*>(input_socket)); 

    std::string input_nick(_main_buffer);
    std::string nick = input_nick.substr(strlen(NICK) + 1); // segfault??
    // std::size_t found = tmp.find(NICK) + 4;

    // tmp.copy((char*)nick.c_str(), tmp.size() - found , found);

    // need to check if nickname is ban

    std::map<std::string, irc::User *>::iterator connected_it = _connected_users.find(nick);
    if (connected_it != _connected_users.end())
    {
        if(connected_it->second->_own_socket == target_socket) 
        {
            User * tmp = connected_it->second; 
            _connected_users.erase(connected_it->second->_nickname); 
            // LOG NICK [tmp->nick] change his nickname for nick
            LOG_NICKCHANGE(_raw_start_time, tmp->_nickname, nick); 
            tmp->_nickname = nick; 
            _connected_users.insert(std::make_pair(nick, tmp));
            
            // REPLY TO CLIENT??
        }
        else 
        {
            // LOG NICK ERROR[SOCKET] the nickname [nick] is already taken
            LOG_NICKTAKEN(_raw_start_time,connected_it->second->_nickname, nick); 
            // SENT ERR MSG IF qui dit que le nickname est deja pris
            // que fait-on si deja pris et pas toi
        }
        return ;
    }

    // LOG NICK [FD] : Connected to the server has [NICKNAME] nickname
    LOG_NICKREGISTER(_raw_start_time, nick);

    std::map<int, pending_socket>::iterator unnamed_it = _unnamed_users.find(target_socket);
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
void irc::Server::cmd_user(void *input_socket)
{
    const int target_socket = *(reinterpret_cast<int*>(input_socket));

    std::string tmp(_main_buffer);
    // just need to check if enought spaces and ":" char 
    // first word after USER cmd use the substr like the previous method 
    // Need to store the real name, find + 1 with end.




    std::size_t start = 5; // username start
    std::size_t end = tmp.find(' ', start); // username end
    std::size_t nb_of_space = std::count(tmp.begin(), tmp.end(), ' ');

    if(nb_of_space < 4 || tmp.find(':') == std::string::npos)
    {
        // LOG USER ERROR [Nick] : Bad request no paramater
        LOG_NOPARAM(_raw_start_time, target_socket, tmp);
        send(target_socket, ERR_NEEDMOREPARAMS, sizeof(int), 0);
        return ; 
    }


    std::string username = tmp.substr(5, end - start);
    std::map<int, pending_socket>::iterator unnamed_it = _unnamed_users.find(target_socket);

    if(unnamed_it != _unnamed_users.end())
    {
        std::map<std::string, User *>::iterator check_even_connected_it = _connected_users.find(unnamed_it->second.nick_name); 
        if(check_even_connected_it != _connected_users.end())
        {
            // LOG USER ERROR [Nick] : [USERNAME] is already taken
            LOG_USERTAKEN(_raw_start_time, unnamed_it->second.nick_name, username); 
            send(target_socket, ERR_ALREADYREGISTRED, sizeof(int), 0); 
            return ;
        }
        else if (unnamed_it->second.nick_name.size() != 0)
        {
            User * new_user = new User(unnamed_it->second.nick_name, username, unnamed_it->first); 
            _connected_users.insert(std::make_pair(unnamed_it->second.nick_name, new_user)); 
            _unnamed_users.erase(target_socket); 
            send_header(new_user);
            // LOG USER : [NICKNAME] is connected to the server 
            LOG_USERCONNECTED(_raw_start_time, unnamed_it->second.nick_name); 
        }

        
        // else if(connected_it->second.nick_name.size() != 0)
        // {
        //     _connected_users.insert(User(_connected_it->second, username, target_socket));
        //     _unnamed_users.erase(target_socket);
        //     //need to set a private function for sending message (Header etc ...)
        // }
    }
}

void irc::Server::cmd_caller(int input_socket)
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
            (this->*(it->second))(&input_socket);
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
    // std::ifstream head ("src/head_message"); 
     int user_socket = input_user->_own_socket; 

    // if(head.is_open())
    // {
    //     while(getline (head, line))
    //     {
    //       send(user_socket, line.c_str(), line.size(), 0);  
    //       line.clear(); 
    //     }
    // }
   send(user_socket, line.c_str(), line.size(), 0);
}
