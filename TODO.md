# RCFW - Robot Operating System (ROS) Car FirmWare

* Use DMA wherever possible (UART?), to get better performance,
* Add target release, especially to disable logs, 
* Add some robustness around; especially check HAL functions return values,
* Workout a real Error_Handler(), HardFault_Handler(), etc.