#ifndef NICK_HPP
#define NICK_HPP

#include "../Command.hpp"

class Nick : public Command 
{
    public :
        Nick();
        ~Nick();
        int  execute(std::vector<std::string> commands , std::map<int, Client>::iterator &it_client) ;
        bool parsingNICK(std::string &nick);
};
#endif