#ifndef __SST_H__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <curses.h>
#include <stdbool.h>

#ifdef DATA_DIR
#define SSTDOC DATA_DIR"/"DOC_NAME
#else
#define SSTDOC DOC_NAME
#endif

#define _(str) gettext(str)

#define min(x, y)	((x)<(y)?(x):(y))
#define max(x, y)	((x)>(y)?(x):(y))

#define PHASEFAC (2.0)
#define GALSIZE	(8)
#define NINHAB (GALSIZE * GALSIZE / 2)
#define MAXUNINHAB (10)
#define PLNETMAX (NINHAB + MAXUNINHAB)
#define QUADSIZE (10)
#define BASEMAX	(5)

/*
 * These macros hide the difference between 0-origin and 1-origin addressing.
 * They're a step towards de-FORTRANizing the code.
 */
#define VALID_QUADRANT(x, y)	((x)>=1 && (x)<=GALSIZE && (y)>=1 && (y)<=GALSIZE)
#define VALID_SECTOR(x, y)	((x)>=1 && (x)<=QUADSIZE && (y)>=1 && (y)<=QUADSIZE)
#define for_quadrants(i)	for (i = 1; i <= GALSIZE; i++)
#define for_sectors(i)		for (i = 1; i <= QUADSIZE; i++)
#define for_commanders(i)	for (i = 1; i <= game.state.remcom; i++)
#define for_local_enemies(i)	for (i = 1; i <= game.nenhere; i++)
#define for_starbases(i)	for (i = 1; i <= game.state.rembase; i++)

typedef struct {int x; int y;} coord;

#define same(c1, c2)	(c1.x == c2.x && c1.y == c2.y)

typedef struct {
    coord w;
    enum {M=0, N=1, O=2} pclass;
    int inhabited;	/* if NZ, an index into a name array */
#define UNINHABITED	-1
    int crystals; /* has crystals */
#define MINED	-1	/* used to have crystals, but they were mined out */
    enum {unknown, known, shuttle_down} known;
} planet;

#define DESTROY(pl)	memset(pl, '\0', sizeof(planet))

typedef struct {
    int snap,		// snapshot taken
	remkl,			// remaining klingons
	remcom,			// remaining commanders
	nscrem,			// remaining super commanders
	rembase,		// remaining bases
	starkl,			// destroyed stars
	basekl,			// destroyed bases
	nromrem,		// Romulans remaining
	nplankl,		// destroyed uninhabited planets
	nworldkl;		// destroyed inhabited planets
    planet plnets[PLNETMAX];  // Planet information
    double date,		// stardate
	remres,		// remaining resources
	remtime;		// remaining time
    coord baseq[BASEMAX+1];	// Base quadrant coordinates
    coord kcmdr[QUADSIZE+1];	// Commander quadrant coordinates
    coord kscmdr;		// Supercommander quadrant coordinates
    struct quadrant {
	int stars;
	int planet;
#define NOPLANET	-1
	bool starbase;
	int klingons;
	int romulans;
	bool supernova;
	bool charted;
	enum {secure, distressed, enslaved} status;
    } galaxy[GALSIZE+1][GALSIZE+1]; 	// The Galaxy (subscript 0 not used)
    struct page {
	int stars;
	int starbase;
	int klingons;
    } chart[GALSIZE+1][GALSIZE+1]; 	// the starchart (subscript 0 not used)
} snapshot;				// Data that is snapshot

#define MAXKLGAME	127
#define MAXKLQUAD	9

#define NKILLK (game.inkling - game.state.remkl)
#define NKILLC (game.incom - game.state.remcom)
#define NKILLSC (game.inscom - game.state.nscrem)
#define NKILLROM (game.inrom - game.state.nromrem)
#define KLINGREM (game.state.remkl + game.state.remcom + game.state.nscrem)
#define INKLINGTOT (game.inkling + game.incom + game.inscom)
#define KLINGKILLED (INKLINGTOT - KLINGREM)

#define SKILL_NONE	0
#define SKILL_NOVICE	1
#define SKILL_FAIR	2
#define SKILL_GOOD	3
#define SKILL_EXPERT	4
#define SKILL_EMERITUS	5

/* game options */
#define OPTION_ALL	0xffffffff
#define OPTION_TTY	0x00000001	/* old interface */
#define OPTION_CURSES	0x00000002	/* new interface */
#define OPTION_IOMODES	0x00000003	/* cover both interfaces */
#define OPTION_PLANETS	0x00000004	/* planets and mining */
#define OPTION_THOLIAN	0x00000008	/* Tholians and their webs */
#define OPTION_THINGY	0x00000010	/* Space Thingy can shoot back */
#define OPTION_PROBE	0x00000020	/* deep-space probes */
#define OPTION_SHOWME	0x00000040	/* bracket Enterprise in chart */
#define OPTION_RAMMING	0x00000080	/* enemies may ram Enterprise */
#define OPTION_MVBADDY	0x00000100	/* more enemies can move */
#define OPTION_BLKHOLE	0x00000200	/* black hole may timewarp you */
#define OPTION_BASE	0x00000400	/* bases have good shields */
#define OPTION_WORLDS	0x00000800	/* logic for inhabited worlds */
#define OPTION_PLAIN	0x01000000	/* user chose plain game */
#define OPTION_ALMY	0x02000000	/* user chose Almy variant */

/* Define devices */
#define DSRSENS 0
#define DLRSENS 1
#define DPHASER 2
#define DPHOTON 3
#define DLIFSUP 4
#define DWARPEN 5
#define DIMPULS 6
#define DSHIELD 7
#define DRADIO  8
#define DSHUTTL 9
#define DCOMPTR 10
#define DTRANSP 11
#define DSHCTRL 12
#define DDRAY   13  // Added deathray
#define DDSP    14  // Added deep space probe
#define NDEVICES (15)	// Number of devices

#define FOREVER	1e30

/* Define future events */
#define FSPY	0	// Spy event happens always (no future[] entry)
					// can cause SC to tractor beam Enterprise
#define FSNOVA  1   // Supernova
#define FTBEAM  2   // Commander tractor beams Enterprise
#define FSNAP   3   // Snapshot for time warp
#define FBATTAK 4   // Commander attacks base
#define FCDBAS  5   // Commander destroys base
#define FSCMOVE 6   // Supercommander moves (might attack base)
#define FSCDBAS 7   // Supercommander destroys base
#define FDSPROB 8   // Move deep space probe
#define FDISTR	9   // Emit distress call from an inhabited world 
#define FENSLV	10  // Inhabited word is enslaved */
#define FREPRO	11  // Klingons build a ship in an enslaved system
#define NEVENTS (12)

typedef struct {
    double date;
    coord quadrant;
} event;

/*
 * abstract out the event handling -- underlying data structures will change
 * when we implement stateful events
 */
extern event *unschedule(int);
extern int is_scheduled(int);
extern event *schedule(int, double);
extern void postpone(int, double);
extern double scheduled(int);

#define SSTMAGIC	"SST2.0\n"

struct game {
    char magic[sizeof(SSTMAGIC)];
    unsigned long options;
    snapshot state;
    snapshot snapsht;
    char quad[QUADSIZE+1][QUADSIZE+1];		// contents of our quadrant
    double kpower[(QUADSIZE+1)*(QUADSIZE+1)];		// enemy energy levels
    double kdist[(QUADSIZE+1)*(QUADSIZE+1)];		// enemy distances
    double kavgd[(QUADSIZE+1)*(QUADSIZE+1)];		// average distances
    double damage[NDEVICES];	// damage encountered
    event future[NEVENTS];	// future events
    char passwd[10];		// Self Destruct password
    coord ks[(QUADSIZE+1)*(QUADSIZE+1)];	// enemy sector locations
    coord quadrant, sector;	// where we are
    coord tholian;		// coordinates of Tholian
    coord base;			// position of base in current quadrant
    coord battle;		// base coordinates being attacked
    coord plnet;		// location of planet in quadrant
    coord probec;	// current probe quadrant
    bool gamewon,	// Finished!
	ididit,		// action taken -- allows enemy to attack
	alive,		// we are alive (not killed)
	justin,		// just entered quadrant
	shldup,		// shields are up
	resting,	// rest time
	alldone,	// game is now finished
	neutz,		// Romulan Neutral Zone
	isarmed,	// probe is armed
	inorbit,	// orbiting a planet
	thawed;		// thawed game
    int inkling,	// initial number of klingons
	inbase,		// initial number of bases
	incom,		// initial number of commanders
	inscom,		// initial number of commanders
	inrom,		// initial number of commanders
	instar,		// initial stars
	intorps,	// initial/Max torpedoes
	condit,		// condition (red/yellow/green/docked)
	torps,		// number of torpedoes
	ship,		// ship type -- 'E' is Enterprise
	length,		// length of game
	skill,		// skill level
	klhere,		// klingons here
	comhere,	// commanders here
	casual,		// causalties
	nhelp,		// calls for help
	nkinks,		// count of energy-barrier crossings
	shldchg,	// shield is changing (affects efficiency)
	landed,		// party on planet (1), on ship (-1)
	iplnet,		// planet # in quadrant
	imine,		// mining
	inplan,		// initial planets
	nenhere,	// number of enemies in quadrant
	ishere,		// super-commander in quandrant
	irhere,		// Romulans in quadrant
	icraft,		// Kirk in Galileo
	ientesc,	// attempted escape from supercommander
	iscraft,	// =1 if craft on ship, -1 if removed from game
	isatb,		// =1 if super commander is attacking base
	iscate,		// super commander is here
	iattak,		// attack recursion elimination (was cracks[4])
	icrystl,	// dilithium crystals aboard
	tourn,		// tournament number
	ithere,		// Tholian is here 
	iseenit,	// seen base attack report
	proben,		// number of moves for probe
	nprobes;	// number of probes available
    double inresor,	// initial resources
	intime,		// initial time
	inenrg,		// initial/max energy
	inshld,		// initial/max shield
	inlsr,		// initial life support resources
	indate,		// initial date
	energy,		// energy level
	shield,		// shield level
	warpfac,	// warp speed
	wfacsq,		// squared warp factor
	lsupres,	// life support reserves
	dist,		// movement distance
	direc,		// movement direction
	optime,		// time taken by current operation
	docfac,		// repair factor when docking (constant?)
	damfac,		// damage factor
	lastchart,	// time star chart was last updated
	cryprob,	// probability that crystal will work
	probex,		// location of probe
	probey,		//
	probeinx,	// probe x,y increment
	probeiny,	//
	height;		// height of orbit around planet
};
extern struct game game;

/* the following global state doesn't need to be saved */
extern char *device[NDEVICES];
extern int iscore, iskill; // Common PLAQ
extern double perdate;
extern double aaitem;
extern char citem[10];
extern int seed;
extern bool randready;
extern bool idebug;
extern FILE *logfp;

/* the Space Thingy's global state should *not* be saved! */
extern coord thing;
extern int iqhere, iqengry;

typedef enum {FWON, FDEPLETE, FLIFESUP, FNRG, FBATTLE,
              FNEG3, FNOVA, FSNOVAED, FABANDN, FDILITHIUM,
			  FMATERIALIZE, FPHASER, FLOST, FMINING, FDPLANET,
			  FPNOVA, FSSC, FSTRACTOR, FDRAY, FTRIBBLE,
			  FHOLE} FINTYPE ;
enum loctype {neither, quadrant, sector};

#define IHR 'R'
#define IHK 'K'
#define IHC 'C'
#define IHS 'S'
#define IHSTAR '*'
#define IHP 'P'
#define IHW '@'
#define IHB 'B'
#define IHBLANK ' '
#define IHDOT '.'
#define IHQUEST '?'
#define IHE 'E'
#define IHF 'F'
#define IHT 'T'
#define IHWEB '#'
#define IHGREEN 'G'
#define IHYELLOW 'Y'
#define IHRED 'R'
#define IHDOCKED 'D'
#define IHDEAD 'Z'
#define IHMATER0 '-'
#define IHMATER1 'o'
#define IHMATER2 '0'


/* Function prototypes */
void prelim(void);
void attack(int);
bool choose(bool);
void setup(int);
void score(void);
void atover(int);
int srscan(int);
void lrscan(void);
void phasers(void);
void photon(void);
void warp(bool);
void doshield(int);
void dock(int);
void dreprt(void);
void chart(int);
void rechart(void);
void impuls(void);
void wait(void);
void setwrp(void);
void events(void);
void report(void);
void eta(void);
void mayday(void);
void abandn(void);
void finish(FINTYPE);
void dstrct(void);
void kaboom(void);
void freeze(bool);
int thaw(void);
void plaque(void);
int scan(void);
#define IHEOL (0)
#define IHALPHA (1)
#define IHREAL (2)
void chew(void);
void chew2(void);
void skip(int);
void prout(char *, ...);
void proutn(char *, ...);
void stars(void);
void newqad(int);
bool ja(void);
void cramen(int);
void crmshp(void);
char *cramlc(enum loctype, coord w);
double expran(double);
double Rand(void);
void iran(int, int *, int *);
#define square(i) ((i)*(i))
void dropin(int, coord*);
void newcnd(void);
void sortkl(void);
void imove(void);
void ram(int, int, coord);
void crmena(int, int, int, coord w);
void deadkl(coord, int, int, int);
void timwrp(void);
void movcom(void);
void torpedo(double, double, int, int, double *, int, int);
void huh(void);
void pause_game(int);
void nova(int, int);
void snova(int, int);
void scom(int *);
void hittem(double *);
void prouts(char *, ...);
int isit(char *);
void preport(void);
void orbit(void);
void sensor(void);
void drawmaps(short);
void beam(void);
void mine(void);
void usecrystals(void);
void shuttle(void);
void deathray(void);
void debugme(void);
void attakreport(int);
void movetho(void);
void probe(void);
void iostart(void);
void setwnd(WINDOW *);
void warble(void);
void boom(int ii, int jj);
void tracktorpedo(int ix, int iy, int l, int i, int n, int iquad);
void cgetline(char *, int);
void waitfor(void);
void setpassword(void);
void commandhook(char *, bool);
void makechart(void);
void enqueue(char *);
char *systemname(int);
void newkling(int, coord *);

/* mode arguments for srscan() */
#define SCAN_FULL		1
#define SCAN_REQUEST		2
#define SCAN_STATUS		3
#define SCAN_NO_LEFTSIDE	4

extern WINDOW *curwnd;
extern WINDOW *fullscreen_window;
extern WINDOW *srscan_window;
extern WINDOW *report_window;
extern WINDOW *lrscan_window;
extern WINDOW *message_window;
extern WINDOW *prompt_window;

extern void clreol(void);
extern void clrscr(void);
extern void textcolor(int color);
extern void highvideo(void);

enum COLORS {
   DEFAULT,
   BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY,
   DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, YELLOW, WHITE
};

#endif
