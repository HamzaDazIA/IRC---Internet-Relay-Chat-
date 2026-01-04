#ifndef PASS_HPP
#define PASS_HPP

#include "../Command.hpp"

class Pass : public Command
{
    public :
        Pass();
        ~Pass();
        int  execute(std::vector<std::string> commands , std::map<int, Client>::iterator &it_client);
        void checkPASS(std::string pass, std::map<int, Client>::iterator &client);

};
#endif