#include "Server.hpp"
// signal(), SIGINT
#include <csignal>

std::map<const std::string, irc::Server::command_function>    irc::Server::_commands;
int     g_signum = 0;

void sig_handle( int sig_number )
{
    g_signum = sig_number;
}

int main(int ac, char **av)
{
    irc::Server * main_server;

    if (ac != 3) {
        std::cout << "Please launch the program using: '" << av[0] << " <port> <password>'\n";
        return (0);
    }

    try
    {
        main_server = new irc::Server(av[1], av[2]);
    }
    catch (std::exception & e)
    {
        std::cerr << e.what();
        return (EXIT_FAILURE);
    }

    signal(SIGINT, sig_handle);

    main_server->loop();

    delete main_server;

    return (0);

}
