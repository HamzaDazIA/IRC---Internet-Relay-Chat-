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


//error Server
#define FAILDE -1  
#define ERR_PASSWDMISMATCH " :Password incorrect.";
#define ERR_UNKNOWNCOMMAND " :Unknown command."
#define ERR_NEEDMOREPARAMS " :Not enough parameters"
class Server
{
    private:
        int port ;
        std::string password;
        int *option_sockopt; // this ingeter of set_sockopt if ==1 the REUSEADDR is ON if ==0 is OFF
        std::vector<struct pollfd> fds;
        std::map<int, Client> clients;
        
    public :
        Server(int port, std::string password);
        ~Server();
        int get_port(void) const ;
        std::string  get_password(void) const;
        void start_server(void);
        
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

            //error part 
            void errorUNKNOWNCOMMAND(int fd, std::string &nick_client, std::string commad);
            void errorPASSWDMISMATCH(int fd, std::string nick_client);
            void errorNEEDMOREPARAMS(int fd, std::string nick_client, std::string comand);
};

#endif