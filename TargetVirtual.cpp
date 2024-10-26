#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <linux/uinput.h>
#include <linux/joystick.h>

int create_virtual_usb_joystick(int num_axes, int num_buttons) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Error opening /dev/uinput" << std::endl;
        return -1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY); 
    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    for (int i = 0; i < num_buttons; ++i) {
        ioctl(fd, UI_SET_KEYBIT, BTN_JOYSTICK + i);
    }

    for (int i = 0; i < num_axes; ++i) {
        ioctl(fd, UI_SET_ABSBIT, ABS_X + i);
    }

    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));

    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Virtual USB Joystick");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x9999;
    uidev.id.product = 0x9999;
    uidev.id.version = 1;

    for (int i = 0; i < num_axes; ++i) {
        uidev.absmin[ABS_X + i] = -32767;
        uidev.absmax[ABS_X + i] = 32767;
        uidev.absfuzz[ABS_X + i] = 0;
        uidev.absflat[ABS_X + i] = 0;
    }

    if (write(fd, &uidev, sizeof(uidev)) < 0) {
        std::cerr << "Error writing device information" << std::endl;
        close(fd);
        return -1;
    }

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

void handle_updates(int fd, bool ActionButton_0, bool ActionButton_1, int value, int axis){
    if(ActionButton_0 && (axis == 1)){
        axis = 3 ;
        send_axis_event(fd, axis, value);
        sync_events(fd);

    }
    else if(ActionButton_0 && (axis == 0)){
        send_axis_event(fd, axis, value);
        sync_events(fd);
    }
    else if(!ActionButton_0 && (axis == 0)){
        return;
    }
    else {
        send_axis_event(fd, axis, value);
        sync_events(fd);
    }
}

int main() {
    const char *device = "/dev/input/js0"; 
    int fd = open(device, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Error: Could not open joystick device at " << device << std::endl;
        return -1;
    }
    char name[128];
    if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0) {
        std::strcpy(name, "Unknown");
    }
    int num_of_axes = 0;
    int num_of_buttons = 0;

    ioctl(fd, JSIOCGAXES, &num_of_axes);
    ioctl(fd, JSIOCGBUTTONS, &num_of_buttons);

    std::cout << "Joystick Name: " << name << std::endl;
    std::cout << "Number of Axes: " << num_of_axes << std::endl;
    std::cout << "Number of Buttons: " << num_of_buttons << std::endl;

    struct js_event jse;
    int axis_state[ABS_MAX] = {0};
    int button_state[KEY_MAX] = {0};
    
    int num_virt_axes = 4;
    int num_virt_buttons = 0;

    int fd_virt = create_virtual_usb_joystick(num_virt_axes, num_virt_buttons);
    if (fd_virt < 0) {
        return 1;
    }
    bool ActionButton_0 = false ;
    bool ActionButton_1 = false ;

    while (true) {
        ssize_t bytes = read(fd, &jse, sizeof(jse));

        if (bytes == sizeof(jse)) {
            jse.type &= ~JS_EVENT_INIT;

            if (jse.type == JS_EVENT_AXIS) {
                if (axis_state[jse.number] != jse.value) {
                    axis_state[jse.number] = jse.value;
                    std::cout << "Axis " << (int)jse.number << " value changed to " << jse.value << std::endl;
                    handle_updates(fd_virt,ActionButton_0,ActionButton_1,(int)jse.value,(int)jse.number);
                }
            } else if (jse.type == JS_EVENT_BUTTON) {
                if (button_state[jse.number] != jse.value) {
                    button_state[jse.number] = jse.value;
                    std::cout << "Button " << (int)jse.number << (jse.value ? " pressed" : " released") << std::endl;
                }
                if ((int)jse.number == 0){
                    if (jse.value == 1 ) {
                        ActionButton_0 = true;
                    }else {
                        ActionButton_0 = false;
                         send_axis_event(fd, 0, 0);
        		 sync_events(fd);
        		 send_axis_event(fd, 1, 0);
        		 sync_events(fd);
                    }
                }else {
                    if (jse.value == 1 ) {
                        ActionButton_1 = true;
                    }else {
                        ActionButton_1 = false;
                    }
                }
            }
        }
        usleep(10);
    }
    close(fd_virt);
    close(fd);
    return 0;
}

