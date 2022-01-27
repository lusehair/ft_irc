#include <string>
#include <map>
#include <utility>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>

#include <poll.h>

#include <fcntl.h>

#include <sys/time.h>

int main(int ac, char **av)
{

    long                endpoint;
    char *              port_number;
    struct fd_set       endpoint_set;
    struct timeval      time_before_timeout;
    int                 select_ret;
    char                buffer[512];

    size_t              address_count;
    char                ip_str[INET6_ADDRSTRLEN];

    int                 getaddrinfo_ret;
    struct fd_set       working_endpoint_set;
    int                 socket_count;
    int                 opt;
    int                 i;
    long                current_endpoint;
    long                new_client_endpoint;

    struct addrinfo *   potential_addresses;
    struct addrinfo *   iterator;
    struct addrinfo     hints;
    struct addrinfo     client_addrinfo;
    std::map<long, struct addrinfo> client_storage;

    if (ac != 3) {
        std::cout << "Please launch the program using: '" << av[0] << " <port> <password>'\n";
        return (0);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    port_number = av[1];
    address_count = 0;
    FD_ZERO(&endpoint_set);

    std::cout << "Detecting addresses\n";

    getaddrinfo_ret = getaddrinfo(NULL, port_number, &hints, &potential_addresses);
    if (getaddrinfo_ret != 0) {
        std::cerr << gai_strerror(getaddrinfo_ret);
        return (-2);
    }

    for (iterator = potential_addresses; iterator != NULL; iterator = iterator->ai_next) {
        endpoint = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);

        if (endpoint == -1) {
            std::cout << "Invalid endpoint\n";
            continue ;
        }

        opt = 1;
        if (setsockopt(endpoint, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            close(endpoint);
            std::cout << "setsockopt fail\n";
            continue ;
        }

        // if (setsockopt(endpoint, getprotobyname("TCP")->p_proto, 0, NULL, 0)) {
        //     close(endpoint);
        //     std::cout << "setsockopt fail\n";
        //     continue ;
        // }

        if (bind(endpoint, iterator->ai_addr, iterator->ai_addrlen) == 0) {
            std::cout << "Successful bind to ";
            // if (iterator->ai_family != AF_INET) {
            //     std::cout << "IPv6 : ";
            // } else {
            //     std::cout << "IPv4 : ";
            // }
            if (getnameinfo(iterator->ai_addr, iterator->ai_addrlen, ip_str, sizeof(ip_str), NULL, 0, 0) != 0) {
                std::cout << "'could not translate the socket address'" ;
            } else {
                std::cout << ip_str << '\n';
            }
            break ;
        }

        close(endpoint);

    }

    if (iterator == NULL) {
        std::cerr << "Could not bind\n";
        return (EXIT_FAILURE);
    }

    freeaddrinfo(potential_addresses);

    if (listen(endpoint, 8) == -1) {
        std::cerr << "Error while listening on socket\n";
        close(endpoint);
        return (-6);
    }

    // fcntl(endpoint, F_SETFL, O_NONBLOCK);

    FD_SET(endpoint, &endpoint_set);

    memset(buffer, 0, sizeof(buffer));

    while (true) {
        time_before_timeout.tv_sec = 20;
        time_before_timeout.tv_usec = 0;

        new_client_endpoint = accept(endpoint, client_addrinfo.ai_addr, &client_addrinfo.ai_addrlen);
        if (new_client_endpoint != -1) {
            FD_SET(new_client_endpoint, &endpoint_set);
            client_storage.insert(std::make_pair(new_client_endpoint, client_addrinfo));

            std::cout << "Accepted conection from : ";
            if (getnameinfo(client_addrinfo.ai_addr, client_addrinfo.ai_addrlen, ip_str, sizeof(ip_str), NULL, 0, NI_NUMERICHOST) != 0) {
                std::cout << "'could not translate the socket address'\n" ;
            } else {
                std::cout << ip_str << '\n';
            }

            fcntl(new_client_endpoint, F_SETFL, O_NONBLOCK);

            send(new_client_endpoint, "Sheeeeeeesh\n", strlen("Sheeeeeeesh\n"), MSG_DONTWAIT);

            // while (true) {
            // 

                memcpy(&working_endpoint_set, &endpoint_set, sizeof(endpoint_set));
                std::cout << "Waiting on select\n";
                select_ret = select(client_storage.rbegin()->first + 1, &working_endpoint_set, NULL, NULL, &time_before_timeout);

                if (select_ret == -1) {
                    std::cerr << "Error in select()\n";
                } else if (select_ret == 0) {
                    std::cout << "Nothing received in last 20 seconds\n";
                    break ;
                } else {

                    for (i = 0; i < client_storage.rbegin()->first + 1 && select_ret > 0; ++i) {
                        if (FD_ISSET(i, &working_endpoint_set)) {
                            --select_ret;

                            if (i == endpoint) {
                                std::cout << "connections to accept\n";
                            } else {
                                // for (;;) {
                                    // send(i, "Sheeeeeeesh\n", strlen("Sheeeeeeesh\n"), MSG_DONTWAIT);
                                // }
                                recv(i, buffer, sizeof(buffer), 0);
                                std::cout << buffer;
                            }
                        }
                    }
                    
                }
        //         for (i = 0; i < socket_count; ++i) {
        //             current_endpoint = endpoint_set.fds_bits[i];

        //             if (current_endpoint == endpoint) {
        // // std::cout << "toto\n";
        //                 // Accept connection;
        //                 new_client_endpoint = accept(endpoint, NULL, NULL);
        //                 FD_SET(new_client_endpoint, &endpoint_set);
        //                 send(new_client_endpoint, "Sheeeeeeesh\n", 12, 0);
        //             } else {
        //                 // New message;;
        //             }
        //         }
        //         break ;
            // }
            
        } else {
            std::cerr << "Error while accepting connection\n";
        }

        // Close everything
        client_storage.erase(new_client_endpoint);
        FD_CLR(new_client_endpoint, &endpoint_set);
        close(new_client_endpoint);

    }

    close(endpoint);

    return (0);

}
