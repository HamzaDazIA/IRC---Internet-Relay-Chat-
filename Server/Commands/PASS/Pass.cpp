#include "Pass.hpp"
#include "../../Server.hpp"
#include "../../../helper_functions/help.hpp"
#include <iostream>

Pass::Pass() : Command()
{
    this->upper = "PASS";
    this->lower = "pass";
}

void Pass::checkPASS(std::string pass, std::map<int, Client>::iterator &client)
{
    std::string pass_server = this->server->get_password();
    if (pass == pass_server)
    {
        client->second.setAuthenticated(true);
        return;
    }
    else
    {
        throw std::logic_error("wrong password. Disconnecting.");
    }
    
}

Pass::~Pass(){}

int  Pass::execute(std::vector<std::string> commandss , std::map<int, Client>::iterator &it_client)
{
    if (commandss.empty())
        return 1;

    if (commandss[0] == this->upper || commandss[0] == this->lower)
    {
        if (it_client->second.isAuthenticated())
        {
            std::string err = ERR_ALREADYREGISTERED(it_client->second.getNickname());
            if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
            {
                throw std::runtime_error("Error sending ERR_ALREADYREGISTERED to client.");
            }
            throw 462;
        }

        if (commandss.size() > 1)
        {
            try
            {
                if (commandss[1].empty())
                    throw std::logic_error("Empty password");
                this->checkPASS(commandss[1], it_client);
            }
            catch(const std::exception& e)
            {
                std::string err = ERR_PASSWDMISMATCH(it_client->second.getNickname());
                if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
                {
                    throw std::runtime_error("Error sending ERR_PASSWDMISMATCH to client.");
                }
                throw 464; // This causes client disconnect in handelClient
            }
        }
        else
        {
            std::string err = ERR_NEEDMOREPARAMS(it_client->second.getNickname(), commandss[0]);
            if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
            {
                throw std::runtime_error("Error sending ERR_NEEDMOREPARAMS to client.");
            }
            std::cout << "Send 461 to client " << it_client->first << ": " << commandss[0] << "\"" << err << "\"" << std::endl;
            return 0;
        }
    }
    else
    {
        return 1;
    }
    return 0 ;
}
