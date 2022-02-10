#include "User.hpp"

irc::User::User(const std::string nickname, const std::string username, const int socket)
    : _own_socket(socket)
    , _nickname(nickname)
    , _username(username)
{
}

irc::User::~User()
{
    while (!_joined_channels.empty()) {
        _joined_channels.back()->kick_user(this);
        _joined_channels.remove(_joined_channels.back());
    }
}

void
irc::User::make_current(Channel * current_channel)
{
    _joined_channels.remove(current_channel);
    _joined_channels.push_back(current_channel);
}

void
irc::User::remove_channel(Channel * to_remove_channel)
{
    _joined_channels.remove(to_remove_channel);
}
