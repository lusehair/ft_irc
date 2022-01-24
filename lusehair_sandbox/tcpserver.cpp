#include <cstring> 
#include <iostream> 
#include <string> 

#include <arpa/inet.h> 
#include <netdb.h> 
#include <sys/socket.h> 
#include <sys/types.h> 

#include <unistd.h> 

// need to specify a port on launching
int     main(int ac, char **av)
{
    if(ac != 2)
    {
        std::cerr << "You need to specify a port" << std::endl; 
        return (-1);
    }

    char *portNum = av[1]; 
    const unsigned int backLog = 12; // number of connections in the queue 

    addrinfo hints; 
    addrinfo *res; 
    addrinfo *p; 

    memset(&hints, 0, sizeof(hints)); 

    hints.ai_family = AF_UNSPEC; //IPV4 OR IPV6 in this case we take both
    hints.ai_socktype = SOCK_STREAM; // Type of socket, in this case SOCK_STREAM use for TCP/IP communication (bi/directionnal communication)
    hints.ai_flags = AI_PASSIVE; //Useful for bind and accept adresses for connection 
    hints.ai_protocol = 0; 

    int g_add_res = getaddrinfo(NULL, portNum, &hints, &res); 
    
    if(g_add_res)
    {
        std::cerr << gai_strerror(g_add_res) << std::endl;
        return (-1);
    }
    
    std::cout << "Ipv4 or Ipv6" << std::endl; 
    unsigned int num_of_addr = 0; 
    char ip_str[INET6_ADDRSTRLEN]; // we take the Ipv6 lenght is bigger than Ipv4 

    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr; 
        std::string ip_ver; 
        
        // Case if IPv4
        if(p->ai_family == AF_INET)
        {
            ip_ver = "IPv4"; 
            sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(p->ai_addr); 
            addr = &(ipv4->sin_addr);
            num_of_addr++;
        }
        // Case of Ipv6
        else 
        {
            ip_ver = "IPv6"; 
            sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(p->ai_addr); 
            addr = &(ipv6->sin6_addr);
            num_of_addr++; 
        }

        inet_ntop(p->ai_family, addr, ip_str, sizeof(ip_str));
        std::cout << "(" << num_of_addr << ")" << ip_ver << ":" << ip_str << std::endl; 
    }
        if(!num_of_addr)
        {
            std::cerr << "No host adress for using " << std::endl;
            return (-3);
        }

        // std::cout << "Wich host do you want bind() : "; 
        // unsigned int choice = 0; 
        // bool made_choice = false; 
        
        // do
        // {
        //     std::cin >> choice;
            
        //     if (choice > (num_of_addr + 1) || choice < 1)
        //     {
        //     made_choice = false; 
        //     std::cout << "Wrong Choice, please choose good number" << std::endl;
        //     }
        //     else
        //     {
        //         made_choice = true;
        //     }

        // }while (!made_choice);

        p = res; 

        int sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol); 

        if(sock_fd == -1)
        {
            std::cerr << "Error: socket cannot be created" << std::endl; 
            freeaddrinfo(res);
            return (-4);
        }

        int bind_r = bind(sock_fd, p->ai_addr, p->ai_addrlen); 
        
        if( bind_r == -1)
        {
            std::cerr << "Error: cannot bind this socket" << std::endl; 
            close(sock_fd); 
            freeaddrinfo(res);
            return(-5);
        }

        int listen_r = listen(sock_fd, backLog); 
        
        if (listen_r == -1)
        {
            std::cerr << "Error in listen socket" << std::endl; 
            close(sock_fd);
            freeaddrinfo(res);
            return (-6);
        }

        sockaddr_storage client_addr; 
        socklen_t client_addr_size = sizeof(client_addr); 
        std::string response = "From Lucas Server"; 

        while(1)
        {
            int new_fd = accept(sock_fd, (sockaddr *) &client_addr, &client_addr_size); 
            if(new_fd == -1)
            {
                std::cerr << "Somes error in accept" << std::endl; 
                continue; 
            }
            while(1)
            {
                std::cin >> response; 
                //const char *c = response.c_str()
                //puts("Hello there"); 
                //uint32_t responselength = htonl(response.size()); 
                //std::cout << response << std::endl; 
                 //send(new_fd, &responselength, sizeof(uint32_t), 0); 
                send(new_fd, response.c_str(), response.size(), 0); 
                if(!response.compare("exit"))
                    break;
                response.clear();

            }
            //ssize_t bytes_sent = send(new_fd, response.data(), response.length(), 0); 
            close(new_fd);
            
        }

        close(sock_fd); 
        freeaddrinfo(res);

        return(0); 
    



}