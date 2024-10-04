#include <linux/joystick.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

int main() {
    const char *device = "/dev/input/js1"; 
    
    int fd = open(device, O_RDONLY | O_NONBLOCK);

    if (fd < 0) {
        std::cerr << "Error: Could not open joystick device at " << device << std::endl;
        return -1;
    }

    char name[128]; // Joystick name
    if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0) {
        std::strcpy(name, "Unknown");
    }

    int num_of_axes = 0;
    int num_of_buttons = 0;

    // Get number of axes
    ioctl(fd, JSIOCGAXES, &num_of_axes);
    // Get number of buttons
    ioctl(fd, JSIOCGBUTTONS, &num_of_buttons);

    std::cout << "Joystick Name: " << name << std::endl;
    std::cout << "Number of Axes: " << num_of_axes << std::endl;
    std::cout << "Number of Buttons: " << num_of_buttons << std::endl;

    struct js_event jse;
    int axis_state[ABS_MAX] = {0};
    int button_state[KEY_MAX] = {0};

    while (true) {
        ssize_t bytes = read(fd, &jse, sizeof(jse));

        if (bytes == sizeof(jse)) {
            jse.type &= ~JS_EVENT_INIT;

            if (jse.type == JS_EVENT_AXIS) {
                if (axis_state[jse.number] != jse.value) {
                    axis_state[jse.number] = jse.value;
                    std::cout << "Axis " << (int)jse.number << " value changed to " << jse.value << std::endl;
                }
            } else if (jse.type == JS_EVENT_BUTTON) {
                if (button_state[jse.number] != jse.value) {
                    button_state[jse.number] = jse.value;
                    std::cout << "Button " << (int)jse.number << (jse.value ? " pressed" : " released") << std::endl;
                }
            }
        }
        usleep(10); // Polling rate control (1ms)
    }

    close(fd);
    return 0;
}

