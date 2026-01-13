#ifndef HELP_HPP
#define HELP_HPP

#include <iostream>
#include <vector>
#include <sstream>

class Help
{
    private:
        Help();
    public:
        static std::string trim(std::string &str) throw();
        static std::vector<std::string> split_command(std::string commads);
        static std::string nick_name(std::string nick);
        static std::string clear_stirng(std::string str);
};

#endif
