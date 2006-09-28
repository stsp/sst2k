#include <stdio.h>
#include <unistd.h>
#include <wchar.h>

#include "config.h"
#include "sst.h"
#include "sstlinux.h"

static int rows, linecount;	/* for paging */
static bool pause_latch;

WINDOW *curwnd;
WINDOW *fullscreen_window;
WINDOW *srscan_window;
WINDOW *report_window;
WINDOW *status_window;
WINDOW *lrscan_window;
WINDOW *message_window;
WINDOW *prompt_window;

static void outro(void)
/* wrap up, either normally or due to signal */
{
    if (game.options & OPTION_CURSES) {
	//clear();
	//curs_set(1);
	//refresh();
	//resetterm();
	//echo();
	endwin();
	putchar('\n');
    }
    if (logfp)
	fclose(logfp);
}

void iostart(void) 
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    if (atexit(outro)){
	fprintf(stderr,"Unable to register outro(), exiting...\n");
	exit(1);
    }
    if (!(game.options & OPTION_CURSES)) {
	char *ln_env = getenv("LINES");
	rows = ln_env ? atoi(ln_env) : 25;
    } else {
	initscr();
#ifdef KEY_MIN
	keypad(stdscr, TRUE);
#endif /* KEY_MIN */
	saveterm();
	nonl();
	cbreak();
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
	//noecho();
	fullscreen_window = stdscr;
	srscan_window     = newwin(12, 25, 0,       0);
	report_window     = newwin(11, 0,  1,       25);
	status_window     = newwin(10, 0,  1,       39);
	lrscan_window     = newwin(5,  0,  0,       64); 
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

void pause_reset(void)
{
    pause_latch = false;
}

void pause_game(bool announcement) 
{
    if (pause_latch)
	return;
    else {
	char *prompt;
	char buf[BUFSIZ];
	if (announcement) {
	    if (game.skill > SKILL_FAIR)
		prompt = _("[ANOUNCEMENT ARRIVING...]");
	    else
		prompt = _("[IMPORTANT ANNOUNCEMENT ARRIVING -- PRESS ENTER TO CONTINUE]");
	}
	else {
	    if (game.skill > SKILL_FAIR)
		prompt = _("[CONTINUE?]");
	    else
		prompt = _("[PRESS ENTER TO CONTINUE]");

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
	    if (announcement) {
		int j;
		for (j = 0; j < rows; j++)
		    putchar('\n');
	    }
	    linecount = 0;
	}
	pause_latch = true;
    }
}


void skip(int i) 
{
    while (i-- > 0) {
	if (game.options & OPTION_CURSES) {
	    if (curwnd == message_window && getcury(curwnd) >= getmaxy(curwnd) - 3) {
		pause_game(false);
		clrscr();
	    } else {
		proutn("\n");
	    }
	} else {
	    linecount++;
	    if (linecount >= rows)
		pause_game(false);
	    else
		putchar('\n');
	}
    }
}

static void vproutn(const char *fmt, va_list ap) 
{
    if (game.options & OPTION_CURSES) {
	vwprintw(curwnd, fmt, ap);
	wrefresh(curwnd);
    }
    else
	vprintf(fmt, ap);
}

void proutn(const char *fmt, ...) 
{
    va_list ap;
    va_start(ap, fmt);
    vproutn(fmt, ap);
    va_end(ap);
}

void prout(const char *fmt, ...) 
{
    va_list ap;
    va_start(ap, fmt);
    vproutn(fmt, ap);
    va_end(ap);
    skip(1);
}

void prouts(const char *fmt, ...) 
/* print slowly! */
{
    char buf[BUFSIZ];
    wchar_t *s, mbuf[BUFSIZ];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    mbstowcs(mbuf, buf, BUFSIZ);
    for (s = mbuf; *s; s++) {
	/* HOW to convince ncurses to use wchar_t?? */
	/* WHY putwchar() doesn't work?? */
	/* OK then, convert back to mbs... */
	char c[MB_CUR_MAX*2];
	int n;
	n = wctomb(c, *s);
	c[n] = 0;
	delay(30);
	proutn(c);
	if (game.options & OPTION_CURSES)
	    wrefresh(curwnd);
	else
	    fflush(stdout);
    }
    delay(300);
}

void cgetline(char *line, int max)
{
    if (game.options & OPTION_CURSES) {
	wgetnstr(curwnd, line, max);
	strcat(line, "\n");
	wrefresh(curwnd);
    } else {
	if (replayfp && !feof(replayfp))
	    fgets(line, max, replayfp);
	else
	    fgets(line, max, stdin);
    }
    if (logfp)
	fputs(line, logfp);
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

void clreol(void)
/* clear to end of line -- can be a no-op in tty mode */
{
   if (game.options & OPTION_CURSES) {
       wclrtoeol(curwnd);
       wrefresh(curwnd);
   }
}

void clrscr(void)
/* clear screen -- can be a no-op in tty mode */
{
   if (game.options & OPTION_CURSES) {
       wclear(curwnd);
       wmove(curwnd,0,0);
       wrefresh(curwnd);
   }
   linecount = 0;
}

void textcolor(int color)
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

void highvideo(void)
{
    if (game.options & OPTION_CURSES) {
	wattron(curwnd, A_REVERSE);
    }
}
 
void commandhook(char *cmd, bool before) {
}

/*
 * Things past this point have policy implications.
 */

void drawmaps(int mode)
/* hook to be called after moving to redraw maps */
{
    if (game.options & OPTION_CURSES) {
	if (mode == 1)
	    sensor();
        setwnd(srscan_window);
        wmove(curwnd, 0, 0);
        srscan();
	if (mode != 2) {
	    setwnd(status_window);
	    wclear(status_window);
	    wmove(status_window, 0, 0);
	    setwnd(report_window);
	    wclear(report_window);
	    wmove(report_window, 0, 0);
	    status(0);
	    setwnd(lrscan_window);
	    wclear(lrscan_window);
	    wmove(lrscan_window, 0, 0);
	    lrscan();
	}
    }
}

static void put_srscan_sym(coord w, char sym)
{
    wmove(srscan_window, w.x+1, w.y*2+2);
    waddch(srscan_window, sym);
    wrefresh(srscan_window);
}

void boom(coord w)
/* enemy fall down, go boom */ 
{
    if (game.options & OPTION_CURSES) {
	drawmaps(2);
	setwnd(srscan_window);
	wattron(srscan_window, A_REVERSE);
	put_srscan_sym(w, game.quad[w.x][w.y]);
	sound(500);
	delay(1000);
	nosound();
	wattroff(srscan_window, A_REVERSE);
	put_srscan_sym(w, game.quad[w.x][w.y]);
	delay(500);
	setwnd(message_window);
    }
} 

void warble(void)
/* sound and visual effects for teleportation */
{
    if (game.options & OPTION_CURSES) {
	drawmaps(2);
	setwnd(message_window);
	sound(50);
    }
    prouts("     . . . . .     ");
    if (game.options & OPTION_CURSES) {
	delay(1000);
	nosound();
    }
}

void tracktorpedo(coord w, int l, int i, int n, int iquad)
/* torpedo-track animation */
{
    if (!game.options & OPTION_CURSES) {
	if (l == 1) {
	    if (n != 1) {
		skip(1);
		proutn(_("Track for torpedo number %d-  "), i);
	    }
	    else {
		skip(1);
		proutn(_("Torpedo track- "));
	    }
	} else if (l==4 || l==9) 
	    skip(1);
	proutn("%d - %d   ", w.x, w.y);
    } else {
	if (!damaged(DSRSENS) || game.condition==docked) {
	    if (i != 1 && l == 1) {
		drawmaps(2);
		delay(400);
	    }
	    if ((iquad==IHDOT)||(iquad==IHBLANK)){
		put_srscan_sym(w, '+');
		sound(l*10);
		delay(100);
		nosound();
		put_srscan_sym(w, iquad);
	    }
	    else {
		wattron(curwnd, A_REVERSE);
		put_srscan_sym(w, iquad);
		sound(500);
		delay(1000);
		nosound();
		wattroff(curwnd, A_REVERSE);
		put_srscan_sym(w, iquad);
	    }
	} else {
	    proutn("%d - %d   ", w.x, w.y);
	}
    }
}

void makechart(void) 
{
    if (game.options & OPTION_CURSES) {
	setwnd(message_window);
	wclear(message_window);
    }
    chart();
    if (game.options & OPTION_TTY) {
	skip(1);
    }
}

void prstat(const char *txt, const char *fmt, ...)
{
#define NSYM 14
    int i;
    va_list args;
    proutn(txt);
    if (game.options & OPTION_CURSES) {
	skip(1);
    } else  {
	for (i = mblen(txt, strlen(txt)); i < NSYM; i++)
	    proutn(" ");
    }
    if (game.options & OPTION_CURSES)
	setwnd(status_window);
    va_start(args, fmt);
    vproutn(fmt, args);
    va_end(args);
    skip(1);
    if (game.options & OPTION_CURSES)
	setwnd(report_window);
}
