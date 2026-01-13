#include "User.hpp"
#include "../../Server.hpp"
#include "../../../helper_functions/help.hpp"

User::User()
{
    this->upper = "USER";
    this->lower = "user";
}
User::~User()
{

}

int User::execute(std::vector<std::string> commandss, std::map<int, Client>::iterator &it_client)
{
    std::vector<std::string>::iterator it = commandss.begin();
    if (it[0] == this->upper || it[0] == this->lower)
    {
        std::string cmd = it[0];
        // Client must be authenticated (PASS) before sending USER command
        if (it_client->second.isAuthenticated() == true)
        {
            // USER command requires 4 parameters: <username> <hostname> <servername> <realname>
            if (commandss.size() < 5)
            {
                // 461 ERR_NEEDMOREPARAMS - Not enough parameters
                std::string nick = Help::nick_name(it_client->second.getNickname());
                server->errorNEEDMOREPARAMS(it_client->first, nick, cmd);
                throw 461;
            }
            else
            {
                // Check if client already fully registered (both NICK and USER completed)
                if (it_client->second.isRegistered() == true)
                {
                    // 462 ERR_ALREADYREGISTERED - Can't send USER after registration complete
                    std::string nick = Help::nick_name(it_client->second.getNickname());
                    server->errorALREADYREGISTERED(it_client->first, nick);
                    std::cout << "Send 462 to client " << it_client->first << ": " << cmd << "\"" << ERR_ALREADYREGISTERED << "\"" << std::endl;
                    return 0;
                }
                else
                {
                    it_client->second.setUsername(it[1]);
                    it_client->second.setRealname(it[4]);
                    if (it_client->second.getNickname() != "" && it_client->second.getUsername() != "")
                    {
                        it_client->second.setRegistered(true);
                        // Send welcome messages upon successful registration
                        server->wellcomeMSG(it_client);
                    }
                }
            }
        }
        else
        {
            // 464 ERR_PASSWDMISMATCH - Client not authenticated, must send PASS first
            std::string nick_name = Help::nick_name(it_client->second.getNickname());
            server->errorPASSWDMISMATCH(it_client->first, nick_name);
            throw 464; 
        }
    }
    else
    {
        return 1;
    }
    return 0;
}