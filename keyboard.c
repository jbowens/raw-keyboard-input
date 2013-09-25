#include<fcntl.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<sys/kd.h>
#include<sys/types.h>
#include<termios.h>
#include<unistd.h>

int tty_fd = 0;
struct termios origtermios;
struct termios newtermios;

void sighandler(int sig)
{
    // Restore the old settings
    tcsetattr(tty_fd, TCSAFLUSH, &origtermios);
}

int main(int argc, char **argv)
{
    fd_set inset;

    if (tcgetattr(tty_fd, &origtermios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    newtermios = origtermios;
    newtermios.c_iflag &= ~(BRKINT);
    newtermios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    
    if (tcsetattr(tty_fd, TCSAFLUSH, &newtermios) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    signal(SIGTERM, sighandler);

    if (ioctl(tty_fd, KDSKBMODE, K_RAW) == -1) {
        sighandler(0);
        perror("ioctl raw mode");
        exit(EXIT_FAILURE);
    }

    int scan_code;

    for (;;)
    {
        FD_ZERO(&inset);
        FD_SET(tty_fd, &inset);

        int found = select(1, &inset, 0, 0, 0);

        if (FD_ISSET(tty_fd, &inset))
        {
            ssize_t bytes_read = read(tty_fd, &scan_code, 1);
            if (bytes_read == 0)
            {
                sighandler(0);
                exit(EXIT_SUCCESS);
            }
            printf("Read scan_code %d\n", scan_code);
        }
    }

    close(tty_fd);
}
