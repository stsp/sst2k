#ifndef __SSTLINUX_H
#define __SSTLINUX_H

int c_printf (char *format, ... );
void sound(unsigned int);
void nosound(void);

#ifdef SERGEEV
extern WINDOW *conio_scr;
#endif /* SERGEEV */

#define delay(x) usleep(x*1000)
#define randomize() srand((unsigned)time(NULL))

#endif
