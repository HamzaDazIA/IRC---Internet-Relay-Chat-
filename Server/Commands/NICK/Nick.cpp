#include "Nick.hpp"
#include "../../Server.hpp"
#include "../../../helper_functions/help.hpp"
#include <iostream>

Nick::Nick() : Command()
{
    this->upper = "NICK";
    this->lower = "nick";
}

Nick::~Nick() {}

bool Nick::parsingNICK(std::string &nick)
{

    if (isdigit(nick[0]))
        return false;


    std::string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]\\`_^{|}-";


    for (size_t i = 0; i < nick.size(); i++)
    {
        if (allowed.find(nick[i]) == std::string::npos)
        {
            return false;
        }
    }

    return true;
}


int Nick::execute(std::vector<std::string> commandss, std::map<int, Client>::iterator &it_client)
{

    std::vector<std::string>::iterator it = commandss.begin();
    if (it[0] == this->upper || it[0] == this->lower) 
    {
        // Client must be authenticated (PASS) before setting nickname
        if (it_client->second.isAuthenticated() == true)
        {
            // Check if nickname parameter is provided
            if (commandss.size() < 2)
            {

                std::string nick = Help::nick_name(it_client->second.getNickname());
                std::string err = ERR_NONICKNAMEGIVEN(nick);
                if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
                {
                    throw std::runtime_error("Error sending ERR_NONICKNAMEGIVEN to client.");
                }
                throw 431;
            }
            else
            {

                bool status = this->parsingNICK(it[1]);
                if (status == false)
                {
                    // 432 ERR_ERRONEUSNICKNAME - Invalid nickname format
                    std::string err = ERR_ERRONEUSNICKNAME(it[1]);
                    if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
                    {
                        throw std::runtime_error("Error sending ERR_ERRONEUSNICKNAME to client.");
                    }
                    throw 432;
                }
                else
                {
                    try
                    {
                        // Try to register/update nickname
                        std::string oldNickname = it_client->second.getNickname();
                        server->set_newNICKNAMEs(it[1], oldNickname);
                        it_client->second.setNickname(it[1]);

                        if (it_client->second.getNickname() != "" && it_client->second.getUsername() != "")
                        {
                            if (it_client->second.isRegistered() == false)
                            {
                                it_client->second.setRegistered(true);
                                server->wellcomeMSG(it_client);
                            }
                        }
                    }
                    catch (int e)
                    {
                        if (e == 433)
                        {
                            std::string err = ERR_NICKNAMEINUSE(it[1]);
                            if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
                            {
                                throw std::runtime_error("Error sending ERR_NICKNAMEINUSE to client.");
                            }
                            throw 433;
                        }
                    }
                }
            }
        }
        else
        {
            std::string nick_name = Help::nick_name(it_client->second.getNickname());
            std::string err = ERR_PASSWDMISMATCH(nick_name);
            if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
            {
                throw std::runtime_error("Error sending ERR_PASSWDMISMATCH to client.");
            }
            throw 464; // Causes client disconnect
        }
            
    }
    else
    { 
        return (1);
    }
    return (0);
}