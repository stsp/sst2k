/***************************************************************************/
/** File: conio.h     Date: 03/09/1997       Version: 1.02                **/
/** --------------------------------------------------------------------- **/
/** CONIO.H an implementation of the conio.h for Linux based on ncurses.  **/
/** This is copyright (c) 1996,97 by Fractor / Mental EXPlosion.          **/
/** If you want to copy it you must do this following the terms of the    **/
/** GNU Library Public License                                            **/
/** Please read the file "README" before using this library.              **/
/** --------------------------------------------------------------------- **/
/** To use this thing you must have got the curses library.               **/
/** This thing was tested on Linux Kernel 2.0.29, GCC 2.7.2 and           **/
/** ncurses 1.9.9e which is (c) 1992-1995 by Zeyd M. Ben-Halim and Eric S.**/
/** Raymond.                                                              **/
/** Please also read the copyright notices for ncurses before using this !**/
/***************************************************************************/             

#ifndef __linux__
#error This conio.h was only tested to work under LINUX !
#endif

#ifndef __LINUXCONIO_H
#define __LINUXCONIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ncurses.h>

#define linux_conio_version 1.04

extern int directvideo; /* ignored by linux conio.h */
extern char color_warning; /* Tell the users when terminal can't display colors ? */
extern WINDOW *conio_scr;
 
#define _NOCURSOR      0
#define _NORMALCURSOR  1

struct text_info {
    unsigned char winleft;
    unsigned char wintop;
    unsigned char winright;
    unsigned char winbottom;
    unsigned char attribute;
    unsigned char normattr;
    unsigned char currmode;
    unsigned char screenheight;
    unsigned char screenwidth;
    unsigned char curx;
    unsigned char cury;
};

enum text_modes { LASTMODE=-1, BW40=0, C40, BW80, C80, MONO=7, C4350=64 };

enum COLORS {
   BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY,
   DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, YELLOW, WHITE
};

#define BLINK 128    

extern void initconio(void); /* Please run this function before any other */ 
extern void doneconio(void); /* Please run this function before exiting your program */

extern int wherex(void);
extern int wherey(void);
extern int putch(int c);
extern int getche(void); 
extern int kbhit(void);
extern void _setcursortype(int);
extern int cprintf(char *format, ...);
extern int cscanf(const char *format, ...); 

extern unsigned inp(unsigned port);
extern unsigned inpw(unsigned port);
extern unsigned outp(unsigned port, unsigned value);
extern unsigned outpw(unsigned port,unsigned value);
extern unsigned inpd(unsigned port);
extern unsigned outpd(unsigned port, unsigned value);

extern void clreol(void);
extern void clrscr(void);
extern void gotoxy(int x, int y);
extern void delline(void);
extern void gettextinfo(struct text_info *r);
extern void highvideo(void);
extern void insline(void);
extern void lowvideo(void);
extern void normvideo(void);
extern void textattr(int attribute);
extern void textbackground(int color);
extern void textcolor(int color);
extern void textmode(int unused_mode);
extern void window(int left, int top, int right, int bottom);
extern void cputs(char *str);

extern char *cgets(char *str); 

#ifdef  __cplusplus
}
#endif

#endif
