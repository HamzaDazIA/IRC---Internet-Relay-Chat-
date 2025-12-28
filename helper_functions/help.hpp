#ifndef HELP_HPP
#define HELP_HPP

#include <iostream>
#include <vector>
class Help
{
    private:
        Help();
    public:
        static std::string trim(std::string &str) throw();
        static std::vector<std::string>& split_command(std::string commads);

};

#endif