# RCFW - Robot Operating System (ROS) Car FirmWare

* Use DMA wherever possible (UART?), to get better performance,
* Workout error handlers: real Error_Handler(), HardFault_Handler(), etc.
* Fix clock configuration issue while debugging (try OpenOCD),
* Use STM32CubeMX code generator & HAL layer to rework SPI/bluetooth?
* Add support for embedded MPU-6050 inertial unit? (use case TBD...),
* Redesign/refactor drive.s as it's getting too big,
* While starting in debug/fixed speed, deal with unconnected PS2 controller,
* Tune up PID values.