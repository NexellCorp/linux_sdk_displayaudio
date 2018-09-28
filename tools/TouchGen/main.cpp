#include <linux/uinput.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h> // srand()
#include <time.h>

#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   600

void emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

int main(int argc, char **argv)
{
    struct uinput_user_dev uud;
    int fd;
    int x = 0, y = 0;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        printf("open error(%s)\n", "/dev/uinput");
        return -1;
    }

    /* enable mouse button left and relative events */
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    ioctl(fd, UI_SET_ABSBIT, ABS_X);
    ioctl(fd, UI_SET_ABSBIT, ABS_Y);

    memset(&uud, 0, sizeof(uud));
    snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "uinput old interface");
    write(fd, &uud, sizeof(uud));

    ioctl(fd, UI_DEV_CREATE);

    /*
    * On UI_DEV_CREATE the kernel will create the device node for this
    * device. We are inserting a pause here so that userspace has time
    * to detect, initialize the new device, and can start listening to
    * the event, otherwise it will not notice the event we are about
    * to send. This pause is only needed in our example code!
    */
    sleep(1);

    /* Move the mouse diagonally, 5 units per axis */
    srand(time(NULL));
    while (1)
    {
        x = rand() % SCREEN_WIDTH;
        y = rand() % SCREEN_HEIGHT;

        emit(fd, EV_ABS, ABS_X, x);
        emit(fd, EV_ABS, ABS_Y, y);
        emit(fd, EV_KEY, BTN_LEFT, 1);
        emit(fd, EV_SYN, SYN_REPORT, 0);
        emit(fd, EV_KEY, BTN_LEFT, 0);
        emit(fd, EV_SYN, SYN_REPORT, 0);

        usleep(1000);
    }
    
    /*
    * Give userspace some time to read the events before we destroy the
    * device with UI_DEV_DESTOY.
    */
    sleep(1);

    ioctl(fd, UI_DEV_DESTROY);

    close(fd);
    return 0;
}
