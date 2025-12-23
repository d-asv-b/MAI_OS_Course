#include "common.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

std::string Message::toString() const {
    std::stringstream ss;
    ss << static_cast<int>(type) << " " << client_pid << " " << payload.length() << " " << payload;
    
    return ss.str();
}

Message Message::fromString(const std::string& s) {
    std::stringstream ss(s);

    int type_int;
    ss >> type_int;
    MessageType type = static_cast<MessageType>(type_int);

    int pid;
    ss >> pid;

    size_t payload_len;
    ss >> payload_len;

    std::string payload = s.substr(s.size() - payload_len, payload_len + 1);
    
    return {type, pid, payload};
}

void create_FIFO(const std::string& path) {
    if (mkfifo(path.c_str(), 0666) == -1) {
        perror("mkfifo");
    }
}

void remove_FIFO(const std::string& path) {
    unlink(path.c_str());
}

void write_to_FIFO(const std::string& path, const std::string& message) {
    int fd = open(path.c_str(), O_WRONLY | O_NONBLOCK);
    if (fd == -1) {
        if (errno == ENXIO) {
            std::cerr << "Warning: No reader for FIFO " << path << ", message not sent.\n";
        } else {
            perror(("open for write to FIFO " + path).c_str());
        }
        return;
    }

    ssize_t bytes_written = write(fd, message.c_str(), message.length());
    if (bytes_written == -1) {
        perror(("write to FIFO " + path).c_str());
    } else if (bytes_written < message.length()) {
        std::cerr << "Warning: Partial write to FIFO " << path << ". Sent " << bytes_written << " of " << message.length() << " bytes.\n";
    }

    close(fd);
}

std::string read_from_FIFO(const std::string& path) {
    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        if (errno != ENXIO && errno != ENOENT) {
            perror(("open for read from FIFO " + path).c_str());
        }
        return "";
    }

    char buffer[4096];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes_read == -1) {
        if (errno != EAGAIN) {
            perror(("read from FIFO " + path).c_str());
        }
        return "";
    } else if (bytes_read == 0) {
        return "";
    }

    buffer[bytes_read] = '\0';

    return std::string(buffer);
}

