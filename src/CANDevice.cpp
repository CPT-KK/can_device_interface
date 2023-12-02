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
    CANDevice(unsigned int id, bool isRead, int dlc = 8, std::function<void(const struct can_frame&)> callback = nullptr)
        : id_(id), isRead_(isRead), dlc_(dlc), callback_(callback), stopThread_(false) {
        initSocket();
    }

    ~CANDevice() {
        stopThread_ = true;
        if (receiveThread_.joinable()) {
            receiveThread_.join();
        }
        close(socket_);
    }

    void read() {
        if (isRead_) {
            if (!callback_) {
                throw std::logic_error("Callback function is not set for reading device.");
            }

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
                        if (frame.can_id == id_) {
                            callback_(frame);
                        }
                    }
                }
            });
        } else {
            throw std::logic_error("This device is not configured for reading.");
        }
    }

    void send(const void* payload) {
        struct can_frame frame;
        frame.can_dlc = dlc_;
        std::memcpy(frame.data, payload, std::min(static_cast<size_t>(dlc_), sizeof(frame.data)));

        if (id_ <= CAN_SFF_MASK) {
            // Standard ID
            frame.can_id = id_;
        } else {
            // Extended ID
            frame.can_id = id_ | CAN_EFF_FLAG;
        }

        ssize_t bytesSent = write(socket_, &frame, sizeof(struct can_frame));
        if (bytesSent == -1) {
            throw std::runtime_error(std::string("Error sending data frame: ") + strerror(errno));
        }
    }

    void initSocket() {
        socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (socket_ == -1) {
            throw std::runtime_error(std::string("Error creating socket: ") + strerror(errno));
        }

        struct sockaddr_can addr;
        struct ifreq ifr;

        std::strcpy(ifr.ifr_name, "can0");  // Replace "can0" with the actual interface name
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

        if (id_ <= CAN_SFF_MASK) {
            // Standard ID
            filter[0].can_id = id_;
            filter[0].can_mask = CAN_SFF_MASK;
        } else {
            // Extended ID
            filter[0].can_id = id_;
            filter[0].can_mask = CAN_EFF_MASK;
        }

        if (setsockopt(socket_, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter)) == -1) {
            close(socket_);
            throw std::runtime_error(std::string("Error setting socket options: ") + strerror(errno));
        }
    }

   private:
    unsigned int id_;
    int dlc_;  // 载荷长度
    int socket_;
    bool isRead_ = true;  // 设备是否配置为读操作
    std::function<void(const struct can_frame&)> callback_;
    std::thread receiveThread_;
    bool stopThread_;
};

void mycallback(const struct can_frame& frame) {
    std::cout << "CAN ID: " << frame.can_id << std::endl;
}

int main() {
    try {
        int canId;
        std::cout << "Enter CAN device ID: ";
        std::cin >> canId;

        bool isRead;
        std::cout << "Enter device property (0 for write, 1 for read): ";
        std::cin >> isRead;

        if (isRead) {
            // 如果是读设备，获取回调函数句柄或指针
            std::cout << "Enter callback function handle or pointer (0 for no callback): ";
            int callbackHandle;
            std::cin >> callbackHandle;

            // 创建CANDevice对象并配置为读操作
            CANDevice canDevice(canId, true, 8, mycallback);

            // 读取数据
            canDevice.read();
        } else {
            // 创建CANDevice对象并配置为写操作
            CANDevice canDevice(canId, false, 8);

            // 准备要发送的数据
            std::string payload;
            std::cout << "Enter payload for CAN frame: ";
            std::cin >> payload;

            // 发送数据
            canDevice.send(payload.c_str());
        }

        // 在此处可以添加其他业务逻辑

        // 主线程会等待用户输入，直到用户按下 Enter 键
        std::cout << "Press Enter to exit." << std::endl;
        std::cin.ignore();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
