#include "User.hpp"

irc::User::User(const std::string nickname, const std::string username, const int socket)
    : _own_socket(socket)
    , _nickname(nickname)
    , _username(username)
{
}

irc::User::~User()
{
}
