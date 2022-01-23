#include <string>
#include <iostream>
#include <sys/socket.h>

int
main(
    int argc
    , char **argv )
{
    if (argc != 3) {
        std::cout << "Please launch the program using: './" << argv[0] << " <port> <password>'\n";
        return (0);
    }

    std::string port_number = argv[1];

}