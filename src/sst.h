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

#define square(i)		((i)*(i))
#define same(c1, c2)		((c1.x == c2.x) && (c1.y == c2.y))
#define distance(c1, c2)	sqrt(square(c1.x - c2.x) + square(c1.y - c2.y))
#define invalidate(w)		w.x = w.y = 0
#define is_valid(w)		(w.x != 0 && w.y != 0)

typedef struct {
    coord w;
    enum {M=0, N=1, O=2} pclass;
    int inhabited;	/* if NZ, an index into a name array */
#define UNINHABITED	-1
    enum {mined=-1, present=0, absent=1} crystals; /* has crystals */
    enum {unknown, known, shuttle_down} known;
} planet;

#define DESTROY(pl)	memset(pl, '\0', sizeof(planet))

typedef enum {
    IHR = 'R',
    IHK = 'K',
    IHC = 'C',
    IHS = 'S',
    IHSTAR = '*',
    IHP = 'P',
    IHW = '@',
    IHB = 'B',
    IHBLANK = ' ',
    IHDOT = '.',
    IHQUEST = '?',
    IHE = 'E',
    IHF = 'F',
    IHT = 'T',
    IHWEB = '#',
    IHMATER0 = '-',
    IHMATER1 = 'o',
    IHMATER2 = '0',
} feature;

typedef struct {
    bool snap;		// snapshot taken
    int crew,		// crew complement
#define FULLCREW	428	/* BSD Trek was 387, that's wrong */
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
	bool starbase;
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
#define DNAVSYS	11
#define DTRANSP 12
#define DSHCTRL 13
#define DDRAY   14
#define DDSP    15
#define NDEVICES (16)	// Number of devices

#define damaged(dev)	(game.damage[dev] != 0.0)

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
#define findevent(evtype)	&game.future[evtype]

#define SSTMAGIC	"SST2.0\n"

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

struct game {
    char magic[sizeof(SSTMAGIC)];
    unsigned long options;
    snapshot state;
    snapshot snapsht;
    feature quad[QUADSIZE+1][QUADSIZE+1];		// contents of our quadrant
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
	shldchg,	// shield is changing (affects efficiency)
	comhere,	// commander here
	ishere,		// super-commander in quadrant
	iscate,		// super commander is here
	ientesc,	// attempted escape from supercommander
	ithere,		// Tholian is here 
	resting,	// rest time
	icraft,		// Kirk in Galileo
	landed,		// party on planet (true), on ship (false)
	alldone,	// game is now finished
	neutz,		// Romulan Neutral Zone
	isarmed,	// probe is armed
	inorbit,	// orbiting a planet
	imine,		// mining
	icrystl,	// dilithium crystals aboard
	iseenit,	// seen base attack report
	thawed;		// thawed game
    enum {
	green,
	yellow,
	red,
	docked,
	dead,
    } condition;		// condition (red/yellow/green/docked)
    enum {
	onship,
	offship,
	removed,
    } iscraft;		// 'onship' if craft on ship, 'removed' if out of game
    enum {
	SKILL_NONE,
	SKILL_NOVICE,
	SKILL_FAIR,
	SKILL_GOOD,
	SKILL_EXPERT,
	SKILL_EMERITUS,
    } skill;		// skill level
    int inkling,	// initial number of klingons
	inbase,		// initial number of bases
	incom,		// initial number of commanders
	inscom,		// initial number of commanders
	inrom,		// initial number of commanders
	instar,		// initial stars
	intorps,	// initial/max torpedoes
	torps,		// number of torpedoes
	ship,		// ship type -- 'E' is Enterprise
	abandoned,	// count of crew abandoned in space
	length,		// length of game
	klhere,		// klingons here
	casual,		// causalties
	nhelp,		// calls for help
	nkinks,		// count of energy-barrier crossings
	iplnet,		// planet # in quadrant
	inplan,		// initial planets
	nenhere,	// number of enemies in quadrant
	irhere,		// Romulans in quadrant
	isatb,		// =1 if super commander is attacking base
	tourn,		// tournament number
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
extern char *systnames[NINHAB + 1];
extern int iscore, iskill; // Common PLAQ
extern double perdate;
extern double aaitem;
extern char citem[10];
extern int seed;
extern bool idebug;
extern FILE *logfp, *replayfp;

/* the Space Thingy's global state should *not* be saved! */
extern coord thing;
extern bool iqhere, iqengry;

typedef enum {
    FWON, FDEPLETE, FLIFESUP, FNRG, FBATTLE,
    FNEG3, FNOVA, FSNOVAED, FABANDN, FDILITHIUM,
    FMATERIALIZE, FPHASER, FLOST, FMINING, FDPLANET,
    FPNOVA, FSSC, FSTRACTOR, FDRAY, FTRIBBLE,
    FHOLE, FCREW
} FINTYPE ;

enum loctype {neither, quadrant, sector};

/* Function prototypes */
void prelim(void);
void attack(bool);
bool choose(bool);
void setup(bool);
void score(void);
void atover(bool);
void srscan(void);
void lrscan(void);
void phasers(void);
void photon(void);
void warp(bool);
void doshield(bool);
void dock(bool);
void dreprt(void);
void chart(void);
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
void selfdestruct(void);
void kaboom(void);
void freeze(bool);
bool thaw(void);
void plaque(void);
int scan(void);
void status(int req);
void request(void);
#define IHEOL (0)
#define IHALPHA (1)
#define IHREAL (2)
void chew(void);
void chew2(void);
void skip(int);
void prout(const char *, ...) __attribute__((format(printf, 1, 2)));
void proutn(const char *, ...) __attribute__((format(printf, 1, 2)));
void prouts(const char *, ...) __attribute__((format(printf, 1, 2)));
void prstat(const char *txt, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
void stars(void);
void newqad(bool);
bool ja(void);
void cramen(feature);
void crmshp(void);
char *cramlc(enum loctype, coord w);
double expran(double);
double Rand(void);
coord randplace(int);
coord dropin(feature);
void newcnd(void);
void sortkl(void);
void imove(bool);
void ram(bool, feature, coord);
void crmena(bool, feature, enum loctype, coord w);
void deadkl(coord, feature, coord);
void timwrp(void);
void movcom(void);
void torpedo(double, double, coord, double *, int, int);
void huh(void);
void pause_game(bool);
void nova(coord);
void snova(bool, coord *);
void scom(bool *);
void hittem(double *);
bool isit(char *);
void preport(void);
void orbit(void);
void sensor(void);
void drawmaps(int);
void beam(void);
void mine(void);
void usecrystals(void);
void shuttle(void);
void deathray(void);
void debugme(void);
void attakreport(bool);
void movetho(void);
void probe(void);
void iostart(void);
void setwnd(WINDOW *);
void warble(void);
void boom(coord);
void tracktorpedo(coord, int, int, int, int);
void cgetline(char *, int);
void waitfor(void);
void setpassword(void);
void commandhook(char *, bool);
void makechart(void);
coord newkling(int);
#if BSD_BUG_FOR_BUG
void visual(void);
#endif

extern WINDOW *curwnd;
extern WINDOW *fullscreen_window;
extern WINDOW *srscan_window;
extern WINDOW *report_window;
extern WINDOW *lrscan_window;
extern WINDOW *message_window;
extern WINDOW *prompt_window;

extern void clreol(void);
extern void clrscr(void);
extern void textcolor(int);
extern void highvideo(void);

enum COLORS {
   DEFAULT,
   BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY,
   DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, YELLOW, WHITE
};

#endif
