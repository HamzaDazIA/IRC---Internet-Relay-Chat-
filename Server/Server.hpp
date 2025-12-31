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

//error Server
#define FAILDE -1  
#define ERR_PASSWDMISMATCH " :Password incorrect.";
#define ERR_UNKNOWNCOMMAND " :Unknown command."
#define ERR_NEEDMOREPARAMS " :Not enough parameters"
#define ERR_ALREADYREGISTERED " :You may not reregister"
#define ERR_NICKNAMEINUSE " :Nickname is already in use"
#define ERR_NONICKNAMEGIVEN " :No nickname given"
#define ERR_ERRONEUSNICKNAME " :Erroneus nickname"
class Server
{
    private:
        int port ;
        std::string password;
        int *option_sockopt; // this ingeter of set_sockopt if ==1 the REUSEADDR is ON if ==0 is OFF
        std::vector<struct pollfd> fds;
        std::map<int, Client> clients;
        std::set<std::string> nicknames;
        std::string serverNAME;
        std::string serverVERSION;
    public :
        Server(int port, std::string password);
        ~Server();
        int get_port(void) const ;
        std::string  get_password(void) const;
        void start_server(void);

        //set Containers
            void set_newNICKNAME(std::string nickname);
        //opt set_sockopt 
            int * get_option_sockopt(void) const;
            void set_option_sockopt(int opt);      
            void fd_to_NonBlocking(int &fd);

        //handel clients
            void handelNewClient(int &server_fd);
            void handelClient(struct pollfd &even_client);
            void handelBuffer(std::map<int, Client>::iterator &it);
            void handelCommand(std::map<int, Client>::iterator &it_client , std::string commad);
            void checkPASS(std::string pass,  std::map<int , Client>::iterator& Client);
            //parsing
            bool parsingNICK(std::string &nick);
        
        // error part 
            void errorUNKNOWNCOMMAND(int fd, std::string &nick_client, std::string commad);
            void errorPASSWDMISMATCH(int fd, std::string nick_client);
            void errorNEEDMOREPARAMS(int fd, std::string nick_client, std::string comand);
            void errorALREADYREGISTERED(int fd, std::string nick_client);
            void errorNICKNAMEINUSE(int fd, std::string nick_clint);
            void errorNONICKNAMEGIVEN(int fd, std::string nick_client);
            void errorERRONEUSNICKNAME(int fd, std::string nick_client);
            void wellcomeMSG(std::map<int, Client>::iterator &it_client);
};

#endif