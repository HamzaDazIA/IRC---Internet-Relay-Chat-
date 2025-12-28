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
        std::string mod; // If they want nothing: They send 0. if is i == 8 the we can not 
        bool registration;
        std::string buffer;  // Partial messages
        bool authenticated;
    public:
        Client();
        ~Client();
        //getters and setters
            std::string getBuffer() const;
            void setBuffer(const std::string& buf);
            int getFd() const;
            void setFd(int fd);
            void erase_buffer(size_t start , size_t end);
            //Registration Commands
            std::string getNickname() const;
            void setNickname(const std::string& nick);
            std::string getUsername() const;
            void setUsername(const std::string& user);
            std::string getRealname() const;
            void setRealname(const std::string& real);
            std::string getMod() const;
            void setMod(const std::string& mode);
            bool isRegistered() const;
            void setRegistered(bool reg);
            bool isAuthenticated() const;
            void setAuthenticated(bool auth);

        };


#endif 