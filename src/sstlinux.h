#ifndef __SSTLINUX_H
#define __SSTLINUX_H

void sound(unsigned int);
void nosound(void);

#define delay(x) usleep(x*1000)

#endif
