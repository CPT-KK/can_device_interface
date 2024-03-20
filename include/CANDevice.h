//
// @brief: CAN 设备读写操作类（接口） interfaces for reading and writing CAN frames for CAN devices
// @copyright: Copyright 2023 H.L. Kuang
// @license: See repo license file
// @birth: created by H.L. Kuang on 2023-12-02
// @version: 1.0.0
// @revision: last revised by H.L. Kuang on 2023-12-02
//

#ifndef CAN_DEVICE_INTERFACE_H
#define CAN_DEVICE_INTERFACE_H
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
    /**
     *  @brief Constructor for read-only CAN device.
     *  @param interface The CAN interface of the system, e.g. slcan0, vcan0.
     *  @param readId The device CAN ID to read(receive), e.g. 0x12F, 0x123456E8.
     *  @param callback The callback function to process the CAN frame after reading. Only accept one input parameter of type struct can_frame.
    */
    CANDevice(const char* interface, unsigned int readId, std::function<void(const struct can_frame&)> callback);

    /**
     *  @brief Constructor for write-only CAN device.
     *  @param interface The CAN interface of the system, e.g. slcan0, vcan0.
     *  @param writeId The device CAN ID to write(send).
    */
    CANDevice(const char* interface, unsigned int writeId);

    /**
     *  @brief Constructor for read-write CAN device.
     *  @param interface The CAN interface of the system, e.g. slcan0, vcan0.
     *  @param readId The device CAN ID to read(receive), e.g. 0x12F, 0x123456E8.
     *  @param callback The callback function to process the CAN frame after reading. Only accept one input parameter of type struct can_frame.
     *  @param writeId The device CAN ID to write(send), e.g. 0x12F, 0x123456E8.
    */
    CANDevice(const char* interface, unsigned int readId, std::function<void(const struct can_frame&)> callback, unsigned int writeId);

    // Destructor
    ~CANDevice();

    /**
     *  @brief Start reading CAN frames from <readId>(defined in constructor), and send them to <callback>(defined in constructor).
    */
    void read();

    /**
     *  @brief Send <payload> with length <dlc> to <writeId>(defined in the constructor).
     *  @param payload The CAN frame payload to send.
     *  @param dlc The payload length.
    */
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
#endif
