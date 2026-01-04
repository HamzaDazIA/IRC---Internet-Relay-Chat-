#ifndef USER_HPP
#define USER_HPP


#include "../Command.hpp"
class User : public Command
{
    public :
        User();
        ~User();
        int  execute(std::vector<std::string> commandss , std::map<int, Client>::iterator &it_client);    
};
#endif