#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include "sstlinux.h"

static int fd = 0;

void sound(unsigned int freq)
{
    if(fd==0) fd=open("/dev/console", O_RDONLY);
    if(fd>0) ioctl(fd, KDMKTONE, 1193180/freq + (0xFFFF<<16));
}

void nosound(void)
{
    if(fd>0) ioctl(fd, KDMKTONE, 0);
}
