#include "Client.hpp"


Client::Client() : fd(-1), registration(false), authenticated(false) , flage(0) {}
Client::~Client()
{
}
std::string Client::getBuffer() const
{
    return buffer;
}

void Client::setBuffer(const std::string buf)
{
    buffer = buf;
}

int Client::getFd() const
{
    return fd;
}

void Client::setFd(int fd)
{
    this->fd = fd;
}

std::string Client::getNickname() const
{
    return nickname;
}

void Client::setNickname(const std::string nick)
{
    nickname = nick;
}

std::string Client::getUsername() const
{
    return username;
}

void Client::setUsername(const std::string user)
{
    username = user;
}

std::string Client::getRealname() const
{
    return realname;
}

void Client::setRealname(const std::string real)
{
    realname = real;
}

bool Client::isRegistered() const
{
    return registration;
}

void Client::setRegistered(bool reg)
{
    registration = reg;
}

bool Client::isAuthenticated() const
{
    return authenticated;
}

void Client::setAuthenticated(bool auth)
{
    authenticated = auth;
}


void Client::erase_buffer(size_t start , size_t end)
{
    this->buffer.erase(start , end);
}

void Client::setFlage()
{

    if (flage > 2)
    {
        return;
    }
    flage++;
}

short Client::getFlage() const
{
    return flage;
}

