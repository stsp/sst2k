/****************************************************************************
 * This is the implementation file for conio.h - a conio.h for Linux.       *
 * It uses ncurses and some internal functions of Linux to simulate the     *
 * I/O-functions.                                                           *
 * This is copyright (c) 1996,97 by Fractor / Mental eXPlosion (MXP)        *
 * Use and distribution is only allowed if you follow the terms of the      *
 * GNU Library Public License Version 2.                                    *
 * Since this work is based on ncurses please read its copyright notices as *
 * well !                                                                   *
 * Look into the readme to this file for further information.               *
 * Thanx to SubZero / MXP for his little tutorial on the curses library !   *
 * Many thanks to Mark Hahn and Rich Cochran for solving the inpw and inpd  *
 * mega-bug !!!                                                             *
 * Watch out for other MXP releases, too !                                  *
 * Send bugreports to: fractor@germanymail.com                              *
 ****************************************************************************/

#define _ISOC99_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>
#include <ncurses.h>
#include "conio.h" 

static attr_t txtattr,oldattr;
static int fgc,bgc;
char color_warning=1;
int directvideo;
WINDOW *conio_scr;

#ifdef SERGEEV 
/* Some internals... */
static int colortab(int a) /* convert LINUX Color code to DOS-standard */
{
   switch(a) {
      case 0 : return COLOR_BLACK;
      case 1 : return COLOR_BLUE;
      case 2 : return COLOR_GREEN;
      case 3 : return COLOR_CYAN;
      case 4 : return COLOR_RED;
      case 5 : return COLOR_MAGENTA;
      case 6 : return COLOR_YELLOW;
      case 7 : return COLOR_WHITE;
   }
   return COLOR_BLACK;
} 
#endif /* SERGEEV */

static void docolor (int color) /* Set DOS-like text mode colors */
{
   wattrset(conio_scr,0); /* My curses-version needs this ... */
   if ((color&128)==128) txtattr=A_BLINK; else txtattr=A_NORMAL;
   if ((color&15)>7) txtattr|=A_STANDOUT; else txtattr|=A_NORMAL;
   txtattr|=COLOR_PAIR((1+(color&7)+(color&112)) >> 1);
   if (((color&127)==15) | ((color&127)==7)) txtattr=COLOR_PAIR(0);
   if (((color&127)==127) | ((color&127)==119)) txtattr=COLOR_PAIR(8);
   wattron(conio_scr,txtattr);
   wattron(conio_scr,COLOR_PAIR(1+(color&7)+((color&112)>>1))); 
}

#ifdef SERGEEV
/* Call this before any call to linux conio - except the port functions ! */
void __attribute__((constructor)) initconio (void) /* This is needed, because ncurses needs to be initialized */
{
   int x,y;
   short pair;
   if (atexit(doneconio)){
      fprintf(stderr,"Unable to register doneconio(), exiting...\n");
      exit(1);
   }
   initscr();
   start_color();
//   attr_get(&oldattr,&pair,NULL);
   nonl();
   raw();
   if (!has_colors() & (color_warning>-1))
      fprintf(stderr,"Attention: A color terminal may be required to run this application !\n");   
   noecho();
   conio_scr=newwin(0,0,0,0);
   keypad(conio_scr,TRUE);
   meta(conio_scr,TRUE);
   idlok(conio_scr,TRUE);
   scrollok(conio_scr,TRUE);
   /* Color initialization */
   for (y=0;y<=7;y++)
      for (x=0;x<=7;x++)
         init_pair((8*y)+x+1, colortab(x), colortab(y));              
   wattr_get(conio_scr,&txtattr,&pair,NULL);
   bgc=0;
   textcolor(7);
   textbackground(0);
}
#endif /* SERGEEV */

/* Call this on exiting your program */
void doneconio (void)
{
   endwin();
}

/* Here it starts... */
void _setcursortype (int cur_t)
{
   curs_set(cur_t);
   wrefresh(conio_scr);
}

char *cgets (char *str) /* ugly function :-( */
{
   char strng[257];
   unsigned char i=2;
   echo();
   strncpy(strng,str,1);
   wgetnstr(conio_scr,&strng[2],(int) strng[0]);
   while (strng[i]!=0) i++;
   i=i-2;
   strng[1]=i;
   memcpy(str,strng,i+3);
   noecho();
   return(&str[2]);
}

void clreol (void)
/* clear to end of line -- can be a no-op in tty mode */
{
#ifdef SERGEEV
   wclrtoeol(conio_scr);
   wrefresh(conio_scr);
#endif /* SERGEEV */
}

void clrscr (void)
{
   wclear(conio_scr);
   wmove(conio_scr,0,0);
   wrefresh(conio_scr);
}

int cprintf (char *format, ... )
{
   int i;
   char buffer[BUFSIZ]; /* Well, BUFSIZ is from ncurses...  */
   va_list argp;
   va_start(argp,format);
   vsprintf(buffer,format,argp);
   va_end(argp);
   i=waddstr(conio_scr,buffer);
   wrefresh(conio_scr);
   return(i);
}

void cputs (char *str)
{
   waddstr(conio_scr,str);
   wrefresh(conio_scr);
}

int cscanf (const char *format, ...)
{
   int i;
   char buffer[BUFSIZ]; /* See cprintf */
   va_list argp;
   echo();
   if (wgetstr(conio_scr,buffer)==ERR) return(-1);                    
   va_start(argp,format);
   i=vsscanf(buffer,format,argp);                         
   va_end(argp);
   noecho();
   return(i);
}

void delline (void)
{
   wdeleteln(conio_scr);
   wrefresh(conio_scr);
}

int getche (void)
{
   int i;
   echo();
   i=wgetch(conio_scr);
   if (i==-1) i=0;
   noecho();
   return(i);
}

void gettextinfo(struct text_info *inforec)
{
   short pair;
   unsigned char xp,yp;
   unsigned char x1,x2,y1,y2;
   attr_t dattr,dnattr,a; /* The "d" stands for DOS */
   getyx(conio_scr,yp,xp);
   getbegyx(conio_scr,y1,x1);
   getmaxyx(conio_scr,y2,x2);
   dattr=(bgc*16)+fgc;
   wattr_get(conio_scr,&a,&pair,NULL);
   if (a==(a & A_BLINK)) dattr=dattr+128;
   dnattr=oldattr;  /* Well, this cannot be right, 
                       because we don't know the COLORPAIR values from before initconio() !*/
   inforec->winleft=x1+1;
   inforec->wintop=y1+1;
   if (x1==0) x2--;
   if (y1==0) y2--;
   inforec->winright=x1+x2+1;
   inforec->winbottom=y1+y2+1;
   inforec->curx=xp+1;
   inforec->cury=yp+1;
   inforec->screenheight=y2+1;
   inforec->screenwidth=x2+1;
   inforec->currmode=3; /* This is C80 */
   inforec->normattr=dnattr; /* Don't use this ! */
   inforec->attribute=dattr;
} 

void gotoxy (int x, int y)
/* address cusor -- OK for this to be a no-op in TTY mode */
{
#ifdef SERGEEV
   y--;
   x--;
   wmove(conio_scr,y,x);
   wrefresh(conio_scr);
#endif /* SERGEEV */
}

void highvideo (void)
{
#ifdef SERGEEV
   textcolor(15); /* White */
   textbackground(0); /* Black */
#endif /* SERGEEV */
}

void insline (void)
{ 
   winsertln(conio_scr);
   wrefresh(conio_scr);
}

int kbhit (void)
{
   int i;
   nodelay(conio_scr,TRUE);
   i=wgetch(conio_scr);
   nodelay(conio_scr,FALSE);
   if (i==-1) i=0;
   return(i);
}

void lowvideo (void)
{
   textbackground(0); /* Black */
   textcolor(8); /* Should be darkgray */
}

void normvideo (void)
{
   wattrset(conio_scr,oldattr);
}

int putch (int c)
{
   if (waddch(conio_scr,c)!=ERR) {
      wrefresh(conio_scr); 
      return(c);
   }
   return(0);
}
                                     
void textattr (int attr)
{
   docolor(attr);
}

void textbackground (int color)
{
   bgc=color;
   color=(bgc*16)+fgc;
   docolor(color);
}


void textcolor (int color)
{
#ifdef SERGEEV
   fgc=color;
   color=(bgc*16)+fgc;
   docolor(color);
#endif /* SERGEEV */
}
 
void textmode (int mode)
{
   /* Ignored */
}

int wherex (void)
{
   int y;
   int x;
   getyx(conio_scr,y,x);
   x++;
   return(x);
}

int wherey (void)
{
   int y;
   int x;
   getyx(conio_scr,y,x);
   y++;
   return(y);
}

void window (int left,int top,int right,int bottom)
{
   int nlines,ncols;
   delwin(conio_scr);
   top--;
   left--;
   right--;
   bottom--;
   nlines=bottom-top;
   ncols=right-left;
   if (top==0) nlines++;
   if (left==0) ncols++;
   if ((nlines<1) | (ncols<1)) return;
   conio_scr=newwin(nlines,ncols,top,left);   
   keypad(conio_scr,TRUE);
   meta(conio_scr,TRUE);
   idlok(conio_scr,TRUE);
   scrollok(conio_scr,TRUE);
   wrefresh(conio_scr);
}

/* Linux is cool */
