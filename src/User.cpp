#include "User.hpp"

irc::User::User(std::string nickname, std::string username, int socket)
    : _own_socket(socket)
    , _nickname(nickname)
    , _username(username)
{
}

irc::User::~User()
{
}
