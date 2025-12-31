#include "Server.hpp"
#include <cstdlib>


int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Error : you need enter this forme -> \"/ircserv <port> <password> \" " << std::endl; 
        return 1;
    }
    char *end;
    long long l_port  = strtoll(av[1], &end, 10);
    if (*end != '\0'|| l_port >= 65536 || l_port < 0)
    {
        std::cerr << "Error : The port number is not valide " << std::endl;
        return 1;
    }
    Server server(static_cast<int> (l_port), av[2]);
    
    try
    {
        server.start_server();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    
}