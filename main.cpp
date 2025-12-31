#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>
#define FAILDE -1  
// ============================================
// STEP 1: Make socket non-blocking
// ============================================
bool setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == FAILDE)
        return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != FAILDE;
}

// ============================================
// STEP 2: Create and configure server socket
// ============================================
int createServerSocket(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == FAILDE)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return FAILDE;
    }

    // Allow address reuse (important for testing!)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Make it non-blocking
    if (!setNonBlocking(server_fd))
    {
        std::cerr << "Failed to set non-blocking" << std::endl;
        close(server_fd);
        return FAILDE;
    }

    // Bind to address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == FAILDE)
    {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return FAILDE;
    }

    // Listen with backlog of 10
    if (listen(server_fd, 10) == FAILDE)
    {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return FAILDE;
    }

    std::cout << "Server listening on port " << port << std::endl;
    return server_fd;
}

// ============================================
// STEP 3: Handle new client connection
// ============================================
void handleNewConnection(int server_fd, std::vector<struct pollfd> &fds,
                         std::map<int, std::string> &clientBuffers)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // Non-blocking accept - might return FAILDE if no client waiting
    int client_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &clientLen);

    if (client_fd == FAILDE)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "Accept error" << std::endl;
        }
        return;
    }
    

    // Make client socket non-blocking too!
    if (!setNonBlocking(client_fd))
    {
        std::cerr << "Failed to set client non-blocking" << std::endl;
        close(client_fd);
        return;
    }

    std::cout << "New client connected! fd=" << client_fd << std::endl;

    // Add to poll array
    struct pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN; // We want to read from clients
    pfd.revents = 0;
    fds.push_back(pfd);

    // Initialize client buffer for partial messages
    clientBuffers[client_fd] = "";

    // Send welcome message
    const char *msg = ":server 001 * :Welcome to IRC Server\r\n";
    send(client_fd, msg, strlen(msg), 0);
}

// ============================================
// STEP 4: Handle client data (IRC messages)
// ============================================
void handleClientData(int client_fd, std::vector<struct pollfd> &fds, size_t index,
                      std::map<int, std::string> &clientBuffers)
{
    char buffer[1024];

    // Non-blocking read - returns immediately
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read == FAILDE)
    {
        // No data available right now (shouldn't happen after poll, but safe!)
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }
        // Real error
        std::cerr << "Recv error on fd=" << client_fd << std::endl;
        close(client_fd);
        fds.erase(fds.begin() + index);
        clientBuffers.erase(client_fd);
        return;
    }

    if (bytes_read == 0)
    {
        // Client disconnected gracefully
        std::cout << "Client fd=" << client_fd << " disconnected" << std::endl;
        close(client_fd);
        fds.erase(fds.begin() + index);
        clientBuffers.erase(client_fd);
        return;
    }

    // We got data! Add to buffer
    buffer[bytes_read] = '\0';
    clientBuffers[client_fd] += buffer;

    std::cout << "Received from fd=" << client_fd << ": " << buffer;

    // IRC messages end with \r\n - check if we have complete message(s)
    size_t pos;
    while ((pos = clientBuffers[client_fd].find("\r\n")) != std::string::npos)
    {
        std::string message = clientBuffers[client_fd].substr(0, pos);
        clientBuffers[client_fd].erase(0, pos + 2);

        std::cout << "Complete IRC message from fd=" << client_fd << ": " << message << std::endl;

        // TODO: Parse and handle IRC commands here
        // For now, just echo back
        std::string response = ":server PRIVMSG * :" + message + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
    }
}

// ============================================
// STEP 5: Main server loop
// ============================================
int main()
{
    const int PORT = 6667;

    // Create server socket
    int server_fd = createServerSocket(PORT);
    if (server_fd == FAILDE)
    {
        return 1;
    }

    // Setup poll array
    std::vector<struct pollfd> fds;

    struct pollfd server_pfd;
    server_pfd.fd = server_fd;
    server_pfd.events = POLLIN;
    server_pfd.revents = 0;
    fds.push_back(server_pfd);

    // Store partial messages for each client
    std::map<int, std::string> clientBuffers;

    std::cout << "\n=== IRC Server Started ===\n"
              << std::endl;

    // Main event loop
    while (true)
    {

        int poll_count = poll(fds.data(), fds.size(), FAILDE);

        if (poll_count == FAILDE)
        {
            std::cerr << "Poll failed" << std::endl;
            break;
        }

        // Check which sockets have events
        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents == 0)
            {
                continue; // No event on this socket
            }

            // Check for errors
            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                std::cerr << "Socket error on fd=" << fds[i].fd << std::endl;
                close(fds[i].fd);
                fds.erase(fds.begin() + i);
                clientBuffers.erase(fds[i].fd);
                i--;
                continue;
            }

            // Data available to read (POLLIN)
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == server_fd)
                {
                    // New connection on server socket
                    handleNewConnection(server_fd, fds, clientBuffers);
                }
                else
                {
                    // Data from existing client
                    handleClientData(fds[i].fd, fds, i, clientBuffers);
                    i--; // Adjust if client was removed
                }
            }
        }
    }

    // Cleanup
    for (size_t i = 0; i < fds.size(); i++)
    {
        close(fds[i].fd);
    }

    return 0;
}

// ============================================
// HOW TO TEST:
// ============================================
// 1. Compile: g++ -std=c++98 -Wall -Wextra -Werror server.cpp -o ircserv
// 2. Run: ./ircserv
// 3. Connect with netcat: nc localhost 6667
// 4. Or use real IRC client: irssi -c localhost -p 6667
// 5. Type messages ending with Enter (sends \r\n)
// ============================================c