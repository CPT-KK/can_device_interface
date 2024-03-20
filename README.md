# CAN_DEVICE_INTERFACE

The **CAN_DEVICE_INTERFACE** package provides a class for quick and easy access to CAN devices on Linux platforms.

The package comes with CMAKE for being a submodule of the user's main code/packages.

## Usage

There are 3 modes for accessing a CAN device, defined by three different constructors.

- [Read-only constructor](https://github.com/CPT-KK/can_device_interface/blob/main/include/CANDevice.h#L34)
- [Write-only constructor](https://github.com/CPT-KK/can_device_interface/blob/main/include/CANDevice.h#L41)
- [Read-and-write constructor](https://github.com/CPT-KK/can_device_interface/blob/main/include/CANDevice.h#L50)

For CAN devices with read access, the method `read()` is available for reading the **raw data** from CAN devices and uses the callback function (defined by users in the constructor) to interpret it.

For CAN devices with read access, the method `send()` is available for sending the **payload** to CAN devices.

The read and write mode **CANNOT** be changed after being defined. 

## Building with your code

See [motor_ros_driver](https://github.com/CPT-KK/motor_ros_driver) for an example.
