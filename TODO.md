# RCFW - Robot Operating System (ROS) Car FirmWare

* Use DMA wherever possible (UART?), to get better performance,
* Workout error handlers: real Error_Handler(), HardFault_Handler(), etc.
* Fix clock configuration issue while debugging (try OpenOCD),
* Use STM32CubeMX code generator & HAL layer to rework SPI/bluetooth?
* Add support for embedded MPU-6050 inertial unit? (use case TBD...),
* Improve UTI_getTimeUs() as it's limited to 65ms before it wraps up:
  create UTI_startUsChrono()/readUsChrono()/resetUsChrono(), that will
  provide a similar service but will be capable of raising an error when
  the 65ms capability will be passed, 
* Tune up PID values.