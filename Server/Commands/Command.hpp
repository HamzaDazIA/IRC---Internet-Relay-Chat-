#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
#include <map>
#include "../Server.hpp"


class Client;
class Server;

class Command
{
    protected :
        std::string lower;
        std::string upper;
        Server* server;
    public:
        Command();
        virtual ~Command();
        void setServer(Server* srv);
        virtual int  execute(std::vector<std::string> commands , std::map<int, Client>::iterator &it_client) = 0;
};

#endif 
