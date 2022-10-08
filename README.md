# Project Description

In this project we implement a userspace device driver that connects to and reads data from the device in the USB subsystem, interprets the data to determine the current device state, and routes it back into the Linux input system as a conventional joystick.

My first task was to initialize the uinput device by opening the uinput folder with the libusb open function and providing the x, y, and button variables along with the desired vendor id and product id of the device to be created. I then created the virtual USB joystick device with ioctl(fd, UI_DEV_CREATE) and opened it with libusb_open_device_with_vid_pid() and the vendor id and product id, from libusb. 

We then enter the main loop of the program and wait for an interrupt from the virtual joystick with libusb_interrupt_transfer() with the virtual device, also from libusb (it will retrieve data from the virtual joystick and return 1 byte of data, which will be parsed into bits). After the byte of data (within an array of unsigned chars) is parsed and read as bits, I then find the state of x, y, and the button. We can then emit key events, which is to say we map the state of the keys to the corresponding values (0, -32767, or 32767) outlined in the spec.

The libusb library and uinput module were essential to the execution of all of these tasks, as they allowed us to perform a variety of actions within the program, such as connecting to the chompapp virtual device (given vendor id and product id), emulating a userspace input device, creating, writing to, and reading from the virtual device, and detecting interrupts raised by the virtual device.
