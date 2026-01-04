#include "Command.hpp"

Command::Command()
{
    this->upper = "default";
    this->lower = "default";
    this->server = NULL;
}

Command::~Command()
{}

void Command::setServer(Server* srv)
{
    this->server = srv;
}
