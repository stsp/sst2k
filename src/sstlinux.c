#include "config.h"
#include "sstlinux.h"

#ifdef HAVE_LINUX_KD_H

#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

static int fd = 0;
#endif

void sound(unsigned int freq)
{
#ifdef HAVE_LINUX_KD_H
    if(fd==0) fd=open("/dev/console", O_RDONLY);
    if(fd>0) ioctl(fd, KDMKTONE, 1193180/freq + (0xFFFF<<16));
#endif
}

void nosound(void)
{
#ifdef HAVE_LINUX_KD_H
    if(fd>0) ioctl(fd, KDMKTONE, 0);
#endif
}
