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
} d, snapsht;			// Data that is snapshot

EXTERN char
		quad[11][11];	// contents of our quadrant

// Scalar variables that are needed for freezing the game
// are placed in a structure. #defines are used to access by their
// original names. Gee, I could have done this with the d structure,
// but I just didn't think of it back when I started.

EXTERN struct foo2 {
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
#define thingx game.thingx			// location of strange object in galaxy
#define thingy game.thingy			//
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

EXTERN int
		kx[21],			// enemy sector locations
		ky[21],
		starch[9][9];	// star chart

EXTERN int fromcommandline; // Game start from command line options


EXTERN char	passwd[10],		// Self Destruct password
		*device[ndevice+1];

EXTERN PLANETS nulplanet;	// zeroed planet structure

EXTERN double
		kpower[21],		// enemy energy levels
		kdist[21],		// enemy distances
		kavgd[21],		// average distances
		damage[ndevice+1],		// damage encountered
		future[NEVENTS+1];		// future events

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
