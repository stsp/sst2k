#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#ifndef INCLUDED
#define EXTERN extern
#else
#define EXTERN
#endif

#define min(x, y)	((x)<(y)?(x):(y))
#define max(x, y)	((x)>(y)?(x):(y))

// #define DEBUG

#define PHASEFAC (2.0)
#define PLNETMAX (10)
#define NEVENTS (8)
#define GALSIZE	(8)
#define QUADSIZE (10)
#define BASEMAX	(6)

typedef struct {
    int x;	/* Quadrant location of planet */
    int y;
    enum {M=0, N=1, O=2} pclass;
    int crystals; /* has crystals */
    enum {unknown, known, shuttle_down} known;
} planet;

#define DESTROY(pl)	memset(pl, '\0', sizeof(planet))

typedef struct {
    int snap,		// snapshot taken
	remkl,			// remaining klingons
	remcom,			// remaining commanders
	rembase,		// remaining bases
	starkl,			// destroyed stars
	basekl,			// destroyed bases
	killk,			// Klingons killed
	killc,			// commanders killed
	cx[QUADSIZE+1],cy[QUADSIZE+1],	// Commander quadrant coordinates
	baseqx[BASEMAX],		// Base quadrant X
	baseqy[BASEMAX],		// Base quadrant Y
	isx, isy,		// Coordinate of Super Commander
	nscrem,			// remaining super commanders
	nromkl,			// Romulans killed
	nromrem,		// Romulans remaining
	nsckill,		// super commanders killed
	nplankl;		// destroyed planets
	planet plnets[PLNETMAX];  // Planet information
	double date,		// stardate
	    remres,		// remaining resources
	    remtime;		// remaining time
    struct {
	int stars;
	int planets;
	int starbase;
	int klingons;
	int romulans;
	int supernova;
	int charted;
    } galaxy[GALSIZE+1][GALSIZE+1]; 	// The Galaxy (subscript 0 not used)
    struct {
	int stars;
	int starbase;
	int klingons;
    } chart[GALSIZE+1][GALSIZE+1]; 	// the starchart (subscript 0 not used)
} snapshot;				// Data that is snapshot

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

// Scalar variables that are needed for freezing the game
// are placed in a structure. #defines are used to access by their
// original names. Gee, I could have done this with the d structure,
// but I just didn't think of it back when I started.

#define SSTMAGIC	"SST2.0\n"

EXTERN WINDOW *curwnd;

EXTERN struct {
    char magic[sizeof(SSTMAGIC)];
    unsigned long options;
    snapshot state;
    snapshot snapsht;
    char quad[QUADSIZE+1][QUADSIZE+1];		// contents of our quadrant
    double kpower[(QUADSIZE+1)*(QUADSIZE+1)];		// enemy energy levels
    double kdist[(QUADSIZE+1)*(QUADSIZE+1)];		// enemy distances
    double kavgd[(QUADSIZE+1)*(QUADSIZE+1)];		// average distances
    double damage[NDEVICES];	// damage encountered
    double future[NEVENTS+1];	// future events
    char passwd[10];		// Self Destruct password
    int kx[(QUADSIZE+1)*(QUADSIZE+1)];			// enemy sector locations
    int ky[(QUADSIZE+1)*(QUADSIZE+1)];
    /* members with macro definitions start here */
    int inkling,
	inbase,
	incom,
	instar,
	intorps,
	condit,
	torps,
	ship,
	quadx,
	quady,
	sectx,
	secty,
	length,
	skill,
	basex,
	basey,
	klhere,
	comhere,
	casual,
	nhelp,
	nkinks,
	ididit,
	gamewon,
	alive,
	justin,
	alldone,
	shldchg,
	plnetx,
	plnety,
	inorbit,
	landed,
	iplnet,
	imine,
	inplan,
	nenhere,
	ishere,
	neutz,
	irhere,
	icraft,
	ientesc,
	iscraft,
	isatb,
	iscate,
#ifdef DEBUG
	idebug,
#endif
	iattak,
	icrystl,
	tourn,
	thawed,
	batx,
	baty,
	ithere,
	ithx,
	ithy,
	iseenit,
	probecx,
	probecy,
	proben,
	isarmed,
	nprobes;
    double inresor,
	intime,
	inenrg,
	inshld,
	inlsr,
	indate,
	energy,
	shield,
	shldup,
	warpfac,
	wfacsq,
	lsupres,
	dist,
	direc,
	Time,
	docfac,
	resting,
	damfac,
	stdamtim,
	cryprob,
	probex,
	probey,
	probeinx,
	probeiny;
} game;

#define inkling game.inkling		// Initial number of klingons
#define inbase game.inbase			// Initial number of bases
#define incom game.incom			// Initian number of commanders
#define instar game.instar			// Initial stars
#define intorps game.intorps		// Initial/Max torpedoes
#define condit game.condit			// Condition (red, yellow, green docked)
#define torps game.torps			// number of torpedoes
#define ship game.ship				// Ship type -- 'E' is Enterprise
#define quadx game.quadx			// where we are
#define quady game.quady			//
#define sectx game.sectx			// where we are
#define secty game.secty			//
#define length game.length			// length of game
#define skill game.skill			// skill level
#define basex game.basex			// position of base in current quad
#define basey game.basey			//
#define klhere game.klhere			// klingons here
#define comhere game.comhere		// commanders here
#define casual game.casual			// causalties
#define nhelp game.nhelp			// calls for help
#define nkinks game.nkinks			//
#define ididit game.ididit			// Action taken -- allows enemy to attack
#define gamewon game.gamewon		// Finished!
#define alive game.alive			// We are alive (not killed)
#define justin game.justin			// just entered quadrant
#define alldone game.alldone		// game is now finished
#define shldchg game.shldchg		// shield is changing (affects efficiency)
#define plnetx game.plnetx			// location of planet in quadrant
#define plnety game.plnety			//
#define inorbit game.inorbit		// orbiting
#define landed game.landed			// party on planet (1), on ship (-1)
#define iplnet game.iplnet			// planet # in quadrant
#define imine game.imine			// mining
#define inplan game.inplan			// initial planets
#define nenhere game.nenhere		// Number of enemies in quadrant
#define ishere game.ishere			// Super-commander in quandrant
#define neutz game.neutz			// Romulan Neutral Zone
#define irhere game.irhere			// Romulans in quadrant
#define icraft game.icraft			// Kirk in Galileo
#define ientesc game.ientesc		// Attempted escape from supercommander
#define iscraft game.iscraft		// =1 if craft on ship, -1 if removed from game
#define isatb game.isatb			// =1 if SuperCommander is attacking base
#define iscate game.iscate			// Super Commander is here
#ifdef DEBUG
#define idebug game.idebug			// Debug mode
#endif
#define iattak game.iattak			// attack recursion elimination (was cracks[4])
#define icrystl game.icrystl		// dilithium crystals aboard
#define tourn game.tourn			// Tournament number
#define thawed game.thawed			// Thawed game
#define batx game.batx				// Base coordinates being attacked
#define baty game.baty				//
#define ithere game.ithere			// Tholean is here 
#define ithx game.ithx				// coordinates of tholean
#define ithy game.ithy
#define iseenit game.iseenit		// Seen base attack report
#define inresor game.inresor		// initial resources
#define intime game.intime			// initial time
#define inenrg game.inenrg			// Initial/Max Energy
#define inshld game.inshld			// Initial/Max Shield
#define inlsr game.inlsr			// initial life support resources
#define indate game.indate			// Initial date
#define energy game.energy			// Energy level
#define shield game.shield			// Shield level
#define shldup game.shldup			// Shields are up
#define warpfac game.warpfac		// Warp speed
#define wfacsq game.wfacsq			// squared warp factor
#define lsupres game.lsupres		// life support reserves
#define dist game.dist				// movement distance
#define direc game.direc			// movement direction
#define Time game.Time				// time taken by current operation
#define docfac game.docfac			// repair factor when docking (constant?)
#define resting game.resting		// rest time
#define damfac game.damfac			// damage factor
#define stdamtim game.stdamtim		// time that star chart was damaged
#define cryprob game.cryprob		// probability that crystal will work
#define probex game.probex			// location of probe
#define probey game.probey
#define probecx game.probecx		// current probe quadrant
#define probecy game.probecy	
#define probeinx game.probeinx		// Probe x,y increment
#define probeiny game.probeiny		
#define proben game.proben			// number of moves for probe
#define isarmed game.isarmed		// Probe is armed
#define nprobes game.nprobes		// number of probes available

/* the following global state doesn't need to be saved */
EXTERN char	*device[NDEVICES];
EXTERN int iscore, iskill; // Common PLAQ
EXTERN double perdate;
EXTERN double aaitem;
EXTERN char citem[10];

/* the Space Thingy's global state should *not* be saved! */
EXTERN int thingx, thingy, iqhere, iqengry;

typedef enum {FWON, FDEPLETE, FLIFESUP, FNRG, FBATTLE,
              FNEG3, FNOVA, FSNOVAED, FABANDN, FDILITHIUM,
			  FMATERIALIZE, FPHASER, FLOST, FMINING, FDPLANET,
			  FPNOVA, FSSC, FSTRACTOR, FDRAY, FTRIBBLE,
			  FHOLE} FINTYPE ;
enum loctype {neither, quadrant, sector};

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

#ifdef INCLUDED
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
	"Transporter",
	"Shield Control",
	"Death Ray",
	"D. S. Probe"};									
#endif

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

#define IHR 'R'
#define IHK 'K'
#define IHC 'C'
#define IHS 'S'
#define IHSTAR '*'
#define IHP 'P'
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
int choose(int);
void setup(int);
void score(void);
void atover(int);
int srscan(int);
void lrscan(void);
void phasers(void);
void photon(void);
void warp(int);
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
void help(void);
void abandn(void);
void finish(FINTYPE);
void dstrct(void);
void kaboom(void);
void freeze(int);
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
int ja(void);
void cramen(int);
void crmshp(void);
char *cramlc(enum loctype, int, int);
double expran(double);
double Rand(void);
void iran(int, int *, int *);
#define square(i) ((i)*(i))
void dropin(int, int*, int*);
void newcnd(void);
void sortkl(void);
void imove(void);
void ram(int, int, int, int);
void crmena(int, int, int, int, int);
void deadkl(int, int, int, int, int);
void timwrp(void);
void movcom(void);
void torpedo(double, double, int, int, double *, int, int, int);
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
void sensor(int);
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
void tracktorpedo(int x, int y, int ix, int iy, int wait, int l, int i, int n, int iquad);
void cgetline(char *, int);
void waitfor(void);
void setpassword(void);
void commandhook(char *, int);
void makechart(void);
void enqueue(char *);

/* mode arguments for srscan() */
#define SCAN_FULL		1
#define SCAN_REQUEST		2
#define SCAN_STATUS		3
#define SCAN_NO_LEFTSIDE	4

WINDOW *fullscreen_window;
WINDOW *srscan_window;
WINDOW *report_window;
WINDOW *lrscan_window;
WINDOW *message_window;
WINDOW *prompt_window;

extern void clreol(void);
extern void clrscr(void);
extern void textcolor(int color);
extern void highvideo(void);

enum COLORS {
   DEFAULT,
   BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY,
   DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, YELLOW, WHITE
};

#define DAMAGED	128	/* marker for damaged ship in starmap */
