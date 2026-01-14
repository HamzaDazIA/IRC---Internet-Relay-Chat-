#ifndef Server_HPP
#define Server_HPP

#include <sys/socket.h>
#include <iostream>
#include <fcntl.h>
#include <map>
#include <vector>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include "../Client/Client.hpp"
#include "../helper_functions/help.hpp"
#include <sstream>
#include <set>
#include <cerrno>
#include <cstdlib>
#include <signal.h>

//=== ERROR CODES AND MESSAGES ===
// Return value indicating failure for system calls
#define FAILED -1  

// IRC Protocol Error Messages (RFC 1459)
// 464 ERR_PASSWDMISMATCH - Returned when password authentication fails
#define ERR_PASSWDMISMATCH " :Password incorrect."

// 421 ERR_UNKNOWNCOMMAND - Returned when client sends unrecognized command
#define ERR_UNKNOWNCOMMAND(client, command) (":ft_irc.1337 421 " + (client) + " " + (command) + " :Unknown command.")
// 461 ERR_NEEDMOREPARAMS - Returned when command doesn't have enough parameters
#define ERR_NEEDMOREPARAMS(nick, comand) (" :Not enough parameters."

// 462 ERR_ALREADYREGISTERED - Returned when client tries to re-register (PASS after authenticated)
#define ERR_ALREADYREGISTERED " :You may not reregister."

// 433 ERR_NICKNAMEINUSE - Returned when nickname is already taken by another client
#define ERR_NICKNAMEINUSE " :Nickname is already in use."

// 431 ERR_NONICKNAMEGIVEN - Returned when NICK command is sent without a nickname parameter
#define ERR_NONICKNAMEGIVEN " :No nickname given."

// 432 ERR_ERRONEUSNICKNAME - Returned when nickname contains invalid characters or format
#define ERR_ERRONEUSNICKNAME " :Erroneus nickname."


class Server
{
    private:
        int port ;
        std::string password;
        std::vector<struct pollfd> fds;
        std::map<int, Client> clients;
        std::set<std::string> nicknames;
        std::string serverNAME;
        std::string serverVERSION;
    public :
        Server();
        Server(int port, std::string password);
        ~Server();
        int get_port(void) const ;
        std::string  get_password(void) const;
        void start_server(void);

        //set Containers
            void set_newNICKNAMEs(std::string nickname , std::string old);

            void fd_to_NonBlocking(int &fd);

        //handel clients
            void handelNewClient(int &server_fd);
            void handelClient(struct pollfd &even_client);
            void handelBuffer(std::map<int, Client>::iterator &it);
            void handelCommand(std::map<int, Client>::iterator &it_client , std::string commad);
        
        //=== ERROR HANDLING FUNCTIONS ===
            // Send 421 ERR_UNKNOWNCOMMAND when client uses unrecognized command
            void errorUNKNOWNCOMMAND(int fd, std::string nick_client, std::string commad);
            
            // Send 464 ERR_PASSWDMISMATCH when password is incorrect
            void errorPASSWDMISMATCH(int fd, std::string nick_client);
            
            // Send 461 ERR_NEEDMOREPARAMS when command lacks required parameters
            void errorNEEDMOREPARAMS(int fd, std::string nick_client, std::string comand);
            
            // Send 462 ERR_ALREADYREGISTERED when client tries to re-authenticate
            void errorALREADYREGISTERED(int fd, std::string nick_client);
            
            // Send 433 ERR_NICKNAMEINUSE when requested nickname is taken
            void errorNICKNAMEINUSE(int fd, std::string nick_clint);
            
            // Send 431 ERR_NONICKNAMEGIVEN when NICK command has no parameter
            void errorNONICKNAMEGIVEN(int fd, std::string nick_client);
            
            // Send 432 ERR_ERRONEUSNICKNAME when nickname has invalid format
            void errorERRONEUSNICKNAME(int fd, std::string nick_client);
            
            // Send welcome messages (001-004) to successfully registered client
            void wellcomeMSG(std::map<int, Client>::iterator &it_client);
};

#endif