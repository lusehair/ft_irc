#include <string>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>

#include <poll.h>

#include <fcntl.h>

int
main(
    int argc
    , char **argv )
{

    if (argc != 3) {
        std::cout << "Please launch the program using: '" << argv[0] << " <port> <password>'\n";
        return (0);
    }

    const char * port_number = argv[1];
    unsigned int address_count = 0;
    char ip_str[INET6_ADDRSTRLEN];

    struct addrinfo *potential_addresses;
    struct addrinfo *iterator;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    int getaddrinfo_ret;
    int endpoint;
    int clients[2];
    int i = 0;

    getaddrinfo_ret = getaddrinfo(NULL, port_number, &hints, &potential_addresses);
    if (getaddrinfo_ret != 0) {
        std::cerr << gai_strerror(getaddrinfo_ret);
        return (-2);
    }

    std::cout << "Detecting addresses\n";

    for (iterator = potential_addresses; iterator != NULL; iterator = iterator->ai_next) {
        void * address;
        std::string ip_version;

        if (iterator->ai_family == AF_INET) {
            ip_version = "IPv4";
            address = &((reinterpret_cast<sockaddr_in *>(iterator->ai_addr))->sin_addr);
            ++address_count;
        } else {
            ip_version = "IPv6";
            address = &((reinterpret_cast<sockaddr_in6 *>(iterator->ai_addr))->sin6_addr);
            ++address_count;
        }
        inet_ntop(iterator->ai_family, address, ip_str, sizeof(ip_str));
        std::cout << "(" << address_count << ") " << ip_version << " : " << ip_str << '\n';
    }

    if (!address_count) {
        std::cerr << "Found no host address to use\n";
        return (-3);
    }

    iterator = potential_addresses;

    endpoint = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);
    if (endpoint == -1) {
        std::cerr << "Error while creating socket\n";
        freeaddrinfo(potential_addresses);
        return (-4);
    }
fcntl(endpoint, F_SETFL, O_NONBLOCK);

    if (bind(endpoint, iterator->ai_addr, iterator->ai_addrlen) == -1) {
        std::cerr << "Error while binding socket\n";
        close(endpoint);
        freeaddrinfo(potential_addresses);
        return (-5);
    }

    if (listen(endpoint, 3) == -1) {
        std::cerr << "Error while listening on socket\n";
        close(endpoint);
        freeaddrinfo(potential_addresses);
        return (-6);
    }

    sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    std::string response = "Hello World";
    std::string reply(15, ' ');

    while (1) {
        if (i < 2) {
            clients[i] = accept(endpoint, (sockaddr *) &client_addr, &client_addr_size);
            if (clients[i] != -1) {
                ++i;
            }
        }

        // std::cin >> response;
        if (strncmp(response.data(), "exit", 4)) {
            break ;
        }
        recv(endpoint, &reply.front(), reply.size(), 0);
        std::cout << reply << '\n';

        // ssize_t bytes_sent =
        // for (int j = 0; j < i; ++j) {
        //     send(clients[j], response.data(), response.length(), 0);
        // } 
        
        response.clear();
        // std::cout << response;
        close(clients[0]);
        close(clients[1]);
        // close(clients[i]);
    }

    close(endpoint);
    freeaddrinfo(potential_addresses);

    return (0);

}
