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
        // Password correct - mark client as authenticated
        client->second.setAuthenticated(true);
        return;
    }
    else
    {
        // Password wrong - throw exception to trigger 464 error and disconnect
        throw::std::logic_error("wrong password. Disconnecting.");
    }
    
}

Pass::~Pass(){}

int  Pass::execute(std::vector<std::string> commandss , std::map<int, Client>::iterator &it_client)
{
    std::vector<std::string>::iterator it = commandss.begin();
    if (it[0] == this->upper || it[0] == this->lower)
    {
        // Check if client is already authenticated - can't send PASS twice
        // RFC 1459: ERR_ALREADYREGISTERED (462) if already registered
        if (it_client->second.isAuthenticated() == true)
        {
            std::string nick_name = Help::nick_name(it_client->second.getNickname());
            server->errorALREADYREGISTERED(it_client->first, nick_name);
            std::cout << "Send 462 to client " << it_client->first << ": " << it[0] << "\"" << ERR_ALREADYREGISTERED << "\"" << std::endl;
            return 0;
        }

        // Check if password parameter is provided
        if (commandss.size() > 1)
        {
            try
            {
                // Validate password - throws exception if wrong
                this->checkPASS(it[1], it_client);
                it_client->second.setAuthenticated(true);
            }
            catch(const std::exception& e)
            {
                // 464 ERR_PASSWDMISMATCH - Wrong password, disconnect client
                std::string nick_name = Help::nick_name(it_client->second.getNickname());
                
                server->errorPASSWDMISMATCH(it_client->first, nick_name);
                std::cout << "Send 464 to client " <<  it_client->first << " " << e.what();
                throw 464; // This causes client disconnect in handelClient
            }
            
        }
        else
        {
            // 461 ERR_NEEDMOREPARAMS - PASS command requires password parameter
            std::string nick_name = Help::nick_name(it_client->second.getNickname());
            server->errorNEEDMOREPARAMS(it_client->first, nick_name, it[0]);
            std::cout << "Send 461 to client " << it_client->first << ": " << it[0] << "\"" << ERR_NEEDMOREPARAMS << "\"" << std::endl;
            return 0;
        }
    }
    else
    {
        return 1;
    }
    return 0 ;
}
