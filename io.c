#include <stdio.h>
#include <termios.h>
#include <curses.h>
#include <signal.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef MSDOS
#include <dos.h>
#endif
#include <time.h>

#include "sst.h"

static int linecount;	/* for paging */
static int screenheight = 24, screenwidth = 80;
static int curses = FALSE;

static void outro(int sig) {
/* wrap up, either normally or due to signal */
    if (curses) {
	clear();
	(void)refresh();
	(void)resetterm();
	//(void)echo();
	(void)endwin();
    }
}

static void fastexit(int sig) {
    outro(sig);
    putchar('\n');
    exit(0);
}

void iostart(int usecurses) {
	(void) signal(SIGINT, fastexit);
	(void) signal(SIGINT, fastexit);
#ifdef SIGIOT
	(void) signal(SIGIOT,fastexit);		/* for assert(3) */
#endif /* SIGIOT */
	if(signal(SIGQUIT,SIG_IGN) != SIG_IGN)
	    (void)signal(SIGQUIT,fastexit);

	if (curses = usecurses) {
		(void)initscr();
#ifdef KEY_MIN
		keypad(stdscr, TRUE);
#endif /* KEY_MIN */
		(void)saveterm();
		(void)nonl();
		(void)cbreak();
		//(void)noecho();
		scrollok(stdscr, TRUE);
		getmaxyx(stdscr, screenheight, screenwidth);
	} else {
		char *LINES = getenv("LINES");
		if (LINES)
		    screenheight = atoi(LINES);
	}
}

void ioend(void) {
    outro(0);
}

void clearscreen(void) {
	/* Somehow we need to clear the screen */
#ifdef __BORLANDC__
	extern void clrscr(void);
	clrscr();
#else
	if (curses) {
	    wclear(stdscr);
	    wrefresh(stdscr);
	} else {
		// proutn("\033[2J"); /* Hope for an ANSI display */
		/* much more in that old-TTY spirit to just throw linefeeds */
		int i;
		for (i = 0; i < screenheight; i++)
		    putchar('\n');
	}
#endif
}

void pause(int i) {
    char buf[BUFSIZ], *prompt;
	if (i==1) {
		if (skill > 2)
			prompt = "[ANOUNCEMENT ARRIVING...]";
		else
			prompt = "[IMPORTANT ANNOUNCEMENT ARRIVING -- PRESS ENTER TO CONTINUE]";
	}
	else {
	    	if (skill > 2)
	    		prompt = "[CONTINUE?]";
	    	else
			prompt = "[PRESS ENTER TO CONTINUE]";

	}
	if (curses) {
	    waddch(stdscr, '\n');
		waddstr(stdscr, prompt);
		wgetnstr(stdscr, buf, sizeof(buf));
		wclear(stdscr);
		wrefresh(stdscr);
	} else {
		putchar('\n');
		prout(prompt);
		fgets(buf, sizeof(buf), stdin);
		if (i != 0) {
			clearscreen();
		}
		linecount = 0;
	}
}


void skip(int i) {
    while (i-- > 0) {
	if (curses) {
	    int y, x;
	    getyx(stdscr, y, x);
	    if (y == screenheight-1)
		pause(0);
	    else
		waddch(stdscr, '\n');
	} else {
	    linecount++;
	    if (linecount >= screenheight)
		pause(0);
	    else
		putchar('\n');
	}
    }
}

void proutn(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if (curses) {
	vw_printw(stdscr, fmt, ap);
        wrefresh(stdscr);
    } else
	vprintf(fmt, ap);
    va_end(ap);
}

void prout(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if (curses) {
	vw_printw(stdscr, fmt, ap);
	wrefresh(stdscr);
    } else
	vprintf(fmt, ap);
    va_end(ap);
    skip(1);
}

void proutc(char *line) {
    line[strlen(line)-1] = '\0';
    if (curses)
	waddstr(stdscr, line);
    else
	fputs(line, stdout);
    skip(1);
}

void prouts(char *fmt, ...) {
	clock_t endTime;
	char *s, buf[BUFSIZ];
	/* print slowly! */
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	skip(1);
	for (s = buf; *s; s++) {
		endTime = clock() + CLOCKS_PER_SEC*0.05;
		while (clock() < endTime) continue;
		if (curses) {
		    waddch(stdscr, *s);
		    wrefresh(stdscr);
		}
		else {
		    putchar(*s);
		    fflush(stdout);
		}
	}
}

void getline(char *line, int max) {
    if (curses) {
	wgetnstr(stdscr, line, max);
	wrefresh(stdscr);
    } else {
	fgets(line, max, stdin);
        line[strlen(line)-1] = '\0';
    }
}

void commandhook(char *cmd, int before) {
}
