#include "Server.hpp"

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
    (irc::Server::_commands).insert(std::make_pair(QUIT, &irc::Server::cmd_quit));
    (irc::Server::_commands).insert(std::make_pair(LIST, &irc::Server::cmd_list));
    (irc::Server::_commands).insert(std::make_pair(NOTICE, &irc::Server::cmd_notice));
    (irc::Server::_commands).insert(std::make_pair(KICK, &irc::Server::cmd_kick));
}
