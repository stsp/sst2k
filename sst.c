#define INCLUDED	// Define externs here
#include "sst.h"
#include <ctype.h>
#include <stdarg.h>
#ifdef MSDOS
#include <dos.h>
#endif
#include <time.h>
	
int getch(void);

static char line[128], *linep = line;
static int linecount;	/* for paging */

static void clearscreen(void);

/* Compared to original version, I've changed the "help" command to
   "call" and the "terminate" command to "quit" to better match
   user expectations. The DECUS version apparently made those changes
   as well as changing "freeze" to "save". However I like "freeze".

   When I got a later version of Super Star Trek that I was converting
   from, I added the emexit command.

   That later version also mentions srscan and lrscan working when
   docked (using the starbase's scanners), so I made some changes here
   to do this (and indicating that fact to the player), and then realized
   the base would have a subspace radio as well -- doing a Chart when docked
   updates the star chart, and all radio reports will be heard. The Dock
   command will also give a report if a base is under attack.

   Movecom no longer reports movement if sensors are damaged so you wouldn't
   otherwise know it.

   Also added:

   1. Better base positioning at startup

   2. deathray improvement (but keeping original failure alternatives)

   3. Tholian Web

   4. Enemies can ram the Enterprise. Regular Klingons and Romulans can
      move in Expert and Emeritus games. This code could use improvement.

   5. The deep space probe looks interesting! DECUS version

   6. Perhaps cloaking to be added later? BSD version


   */


static char *commands[] = {
	"srscan",
	"lrscan",
	"phasers",
	"photons",
	"move",
	"shields",
	"dock",
	"damages",
	"chart",
	"impulse",
	"rest",
	"warp",
	"status",
	"sensors",
	"orbit",
	"transport",
	"mine",
	"crystals",
	"shuttle",
	"planets",
	"request",
	"report",
	"computer",
	"commands",
    "emexit",
    "probe",
	"abandon",
	"destruct",
	"freeze",
	"deathray",
	"debug",
	"call",
	"quit",
	"help"
};
#define NUMCOMMANDS	sizeof(commands)/sizeof(commands[0])

static void listCommands(int x) {
	prout("   SRSCAN    MOVE      PHASERS   CALL\n"
		  "   STATUS    IMPULSE   PHOTONS   ABANDON\n"
		  "   LRSCAN    WARP      SHIELDS   DESTRUCT\n"
		  "   CHART     REST      DOCK      QUIT\n"
		  "   DAMAGES   REPORT    SENSORS   ORBIT\n"
		  "   TRANSPORT MINE      CRYSTALS  SHUTTLE\n"
		  "   PLANETS   REQUEST   DEATHRAY  FREEZE\n"
		  "   COMPUTER  EMEXIT    PROBE     COMMANDS");
	if (x) prout("   HELP");
}

static void helpme(void) {
	int i, j;
	char cmdbuf[32];
	char linebuf[132];
	FILE *fp;
	/* Give help on commands */
	int key;
	key = scan();
	while (TRUE) {
		if (key == IHEOL) {
			proutn("Help on what command?");
			key = scan();
		}
		if (key == IHEOL) return;
		for (i = 0; i < NUMCOMMANDS; i++) {
			if (strcmp(commands[i], citem)==0) break;
		}
		if (i != NUMCOMMANDS) break;
		skip(1);
		prout("Valid commands:");
		listCommands(FALSE);
		key = IHEOL;
		chew();
		skip(1);
	}
	if (i == 23) {
		strcpy(cmdbuf, " ABBREV");
	}
	else {
		strcpy(cmdbuf, "  Mnemonic:  ");
		j = 0;
		while ((cmdbuf[j+13] = toupper(commands[i][j])) != 0) j++;
	}
	fp = fopen("sst.doc", "r");
	if (fp == NULL) {
		prout("Spock-  \"Captain, that information is missing from the");
		prout("   computer. You need to find SST.DOC and put it in the");
		prout("   current directory.\"");
		return;
	}
	i = strlen(cmdbuf);
	do {
		if (fgets(linebuf, 132, fp) == NULL) {
			prout("Spock- \"Captain, there is no information on that command.\"");
			fclose(fp);
			return;
		}
	} while (strncmp(linebuf, cmdbuf, i) != 0);

	skip(1);
	prout("Spock- \"Captain, I've found the following information:\"");
	skip(1);

	do {
		if (linebuf[0]!=12) { // ignore page break lines 
			linebuf[strlen(linebuf)-1] = '\0'; // No \n at end
			prout(linebuf);
		}
		fgets(linebuf,132,fp);
	} while (strstr(linebuf, "******")==NULL);
	fclose(fp);
}

static void makemoves(void) {
	int i, hitme;
	char ch;
	while (TRUE) { /* command loop */
		hitme = FALSE;
		justin = 0;
		Time = 0.0;
		i = -1;
		while (TRUE)  { /* get a command */
			chew();
			skip(1);
			proutn("COMMAND> ");
			if (scan() == IHEOL) continue;
			for (i=0; i < 26; i++)
				if (isit(commands[i]))
					break;
			if (i < 26) break;
			for (; i < NUMCOMMANDS; i++)
				if (strcmp(commands[i], citem) == 0) break;
			if (i < NUMCOMMANDS) break;

			if (skill <= 2)  {
				prout("UNRECOGNIZED COMMAND. LEGAL COMMANDS ARE:");
				listCommands(TRUE);
			}
			else prout("UNRECOGNIZED COMMAND.");
		}
		switch (i) { /* command switch */
			case 0:			// srscan
				srscan(1);
				break;
			case 1:			// lrscan
				lrscan();
				break;
			case 2:			// phasers
				phasers();
				if (ididit) hitme = TRUE;
				break;
			case 3:			// photons
				photon();
				if (ididit) hitme = TRUE;
				break;
			case 4:			// move
				warp(1);
				break;
			case 5:			// shields
				doshield(1);
				if (ididit) {
					attack(2);
					shldchg = 0;
				}
				break;
			case 6:			// dock
				dock();
				break;
			case 7:			// damages
				dreprt();
				break;
			case 8:			// chart
				chart(0);
				break;
			case 9:			// impulse
				impuls();
				break;
			case 10:		// rest
				wait();
				if (ididit) hitme = TRUE;
				break;
			case 11:		// warp
				setwrp();
				break;
			case 12:		// status
				srscan(3);
				break;
			case 13:			// sensors
				sensor();
				break;
			case 14:			// orbit
				orbit();
				if (ididit) hitme = TRUE;
				break;
			case 15:			// transport "beam"
				beam();
				break;
			case 16:			// mine
				mine();
				if (ididit) hitme = TRUE;
				break;
			case 17:			// crystals
				usecrystals();
				break;
			case 18:			// shuttle
				shuttle();
				if (ididit) hitme = TRUE;
				break;
			case 19:			// Planet list
				preport();
				break;
			case 20:			// Status information
				srscan(2);
				break;
			case 21:			// Game Report 
				report(0);
				break;
			case 22:			// use COMPUTER!
				eta();
				break;
			case 23:
				listCommands(TRUE);
				break;
			case 24:		// Emergency exit
				clearscreen();	// Hide screen
				freeze(TRUE);	// forced save
				exit(1);		// And quick exit
				break;
			case 25:
				probe();		// Launch probe
				break;
			case 26:			// Abandon Ship
				abandn();
				break;
			case 27:			// Self Destruct
				dstrct();
				break;
			case 28:			// Save Game
				freeze(FALSE);
				if (skill > 3)
					prout("WARNING--Frozen games produce no plaques!");
				break;
			case 29:			// Try a desparation measure
				deathray();
				if (ididit) hitme = TRUE;
				break;
			case 30:			// What do we want for debug???
#ifdef DEBUG
				debugme();
#endif
				break;
			case 31:		// Call for help
				help();
				break;
			case 32:
				alldone = 1;	// quit the game
#ifdef DEBUG
				if (idebug) score();
#endif
				break;
			case 33:
				helpme();	// get help
				break;
		}
		for (;;) {
			if (alldone) break;		// Game has ended
#ifdef DEBUG
			if (idebug) prout("2500");
#endif
			if (Time != 0.0) {
				events();
				if (alldone) break;		// Events did us in
			}
			if (game.state.galaxy[quadx][quady] == 1000) { // Galaxy went Nova!
				atover(0);
				continue;
			}
			if (nenhere == 0) movetho();
			if (hitme && justin==0) {
				attack(2);
				if (alldone) break;
				if (game.state.galaxy[quadx][quady] == 1000) {	// went NOVA! 
					atover(0);
					hitme = TRUE;
					continue;
				}
			}
			break;
		}
		if (alldone) break;
	}
}


int main(int argc, char **argv) {
	int i;
	int hitme;
	char ch;
	prelim();

	if (argc > 1) {
		fromcommandline = 1;
		line[0] = '\0';
		while (--argc > 0) {
			strcat(line, *(++argv));
			strcat(line, " ");
		}
	}
	else fromcommandline = 0;


	while (TRUE) { /* Play a game */
		setup();
		if (alldone) {
			score();
			alldone = 0;
		}
		else makemoves();
		skip(2);
		stars();
		skip(1);

		if (tourn && alldone) {
			proutn("Do you want your score recorded?");
			if (ja()) {
				chew2();
				freeze(FALSE);
			}
		}
		proutn("Do you want to play again?");
		if (!ja()) break;
	}
	skip(1);
	prout("May the Great Bird of the Galaxy roost upon your home planet.");
}


void cramen(int i) {
	/* return an enemy */
	char *s;
	
	switch (i) {
		case IHR: s = "Romulan"; break;
		case IHK: s = "Klingon"; break;
		case IHC: s = "Commander"; break;
		case IHS: s = "Super-commander"; break;
		case IHSTAR: s = "Star"; break;
		case IHP: s = "Planet"; break;
		case IHB: s = "Starbase"; break;
		case IHBLANK: s = "Black hole"; break;
		case IHT: s = "Tholian"; break;
		case IHWEB: s = "Tholian web"; break;
		default: s = "Unknown??"; break;
	}
	proutn(s);
}

void cramlc(int key, int x, int y) {
	if (key == 1) proutn(" Quadrant");
	else if (key == 2) proutn(" Sector");
	proutn(" %d - %d", x, y);
}

void crmena(int i, int enemy, int key, int x, int y) {
	if (i == 1) proutn("***");
	cramen(enemy);
	proutn(" at");
	cramlc(key, x, y);
}

void crmshp(void) {
	char *s;
	switch (ship) {
		case IHE: s = "Enterprise"; break;
		case IHF: s = "Faerie Queene"; break;
		default:  s = "Ship???"; break;
	}
	proutn(s);
}

void stars(void) {
	prouts("******************************************************");
	skip(1);
}

double expran(double avrage) {
	return -avrage*log(1e-7 + Rand());
}

double Rand(void) {
	return rand()/(1.0 + (double)RAND_MAX);
}

void iran8(int *i, int *j) {
	*i = Rand()*8.0 + 1.0;
	*j = Rand()*8.0 + 1.0;
}

void iran10(int *i, int *j) {
	*i = Rand()*10.0 + 1.0;
	*j = Rand()*10.0 + 1.0;
}

void chew(void) {
	linecount = 0;
	linep = line;
	*linep = 0;
}

void chew2(void) {
	/* return IHEOL next time */
	linecount = 0;
	linep = line+1;
	*linep = 0;
}

int scan(void) {
	int i;
	char *cp;

	linecount = 0;

	// Init result
	aaitem = 0.0;
	*citem = 0;

	// Read a line if nothing here
	if (*linep == 0) {
		if (linep != line) {
			chew();
			return IHEOL;
		}
		fgets(line, sizeof(line), stdin);
		line[strlen(line)-1] = '\0';
		linep = line;
	}
	// Skip leading white space
	while (*linep == ' ') linep++;
	// Nothing left
	if (*linep == 0) {
		chew();
		return IHEOL;
	}
	if (isdigit(*linep) || *linep=='+' || *linep=='-' || *linep=='.') {
		// treat as a number
	    if (sscanf(linep, "%lf%n", &aaitem, &i) < 1) {
		linep = line; // Invalid numbers are ignored
		*linep = 0;
		return IHEOL;
	    }
	    else {
		// skip to end
		linep += i;
		return IHREAL;
	    }
	}
	// Treat as alpha
	cp = citem;
	while (*linep && *linep!=' ') {
		if ((cp - citem) < 9) *cp++ = tolower(*linep);
		linep++;
	}
	*cp = 0;
	return IHALPHA;
}

int ja(void) {
	chew();
	while (TRUE) {
		scan();
		chew();
		if (*citem == 'y') return TRUE;
		if (*citem == 'n') return FALSE;
		proutn("Please answer with \"Y\" or \"N\":");
	}
}

void cramf(double x, int w, int d) {
	char buf[64];
	sprintf(buf, "%*.*f", w, d, x);
	proutn(buf);
}

double square(double i) { return i*i; }
									
static void clearscreen(void) {
	/* Somehow we need to clear the screen */
#ifdef __BORLANDC__
	extern void clrscr(void);
	clrscr();
#else
	proutn("\033[2J");	/* Hope for an ANSI display */
#endif
}

/* We will pull these out in case we want to do something special later */

void pause(int i) {
	putchar('\n');
	if (i==1) {
		if (skill > 2)
			prout("[ANOUNCEMENT ARRIVING...]");
		else
			prout("[IMPORTANT ANNOUNCEMENT ARRIVING -- HIT SPACE BAR TO CONTINUE]");
		getch();
	}
	else {
		if (skill > 2)
			proutn("[CONTINUE?]");
		else
			proutn("[HIT SPACE BAR TO CONTINUE]");
		getch();
		proutn("\r                           \r");
	}
	if (i != 0) {
		clearscreen();
	}
	linecount = 0;
}


void skip(int i) {
	while (i-- > 0) {
		linecount++;
		if (linecount >= 23)
			pause(0);
		else
			putchar('\n');
	}
}


void proutn(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void prout(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
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
		while (clock() < endTime) ;
		putchar(*s);
		fflush(stdout);
	}
}

void huh(void) {
	chew();
	skip(1);
	prout("Beg your pardon, Captain?");
}

int isit(char *s) {
	/* New function -- compares s to scaned citem and returns true if it
	   matches to the length of s */

	return strncmp(s, citem, max(1, strlen(citem))) == 0;

}

#ifdef DEBUG
void debugme(void) {
	proutn("Reset levels? ");
	if (ja() != 0) {
		if (energy < inenrg) energy = inenrg;
		shield = inshld;
		torps = intorps;
		lsupres = inlsr;
	}
	proutn("Reset damage? ");
	if (ja() != 0) {
		int i;
		for (i=0; i <= NDEVICES; i++) if (damage[i] > 0.0) damage[i] = 0.0;
		stdamtim = 1e30;
	}
	proutn("Toggle idebug? ");
	if (ja() != 0) {
		idebug = !idebug;
		if (idebug) prout("Debug output ON");
		else prout("Debug output OFF");
	}
	proutn("Cause selective damage? ");
	if (ja() != 0) {
		int i, key;
		for (i=1; i <= NDEVICES; i++) {
			proutn("Kill ");
			proutn(device[i]);
			proutn("? ");
			chew();
			key = scan();
			if (key == IHALPHA &&  isit("y")) {
				damage[i] = 10.0;
				if (i == DRADIO) stdamtim = game.state.date;
			}
		}
	}
	proutn("Examine/change events? ");
	if (ja() != 0) {
		int i;
		for (i = 1; i < NEVENTS; i++) {
			int key;
			if (future[i] == 1e30) continue;
			switch (i) {
				case FSNOVA:  proutn("Supernova       "); break;
				case FTBEAM:  proutn("T Beam          "); break;
				case FSNAP:   proutn("Snapshot        "); break;
				case FBATTAK: proutn("Base Attack     "); break;
				case FCDBAS:  proutn("Base Destroy    "); break;
				case FSCMOVE: proutn("SC Move         "); break;
				case FSCDBAS: proutn("SC Base Destroy "); break;
			}
			cramf(future[i]-game.state.date, 8, 2);
			chew();
			proutn("  ?");
			key = scan();
			if (key == IHREAL) {
				future[i] = game.state.date + aaitem;
			}
		}
		chew();
	}
}
			

#endif
