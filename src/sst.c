#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include "sstlinux.h"
#include "sst.h"

#define DOC_NAME "sst.doc"
	
/*

Dave Matuszek says:

   SRSCAN, MOVE, PHASERS, CALL, STATUS, IMPULSE, PHOTONS, ABANDON,
   LRSCAN, WARP, SHIELDS, DESTRUCT, CHART, REST, DOCK, QUIT, and DAMAGE
   were in the original non-"super" version of UT FORTRAN Star Trek.

   Tholians weren't in the original. Dave is dubious about their merits.
   (They are now controlled by OPTION_THOLIAN and turned off if the game
   type is "plain".)

   Planets and dilithium crystals weren't in the original.  Dave is OK
   with this idea. (It's now controlled by OPTION_PLANETS and turned 
   off if the game type is "plain".)

   Dave says the bit about the Galileo getting turned into a
   McDonald's is "consistant with our original vision".  (This has been
   left permanently enabled, as it can only happen if OPTION_PLANETS
   is on.)

   Dave also says the Space Thingy should not be preserved across saved
   games, so you can't prove to others that you've seen it.  He says it
   shouldn't fire back, either.  It should do nothing except scream and
   disappear when hit by photon torpedos.  It's OK that it may move
   when attacked, but it didn't in the original.  (Whether the Thingy
   can fire back is now controlled by OPTION_THINGY and turned off if the
   game type is "plain" or "almy".  The no-save behavior has been restored.)

   The Faerie Queen, black holes, and time warping were in the original.

Here are Tom Almy's changes:

   In early 1997, I got the bright idea to look for references to
   "Super Star Trek" on the World Wide Web. There weren't many hits,
   but there was one that came up with 1979 Fortran sources! This
   version had a few additional features that mine didn't have,
   however mine had some feature it didn't have. So I merged its
   features that I liked. I also took a peek at the DECUS version (a
   port, less sources, to the PDP-10), and some other variations.

   1, Compared to the original UT version, I've changed the "help" command to
      "call" and the "terminate" command to "quit" to better match
      user expectations. The DECUS version apparently made those changes
      as well as changing "freeze" to "save". However I like "freeze".
      (Both "freeze" and "save" work in SST2K.)

   2. The experimental deathray originally had only a 5% chance of
      success, but could be used repeatedly. I guess after a couple
      years of use, it was less "experimental" because the 1979
      version had a 70% success rate. However it was prone to breaking
      after use. I upgraded the deathray, but kept the original set of
      failure modes (great humor!).  (Now controlled by OPTION_DEATHRAY
      and turned off if game type is "plain".)

   3. The 1979 version also mentions srscan and lrscan working when
      docked (using the starbase's scanners), so I made some changes here
      to do this (and indicating that fact to the player), and then realized
      the base would have a subspace radio as well -- doing a Chart when docked
      updates the star chart, and all radio reports will be heard. The Dock
      command will also give a report if a base is under attack.

   4. Tholian Web from the 1979 version.  (Now controlled by
      OPTION_THOLIAN and turned off if game type is "plain".)

   5. Enemies can ram the Enterprise. (Now controlled by OPTION_RAMMING
      and turned off if game type is "plain".)

   6. Regular Klingons and Romulans can move in Expert and Emeritus games. 
      This code could use improvement. (Now controlled by OPTION_MVBADDY
      and turned off if game type is "plain".)

   7. The deep-space probe feature from the DECUS version.  (Now controlled
      by OPTION_PROBE and turned off if game type is "plain").

   8. 'emexit' command from the 1979 version.

   9. Bugfix: Klingon commander movements are no longer reported if long-range 
      sensors are damaged.

   10. Bugfix: Better base positioning at startup (more spread out).
      That made sense to add because most people abort games with 
      bad base placement.

   In June 2002, I fixed two known bugs and a documentation typo.
   In June 2004 I fixed a number of bugs involving: 1) parsing invalid
   numbers, 2) manual phasers when SR scan is damaged and commander is
   present, 3) time warping into the future, 4) hang when moving
   klingons in crowded quadrants.  (These fixes are in SST2K.)

Here are Stas Sergeev's changes:

   1. The Space Thingy can be shoved, if you ram it, and can fire back if 
      fired upon. (Now controlled by OPTION_THINGY and turned off if game 
      type is "plain" or "almy".)

   2. When you are docked, base covers you with an almost invincible shield. 
      (A commander can still ram you, or a Romulan can destroy the base,
      or a SCom can even succeed with direct attack IIRC, but this rarely 
      happens.)  (Now controlled by OPTION_BASE and turned off if game 
      type is "plain" or "almy".)

   3. Ramming a black hole is no longer instant death.  There is a
      chance you might get timewarped instead. (Now controlled by 
      OPTION_BLKHOLE and turned off if game type is "plain" or "almy".)

   4. The Tholian can be hit with phasers.

   5. SCom can't escape from you if no more enemies remain 
      (without this, chasing SCom can take an eternity).

   6. Probe target you enter is now the destination quadrant. Before I don't 
      remember what it was, but it was something I had difficulty using.

   7. Secret password is now autogenerated.

   8. "Plaque" is adjusted for A4 paper :-)

   9. Phasers now tells you how much energy needed, but only if the computer 
      is alive.

   10. Planets are auto-scanned when you enter the quadrant.

   11. Mining or using crystals in presense of enemy now yields an attack.
       There are other minor adjustments to what yields an attack
       and what does not.

   12. "freeze" command reverts to "save", most people will understand this
       better anyway. (SST2K recognizes both.)

   13. Screen-oriented interface, with sensor scans always up.  (SST2K
       supports both screen-oriented and TTY modes.)

Eric Raymond's changes:

Mainly, I translated this C code out of FORTRAN into C -- created #defines
for a lot of magic numbers and refactored the heck out of it.

   1. "sos" and "call" becomes "mayday", "freeze" and "save" are both good.

   2. Status report now indicates when dilithium crystals are on board.

   3. Per Dave Matuszek's remarks, Thingy state is never saved across games.

   4. Added game option selection so you can play a close (but not bug-for-
      bug identical) approximation of older versions.

   5. Half the quadrants now have inhabited planets, from which one 
      cannot mine dilithium (there will still be the same additional number
      of dilithium-bearing planets).  Torpedoing an inhabited world is *bad*.
      There is BSD-Trek-like logic for Klingons to attack and enslave 
      inhabited worlds, producing more ships (only is skill is 'good' or 
      better). (Controlled by OPTION_WORLDS and turned off if game 
      type is "plain" or "almy".)

   6. User input is now logged so we can do regression testing.

   7. More BSD-Trek features: You can now lose if your entire crew
      dies in battle.  When abandoning ship in a game with inhabited
      worlds enabled, they must have one in the quadrant to beam down
      to; otherwise they die in space and this counts heavily against
      your score.  Docking at a starbase replenishes your crew.

   8. Still more BSD-Trek: we now have a weighted damage table.
      Also, the nav subsystem (enabling automatic course
      setting) can be damaged separately from the main computer (which
      handles weapons targeting, ETA calculation, and self-destruct).
*/

/* the input queue */
static char line[128], *linep = line;

struct game game;
coord thing;
bool iqhere, iqengry;
int iscore, iskill;	// Common PLAQ
double aaitem;
double perdate;
char citem[10];
int seed;		// the random-number seed
bool idebug;		// debug mode
FILE *logfp, *replayfp;

char *device[NDEVICES] = {
	"S. R. Sensors",
	"L. R. Sensors",
	"Phasers",
	"Photon Tubes",
	"Life Support",
	"Warp Engines",
	"Impulse Engines",
	"Shields",
	"Subspace Radio",
	"Shuttle Craft",
	"Computer",
	"Navigation System",
	"Transporter",
	"Shield Control",
	"Death Ray",
	"D. S. Probe"};									

static struct 
{
    char *name;
    int value;
    unsigned long option;
}

commands[] = {
#define SRSCAN	0
	{"SRSCAN",	SRSCAN,		OPTION_TTY},
#define STATUS	1
	{"STATUS",	STATUS,		OPTION_TTY},
#define REQUEST	2
	{"REQUEST",	REQUEST,	OPTION_TTY},
#define LRSCAN	3
	{"LRSCAN",	LRSCAN,		OPTION_TTY},
#define PHASERS	4
	{"PHASERS",	PHASERS,	0},
#define TORPEDO	5
        {"TORPEDO",	TORPEDO,	0},
	{"PHOTONS",	TORPEDO,	0},
#define MOVE	7
	{"MOVE",	MOVE,		0},
#define SHIELDS	8
	{"SHIELDS",	SHIELDS,	0},
#define DOCK	9
	{"DOCK",	DOCK,		0},
#define DAMAGES	10
	{"DAMAGES",	DAMAGES,	0},
#define CHART	11
	{"CHART",	CHART,		0},
#define IMPULSE	12
	{"IMPULSE",	IMPULSE,	0},
#define REST	13
	{"REST",	REST,		0},
#define WARP	14
	{"WARP",	WARP,		0},
#define SCORE	15
	{"SCORE",	SCORE,		0},
#define SENSORS	16
	{"SENSORS",	SENSORS,	OPTION_PLANETS},
#define ORBIT	17
	{"ORBIT",	ORBIT,		OPTION_PLANETS},
#define TRANSPORT	18
	{"TRANSPORT",	TRANSPORT,	OPTION_PLANETS},
#define MINE	19
	{"MINE",	MINE,		OPTION_PLANETS},
#define CRYSTALS	20
	{"CRYSTALS",	CRYSTALS,	OPTION_PLANETS},
#define SHUTTLE	21
	{"SHUTTLE",	SHUTTLE,	OPTION_PLANETS},
#define PLANETS	22
	{"PLANETS",	PLANETS,	OPTION_PLANETS},
#define REPORT	23
	{"REPORT",	REPORT,		0},
#define COMPUTER	24
	{"COMPUTER",	COMPUTER,      	0},
#define COMMANDS	25
	{"COMMANDS",	COMMANDS,      	0},
#define EMEXIT	26
	{"EMEXIT",	EMEXIT,		0},
#define PROBE	27
	{"PROBE",	PROBE,		OPTION_PROBE},
#define SAVE	28
	{"SAVE",	SAVE,		0},
	{"FREEZE",	SAVE,		0},
#define ABANDON	30
	{"ABANDON",	ABANDON,	0},
#define DESTRUCT	31
	{"DESTRUCT",	DESTRUCT,	0},
#define DEATHRAY	32
	{"DEATHRAY",	DEATHRAY,	0},
#define DEBUGCMD	33
	{"DEBUG",	DEBUGCMD,	0},
#define MAYDAY	34
	{"MAYDAY",	MAYDAY,		0},
	//{"SOS",		MAYDAY,		0},
	//{"CALL",	MAYDAY,		0},
#define QUIT	35
	{"QUIT",	QUIT,		0},
#define HELP	36
	{"HELP",	HELP,		0},
#define SEED	37
	{"SEED",	SEED,		0},
};

#define NUMCOMMANDS	sizeof(commands)/sizeof(commands[0])
#define ACCEPT(i)	(!commands[i].option || (commands[i].option & game.options))

static void listCommands(void) 
/* generate a list of legal commands */
{
    int i, k = 0;
    proutn("LEGAL COMMANDS ARE:");
    for (i = 0; i < NUMCOMMANDS; i++) {
	if (!ACCEPT(i))
	    continue;
	if (k % 5 == 0)
	    skip(1);
	proutn("%-12s ", commands[i].name); 
	k++;
    }
    skip(1);
}

static void helpme(void)
/* browse on-line help */
{
    int i, j;
    char cmdbuf[32], *cp;
    char linebuf[132];
    FILE *fp;
    /* Give help on commands */
    int key;
    key = scan();
    for(;;) {
	if (key == IHEOL) {
	    setwnd(prompt_window);
	    proutn("Help on what command? ");
	    key = scan();
	}
	setwnd(message_window);
	if (key == IHEOL) return;
	for (i = 0; i < NUMCOMMANDS; i++) {
	    if (ACCEPT(i) && strcasecmp(commands[i].name, citem)==0) {
		i = commands[i].value;
		break;
	    }
	}
	if (i != NUMCOMMANDS) break;
	skip(1);
	prout("Valid commands:");
	listCommands();
	key = IHEOL;
	chew();
	skip(1);
    }
    if (i == COMMANDS) {
	strcpy(cmdbuf, " ABBREV");
    }
    else {
	for (j = 0; commands[i].name[j]; j++)
	    cmdbuf[j] = toupper(commands[i].name[j]);
	cmdbuf[j] = '\0';
    }
    fp = fopen(SSTDOC, "r");
    if (fp == NULL)
        fp = fopen(DOC_NAME, "r");
    if (fp == NULL) {
	prout("Spock-  \"Captain, that information is missing from the");
        prout("   computer. You need to find "DOC_NAME" and put it in the");
        prout("   current directory or to "SSTDOC".\"");
	/*
	 * This used to continue: "You need to find SST.DOC and put 
	 * it in the current directory."
	 */
	return;
    }
    for (;;) {
	if (fgets(linebuf, sizeof(linebuf), fp) == NULL) {
	    prout("Spock- \"Captain, there is no information on that command.\"");
	    fclose(fp);
	    return;
	}
	if (linebuf[0] == '%' && linebuf[1] == '%'&& linebuf[2] == ' ') {
	    for (cp = linebuf+3; isspace(*cp); cp++)
		continue;
	    linebuf[strlen(linebuf)-1] = '\0';
	    if (strcasecmp(cp, cmdbuf) == 0)
		break;
	}
    }

    skip(1);
    prout("Spock- \"Captain, I've found the following information:\"");
    skip(1);

    while (fgets(linebuf, sizeof(linebuf),fp)) {
	if (strstr(linebuf, "******"))
	    break;
	proutn(linebuf);
    }
    fclose(fp);
}

void enqueue(char *s)
/* enqueue input for the command parser */
{
    strcpy(line, s);
}

static void makemoves(void)
/* command-interpretation loop */
{
    int key, i, v = 0;
    bool hitme;
    clrscr();
    setwnd(message_window);
    for(;;) { /* command loop */
	drawmaps(1);
	for(;;)  { /* get a command */
	    hitme = false;
	    game.justin = false;
	    game.optime = 0.0;
	    i = -1;
	    chew();
	    setwnd(prompt_window);
	    clrscr();
	    proutn("COMMAND> ");
	    if (scan() == IHEOL) {
		makechart();
		continue;
	    }
	    game.ididit = false;
	    clrscr();
	    setwnd(message_window);
	    clrscr();
	    for (i=0; i < ABANDON; i++)
		if (ACCEPT(i) && isit(commands[i].name)) {
		    v = commands[i].value;
		    break;
		}
	    if (i < ABANDON && (!commands[i].option || (commands[i].option & game.options))) 
		break;
	    for (; i < NUMCOMMANDS; i++)
		if (ACCEPT(i) && strcasecmp(commands[i].name, citem) == 0) {
		    v = commands[i].value;
		    break;
		}
	    if (i < NUMCOMMANDS && (!commands[i].option || (commands[i].option & game.options))) 
		break;
	    listCommands();
	}
	commandhook(commands[i].name, true);
	switch (v) { /* command switch */
	case SRSCAN:                 // srscan
	    srscan(SCAN_FULL);
	    break;
	case STATUS:                 // status
	    srscan(SCAN_STATUS);
	    break;
	case REQUEST:			// status request 
	    srscan(SCAN_REQUEST);
	    break;
	case LRSCAN:			// lrscan
	    lrscan();
	    break;
	case PHASERS:			// phasers
	    phasers();
	    if (game.ididit) hitme = true;
	    break;
	case TORPEDO:			// photons
	    photon();
	    if (game.ididit) hitme = true;
	    break;
	case MOVE:			// move
	    warp(false);
	    break;
	case SHIELDS:			// shields
	    doshield(false);
	    if (game.ididit) {
		hitme=true;
		game.shldchg = 0;
	    }
	    break;
	case DOCK:			// dock
	    dock(1);
	    if (game.ididit) attack(0);
	    break;
	case DAMAGES:			// damages
	    dreprt();
	    break;
	case CHART:			// chart
	    chart(false);
	    break;
	case IMPULSE:			// impulse
	    impuls();
	    break;
	case REST:			// rest
	    wait();
	    if (game.ididit) hitme = true;
	    break;
	case WARP:			// warp
	    setwrp();
	    break;
	case SCORE:           	 	// score
	    score();
	    break;
	case SENSORS:			// sensors
	    sensor();
	    break;
	case ORBIT:			// orbit
	    orbit();
	    if (game.ididit) hitme = true;
	    break;
	case TRANSPORT:			// transport "beam"
	    beam();
	    break;
	case MINE:			// mine
	    mine();
	    if (game.ididit) hitme = true;
	    break;
	case CRYSTALS:			// crystals
	    usecrystals();
	    if (game.ididit) hitme = true;
	    break;
	case SHUTTLE:			// shuttle
	    shuttle();
	    if (game.ididit) hitme = true;
	    break;
	case PLANETS:			// Planet list
	    preport();
	    break;
	case REPORT:			// Game Report 
	    report();
	    break;
	case COMPUTER:			// use COMPUTER!
	    eta();
	    break;
	case COMMANDS:
	    listCommands();
	    break;
	case EMEXIT:			// Emergency exit
	    clrscr();			// Hide screen
	    freeze(true);		// forced save
	    exit(1);			// And quick exit
	    break;
	case PROBE:
	    probe();			// Launch probe
	    if (game.ididit) hitme = true;
	    break;
	case ABANDON:			// Abandon Ship
	    abandn();
	    break;
	case DESTRUCT:			// Self Destruct
	    selfdestruct();
	    break;
	case SAVE:			// Save Game
	    freeze(false);
	    clrscr();
	    if (game.skill > SKILL_GOOD)
		prout("WARNING--Saved games produce no plaques!");
	    break;
	case DEATHRAY:			// Try a desparation measure
	    deathray();
	    if (game.ididit) hitme = true;
	    break;
	case DEBUGCMD:			// What do we want for debug???
	    debugme();
	    break;
	case MAYDAY:			// Call for help
	    mayday();
	    if (game.ididit) hitme = true;
	    break;
	case QUIT:
	    game.alldone = true;		// quit the game
	    break;
	case HELP:
	    helpme();			// get help
	    break;
	case SEED:			// set random-number seed
	    key = scan();
	    if (key == IHREAL)
		seed = (int)aaitem;
	    break;
	}
	commandhook(commands[i].name, false);
	for (;;) {
	    if (game.alldone) break;		// Game has ended
	    if (game.optime != 0.0) {
		events();
		if (game.alldone) break;	// Events did us in
	    }
	    if (game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova) { // Galaxy went Nova!
		atover(false);
		continue;
	    }
	    if (hitme && !game.justin) {
		attack(2);
		if (game.alldone) break;
		if (game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova) {	// went NOVA! 
		    atover(false);
		    hitme = true;
		    continue;
		}
	    }
	    break;
	}
	if (game.alldone) break;
    }
    if (idebug) prout("=== Ending");
}


int main(int argc, char **argv) 
{
    int i, option;

    game.options = OPTION_ALL &~ (OPTION_IOMODES | OPTION_SHOWME | OPTION_PLAIN | OPTION_ALMY);
    if (getenv("TERM"))
	game.options |= OPTION_CURSES | OPTION_SHOWME;
    else
	game.options |= OPTION_TTY;

    seed = (int)time(NULL);
    while ((option = getopt(argc, argv, "r:tx")) != -1) {
	switch (option) {
	case 'r':
	    replayfp = fopen(optarg, "r");
	    if (replayfp == NULL) {
		fprintf(stderr, "sst: can't open replay file %s\n", optarg);
		exit(1);	
	    }
	    if (fscanf(replayfp, "seed %d\n", &seed) != 1) {
		fprintf(stderr, "sst: replay file %s is ill-formed\n", optarg);
		exit(1);	
	    }
	    /* FALL THROUGH */
	case 't':
	    game.options |= OPTION_TTY;
	    game.options &=~ OPTION_CURSES;
	    break;
	case 'x':
	    idebug = true;
	    break;
	default:
	    fprintf(stderr, "usage: sst [-t] [-x] [startcommand...].\n");
	    exit(0);
	}
    }
    /* where to save the input in case of bugs */
    logfp = fopen("sst-input.log", "w");
    setlinebuf(logfp);
    fprintf(logfp, "seed %d\n", seed);
    srand(seed);

    srand(seed);
    iostart();

    line[0] = '\0';
    for (i = optind; i < argc;  i++) {
	strcat(line, argv[i]);
	strcat(line, " ");
    }
    for(;;) { /* Play a game */
	setwnd(fullscreen_window);
	clrscr();
	prelim();
	setup(line[0] == '\0');
	if (game.alldone) {
	    score();
	    game.alldone = false;
	}
	else makemoves();
	skip(1);
	stars();
	skip(1);

	if (game.tourn && game.alldone) {
	    proutn("Do you want your score recorded?");
	    if (ja() == true) {
		chew2();
		freeze(false);
	    }
	}
	proutn("Do you want to play again? ");
	if (!ja()) break;
    }
    skip(1);
    prout("May the Great Bird of the Galaxy roost upon your home planet.");
    return 0;
}


void cramen(feature i) 
/* print the name of an enemy */
{
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
    case IHQUEST: s = "Stranger"; break;
    default: s = "Unknown??"; break;
    }
    proutn(s);
}

char *cramlc(enum loctype key, coord w)
/* name a location */
{
    static char buf[32];
    buf[0] = '\0';
    if (key == quadrant) strcpy(buf, "Quadrant ");
    else if (key == sector) strcpy(buf, "Sector ");
    sprintf(buf+strlen(buf), "%d - %d", w.x, w.y);
    return buf;
}

void crmena(bool stars, feature enemy, enum loctype key, coord w) 
/* print an enemy and his location */
{
    if (stars) proutn("***");
    cramen(enemy);
    proutn(" at ");
    proutn(cramlc(key, w));
}

void crmshp(void)
/* print our ship name */
{
    char *s;
    switch (game.ship) {
    case IHE: s = "Enterprise"; break;
    case IHF: s = "Faerie Queene"; break;
    default:  s = "Ship???"; break;
    }
    proutn(s);
}

void stars(void)
/* print a line of stars */
{
    prouts("******************************************************");
    skip(1);
}

double expran(double avrage) 
{
    return -avrage*log(1e-7 + Rand());
}

double Rand(void) 
{
    return rand()/(1.0 + (double)RAND_MAX);
}

coord randplace(int size)
/* choose a random location */ 
{
    coord w;
    w.x = Rand()*(size*1.0) + 1.0;
    w.y = Rand()*(size*1.0) + 1.0;
    return w;
}

void chew(void)
{
    linep = line;
    *linep = 0;
}

void chew2(void) 
{
    /* return IHEOL next time */
    linep = line+1;
    *linep = 0;
}

int scan(void) 
{
    int i;
    char *cp;

    // Init result
    aaitem = 0.0;
    *citem = 0;

    // Read a line if nothing here
    if (*linep == 0) {
	if (linep != line) {
	    chew();
	    return IHEOL;
	}
	cgetline(line, sizeof(line));
	fflush(stdin);
	if (curwnd==prompt_window){
	    clrscr();
	    setwnd(message_window);
	    clrscr();
	}
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
	i = 0;
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

bool ja(void)
/* yes-or-no confirmation */
{
    chew();
    for(;;) {
	scan();
	chew();
	if (*citem == 'y') return true;
	if (*citem == 'n') return false;
	proutn("Please answer with \"Y\" or \"N\": ");
    }
}

void huh(void)
/* complain about unparseable input */
{
    chew();
    skip(1);
    prout("Beg your pardon, Captain?");
}

bool isit(char *s) 
/* compares s to citem and returns true if it matches to the length of s */
{
    return strncasecmp(s, citem, max(1, strlen(citem))) == 0;
}

void debugme(void)
/* access to the internals for debugging */
{
    proutn("Reset levels? ");
    if (ja() == true) {
	if (game.energy < game.inenrg) game.energy = game.inenrg;
	game.shield = game.inshld;
	game.torps = game.intorps;
	game.lsupres = game.inlsr;
    }
    proutn("Reset damage? ");
    if (ja() == true) {
	int i;
	for (i=0; i < NDEVICES; i++) 
	    if (game.damage[i] > 0.0) 
		game.damage[i] = 0.0;
    }
    proutn("Toggle debug flag? ");
    if (ja() == true) {
	idebug = !idebug;
	if (idebug) prout("Debug output ON");
	else prout("Debug output OFF");
    }
    proutn("Cause selective damage? ");
    if (ja() == true) {
	int i, key;
	for (i=0; i < NDEVICES; i++) {
	    proutn("Kill ");
	    proutn(device[i]);
	    proutn("? ");
	    chew();
	    key = scan();
	    if (key == IHALPHA &&  isit("y")) {
		game.damage[i] = 10.0;
	    }
	}
    }
    proutn("Examine/change events? ");
    if (ja() == true) {
	event *ev;
	coord w;
	int i;
	for (i = 1; i < NEVENTS; i++) {
	    int key;
	    switch (i) {
	    case FSNOVA:  proutn("Supernova       "); break;
	    case FTBEAM:  proutn("T Beam          "); break;
	    case FSNAP:   proutn("Snapshot        "); break;
	    case FBATTAK: proutn("Base Attack     "); break;
	    case FCDBAS:  proutn("Base Destroy    "); break;
	    case FSCMOVE: proutn("SC Move         "); break;
	    case FSCDBAS: proutn("SC Base Destroy "); break;
	    case FDSPROB: proutn("Probe Move      "); break;
	    case FDISTR:  proutn("Distress Call   "); break;
	    case FENSLV:  proutn("Enlavement      "); break;
	    case FREPRO:  proutn("Klingon Build   "); break;
	    }
	    if (is_scheduled(i)) {
		proutn("%.2f", scheduled(i)-game.state.date);
		if (i == FENSLV || i == FREPRO) {
		    ev = findevent(i);
		    proutn(" in %d-%d", ev->quadrant.x,ev->quadrant.y);
		}
	    } else
		proutn("never");
	    proutn("? ");
	    chew();
	    key = scan();
	    if (key == 'n') {
		unschedule(i);
		chew();
	    } else if (key == IHREAL) {
		ev = schedule(i, aaitem);
		if (i == FENSLV || i == FREPRO) {
		    chew();
		    proutn("In quadrant- ");
		    key = scan();
		    /* IHEOL says to leave coordinates as they are */
		    if (key != IHEOL) {
			if (key != IHREAL) {
			    prout("Event %d canceled, no x coordinate.", i);
			    unschedule(i);
			    continue;
			}
			w.x = (int)aaitem;
			key = scan();
			if (key != IHREAL) {
			    prout("Event %d canceled, no y coordinate.", i);
			    unschedule(i);
			    continue;
			}
			w.y = (int)aaitem;
			ev->quadrant = w;
		    }
		}
	    }
	}
	chew();
    }
    proutn("Induce supernova here? ");
    if (ja() == true) {
	game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova = true;
	atover(true);
    }
}
