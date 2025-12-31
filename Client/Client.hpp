#ifndef CLIENT_HPP 
#define CLIENT_HPP

#include <iostream>

class Client
{
    private:
        int fd;
        std::string nickname;
        std::string username;
        std::string realname;
        bool registration;
        std::string buffer;  // Partial messages
        bool authenticated;
        short flage; // in authenticated is need client set nickname and username if ==2 is do this tow steps 
    public:
        Client();
        ~Client();
        //getters and setters
            std::string getBuffer() const;
            void setBuffer(const std::string buf);
            int getFd() const;
            void setFd(int fd);
            void erase_buffer(size_t start , size_t end);
            //Registration Commands
            std::string getNickname() const;
            void setNickname(const std::string nick);
            std::string getUsername() const;
            void setUsername(const std::string user);
            std::string getRealname() const;
            void setRealname(const std::string real);
            bool isRegistered() const;
            void setRegistered(bool reg);
            bool isAuthenticated() const;
            void setAuthenticated(bool auth);
            void setFlage();
            short getFlage() const;
};

#endif 