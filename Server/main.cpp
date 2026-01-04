#include "Server.hpp"
#include <cstdlib>


int main(int ac, char **av)
{
    // Validate command line arguments: must be exactly 3 (program name, port, password)
    if (ac != 3)
    {
        std::cerr << "Error : you need enter this forme -> \"/ircserv <port> <password> \" " << std::endl; 
        return 1;
    }
    // Validate port number: must be numeric and in valid range (0-65535)
    char *end;
    long long l_port  = strtoll(av[1], &end, 10);
    if (*end != '\0'|| l_port >= 65536 || l_port < 0)
    {
        std::cerr << "Error : The port number is not valide " << std::endl;
        return 1;
    }
    
    Server server(static_cast<int> (l_port), av[2]);
    
    // Start server and handle any fatal errors
    try
    {
        server.start_server();
    }
    catch(const std::exception& e)
    {
        // Catch all server errors (socket errors, system call failures, etc.)
        std::cerr << e.what() << std::endl;
    }
    

}