#include "Server.hpp"

void
irc::Server::remove_killed_users()
{
    std::vector<User *>::iterator      target_to_kill = _to_kill_users.begin();
    std::vector<User *>::iterator      tmp_target_to_kill;

    while(target_to_kill != _to_kill_users.end()) 
    {
        close((*target_to_kill)->_own_socket);
        _opened_sockets.erase((*target_to_kill)->_own_socket);
        _pending_sends.erase((*target_to_kill)->_own_socket);
        FD_CLR((*target_to_kill)->_own_socket, &_client_sockets);
        _connected_users.erase((*target_to_kill)->_nickname);
        delete *target_to_kill;
        tmp_target_to_kill = target_to_kill;
        ++target_to_kill;
    }
    _to_kill_users.clear();
}
