#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <curses.h>
#include <signal.h>
#include <ctype.h>
#include <stdarg.h>

#include "sst.h"
#include "sstlinux.h"

static int rows, linecount;	/* for paging */

WINDOW *curwnd;

static void outro(void)
/* wrap up, either normally or due to signal */
{
    if (game.options & OPTION_CURSES) {
	clear();
	curs_set(1);
	(void)refresh();
	(void)resetterm();
	//(void)echo();
	(void)endwin();
	putchar('\n');
    }
}

void iostart(void) 
{
    if (!(game.options & OPTION_CURSES)) {
	rows = atoi(getenv("LINES"));
    } else {
	if (atexit(outro)){
	    fprintf(stderr,"Unable to register outro(), exiting...\n");
	    exit(1);
	}
	(void)initscr();
#ifdef KEY_MIN
	keypad(stdscr, TRUE);
#endif /* KEY_MIN */
	(void)saveterm();
	(void)nonl();
	(void)cbreak();
#ifdef A_COLOR
	{
	    start_color();
	    init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
	    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
	    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
	    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
	    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	}
#endif /* A_COLOR */
	//(void)noecho();
	fullscreen_window = stdscr;
	srscan_window     = newwin(12, 25, 0,       0);
	report_window     = newwin(10, 0,  1,       25);
	lrscan_window     = newwin(10, 0,  0,       64); 
	message_window    = newwin(0,  0,  12,      0);
	prompt_window     = newwin(1,  0,  LINES-2, 0); 
	scrollok(message_window, TRUE);
	setwnd(fullscreen_window);
	textcolor(DEFAULT);
    }
}


void waitfor(void)
/* wait for user action -- OK to do nothing if on a TTY */
{
    if (game.options & OPTION_CURSES)
	getch();
}

void pause_game(int i) 
{
    char *prompt;
    char buf[BUFSIZ];
    if (i==1) {
	if (skill > SKILL_FAIR)
	    prompt = "[ANOUNCEMENT ARRIVING...]";
	else
	    prompt = "[IMPORTANT ANNOUNCEMENT ARRIVING -- PRESS ENTER TO CONTINUE]";
    }
    else {
	if (skill > SKILL_FAIR)
	    prompt = "[CONTINUE?]";
	else
	    prompt = "[PRESS ENTER TO CONTINUE]";

    }
    if (game.options & OPTION_CURSES) {
	drawmaps(0);
	setwnd(prompt_window);
	wclear(prompt_window);
	waddstr(prompt_window, prompt);
	wgetnstr(prompt_window, buf, sizeof(buf));
	wclear(prompt_window);
	wrefresh(prompt_window);
	setwnd(message_window);
    } else {
	putchar('\n');
	proutn(prompt);
	fgets(buf, sizeof(buf), stdin);
	if (i != 0) {
	    int j;
	    for (j = 0; j < rows; j++)
		putchar('\n');
	}
	linecount = 0;
    }
}


void skip(int i) 
{
    while (i-- > 0) {
	if (game.options & OPTION_CURSES) {
	    proutn("\n\r");
	} else {
	    linecount++;
	    if (linecount >= rows)
		pause_game(0);
	    else
		putchar('\n');
	}
    }
}

static void vproutn(char *fmt, va_list ap) 
{
    if (game.options & OPTION_CURSES) {
	vwprintw(curwnd, fmt, ap);
	wrefresh(curwnd);
    }
    else
	vprintf(fmt, ap);
}

void proutn(char *fmt, ...) 
{
    va_list ap;
    va_start(ap, fmt);
    vproutn(fmt, ap);
    va_end(ap);
}

void prout(char *fmt, ...) 
{
    va_list ap;
    va_start(ap, fmt);
    vproutn(fmt, ap);
    va_end(ap);
    skip(1);
}

void prouts(char *fmt, ...) 
/* print slowly! */
{
    char *s, buf[BUFSIZ];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    skip(1);
    for (s = buf; *s; s++) {
	delay(30);
	if (game.options & OPTION_CURSES) {
	    waddch(curwnd, *s);
	    wrefresh(curwnd);
	}
	else {
	    putchar(*s);
	    fflush(stdout);
	}
    }
}

void cgetline(char *line, int max)
{
    if (game.options & OPTION_CURSES) {
	wgetnstr(curwnd, line, max);
	strcat(line, "\n");
	wrefresh(curwnd);
    } else {
	fgets(line, max, stdin);
    }
    line[strlen(line)-1] = '\0';
}

void setwnd(WINDOW *wnd)
/* change windows -- OK for this to be a no-op in tty mode */
{
    if (game.options & OPTION_CURSES) {
     curwnd=wnd;
     curs_set(wnd == fullscreen_window || wnd == message_window || wnd == prompt_window);
    }
}

void clreol (void)
/* clear to end of line -- can be a no-op in tty mode */
{
   if (game.options & OPTION_CURSES) {
       wclrtoeol(curwnd);
       wrefresh(curwnd);
   }
}

void clrscr (void)
/* clear screen -- can be a no-op in tty mode */
{
   if (game.options & OPTION_CURSES) {
       wclear(curwnd);
       wmove(curwnd,0,0);
       wrefresh(curwnd);
   }
}

void textcolor (int color)
{
#ifdef A_COLOR
    if (game.options & OPTION_CURSES) {
	switch(color) {
	case DEFAULT: 
	    wattrset(curwnd, 0);
	    break;
	case BLACK: 
	    wattron(curwnd, COLOR_PAIR(COLOR_BLACK));
	    break;
	case BLUE: 
	    wattron(curwnd, COLOR_PAIR(COLOR_BLUE));
	    break;
	case GREEN: 
	    wattron(curwnd, COLOR_PAIR(COLOR_GREEN));
	    break;
	case CYAN: 
	    wattron(curwnd, COLOR_PAIR(COLOR_CYAN));
	    break;
	case RED: 
	    wattron(curwnd, COLOR_PAIR(COLOR_RED));
	    break;
	case MAGENTA: 
	    wattron(curwnd, COLOR_PAIR(COLOR_MAGENTA));
	    break;
	case BROWN: 
	    wattron(curwnd, COLOR_PAIR(COLOR_YELLOW));
	    break;
	case LIGHTGRAY: 
	    wattron(curwnd, COLOR_PAIR(COLOR_WHITE));
	    break;
	case DARKGRAY: 
	    wattron(curwnd, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
	    break;
	case LIGHTBLUE: 
	    wattron(curwnd, COLOR_PAIR(COLOR_BLUE) | A_BOLD);
	    break;
	case LIGHTGREEN: 
	    wattron(curwnd, COLOR_PAIR(COLOR_GREEN) | A_BOLD);
	    break;
	case LIGHTCYAN: 
	    wattron(curwnd, COLOR_PAIR(COLOR_CYAN) | A_BOLD);
	    break;
	case LIGHTRED: 
	    wattron(curwnd, COLOR_PAIR(COLOR_RED) | A_BOLD);
	    break;
	case LIGHTMAGENTA: 
	    wattron(curwnd, COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
	    break;
	case YELLOW: 
	    wattron(curwnd, COLOR_PAIR(COLOR_YELLOW) | A_BOLD);
	    break;
	case WHITE:
	    wattron(curwnd, COLOR_PAIR(COLOR_WHITE) | A_BOLD);
	    break;
	}
    }
#endif /* A_COLOR */
}

void highvideo (void)
{
    if (game.options & OPTION_CURSES) {
	wattron(curwnd, A_REVERSE);
    }
}
 
void commandhook(char *cmd, int before) {
}

/*
 * Things past this point have policy implications.
 */

void drawmaps(short l)
/* hook to be called after moving to redraw maps */
{
    if (game.options & OPTION_CURSES) {
	if (l == 1)
	    sensor();
	if (l != 2) {
	    setwnd(srscan_window);
	    wmove(curwnd, 0, 0);
	    enqueue("no");
	    srscan(SCAN_FULL);
	    setwnd(report_window);
	    wclear(report_window);
	    wmove(report_window, 0, 0);
	    srscan(SCAN_NO_LEFTSIDE);
	    setwnd(lrscan_window);
	    wclear(lrscan_window);
	    wmove(lrscan_window, 0, 0);
	    enqueue("l");
	    lrscan();
	}
    }
}

void boom(int ii, int jj)
/* enemy fall down, go boom */ 
{
    if (game.options & OPTION_CURSES) {
	setwnd(srscan_window);
	drawmaps(2);
	wmove(srscan_window, ii*2+3, jj+2);
	wattron(srscan_window, A_REVERSE);
	waddch(srscan_window, game.quad[ii][jj]);
	wrefresh(srscan_window);
	sound(500);
	delay(1000);
	nosound();
	wmove(srscan_window, ii*2+3, jj+2);
	wattroff(srscan_window, A_REVERSE);
	waddch(srscan_window, game.quad[ii][jj]);
	wrefresh(srscan_window);
	setwnd(message_window);
	delay(500);
    }
} 

void warble(void)
/* sound and visual effects for teleportation */
{
    if (game.options & OPTION_CURSES) {
	drawmaps(1);
	setwnd(message_window);
	sound(50);
	delay(1000);
	nosound();
    } else
	prouts(" . . . . . ");
}

void tracktorpedo(int x, int y, int ix, int iy, int wait, int l, int i, int n, int iquad)
/* torpedo-track animation */
{
    if (!game.options & OPTION_CURSES) {
	if (l == 1) {
	    if (n != 1) {
		skip(1);
		proutn("Track for torpedo number %d-  ", i);
	    }
	    else {
		skip(1);
		proutn("Torpedo track- ");
	    }
	} else if (l==4 || l==9) 
	    skip(1);
	proutn("%d - %d   ", (int)x, (int)y);
    } else {
	if (game.damage[DSRSENS]==0 || condit==IHDOCKED) {
	    drawmaps(2);
	    delay((wait!=1)*400);
	    if ((game.quad[ix][iy]==IHDOT)||(game.quad[ix][iy]==IHBLANK)){
		game.quad[ix][iy]='+';
		drawmaps(2);
		game.quad[ix][iy]=iquad;
		sound(l*10);
		delay(100);
		nosound();
	    }
	    else {
		game.quad[ix][iy] |= DAMAGED;
		drawmaps(2);
		game.quad[ix][iy]=iquad;
		sound(500);
		delay(1000);
		nosound();
		wattroff(curwnd, A_REVERSE);
	    }
	} else {
	    proutn("%d - %d   ", (int)x, (int)y);
	}
    }
}

void makechart(void) 
{
    if (game.options & OPTION_CURSES) {
	setwnd(message_window);
	wclear(message_window);
	chart(0);
    }
}

void setpassword(void) 
{
    if (!(game.options & OPTION_CURSES)) {
	while (TRUE) {
	    scan();
	    strcpy(game.passwd, citem);
	    chew();
	    if (*game.passwd != 0) break;
	    proutn("Please type in a secret password-");
	}
    } else {
	int i;
        for(i=0;i<3;i++) game.passwd[i]=(char)(97+(int)(Rand()*25));
        game.passwd[3]=0;
    }
}

