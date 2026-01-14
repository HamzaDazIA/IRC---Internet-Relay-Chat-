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
#include <arpa/inet.h>

//=== ERROR CODES AND MESSAGES ===
// Return value indicating failure for system calls
#define FAILED -1  

#define ENDL "\r\n"
// IRC Protocol Error Messages (RFC 1459)
// 464 ERR_PASSWDMISMATCH - Returned when password authentication fails
#define ERR_PASSWDMISMATCH(client) (":ft_irc.1337 464 " + (client) + " :Password incorrect." + ENDL)

// 421 ERR_UNKNOWNCOMMAND - Returned when client sends unrecognized command
#define ERR_UNKNOWNCOMMAND(client, command) (":ft_irc.1337 421 " + (client) + " " + (command) + " :Unknown command." + ENDL)
// 461 ERR_NEEDMOREPARAMS - Returned when command doesn't have enough parameters
#define ERR_NEEDMOREPARAMS(nick, comand) (":ft_irc.1337 461 " + nick + " " + comand + " :Not enough parameters." + ENDL)

// 462 ERR_ALREADYREGISTERED - Returned when client tries to re-register (PASS after authenticated)
#define ERR_ALREADYREGISTERED(nick) (":ft_irc.1337 462 " + nick + " :You may not reregister." + ENDL)

// 433 ERR_NICKNAMEINUSE - Returned when nickname is already taken by another client
#define ERR_NICKNAMEINUSE(nick) (":ft_irc.1337 433 " + nick + " " + nick + " :Nickname is already in use." + ENDL)

// 431 ERR_NONICKNAMEGIVEN - Returned when NICK command is sent without a nickname parameter
#define ERR_NONICKNAMEGIVEN(nick) (":ft_irc.1337 431 " + nick + " :No nickname given." + ENDL)

// 432 ERR_ERRONEUSNICKNAME - Returned when nickname contains invalid characters or format
#define ERR_ERRONEUSNICKNAME(nick) (":ft_irc.1337 432 " + nick + " :Erroneus nickname." + ENDL)


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

        // Send welcome messages (001-004) to successfully registered client
            void wellcomeMSG(std::map<int, Client>::iterator &it_client);
};

#endif