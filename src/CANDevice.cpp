// Description: simple CAN device class for reading and writing CAN frames
// Author: H.L. Kuang
// Update: 2023/12/02

#include "CANDevice.h"

// Constructor for read-only CAN device
CANDevice::CANDevice(const char* interface, unsigned int readId, std::function<void(const struct can_frame&)> callback) : readId_(readId), writeId_(0), callback_(callback), stopThread_(false), canRead_(true), canWrite_(false) {
    initSocket_(interface);
}

// Constructor for write-only CAN device
CANDevice::CANDevice(const char* interface, unsigned int writeId) : readId_(0), writeId_(writeId), callback_(nullptr), stopThread_(false), canRead_(false), canWrite_(true) {
    initSocket_(interface);
}

// Constructor for read-write CAN device
CANDevice::CANDevice(const char* interface, unsigned int readId, std::function<void(const struct can_frame&)> callback, unsigned int writeId) : readId_(readId), writeId_(writeId), callback_(callback), stopThread_(false), canRead_(true), canWrite_(true) {
    initSocket_(interface);
}

CANDevice::~CANDevice() {
    stopThread_ = true;
    if (receiveThread_.joinable()) {
        receiveThread_.join();
    }
    close(socket_);
}

// Start reading CAN frames from <readId>(defined in constructor), and send them to <callback>(defined in constructor)
void CANDevice::read() {
    if (canRead_) {
        // Check if callback function is set
        if (!callback_) {
            throw std::logic_error("Callback function is not set for reading device.");
        }

        // Start receive thread
        receiveThread_ = std::thread([this]() {
            struct can_frame frame;
            while (!stopThread_) {
                ssize_t bytesRead = recv(socket_, &frame, sizeof(struct can_frame), 0);

                if (bytesRead == -1) {
                    // Handle error
                    throw std::runtime_error(std::string("Error receiving data frame: ") + strerror(errno));
                } else if (bytesRead == 0) {
                    // Connection closed
                    throw std::runtime_error("Connection closed by peer");
                } else if (bytesRead == sizeof(struct can_frame)) {
                    if (frame.can_id == readId_) {
                        callback_(frame);
                    }
                }
            }
        });
    } else {
        throw std::logic_error("This device is not configured for reading.");
    }
}

// Send <payload> with length <dlc> to <writeId>(defined in constructor)
void CANDevice::send(const unsigned char* payload, int dlc) {
    if (canWrite_) {
        // Declare frame to send
        struct can_frame frame;

        // Set data length
        frame.can_dlc = dlc;

        // Check if payload size is valid
        if (dlc > sizeof(frame.data)) {
            throw std::length_error("Payload size exceeds the maximum allowed size.");
        }

        // Copy payload
        std::memcpy(frame.data, payload, dlc);

        // Set ID, and see if we need to set the EFF flag
        if (writeId_ <= CAN_SFF_MASK) {
            // Standard ID
            frame.can_id = writeId_;
        } else {
            // Extended ID
            frame.can_id = writeId_ | CAN_EFF_FLAG;
        }

        // Send frame
        ssize_t bytesSent = write(socket_, &frame, sizeof(struct can_frame));

        // Check if all bytes were sent
        if (bytesSent == -1) {
            throw std::runtime_error(std::string("Error sending data frame: ") + strerror(errno));
        }
    } else {
        throw std::logic_error("This device is not configured for writing.");
    }
}

void CANDevice::initSocket_(const char* interface) {
    socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_ == -1) {
        throw std::runtime_error(std::string("Error creating socket: ") + strerror(errno));
    }

    struct sockaddr_can addr;
    struct ifreq ifr;

    std::strcpy(ifr.ifr_name, interface);
    if (ioctl(socket_, SIOCGIFINDEX, &ifr) == -1) {
        close(socket_);
        throw std::runtime_error(std::string("Error getting interface index: ") + strerror(errno));
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
        close(socket_);
        throw std::runtime_error(std::string("Error binding socket: ") + strerror(errno));
    }

    // 设置过滤器
    struct can_filter filter[1];

    if (readId_ <= CAN_SFF_MASK) {
        // Standard ID
        filter[0].can_id = readId_;
        filter[0].can_mask = CAN_SFF_MASK;
    } else {
        // Extended ID
        filter[0].can_id = readId_;
        filter[0].can_mask = CAN_EFF_MASK;
    }

    if (setsockopt(socket_, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter)) == -1) {
        close(socket_);
        throw std::runtime_error(std::string("Error setting socket options: ") + strerror(errno));
    }
}

void mycallback(const struct can_frame& frame) {
    printf("%X[%d]#%s\n", frame.can_id, frame.can_dlc, frame.data);
}

int main() {
    try {
        CANDevice readDevice("vcan0", 0x301u, mycallback);
        readDevice.read();

        CANDevice writeDevice("vcan0", 0x401u);
        unsigned char payload[8];
        payload[0] = 0xAA;
        payload[1] = 0xBB;
        payload[2] = 0xCC;
        payload[3] = 0xDD;
        writeDevice.send(payload, sizeof(payload));

        // 主线程会等待用户输入，直到用户按下 Enter 键
        std::cout << "Press Ctrl + C to exit." << std::endl;
        std::cin.get();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
