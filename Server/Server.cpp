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


//opt set_sockopt 
int *Server::get_option_sockopt(void) const
{
    return (this->option_sockopt);
}

void Server::set_option_sockopt(int opt)
{
    this->option_sockopt = &opt;
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
std::string Server::errorUNKNOWNCOMMAND(std::string &client, std::string commad)
{
    return ("ft_irc.24 (421) " + client + " " + commad + ERR_UNKNOWNCOMMAND);
}

//handel new CLient

void Server::handelNewClient(int &server_fd)
{

    struct sockaddr_in newClient = {};
    socklen_t clinetlen = sizeof(newClient);
    int newClient_fd = accept(server_fd, (sockaddr *)&newClient, &clinetlen);

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
    
    this->clients.insert({newClient_fd, obj});

}

//handel client;
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
            if (errno & EAGAIN || errno & EWOULDBLOCK)
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
            close(even_client.fd);
            this->clients.erase(it);
            for (std::vector<struct pollfd>::iterator p_it = this->fds.begin(); p_it != this->fds.end(); ++p_it)
            {
                if (p_it->fd == even_client.fd)
                {
                    this->fds.erase(p_it);
                    break;
                }
            }
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
    this->handelBuffer(it); // here we check if client send all command by tirminited /r /n
}

void Server::handelCommand(std::map<int, Client>::iterator &it_client , std::string commad)
{
    std::vector<std::string> commandss = Help::split_command(commad);
    std::string cmd = "CAP";
    std::vector<std::string>::iterator it = commandss.begin();
    if (it[0] ==  cmd)
    {
        std::string nick = it_client->second.getNickname();
        if (nick.empty())
        {
            nick = "*"; //because we dont have name of client we replace to be "*"
        }
        std::string message = this->errorUNKNOWNCOMMAND(nick, cmd);

        std::cout << "Send 421 to clinet " << it_client->first << ": " << cmd << std::endl;
        return ; 
    }
    cmd = "PASS";
    
    if (it[0] == cmd)
    {

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
            this->handelCommand(it_client, commad);
        }
        pos = temp.find('\n');
    }
}


void Server::start_server(void)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == FAILDE)
    {
        throw std::runtime_error("Error Socket : failde create socket.");
    }

    
    set_option_sockopt(1);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, get_option_sockopt(), sizeof(*(get_option_sockopt())));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET; // type of IP is IPv4 
    addr.sin_port = htons(this->port); // set port number we use htons function converts the unsigned short integer hostshort from host byte order to network byte order.
    addr.sin_addr.s_addr = INADDR_ANY; // Address to accept any incoming messages.

    if (bind(server_fd, (sockaddr *) (&addr), sizeof(addr)) == FAILDE )
    {
        close(server_fd);
        throw std::runtime_error("Error bind: Failde bind socket server.");
    }
    
    if (listen(server_fd, SOMAXCONN) == FAILDE) // SOMAXCONN. This tells the OS: "Give me the maximum possible queue size allowed on this specific machine
    {
        close(server_fd);
        throw std::runtime_error("Error listen: Failde listing socket server.");
    } 

    struct pollfd fd_info = {};
    fd_info.fd  = server_fd;
    fd_info.events = POLLIN;
    fd_info.revents = 0;
    this->fds.push_back(fd_info);
    int poll_flag = 0;
    while(true)
    {
        poll_flag = poll(this->fds.data(), this->fds.size(), FAILDE);

        if (poll_flag == FAILDE)
        {
            //here trow expation and in case we have fds of client and server we need remove him ;

        }
        for (size_t i = 0 ; i < this->fds.size(); i++)
        {
            if (this->fds[i].revents & POLLIN)
            {

                if (this->fds[i].fd == server_fd)
                {
                    this->handelNewClient(server_fd);
                    i--; // to avoid skiping next client in fds vector
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
                    i--; // to avoid skiping next client in fds vector
                }
            }
        }
    }

}
