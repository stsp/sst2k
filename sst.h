#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef INCLUDED
#define EXTERN extern
#else
#define EXTERN
#endif

// #define DEBUG

#define ndevice (15)	// Number of devices
#define phasefac (2.0)
#define PLNETMAX (10)
#define NEVENTS (8)

typedef struct {
	int x;	/* Quadrant location of planet */
	int y;
	int pclass; /* class M, N, or O (1, 2, or 3) */
	int crystals; /* has crystals */
	int known;   /* =1 contents known, =2 shuttle on this planet */
} PLANETS;

EXTERN struct foo {
		int snap,		// snapshot taken
		remkl,			// remaining klingons
	    remcom,			// remaining commanders
		rembase,		// remaining bases
		starkl,			// destroyed stars
		basekl,			// destroyed bases
		killk,			// Klingons killed
		killc,			// commanders killed
		galaxy[9][9], 	// The Galaxy (subscript 0 not used)
		cx[11],cy[11],	// Commander quadrant coordinates
		baseqx[6],		// Base quadrant X
		baseqy[6],		// Base quadrant Y
		newstuf[9][9],	// Extended galaxy goodies
		isx, isy,		// Coordinate of Super Commander
		nscrem,			// remaining super commanders
		nromkl,			// Romulans killed
		nromrem,		// Romulans remaining
		nsckill,		// super commanders killed
		nplankl;		// destroyed planets
	PLANETS plnets[PLNETMAX+1];  // Planet information
	double date,		// stardate
		remres,			// remaining resources
	    remtime;		// remaining time
} state, snapsht;			// Data that is snapshot

// Scalar variables that are needed for freezing the game
// are placed in a structure. #defines are used to access by their
// original names. Gee, I could have done this with the d structure,
// but I just didn't think of it back when I started.

EXTERN struct foo2 {
    char quad[11][11];		// contents of our quadrant
    double kpower[21];		// enemy energy levels
    double kdist[21];		// enemy distances
    double kavgd[21];		// average distances
    double damage[ndevice+1];	// damage encountered
    double future[NEVENTS+1];	// future events
    char passwd[10];		// Self Destruct password
    int kx[21];			// enemy sector locations
    int ky[21];
    int starch[9][9];		// star chart
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
	thingx,
	thingy,
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
} frozen;

#define inkling frozen.inkling		// Initial number of klingons
#define inbase frozen.inbase			// Initial number of bases
#define incom frozen.incom			// Initian number of commanders
#define instar frozen.instar			// Initial stars
#define intorps frozen.intorps		// Initial/Max torpedoes
#define condit frozen.condit			// Condition (red, yellow, green docked)
#define torps frozen.torps			// number of torpedoes
#define ship frozen.ship				// Ship type -- 'E' is Enterprise
#define quadx frozen.quadx			// where we are
#define quady frozen.quady			//
#define sectx frozen.sectx			// where we are
#define secty frozen.secty			//
#define length frozen.length			// length of game
#define skill frozen.skill			// skill level
#define basex frozen.basex			// position of base in current quad
#define basey frozen.basey			//
#define klhere frozen.klhere			// klingons here
#define comhere frozen.comhere		// commanders here
#define casual frozen.casual			// causalties
#define nhelp frozen.nhelp			// calls for help
#define nkinks frozen.nkinks			//
#define ididit frozen.ididit			// Action taken -- allows enemy to attack
#define gamewon frozen.gamewon		// Finished!
#define alive frozen.alive			// We are alive (not killed)
#define justin frozen.justin			// just entered quadrant
#define alldone frozen.alldone		// game is now finished
#define shldchg frozen.shldchg		// shield is changing (affects efficiency)
#define thingx frozen.thingx			// location of strange object in galaxy
#define thingy frozen.thingy			//
#define plnetx frozen.plnetx			// location of planet in quadrant
#define plnety frozen.plnety			//
#define inorbit frozen.inorbit		// orbiting
#define landed frozen.landed			// party on planet (1), on ship (-1)
#define iplnet frozen.iplnet			// planet # in quadrant
#define imine frozen.imine			// mining
#define inplan frozen.inplan			// initial planets
#define nenhere frozen.nenhere		// Number of enemies in quadrant
#define ishere frozen.ishere			// Super-commander in quandrant
#define neutz frozen.neutz			// Romulan Neutral Zone
#define irhere frozen.irhere			// Romulans in quadrant
#define icraft frozen.icraft			// Kirk in Galileo
#define ientesc frozen.ientesc		// Attempted escape from supercommander
#define iscraft frozen.iscraft		// =1 if craft on ship, -1 if removed from game
#define isatb frozen.isatb			// =1 if SuperCommander is attacking base
#define iscate frozen.iscate			// Super Commander is here
#ifdef DEBUG
#define idebug frozen.idebug			// Debug mode
#endif
#define iattak frozen.iattak			// attack recursion elimination (was cracks[4])
#define icrystl frozen.icrystl		// dilithium crystals aboard
#define tourn frozen.tourn			// Tournament number
#define thawed frozen.thawed			// Thawed game
#define batx frozen.batx				// Base coordinates being attacked
#define baty frozen.baty				//
#define ithere frozen.ithere			// Tholean is here 
#define ithx frozen.ithx				// coordinates of tholean
#define ithy frozen.ithy
#define iseenit frozen.iseenit		// Seen base attack report
#define inresor frozen.inresor		// initial resources
#define intime frozen.intime			// initial time
#define inenrg frozen.inenrg			// Initial/Max Energy
#define inshld frozen.inshld			// Initial/Max Shield
#define inlsr frozen.inlsr			// initial life support resources
#define indate frozen.indate			// Initial date
#define energy frozen.energy			// Energy level
#define shield frozen.shield			// Shield level
#define shldup frozen.shldup			// Shields are up
#define warpfac frozen.warpfac		// Warp speed
#define wfacsq frozen.wfacsq			// squared warp factor
#define lsupres frozen.lsupres		// life support reserves
#define dist frozen.dist				// movement distance
#define direc frozen.direc			// movement direction
#define Time frozen.Time				// time taken by current operation
#define docfac frozen.docfac			// repair factor when docking (constant?)
#define resting frozen.resting		// rest time
#define damfac frozen.damfac			// damage factor
#define stdamtim frozen.stdamtim		// time that star chart was damaged
#define cryprob frozen.cryprob		// probability that crystal will work
#define probex frozen.probex			// location of probe
#define probey frozen.probey
#define probecx frozen.probecx		// current probe quadrant
#define probecy frozen.probecy	
#define probeinx frozen.probeinx		// Probe x,y increment
#define probeiny frozen.probeiny		
#define proben frozen.proben			// number of moves for probe
#define isarmed frozen.isarmed		// Probe is armed
#define nprobes frozen.nprobes		// number of probes available

/* the following global state doesn't need to be saved */
EXTERN int fromcommandline; // Game start from command line options
EXTERN char	*device[ndevice+1];
EXTERN PLANETS nulplanet;	// zeroed planet structure
EXTERN int iscore, iskill; // Common PLAQ
EXTERN double perdate;

typedef enum {FWON, FDEPLETE, FLIFESUP, FNRG, FBATTLE,
              FNEG3, FNOVA, FSNOVAED, FABANDN, FDILITHIUM,
			  FMATERIALIZE, FPHASER, FLOST, FMINING, FDPLANET,
			  FPNOVA, FSSC, FSTRACTOR, FDRAY, FTRIBBLE,
			  FHOLE} FINTYPE ;


EXTERN double aaitem;
EXTERN char citem[10];


/* Define devices */
#define DSRSENS 1
#define DLRSENS 2
#define DPHASER 3
#define DPHOTON 4
#define DLIFSUP 5
#define DWARPEN 6
#define DIMPULS 7
#define DSHIELD 8
#define DRADIO  9
#define DSHUTTL 10
#define DCOMPTR 11
#define DTRANSP 12
#define DSHCTRL 13
#define DDRAY   14  // Added deathray
#define DDSP    15  // Added deep space probe

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
PLANETS nulplanet = {0};
char *device[ndevice+1] = {
	"",
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

#define TRUE (1)
#define FALSE (0)

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


/* Function prototypes */
void prelim(void);
void attack(int);
int choose(void);
void setup(void);
void score(void);
void atover(int);
void srscan(int);
void lrscan(void);
void phasers(void);
void photon(void);
void warp(int);
void doshield(int);
void dock(void);
void dreprt(void);
void chart(int);
void impuls(void);
void wait(void);
void setwrp(void);
void events(void);
void report(int);
void eta(void);
void help(void);
void abandn(void);
void finish(FINTYPE);
void dstrct(void);
void kaboom(void);
void freeze(int);
void thaw(void);
void plaque(void);
int scan(void);
#define IHEOL (0)
#define IHALPHA (1)
#define IHREAL (2)
void chew(void);
void chew2(void);
void skip(int);
void prout(char *s);
void proutn(char *s);
void stars(void);
void newqad(int);
int ja(void);
void cramen(int);
void crmshp(void);
void cramlc(int, int, int);
double expran(double);
double Rand(void);
void iran8(int *, int *);
void iran10(int *, int *);
double square(double);
void dropin(int, int*, int*);
void newcnd(void);
void sortkl(void);
void move(void);
void ram(int, int, int, int);
void crmena(int, int, int, int, int);
void deadkl(int, int, int, int, int);
void timwrp(void);
void movcom(void);
void torpedo(double, double, int, int, double *);
void cramf(double, int, int);
void crami(int, int);
void huh(void);
void pause(int);
void nova(int, int);
void snova(int, int);
void scom(int *);
void hittem(double *);
void prouts(char *);
int isit(char *);
void preport(void);
void orbit(void);
void sensor(void);
void beam(void);
void mine(void);
void usecrystals(void);
void shuttle(void);
void deathray(void);
void debugme(void);
void attakreport(void);
void movetho(void);
void probe(void);
