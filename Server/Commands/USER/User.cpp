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
    if (commandss[0] == this->upper || commandss[0] == this->lower)
    {
        std::string cmd = commandss[0];
        
        if (it_client->second.isAuthenticated() == true)
        {
            // USER command requires 4 parameters: <username> <hostname> <servername> <realname>
            if (commandss.size() < 5)
            {

                std::string nick = Help::nick_name(it_client->second.getNickname());
                std::string err = ERR_NEEDMOREPARAMS(nick, cmd);
                if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
                {
                        throw std::runtime_error("Error sending ERR_NEEDMOREPARAMS to client.");
                }
                throw 461;
            }
            else
            {
                if (it_client->second.isRegistered() == true)
                {
                    std::string nick = Help::nick_name(it_client->second.getNickname());
                    std::string err = ERR_ALREADYREGISTERED(nick);
                    if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
                    {
                        throw std::runtime_error("Error sending ERR_ALREADYREGISTERED to client.");
                    }
                    throw 462;
                }

                it_client->second.setUsername(Help::clear_stirng(commandss[1]));
                it_client->second.setRealname(Help::clear_stirng(commandss[4]));
                if (it_client->second.getNickname() != "" && it_client->second.getUsername() != "")
                {
                    it_client->second.setRegistered(true);
                    server->wellcomeMSG(it_client);
                }
            }
        }
        else
        {
            std::string nick = Help::nick_name(it_client->second.getNickname());
            std::string err = ERR_PASSWDMISMATCH(nick);
            if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
            {
                throw std::runtime_error("Error sending ERR_PASSWDMISMATCH to client.");
            }
            throw 464; 
        }
    }
    else
    {
        return 1;
    }
    return 0;
}