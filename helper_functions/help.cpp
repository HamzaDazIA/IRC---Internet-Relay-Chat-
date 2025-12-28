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

std::vector<std::string>& Help::split_command(std::string commads)
{
    std::string del = " \t";
    std::vector<std::string> vector ;
    size_t pos = commads.find(del);


    // case if we have one command 

    while(pos != std::string::npos)
    {
        std::string cur = commads.substr(0, pos + 1);
        if (cur.empty())
            continue;
        vector.push_back(cur);
        commads.erase(0, pos + 1);
        pos = commads.find(del);
    }
    return vector;
}
