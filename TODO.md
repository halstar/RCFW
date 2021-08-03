# RCFW - Robot Operating System (ROS) Car FirmWare

* Use DMA wherever possible (UART?), to get better performance,
* Add some robustness around; especially check HAL functions return values,
* Workout error handlers: real Error_Handler(), HardFault_Handler(), etc.
* Fix clock configuration issue while debugging (try OpenOCD),
* Add a SW reset function on input switch PC0?
* Use STM32CubeMX code generator & HAL layer to rework SPI/bluetooth?
* Implement command's reception/decoding from master board (Raspberry Pi),
* Finalize PID integration.