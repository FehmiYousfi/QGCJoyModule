#include <iostream>
#include <fcntl.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <cstring>

int create_virtual_usb_joystick(int num_axes, int num_buttons) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Error opening /dev/uinput" << std::endl;
        return -1;
    }

    // Enable event types for joystick
    ioctl(fd, UI_SET_EVBIT, EV_KEY);  // Buttons
    ioctl(fd, UI_SET_EVBIT, EV_ABS);  // Axes
    ioctl(fd, UI_SET_EVBIT, EV_SYN);  // Synchronization events

    // Enable buttons
    for (int i = 0; i < num_buttons; ++i) {
        ioctl(fd, UI_SET_KEYBIT, BTN_JOYSTICK + i);  // BTN_JOYSTICK starts from button 0
    }

    // Enable axes (ABS_X, ABS_Y, etc.)
    for (int i = 0; i < num_axes; ++i) {
        ioctl(fd, UI_SET_ABSBIT, ABS_X + i);
    }

    // Configure the virtual joystick device
    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));

    // Device name and details (emulating a USB joystick)
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Virtual USB Joystick");
    uidev.id.bustype = BUS_USB;  // Emulate a USB device
    uidev.id.vendor = 0x1234;    // Example vendor ID
    uidev.id.product = 0x5678;   // Example product ID
    uidev.id.version = 1;

    // Set up axes (example for 2 axes, can add more)
    for (int i = 0; i < num_axes; ++i) {
        uidev.absmin[ABS_X + i] = -32767;
        uidev.absmax[ABS_X + i] = 32767;
        uidev.absfuzz[ABS_X + i] = 0;
        uidev.absflat[ABS_X + i] = 0;
    }

    // Write device configuration to uinput
    if (write(fd, &uidev, sizeof(uidev)) < 0) {
        std::cerr << "Error writing device information" << std::endl;
        close(fd);
        return -1;
    }

    // Create the virtual device
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        std::cerr << "Error creating uinput device" << std::endl;
        close(fd);
        return -1;
    }

    return fd;
}

void send_axis_event(int fd, int axis, int value) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_ABS;
    ev.code = ABS_X + axis;
    ev.value = value;
    write(fd, &ev, sizeof(ev));
}

void send_button_event(int fd, int button, bool pressed) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = BTN_JOYSTICK + button;
    ev.value = pressed ? 1 : 0;
    write(fd, &ev, sizeof(ev));
}

void sync_events(int fd) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

int main() {
    int num_axes = 4;     // Example: 4 axes (X, Y, Z, RX)
    int num_buttons = 8;  // Example: 8 buttons

    int fd = create_virtual_usb_joystick(num_axes, num_buttons);
    if (fd < 0) {
        return 1;
    }

    // Simulate joystick events (for demo purposes)
    for (int i = 0; i < 100; ++i) {
        // Move axis 0 (X-axis) back and forth
        int value = (i % 2 == 0) ? -32767 : 32767;
        send_axis_event(fd, 0, value);

        // Press and release button 0
        bool pressed = (i % 2 == 0);
        send_button_event(fd, 0, pressed);

        // Synchronize the events
        sync_events(fd);

        usleep(100000);  // Sleep for 100ms
    }

    // Destroy the virtual joystick
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    return 0;
}

