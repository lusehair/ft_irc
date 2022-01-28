#include "Server.hpp"

int main(int ac, char **av)
{
    irc::Server * main_server;

    if (ac != 3) {
        std::cout << "Please launch the program using: '" << av[0] << " <port> <password>'\n";
        return (0);
    }

    try
    {
        main_server = new irc::Server(av[1]);
    }
    catch (std::exception & e)
    {
        std::cerr << e.what();
        return (EXIT_FAILURE);
    }

    main_server->loop();

    delete main_server;

    return (0);

}
