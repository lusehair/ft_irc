#include "Server.hpp"

/**
 * @brief Create a new user in user_map and transfert all data from pending map 
 * 
 * @param valid_unnamed_user 
 * @return std::string * 
 */
std::string *
irc::Server::user_create(unnamed_users_iterator_t valid_unnamed_user)
{
    User * new_user = new User(valid_unnamed_user->second.nickname, valid_unnamed_user->second.username, valid_unnamed_user->first);

    new_user->_pending_data._recv = valid_unnamed_user->second._pending_data._recv;
    new_user->_pending_data._send = valid_unnamed_user->second._pending_data._send;
    _connected_users.insert(std::make_pair(new_user->_nickname, new_user));
    _unnamed_users.erase(valid_unnamed_user->first);

    new_user->_pending_data._send.append(": NICK :" + new_user->_nickname + "\r\n");
    new_user->_pending_data._send.append(":" + new_user->_nickname + "!" + new_user->_username + "@" + _hostname + " 001 " + new_user->_nickname + " :Hello from irc server\r\n");

    if (_pending_sends.insert(std::make_pair(new_user->_own_socket, &(new_user->_pending_data._send))).second != true) 
    {
        std::cout << "Pending send replace in user creation\n";
        _pending_sends.find(new_user->_own_socket)->second = &(valid_unnamed_user->second._pending_data._send);
    }

    LOG_USERCONNECTED(_raw_start_time, new_user->_nickname);
    return &new_user->_pending_data._recv;
}
