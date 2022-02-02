# ft_irc
ft_irc for 42 school


https://lusehair.notion.site/ft_irc-0325730121e4468babeeb6a98c32134e 


     switch (iterator->ai_family)
                                {
                                    case AF_INET:
                                    {
                                        // use of reinterpret_cast preferred to C style cast
                                        sockaddr_in *sin = reinterpret_cast<sockaddr_in*>(iterator);
                                        inet_ntop(AF_INET, &sin->sin_addr, translated_ip, INET6_ADDRSTRLEN);
                                        break;
                                    }
                                    case AF_INET6:
                                    {
                                        sockaddr_in6 *sin = reinterpret_cast<sockaddr_in6*>(iterator);
                                        // inet_ntoa should be considered deprecated
                                        inet_ntop(AF_INET6, &sin->sin6_addr, translated_ip, INET6_ADDRSTRLEN);
                                        break;
                                    }
                                    default:
                                        abort();
                                
                                }