#ifndef __SSTLINUX_H
#define __SSTLINUX_H

void sound(unsigned int);
void nosound(void);

#define delay(x) usleep(x*1000)
#define randomize() srand((unsigned)time(NULL))

#endif
