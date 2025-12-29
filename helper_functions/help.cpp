#include "help.hpp"


std::string Help::trim(std::string &str) throw()
{

    std::string whitespace = " \t\n\r";
    size_t start = str.find_first_not_of(whitespace);
    
    if (start == std::string::npos)
        return "";
    
    size_t end = str.find_last_not_of(whitespace);
    
    return str.substr(start, end - start + 1);
}

std::vector<std::string> Help::split_command(std::string commads)
{
    std::vector<std::string> args ;
    std::istringstream iss(commads);
    std::string tokens;

    while(iss >> tokens)
    {
        if (tokens[0] == ':')
        {
            size_t  pos  = commads.find(tokens);
            
            std::string cur = commads.substr(pos + 1);
            args.push_back(cur);
            break;
        }
        else
        {
            args.push_back(tokens);
        }
    }
    return args;

}

std::string nick_name(std::string nick)
{
    return (nick.empty() ? "*" : nick); // condition ? value_if_true : value_if_false;

}