#include <stdio.h>
#include <stdlib.h>
#include <linux/joystick.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libusb-1.0/libusb.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

void emit(int fd, int type, int code, int val) {
    struct input_event ie;
    
    ie.type = type;
    ie.code = code;
    ie.value = val;
    //timestamp values below are ignored
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    
    write(fd, &ie, sizeof(ie));
}

int main() {
    struct uinput_user_dev input;
    int fd;
    int op;
    int i;
    int j;
    int scale;
    unsigned char info[1];
    static unsigned char bytes[8];
    unsigned char *r = bytes;
    libusb_device_handle* device;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    op = 0;

    op = ioctl(fd, UI_SET_EVBIT, EV_KEY);
    op = ioctl(fd, UI_SET_EVBIT, EV_SYN);
    ioctl(fd, UI_SET_ABSBIT, ABS_X);
    ioctl(fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    ioctl(fd, UI_SET_KEYBIT, BTN_A);

    if(fd < 0) {
        printf("Error: uinput not opened\n");
        return -1;
    }

    memset(&input, 0, sizeof(input));
    input.id.bustype = BUS_USB;
    input.id.version = 1;
    input.id.vendor = 0x9A7A;
    input.id.product = 0xBA17;
    snprintf(input.name, UINPUT_MAX_NAME_SIZE, "ChompStick");

    op = write(fd, &input, sizeof(input));
    if(op < 0) {
        printf("Error: uinput not written\n");
        return -1;
    }

    op = ioctl(fd, UI_DEV_CREATE);

    if(op < 0) {
        printf("Error: device not created\n");
        return -1;
    }
    
    libusb_init(NULL);

    device = libusb_open_device_with_vid_pid(NULL, 0x9A7A, 0xBA17);
    if (device == NULL) {
        printf("Error: device not opened\n");
        return -1;
    }

    op = libusb_claim_interface(device, 0);
    op = libusb_set_interface_alt_setting(device, 0, 0);

    while (true) {
        op = libusb_interrupt_transfer(device, 0x81, info, sizeof(info), &scale, 1000);

        i = 7;
        j = 0;
        while(i >= 0) {
            r[i] = ((info[0] & (1 << j)) > 0) + '0';
            i--;
            j++;
        }

        int space = r[3] - '0';

        int x = (r[5] - '0');
        int y = (r[7] - '0');
        
        x += 2 * (r[4] - '0');
        y += 2 * (r[6] - '0');

        for(int i = 0; i < sizeof(r); i++) {
            printf("%c", r[i]);
        }
        printf("\n");

        if (x == 1) {
            emit(fd, EV_ABS, ABS_X, -32767);
            emit(fd, EV_SYN, SYN_REPORT,0);
        }
        else if (x == 2) {
            emit(fd, EV_ABS, ABS_X, 0);
            emit(fd, EV_SYN, SYN_REPORT, 0);
        }
        else if (x == 3) {
            emit(fd, EV_ABS, ABS_X, 32767);
            emit(fd, EV_SYN, SYN_REPORT,0);
        }

        if (y == 1) {
            emit(fd, EV_ABS, ABS_Y, 32767);
            emit(fd, EV_SYN, SYN_REPORT,0);
        }
        else if (y == 2) {
            emit(fd, EV_ABS, ABS_Y, 0);
            emit(fd, EV_SYN, SYN_REPORT, 0);
        }
        else if (y == 3) {
            emit(fd, EV_ABS, ABS_Y, -32767);
            emit(fd, EV_SYN, SYN_REPORT,0);
        }

        if (space == 0) {
            emit(fd, EV_KEY, BTN_A, 0);
            emit(fd, EV_SYN, SYN_REPORT, 0);
        }
        else if (space == 1) {
            emit(fd, EV_KEY, BTN_A, 1);
            emit(fd, EV_SYN, SYN_REPORT, 0);
        }
    }

    return 0;
}