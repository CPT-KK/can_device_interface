#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <thread>

class CANDevice {
public:
    // Constructor for read-only CAN device
    CANDevice(const char* interface, unsigned int readId, std::function<void(const struct can_frame&)> callback);

    // Constructor for write-only CAN device
    CANDevice(const char* interface, unsigned int writeId);

    // Constructor for read-write CAN device
    CANDevice(const char* interface, unsigned int readId, std::function<void(const struct can_frame&)> callback, unsigned int writeId);

    ~CANDevice();

    // Function to start reading CAN frames from readId
    void read();

    // Function to send CAN frames to writeId
    void send(const unsigned char* payload, int dlc);


private:
    bool canRead_;
    unsigned int readId_;
    std::function<void(const struct can_frame&)> callback_;

    bool canWrite_;
    unsigned int writeId_;

    void initSocket_(const char* interface);
    int socket_;

    std::thread receiveThread_;
    bool stopThread_;
    
};