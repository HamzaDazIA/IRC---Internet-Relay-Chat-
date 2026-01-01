#include "Server.hpp"

Server::Server(int port, std::string password)
{
    this->port = port;
    this->password = password;
}

Server::~Server(){}

int Server::get_port(void) const{
    return this->port;
}





std::string Server::get_password() const
{
    return this->password;
}

void Server::fd_to_NonBlocking(int &fd)
{
    int status = fcntl(fd, F_GETFL, 0);

    if (status == FAILDE )
    {
        throw std::runtime_error("Error fcntl: Failde Get file status flags.");
    }

    if (fcntl(fd, F_SETFL, status | O_NONBLOCK) == FAILDE)
    {
        throw std::runtime_error("Error fnctl: Failde set file  NON-BLOCK");
    }

}

//error part 
void Server::errorUNKNOWNCOMMAND(int fd, std::string &client, std::string commad)
{
    std::string err = "ft_irc.1337 (421) " + client + " " + commad + ERR_UNKNOWNCOMMAND;
    // ERROR LOGIC: send() return value not checked - can fail silently
    // Should check if send() returns -1 and handle the error
    send(fd, err.c_str(), err.length(), 0);
}

void Server::errorPASSWDMISMATCH(int fd, std::string nick_client)
{
    // ERROR LOGIC: Missing \r\n terminator - IRC protocol requires CRLF line endings
    // All IRC messages must end with \r\n according to RFC 1459
    std::string err = "ft_irc.1337 (464) " + nick_client + ERR_PASSWDMISMATCH;
    send(fd, err.c_str(), err.length(), 0);
}

void Server::errorNEEDMOREPARAMS(int fd, std::string nick_client, std::string comand)
{
    std::string err = "ft_irc.1337 (461) " + nick_client + " " + comand + ERR_NEEDMOREPARAMS;
    send(fd, err.c_str(), err.length(), 0);
}

void Server::errorALREADYREGISTERED(int fd, std::string nick_client)
{
    std::string err = "ft_irc.1337 (462) " + nick_client + ERR_ALREADYREGISTERED;
    send (fd, err.c_str(), err.length(), 0);
}

void Server::errorNICKNAMEINUSE(int fd, std::string nick_clint)
{
    std::string err = "ft_irc.1337 (433) " + nick_clint + ERR_NICKNAMEINUSE;
    send(fd, err.c_str(), err.length(), 0);
}

void Server::errorNONICKNAMEGIVEN(int fd, std::string nick_client)
{
    std::string err = "ft_irc.1337 (461) " + nick_client + ERR_NONICKNAMEGIVEN;
    send(fd, err.c_str(), err.length(), 0);
}

void Server::errorERRONEUSNICKNAME(int fd, std::string nick_client)
{
    std::string err = "ft_irc.1337 (432) " + nick_client + ERR_ERRONEUSNICKNAME;
    send(fd, err.c_str(), err.length(), 0);
}


// handel new CLient

void Server::checkPASS(std::string pass, std::map<int, Client>::iterator &client)
{
    std::string pass_server = this->get_password();
    if (pass == pass_server)
    {
        // ERROR LOGIC: Setting registered here is wrong - registration happens after NICK+USER
        // This function should only validate password, not set registration status
        // Authentication is set by the caller after this returns successfully
        client->second.setRegistered(true);
        return;
    }
    else
    {
        throw::std::logic_error("wrong password. Disconnecting.");
    }
    
}
void Server::handelNewClient(int &server_fd)
{

    struct sockaddr_in newClient = {};
    socklen_t clinetlen = sizeof(newClient);
    int newClient_fd = accept(server_fd, (sockaddr *)&newClient, &clinetlen);

    std::cout << "New client connected: " << newClient_fd << std::endl;
    if (newClient_fd == FAILDE)
    {
        throw std::runtime_error ("Error ACCEPT: Failde accept");
    }
    this->fd_to_NonBlocking(newClient_fd);

    struct pollfd client_poll = {};
    client_poll.fd = newClient_fd;
    client_poll.events = POLLIN;
    client_poll.revents = 0;

    this->fds.push_back(client_poll);
    Client obj;
    obj.setFd(newClient_fd);
    
    this->clients.insert(std::make_pair(newClient_fd, obj));

}

//set container
void Server::set_newNICKNAME(std::string nick)
{
    // ERROR LOGIC: No handling for nickname changes (user changing existing nickname)
    // If a user already has a nickname and changes it, the old nickname should be removed
    // from the set, but this function doesn't remove old nicknames - causing nickname leak
    if (this->nicknames.empty())
    {
        this->nicknames.insert(nick);
        return;
    }
    else
    {
        std::set<std::string>::iterator it = this->nicknames.find(nick);
        if (it == this->nicknames.end())
        {
            this->nicknames.insert(nick);
            return;
        }
        else
        {
            throw 433;
        }
    }
}

//handel client;

bool Server::parsingNICK(std::string &nick)
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

void Server::handelCommand(std::map<int, Client>::iterator &it_client , std::string commad)
{
    std::vector<std::string> commandss = Help::split_command(commad);
    std::string cmd = "CAP";
    std::vector<std::string>::iterator it = commandss.begin();
    if (commandss.size() > 0)
    {
        if (it[0] ==  cmd)
        {
            std::string nick = it_client->second.getNickname();
            if (nick.empty())
            {
                nick = "*"; //because we dont have name of client we replace to be "*"
            }
            this->errorUNKNOWNCOMMAND(it_client->first, nick, cmd);
    
            std::cout << "Send 421 to clinet " << it_client->first << ": " << cmd << std::endl;
            return ; 
        }
        cmd = "PASS"; // error logic pass in comper
        if (it[0] == cmd) // PASS comand
        {

            if (it_client->second.isAuthenticated() == true)//check is already Authenticated if already regester send erro 462.
            {
                std::string nick_name = Help::nick_name(it_client->second.getNickname());
                this->errorALREADYREGISTERED(it_client->first, nick_name);

                std::cout << "Send 462 to client " << it_client->first << ": " << cmd << "\"" << ERR_ALREADYREGISTERED << "\"" << std::endl;
                return;
            } 

            if (commandss.size() > 1)
            {
                try
                {
                    this->checkPASS(it[1], it_client);
                    it_client->second.setAuthenticated(true);

                }
                catch(const std::exception& e)
                {
                    std::string nick_name = Help::nick_name(it_client->second.getNickname());
                    
                    this->errorPASSWDMISMATCH(it_client->first, nick_name);
                    std::cout << "Send 464 to client " <<  it_client->first << " " << e.what();
                    throw 464;
                }
                
            }
            else
            {
                std::string nick_name = Help::nick_name(it_client->second.getNickname());
                this->errorNEEDMOREPARAMS(it_client->first, nick_name, cmd);

                std::cout << "Send 461 to client " << it_client->first << ": " << cmd << "\"" << ERR_NEEDMOREPARAMS << "\"" << std::endl;
                return;
            }
            
        }
        cmd = "NICK";
        if (it[0] == cmd) // NICK COMAND
        {
            if (it_client->second.isAuthenticated() == true )
            {
                if (commandss.size() < 2)
                {
                    std::string nick = Help::nick_name(it_client->second.getNickname());
                    this->errorNONICKNAMEGIVEN(it_client->first, nick);
                    throw 431;
                }
                else
                {
                    bool status = this->parsingNICK(it[1]);
                    if (status == false)
                    {
                        this->errorERRONEUSNICKNAME(it_client->first, it[1]);
                        throw 432;
                    }
                    else
                    {
                        try
                        {
                            this->set_newNICKNAME(it[1]);
                            it_client->second.setNickname(it[1]);

                            if (it_client->second.getFlage() == 2) {
                                it_client->second.setRegistered(true);
                                this->wellcomeMSG(it_client);
                            }
                        }
                        catch(int e)
                        {
                            this->errorNICKNAMEINUSE(it_client->first, it[1]);
                            throw 433;
                        }
                        // ERROR LOGIC: setFlage() called after try block - won't execute if exception thrown
                        // If nickname is already in use, flag never increments
                        // This should be inside the try block or the flag logic needs redesign
                        it_client->second.setFlage();
                    }
                }
            }
            else
            {
                std::string nick_name = Help::nick_name(it_client->second.getNickname());
                this->errorPASSWDMISMATCH(it_client->first, nick_name);
                throw 464;
            }
        }
        cmd = "USER";
        if (it[0] == "USER")
        {
            if (it_client->second.isAuthenticated() == true)
            {
                if (commandss.size() < 5)
                {
                    std::string nick = Help::nick_name(it_client->second.getNickname());
                    this->errorNEEDMOREPARAMS(it_client->first, nick, cmd);
                    throw 461;
                }
                else
                {
                    if (it_client->second.getFlage() == 2)
                    {
                        std::string nick = Help::nick_name(it_client->second.getNickname());
                        this->errorALREADYREGISTERED(it_client->first, nick);
                        std::cout << "Send 462 to client " << it_client->first << ": " << cmd << "\"" << ERR_ALREADYREGISTERED << "\"" << std::endl;
                        return ;
                    }
                    else
                    {
                        it_client->second.setUsername(it[1]);
                        it_client->second.setRealname(it[4]);
                        it_client->second.setFlage();
                        if (it_client->second.getFlage() == 2)
                        {
                            it_client->second.setRegistered(true);
                            this->wellcomeMSG(it_client);
                        }
                        
                    }
                }
            }
            else
            {
                std::string nick_name = Help::nick_name(it_client->second.getNickname());
                this->errorPASSWDMISMATCH(it_client->first, nick_name);
                throw 464;
            }
        }
    }

    
}

void Server::handelBuffer(std::map<int, Client>::iterator &it_client)
{
    std::string temp = it_client->second.getBuffer();
    size_t pos = temp.find('\n');
    while((pos != std::string::npos ))
    {
        std::string commad  = temp.substr(0, pos);

        commad = Help::trim(commad);
        it_client->second.erase_buffer(0 , pos + 1 );
        if (!commad.empty())
        {
            this->handelCommand(it_client, commad); // send command after split him to proccesing 
        }
        temp = it_client->second.getBuffer();
        pos = temp.find('\n');
    }
}

void Server::handelClient(struct pollfd &even_client)
{
    std::map<int, Client>::iterator  it = this->clients.find(even_client.fd);

    if (it == this->clients.end())
    {
        throw std::runtime_error("Error: Client not found in the clients map.");
    }
    else
    {
        char buffer[512];
        ssize_t bytesRead = recv(even_client.fd, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead < 0)
        {
            //EWOULDBLOCK or EAGAIN
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return ;
            }
            else
            {
                close (even_client.fd);
                this->clients.erase(it);
                for (std::vector<struct pollfd>::iterator i_itr = this->fds.begin() ; i_itr != this->fds.end() ; i_itr++)
                {
                    if (i_itr->fd == even_client.fd)
                    {
                        this->fds.erase(i_itr);
                        break;
                    }
                }

                throw std::runtime_error("Error recv: Failde to receive data from client.");
            }
        }
        else if (bytesRead == 0)
        {
            // Client disconnected
            this->clients.erase(it);
            for (std::vector<struct pollfd>::iterator p_it = this->fds.begin(); p_it != this->fds.end(); ++p_it)
            {
                if (p_it->fd == even_client.fd)
                {
                    this->fds.erase(p_it);
                    break;
                }
            }
            close(even_client.fd);
        }
        else
        {
            buffer[bytesRead] = '\0';
            std::string msg(buffer);
            std::string currentBuffer = it->second.getBuffer();
            currentBuffer += msg;
            it->second.setBuffer(currentBuffer);
            // Here you can add further processing of the client's message
        }
    }
    try
    {
        this->handelBuffer(it); // here we check if client send all command by tirminited /r /n
        
    }
    catch(int err)
    {
        if (err == 464)
        {
            this->clients.erase(it);
            for (std::vector<struct pollfd>::iterator p_it = this->fds.begin(); p_it != this->fds.end(); ++p_it)
            {
                if (p_it->fd == even_client.fd)
                {
                    this->fds.erase(p_it);
                    break;
                }
            }
            close(even_client.fd);
        }
        if (err == 431)
            std::cout << "Send 431 to client " << even_client.fd << ERR_NONICKNAMEGIVEN;
        if (err == 432)
            std::cout << "Send 432 to client " << even_client.fd << ERR_ERRONEUSNICKNAME;
        if (err == 433)
            std::cout << "Send 433 to client " << even_client.fd << ERR_NICKNAMEINUSE;
    }
    
}



void Server::start_server(void)
{
    std::cout << "Starting server on port " << this->port << "..." << std::endl;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == FAILDE)
    {
        throw std::runtime_error("Error Socket : failde create socket.");
    }

    this->serverNAME = "ft_irc.1337";
    this->serverVERSION = "0.1";
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt , sizeof(*(get_option_sockopt())));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET; // type of IP is IPv4 
    addr.sin_port = htons(this->port); // set port number we use htons function converts the unsigned short integer hostshort from host byte order to network byte order.
    addr.sin_addr.s_addr = INADDR_ANY; // Address to accept any incoming messages.

    if (bind(server_fd, (sockaddr *) (&addr), sizeof(addr)) == FAILDE )
    {
        close(server_fd);
        throw std::runtime_error("Error bind: Failde bind socket server.");
    }
    
    std::cout << "Socket server binded to port " << this->port << std::endl;
    if (listen(server_fd, SOMAXCONN) == FAILDE) // SOMAXCONN. This tells the OS: "Give me the maximum possible queue size allowed on this specific machine
    {
        close(server_fd);
        throw std::runtime_error("Error listen: Failde listing socket server.");
    } 

    std::cout << "Server is listening on port " << this->port << std::endl;
    struct pollfd fd_info = {};
    fd_info.fd  = server_fd;
    fd_info.events = POLLIN;
    fd_info.revents = 0;
    this->fds.push_back(fd_info);
    int poll_flag = 0;
    while(true)
    {
        // ERROR LOGIC: Using FAILDE (-1) as timeout is incorrect
        // poll() expects timeout in milliseconds: -1 means infinite wait (correct)
        // but FAILDE might be confusing - should use -1 directly or create POLL_INFINITE macro
        poll_flag = poll(this->fds.data(), this->fds.size(), FAILDE);

        if (poll_flag == FAILDE)
        {
            // ERROR LOGIC: Empty error handling - should throw exception or handle error
            // If poll fails, program continues with undefined behavior
            //here trow expation and in case we have fds of client and server we need remove him ;

        }
        for (size_t i = 0 ; i < this->fds.size(); i++)
        {
            if (this->fds[i].revents & POLLIN)
            {

                if (this->fds[i].fd == server_fd)
                {
                    this->handelNewClient(server_fd);

                }
                else
                {
                    try
                    {
                        this->handelClient(fds[i]);
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << std::endl;
                    }
                    // ERROR LOGIC: Decrementing i after potential removal still problematic
                    // The check 'if (i > 0)' doesn't fix the core issue:
                    // - If handelClient removes the current fd from vector, next element shifts to position i
                    // - Decrementing i makes us check position i-1 again in next iteration
                    // - This means we skip checking the element that shifted to position i
                    // Better solution: iterate backwards or track if removal occurred
                    if (i > 0)
                        i--; // to avoid skiping next client in fds vector
                }
            }
        }
    }

}

void Server::wellcomeMSG(std::map<int, Client>::iterator &it_client)
{
    Client &client = it_client->second;
    int fd = it_client->first;
    std::string nick = client.getNickname();
    std::string user = client.getUsername();
    std::string host = "localhost";

    std::string rpl1 = ":" + this->serverNAME + " 001 " + nick + " :Welcome to the ft_irc Network " + nick + "!" + user + "@" + host + "\r\n";
    send(fd, rpl1.c_str(), rpl1.length(), 0);

    std::string rpl2 = ":" + this->serverNAME + " 002 " + nick + " :Your host is " + this->serverNAME + ", running version " + this->serverVERSION + "\r\n";
    send(fd, rpl2.c_str(), rpl2.length(), 0);

    std::string rpl3 = ":" + this->serverNAME + " 003 " + nick + " :This server was created Dec 21 2025\r\n";
    send(fd, rpl3.c_str(), rpl3.length(), 0);

    std::string rpl4 = ":" + this->serverNAME + " 004 " + nick + " " + this->serverNAME + " " + this->serverVERSION + " io itkol\r\n";
    send(fd, rpl4.c_str(), rpl4.length(), 0);
}