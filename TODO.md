# RCFW - Robot Operating System (ROS) Car FirmWare

* Use DMA wherever possible (UART?), to get better performance,
* Workout error handlers: real Error_Handler(), HardFault_Handler(), etc.
* Fix clock configuration issue while debugging (try OpenOCD),
* Use STM32CubeMX code generator & HAL layer to rework SPI/bluetooth?
* Add support for embedded MPU-6050 inertial unit? (use case TBD...),
* Combine 2 16-bit timers to get a 32-bit timer & extend UTI_delayUs(),
* Tune up PID values.