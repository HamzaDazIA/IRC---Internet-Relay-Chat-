#include "Server.hpp"
#include "Commands/NICK/Nick.hpp"
#include "Commands/USER/User.hpp"
#include "Commands/PASS/Pass.hpp"

Server::Server(){}
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

    if (status == FAILED )
    {
        throw std::runtime_error("Error fcntl: FAILED Get file status flags.");
    }

    if (fcntl(fd, F_SETFL, status | O_NONBLOCK) == FAILED)
    {
        throw std::runtime_error("Error fnctl: FAILED set file  NON-BLOCK");
    }

}

void Server::set_newNICKNAMEs(std::string nick , std::string old)
{

    if (this->nicknames.find(nick) != this->nicknames.end())
    {
        throw 433; // ERR_NICKNAMEINUSE - nickname collision
    }
    if (!old.empty())
    {
        this->nicknames.erase(old);
    }

    this->nicknames.insert(nick);
    
}



void Server::handelNewClient(int &server_fd)
{

    struct sockaddr_in newClient = {};
    socklen_t clinetlen = sizeof(newClient);
    int newClient_fd = accept(server_fd, (sockaddr *)&newClient, &clinetlen);

    std::cout << "New client connected: " << newClient_fd << std::endl;
    if (newClient_fd == FAILED)
    {
        throw std::runtime_error ("Error ACCEPT: FAILED accept");
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

void Server::handelCommand(std::map<int, Client>::iterator &it_client , std::string commad)
{
    std::vector<std::string> commandss = Help::split_command(commad);
    std::string cmd = "CAP";
    std::vector<std::string>::iterator it = commandss.begin();
    Nick nick;
    User user;
    Pass pass;
    
    nick.setServer(this);
    user.setServer(this);
    pass.setServer(this);
    
    if (commandss.size() > 0)
    {
        if (it[0] ==  cmd)
        {
            std::string nick = it_client->second.getNickname();
            if (nick.empty())
            {
                nick = "*"; //because we dont have name of client we replace to be "*"
            }
            std::string err = ERR_UNKNOWNCOMMAND(it_client->second.getNickname(), cmd);
            if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
            {
                throw std::runtime_error("Error sending ERR_UNKNOWNCOMMAND to client.");
            }

            std::cout << "Send 421 to clinet " << it_client->first << ": " << cmd << std::endl;
            return ; 
        }
        if (nick.execute(commandss, it_client) == 0)
            return;
        else if (pass.execute(commandss, it_client) == 0)
            return ;
        else if (user.execute(commandss, it_client) == 0)
            return;
        else
        {
            std::string err = ERR_UNKNOWNCOMMAND(it_client->second.getNickname(), it[0]);
            if (send(it_client->first, err.c_str(), err.length(), 0) < 0)
            {
                throw std::runtime_error("Error sending ERR_UNKNOWNCOMMAND to client.");
            }
            std::cout << "Send 421 to clinet " << it_client->first << ": " << it[0] << std::endl;
            return ;
        }
    }  

    
}

void Server::handelBuffer(std::map<int, Client>::iterator &it_client)
{
    std::string temp = it_client->second.getBuffer();
    size_t pos = temp.find('\n');
    while((pos != std::string::npos))
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
            // Handle non-blocking socket errors
            // EWOULDBLOCK or EAGAIN means no data available yet (not an error for non-blocking)
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return ;
            }
            else
            {
    
               std::cout << "Error receiving data from client " << even_client.fd << std::endl;
               throw -1;
            }
        }
        else if (bytesRead == 0)
        {
            // Client disconnected gracefully (closed connection)

           std::cout << "Client " << even_client.fd << " disconnected." << std::endl;
           throw -1;
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
        // Process complete commands from buffer (terminated by \r\n)
        this->handelBuffer(it);
        
    }
    catch(int err) // Catch integer exceptions thrown by command handlers
    {
        // 464 ERR_PASSWDMISMATCH - Wrong password, disconnect client
        if (err == 464)
        {
           std::cout << "Disconnecting client " << even_client.fd << " due to PASSWDMISMATCH." << std::endl;
              throw -1;
        }

        if (err == 431)
            std::cout << "Send message to client " << even_client.fd << "->" << ERR_NONICKNAMEGIVEN(it->second.getNickname());

        if (err == 432)
            std::cout << "Send 432 to client " << even_client.fd << ERR_ERRONEUSNICKNAME(it->second.getNickname());

        if (err == 433)
            std::cout << "Send 433 to client " << even_client.fd << ERR_NICKNAMEINUSE(it->second.getNickname());
        if (err == 462)
            std::cout << "Send 462 to client " << even_client.fd << ERR_ALREADYREGISTERED(it->second.getNickname());
    }

    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        throw -1;
    }
    
}


void Server::start_server(void)
{
    std::cout << "Starting server on port " << this->port << "..." << std::endl;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == FAILED)
    {
        throw std::runtime_error("Error Socket : FAILED create socket.");
    }

    this->serverNAME = "ft_irc.1337";
    this->serverVERSION = "0.1";
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt , sizeof(opt));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET; // type of IP is IPv4 
    addr.sin_port = htons(this->port); // set port number we use htons function converts the unsigned short integer hostshort from host byte order to network byte order.
    addr.sin_addr.s_addr = INADDR_ANY; // Address to accept any incoming messages.
    //inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(server_fd, (sockaddr *) (&addr), sizeof(addr)) == FAILED )
    {
        close(server_fd);
        throw std::runtime_error("Error bind: FAILED bind socket server.");
    }
    
    std::cout << "Socket server bound to port " << this->port << std::endl;
    if (listen(server_fd, SOMAXCONN) == FAILED) // SOMAXCONN. This tells the OS: "Give me the maximum possible queue size allowed on this specific machine
    {
        close(server_fd);
        throw std::runtime_error("Error listen: FAILED listing socket server.");
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

        poll_flag = poll(this->fds.data(), this->fds.size(), -1);

        if (poll_flag == FAILED)
        {
            throw std::runtime_error("Error poll: FAILED polling fds.");
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

                    catch(int err)
                    {
                        if (err == -1)
                        {
                            std::map<int, Client>::iterator it = this->clients.find(this->fds[i].fd);
                            if (it != this->clients.end())
                            {
                                std::string nick = it->second.getNickname();
                                std::set<std::string>::iterator it_nick  = this->nicknames.find(nick);
                                if (it_nick != this->nicknames.end())
                                {
                                    this->nicknames.erase(it_nick);
                                }
                            }
                            this->clients.erase(it);
                            for(std::vector<struct pollfd>::iterator p_it = this->fds.begin(); p_it != this->fds.end(); ++p_it)
                            {
                                if (p_it->fd == this->fds[i].fd)
                                {
                                    close(p_it->fd);
                                    this->fds.erase(p_it);
                                    i--; // Adjust index after erasure
                                    break;
                                }

                            }
                        }
                    }
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