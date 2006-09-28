#include <time.h>
#include <sys/stat.h>
#include "sst.h"

static long filelength(int fd) {
struct stat buf;
    fstat(fd, &buf);
    return buf.st_size;
}

void prelim(void)
/* issue a historically correct banner */
{
    skip(2);
    prout(_("-SUPER- STAR TREK"));
    skip(1);
#ifdef __HISTORICAL__
    prout(_("Latest update-21 Sept 78"));
    skip(1);
#endif /* __HISTORICAL__ */
}

void freeze(bool boss)
/* save game */
{
    FILE *fp;
    int key;
    if (boss) {
	strcpy(citem, "emsave.trk");
    }
    else {
	if ((key = scan()) == IHEOL) {
	    proutn(_("File name: "));
	    key = scan();
	}
	if (key != IHALPHA) {
	    huh();
	    return;
	}
	chew();
	if (strchr(citem, '.') == NULL) {
	    strcat(citem, ".trk");
	}
    }
    if ((fp = fopen(citem, "wb")) == NULL) {
	proutn(_("Can't freeze game as file "));
	proutn(citem);
	skip(1);
	return;
    }
    strcpy(game.magic, SSTMAGIC);
    fwrite(&game, sizeof(game), 1, fp);

    fclose(fp);
}


bool thaw(void) 
/* retrieve saved game */
{
    FILE *fp;
    int key;

    game.passwd[0] = '\0';
    if ((key = scan()) == IHEOL) {
	proutn(_("File name: "));
	key = scan();
    }
    if (key != IHALPHA) {
	huh();
	return true;
    }
    chew();
    if (strchr(citem, '.') == NULL) {
	strcat(citem, ".trk");
    }
    if ((fp = fopen(citem, "rb")) == NULL) {
	proutn(_("Can't find game file "));
	proutn(citem);
	skip(1);
	return 1;
    }
    fread(&game, sizeof(game), 1, fp);
    if (feof(fp) || ftell(fp) != filelength(fileno(fp)) || strcmp(game.magic, SSTMAGIC)) {
	proutn(_("Game file format is bad, should begin with "));
	prout(SSTMAGIC);
	skip(1);
	fclose(fp);
	return 1;
    }

    fclose(fp);

    return false;
}

#define SYSTEM_NAMES \
    { \
	/* \
	 * I used <http://www.memory-alpha.org> to find planets \
	 * with references in ST:TOS.  Eath and the Alpha Centauri \
	 * Colony have been omitted. \
 	 * \
	 * Some planets marked Class G and P here will be displayed as class M \
	 * because of the way planets are generated. This is a known bug. \
	 */ \
	"ERROR", \
	/* Federation Worlds */ \
	_("Andoria (Fesoan)"),	/* several episodes */ \
	_("Tellar Prime (Miracht)"),	/* TOS: "Journey to Babel" */ \
	_("Vulcan (T'Khasi)"),	/* many episodes */ \
	_("Medusa"),		/* TOS: "Is There in Truth No Beauty?" */ \
	_("Argelius II (Nelphia)"),/* TOS: "Wolf in the Fold" ("IV" in BSD) */ \
	_("Ardana"),		/* TOS: "The Cloud Minders" */ \
	_("Catulla (Cendo-Prae)"),	/* TOS: "The Way to Eden" */ \
	_("Gideon"),		/* TOS: "The Mark of Gideon" */ \
	_("Aldebaran III"),	/* TOS: "The Deadly Years" */ \
	_("Alpha Majoris I"),	/* TOS: "Wolf in the Fold" */ \
	_("Altair IV"),		/* TOS: "Amok Time */ \
	_("Ariannus"),		/* TOS: "Let That Be Your Last Battlefield" */ \
	_("Benecia"),		/* TOS: "The Conscience of the King" */ \
	_("Beta Niobe I (Sarpeidon)"),	/* TOS: "All Our Yesterdays" */ \
	_("Alpha Carinae II"),	/* TOS: "The Ultimate Computer" */ \
	_("Capella IV (Kohath)"),	/* TOS: "Friday's Child" (Class G) */ \
	_("Daran V"),		/* TOS: "For the World is Hollow and I Have Touched the Sky" */ \
	_("Deneb II"),		/* TOS: "Wolf in the Fold" ("IV" in BSD) */ \
	_("Eminiar VII"),		/* TOS: "A Taste of Armageddon" */ \
	_("Gamma Canaris IV"),	/* TOS: "Metamorphosis" */ \
	_("Gamma Tranguli VI (Vaalel)"),	/* TOS: "The Apple" */ \
	_("Ingraham B"),		/* TOS: "Operation: Annihilate" */ \
	_("Janus IV"),		/* TOS: "The Devil in the Dark" */ \
	_("Makus III"),		/* TOS: "The Galileo Seven" */ \
	_("Marcos XII"),		/* TOS: "And the Children Shall Lead", */ \
	_("Omega IV"),		/* TOS: "The Omega Glory" */ \
	_("Regulus V"),		/* TOS: "Amok Time */ \
	_("Deneva"),		/* TOS: "Operation -- Annihilate!" */ \
	/* Worlds from BSD Trek */ \
	_("Rigel II"),		/* TOS: "Shore Leave" ("III" in BSD) */ \
	_("Beta III"),		/* TOS: "The Return of the Archons" */ \
	_("Triacus"),		/* TOS: "And the Children Shall Lead", */ \
	_("Exo III"),		/* TOS: "What Are Little Girls Made Of?" (Class P) */ \
    }
#if 0	/* Others */
	_("Hansen's Planet"),	/* TOS: "The Galileo Seven" */
	_("Taurus IV"),		/* TOS: "The Galileo Seven" (class G) */
	_("Antos IV (Doraphane)"),	/* TOS: "Whom Gods Destroy", "Who Mourns for Adonais?" */
	_("Izar"),			/* TOS: "Whom Gods Destroy" */
	_("Tiburon"),		/* TOS: "The Way to Eden" */
	_("Merak II"),		/* TOS: "The Cloud Minders" */
	_("Coridan (Desotriana)"),	/* TOS: "Journey to Babel" */
	_("Iotia"),		/* TOS: "A Piece of the Action" */
#endif

#define DEVICE_NAMES \
    { \
	_("S. R. Sensors"), \
	_("L. R. Sensors"), \
	_("Phasers"), \
	_("Photon Tubes"), \
	_("Life Support"), \
	_("Warp Engines"), \
	_("Impulse Engines"), \
	_("Shields"), \
	_("Subspace Radio"), \
	_("Shuttle Craft"), \
	_("Computer"), \
	_("Navigation System"), \
	_("Transporter"), \
	_("Shield Control"), \
	_("Death Ray"), \
	_("D. S. Probe") \
    }

static void setup_names(void)
/* Sets up some arrays with localized names.
 * Must be done after iostart() for localization to work. */
{
    char *tmp1[ARRAY_SIZE(systnames)] = SYSTEM_NAMES;
    char *tmp2[ARRAY_SIZE(device)] = DEVICE_NAMES;

    memcpy(systnames, tmp1, sizeof(systnames));
    memcpy(device, tmp2, sizeof(device));
}
	
void setup(bool needprompt) 
/* prepare to play, set up cosmos */
{
    int i,j, krem, klumper;
    coord w;

    /* call the setup hooks here */
    setup_names();

    //  Decide how many of everything
    if (choose(needprompt)) return; // frozen game
    // Prepare the Enterprise
    game.alldone = game.gamewon = false;
    game.ship = IHE;
    game.state.crew = FULLCREW;
    game.energy = game.inenrg = 5000.0;
    game.shield = game.inshld = 2500.0;
    game.shldchg = false;
    game.shldup = false;
    game.inlsr = 4.0;
    game.lsupres = 4.0;
    game.quadrant = randplace(GALSIZE);
    game.sector = randplace(QUADSIZE);
    game.torps = game.intorps = 10;
    game.nprobes = (int)(3.0*Rand() + 2.0);	/* Give them 2-4 of these wonders */
    game.warpfac = 5.0;
    game.wfacsq = game.warpfac * game.warpfac;
    for (i=0; i < NDEVICES; i++) 
	game.damage[i] = 0.0;
    // Set up assorted game parameters
    invalidate(game.battle);
    game.state.date = game.indate = 100.0*(int)(31.0*Rand()+20.0);
    game.nkinks = game.nhelp = game.casual = game.abandoned = 0;
    game.iscate = game.resting = game.imine = game.icrystl = game.icraft = false;
    game.isatb = game.state.nplankl = 0;
    game.state.starkl = game.state.basekl = 0;
    game.iscraft = onship;
    game.landed = false;
    game.alive = true;
    game.docfac = 0.25;
    for_quadrants(i)
	for_quadrants(j) {
	struct quadrant *quad = &game.state.galaxy[i][j];
	    quad->charted = 0;
	    quad->planet = NOPLANET;
	    quad->romulans = 0;
	    quad->klingons = 0;
	    quad->starbase = false;
	    quad->supernova = false;
	    quad->status = secure;
	}
    // Initialize times for extraneous events
    schedule(FSNOVA, expran(0.5 * game.intime));
    schedule(FTBEAM, expran(1.5 * (game.intime / game.state.remcom)));
    schedule(FSNAP, 1.0 + Rand()); // Force an early snapshot
    schedule(FBATTAK, expran(0.3*game.intime));
    unschedule(FCDBAS);
    if (game.state.nscrem)
	schedule(FSCMOVE, 0.2777);
    else
	unschedule(FSCMOVE);
    unschedule(FSCDBAS);
    unschedule(FDSPROB);
    if ((game.options & OPTION_WORLDS) && game.skill >= SKILL_GOOD)
	schedule(FDISTR, expran(1.0 + game.intime));
    else
	unschedule(FDISTR);
    unschedule(FENSLV);
    unschedule(FREPRO);
    // Starchart is functional but we've never seen it
    game.lastchart = FOREVER;
    // Put stars in the galaxy
    game.instar = 0;
    for_quadrants(i)
	for_quadrants(j) {
	    int k = Rand()*9.0 + 1.0;
	    game.instar += k;
	    game.state.galaxy[i][j].stars = k;
	}
    // Locate star bases in galaxy
    for (i = 1; i <= game.inbase; i++) {
	bool contflag;
	do {
	    do w = randplace(GALSIZE);
	    while (game.state.galaxy[w.x][w.y].starbase);
	    contflag = false;
	    for (j = i-1; j > 0; j--) {
		/* Improved placement algorithm to spread out bases */
		double distq = square(w.x-game.state.baseq[j].x) + square(w.y-game.state.baseq[j].y);
		if (distq < 6.0*(BASEMAX+1-game.inbase) && Rand() < 0.75) {
		    contflag = true;
		    if (idebug)
			prout("=== Abandoning base #%d at %d-%d", i, w.x, w.y);
		    break;
		}
		else if (distq < 6.0 * (BASEMAX+1-game.inbase)) {
		    if (idebug)
			prout("=== Saving base #%d, close to #%d", i, j);
		}
	    }
	} while (contflag);
			
	game.state.baseq[i] = w;
	game.state.galaxy[w.x][w.y].starbase = true;
	game.state.chart[w.x][w.y].starbase = true;
    }
    // Position ordinary Klingon Battle Cruisers
    krem = game.inkling;
    klumper = 0.25*game.skill*(9.0-game.length)+1.0;
    if (klumper > MAXKLQUAD) 
	klumper = MAXKLQUAD;
    do {
	double r = Rand();
	int klump = (1.0 - r*r)*klumper;
	if (klump > krem)
	    klump = krem;
	krem -= klump;
	do w = randplace(GALSIZE);
	while (game.state.galaxy[w.x][w.y].supernova ||
		game.state.galaxy[w.x][w.y].klingons + klump > 9);
	game.state.galaxy[w.x][w.y].klingons += klump;
    } while (krem > 0);
    // Position Klingon Commander Ships
#ifdef ODEBUG
    klumper = 1;
#endif /* ODEBUG */
    for (i = 1; i <= game.incom; i++) {
	do {
	    do { /* IF debugging, put commanders by bases, always! */
#ifdef ODEBUG
		if (game.idebug && klumper <= game.inbase) {
		    w = game.state.baseq[klumper];
		    klumper++;
		}
		else
#endif /* ODEBUG */
		    w = randplace(GALSIZE);
	    }
	    while ((!game.state.galaxy[w.x][w.y].klingons && Rand() < 0.75)||
		   game.state.galaxy[w.x][w.y].supernova||
		   game.state.galaxy[w.x][w.y].klingons > 8);
	    // check for duplicate
	    for (j = 1; j < i; j++)
		if (same(game.state.kcmdr[j], w))
		    break;
	} while (j < i);
	game.state.galaxy[w.x][w.y].klingons++;
	game.state.kcmdr[i] = w;
    }
    // Locate planets in galaxy
    for (i = 0; i < game.inplan; i++) {
	do w = randplace(GALSIZE); 
	while (game.state.galaxy[w.x][w.y].planet != NOPLANET);
	game.state.plnets[i].w = w;
	if (i < NINHAB) {
	    game.state.plnets[i].pclass = M;	// All inhabited planets are class M
	    game.state.plnets[i].crystals = absent;
	    game.state.plnets[i].known = known;
	    game.state.plnets[i].inhabited = i;
	} else {
	    game.state.plnets[i].pclass = Rand()*3.0; // Planet class M N or O
	    game.state.plnets[i].crystals = Rand()*1.5;		// 1 in 3 chance of crystals
	    game.state.plnets[i].known = unknown;
	    game.state.plnets[i].inhabited = UNINHABITED;
	}
	if ((game.options & OPTION_WORLDS) || i >= NINHAB)
	    game.state.galaxy[w.x][w.y].planet = i;
    }
    // Locate Romulans
    for (i = 1; i <= game.state.nromrem; i++) {
	w = randplace(GALSIZE);
	game.state.galaxy[w.x][w.y].romulans = 1;
    }
    // Locate the Super Commander
    if (game.state.nscrem > 0) {
	do w = randplace(GALSIZE);
	while (game.state.galaxy[w.x][w.y].supernova || game.state.galaxy[w.x][w.y].klingons > 8);
	game.state.kscmdr = w;
	game.state.galaxy[w.x][w.y].klingons++;
    }
    // Place thing (in tournament game, thingx == -1, don't want one!)
    if (thing.x != -1) {
	thing = randplace(GALSIZE);
    }
    else
	invalidate(thing);

    skip(2);
    game.state.snap = false;
		
    if (game.skill == SKILL_NOVICE) {
	prout(_("It is stardate %d. The Federation is being attacked by"),
	      (int)game.state.date);
	prout(_("a deadly Klingon invasion force. As captain of the United"));
	prout(_("Starship U.S.S. Enterprise, it is your mission to seek out"));
	prout(_("and destroy this invasion force of %d battle cruisers."),
	      INKLINGTOT);
	prout(_("You have an initial allotment of %d stardates to complete"), (int)game.intime);
	prout(_("your mission.  As you proceed you may be given more time."));
	skip(1);
	prout(_("You will have %d supporting starbases."), game.inbase);
	proutn(_("Starbase locations-  "));
    }
    else {
	prout(_("Stardate %d."), (int)game.state.date);
	skip(1);
	prout(_("%d Klingons."), INKLINGTOT);
	prout(_("An unknown number of Romulans."));
	if (game.state.nscrem)
	    prout(_("And one (GULP) Super-Commander."));
	prout(_("%d stardates."),(int)game.intime);
	proutn(_("%d starbases in "), game.inbase);
    }
    for (i = 1; i <= game.inbase; i++) {
	proutn(cramlc(0, game.state.baseq[i]));
	proutn("  ");
    }
    skip(2);
    proutn(_("The Enterprise is currently in "));
    proutn(cramlc(quadrant, game.quadrant));
    proutn(" ");
    proutn(cramlc(sector, game.sector));
    skip(2);
    prout(_("Good Luck!"));
    if (game.state.nscrem)
	prout(_("  YOU'LL NEED IT."));
    waitfor();
    newqad(false);
    if (game.nenhere-iqhere-game.ithere)
	game.shldup = true;
    if (game.neutz)	// bad luck to start in a Romulan Neutral Zone
	attack(false);
}

bool choose(bool needprompt) 
/* choose your game type */
{
    for(;;) {
	game.tourn = 0;
	game.thawed = false;
	game.skill = SKILL_NONE;
	game.length = 0;
	if (needprompt) /* Can start with command line options */
	    proutn(_("Would you like a regular, tournament, or saved game? "));
	scan();
	if (strlen(citem)==0) continue; // Try again
	if (isit("tournament")) {
	    while (scan() == IHEOL) {
		proutn(_("Type in tournament number-"));
	    }
	    if (aaitem == 0) {
		chew();
		continue; // We don't want a blank entry
	    }
	    game.tourn = (int)aaitem;
	    thing.x = -1;
	    srand((unsigned int)(int)aaitem);
	    break;
	}
	if (isit("saved") || isit("frozen")) {
	    if (thaw())
		continue;
	    chew();
	    if (*game.passwd==0)
		continue;
	    if (!game.alldone)
		game.thawed = true; // No plaque if not finished
	    report();
	    waitfor();
	    return true;
	}
	if (isit("regular"))
	    break;
	proutn(_("What is \""));
	proutn(citem);
	prout("\"?");
	chew();
    }
    while (game.length==0 || game.skill==SKILL_NONE) {
	if (scan() == IHALPHA) {
	    if (isit("short"))
		game.length = 1;
	    else if (isit("medium"))
		game.length = 2;
	    else if (isit("long"))
		game.length = 4;
	    else if (isit("novice"))
		game.skill = SKILL_NOVICE;
	    else if (isit("fair"))
		game.skill = SKILL_FAIR;
	    else if (isit("good"))
		game.skill = SKILL_GOOD;
	    else if (isit("expert"))
		game.skill = SKILL_EXPERT;
	    else if (isit("emeritus"))
		game.skill = SKILL_EMERITUS;
	    else {
		proutn(_("What is \""));
		proutn(citem);
		prout("\"?");
	    }
	}
	else {
	    chew();
	    if (game.length==0)
		proutn(_("Would you like a Short, Medium, or Long game? "));
	    else if (game.skill == SKILL_NONE)
		proutn(_("Are you a Novice, Fair, Good, Expert, or Emeritus player? "));
	}
    }
    // Choose game options -- added by ESR for SST2K
    if (scan() != IHALPHA) {
	chew();
	proutn(_("Choose your game style (or just press enter): "));
	scan();
    }
    if (isit("plain")) {
	// Approximates the UT FORTRAN version.
	game.options &=~ (OPTION_THOLIAN | OPTION_PLANETS | OPTION_THINGY | OPTION_PROBE | OPTION_RAMMING | OPTION_MVBADDY | OPTION_BLKHOLE | OPTION_BASE | OPTION_WORLDS);
	game.options |= OPTION_PLAIN;
    } 
    else if (isit("almy")) {
	// Approximates Tom Almy's version.
	game.options &=~ (OPTION_THINGY | OPTION_BLKHOLE | OPTION_BASE | OPTION_WORLDS);
	game.options |= OPTION_ALMY;
    }
    else if (isit("fancy"))
	/* do nothing */;
    else if (strlen(citem)) {
	    proutn(_("What is \""));
	    proutn(citem);
	    prout("\"?");
    }
    setpassword();
    if (strcmp(game.passwd, "debug")==0) {
	idebug = true;
	fputs("=== Debug mode enabled\n", stdout);
    }

    // Use parameters to generate initial values of things
    game.damfac = 0.5 * game.skill;
    game.state.rembase = 2.0 + Rand()*(BASEMAX-2.0);
    game.inbase = game.state.rembase;
    if (game.options & OPTION_PLANETS)
	game.inplan = NINHAB + (MAXUNINHAB/2) + (MAXUNINHAB/2+1)*Rand();
    game.state.nromrem = game.inrom = (2.0+Rand())*game.skill;
    game.state.nscrem = game.inscom = (game.skill > SKILL_FAIR ? 1 : 0);
    game.state.remtime = 7.0 * game.length;
    game.intime = game.state.remtime;
    game.state.remkl = game.inkling = 2.0*game.intime*((game.skill+1 - 2*Rand())*game.skill*0.1+.15);
    game.incom = game.skill + 0.0625*game.inkling*Rand();
    game.state.remcom = min(10, game.incom);
    game.incom = game.state.remcom;
    game.state.remres = (game.inkling+4*game.incom)*game.intime;
    game.inresor = game.state.remres;
    if (game.inkling > 50) {
	game.inbase = (game.state.rembase += 1);
    }
    return false;
}

coord dropin(feature iquad)
/* drop a feature on a random dot in the current quadrant */
{
    coord w;
    do w = randplace(QUADSIZE);
    while (game.quad[w.x][w.y] != IHDOT);
    game.quad[w.x][w.y] = iquad;
    return w;
}

void newcnd(void)
/* update our alert status */
{
    game.condition = green;
    if (game.energy < 1000.0)
	game.condition = yellow;
    if (game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons || game.state.galaxy[game.quadrant.x][game.quadrant.y].romulans)
	game.condition = red;
    if (!game.alive)
	game.condition=dead;
}

coord newkling(int i)
/* drop new Klingon into current quadrant */
{
    coord pi = dropin(IHK);
    game.ks[i] = pi;
    game.kdist[i] = game.kavgd[i] = distance(game.sector, pi);
    game.kpower[i] = Rand()*150.0 +300.0 +25.0*game.skill;
    return pi;
}

void newqad(bool shutup)
/* set up a new state of quadrant, for when we enter or re-enter it */
{
    int i, j;
    coord w;
    struct quadrant *q;

    game.justin = true;
    invalidate(game.base);
    game.klhere = 0;
    game.comhere = false;
    invalidate(game.plnet);
    game.ishere = false;
    game.irhere = 0;
    game.iplnet = 0;
    game.nenhere = 0;
    game.neutz = false;
    game.inorbit = false;
    game.landed = false;
    game.ientesc = false;
    game.ithere = false;
    iqhere = false;
    iqengry = false;
    game.iseenit = false;
    if (game.iscate) {
	// Attempt to escape Super-commander, so tbeam back!
	game.iscate = false;
	game.ientesc = true;
    }
    // Clear quadrant
    for_sectors(i)
	for_sectors(j) 
	    game.quad[i][j] = IHDOT;
    q = &game.state.galaxy[game.quadrant.x][game.quadrant.y];
    // cope with supernova
    if (q->supernova)
	return;
    game.klhere = q->klingons;
    game.irhere = q->romulans;
    game.nenhere = game.klhere + game.irhere;

    // Position Starship
    game.quad[game.sector.x][game.sector.y] = game.ship;

    if (q->klingons) {
	w.x = w.y = 0;	/* quiet a gcc warning */
	// Position ordinary Klingons
	for (i = 1; i <= game.klhere; i++)
	    w = newkling(i);
	// If we need a commander, promote a Klingon
	for_commanders(i)
	    if (same(game.state.kcmdr[i], game.quadrant))
		break;
			
	if (i <= game.state.remcom) {
	    game.quad[w.x][w.y] = IHC;
	    game.kpower[game.klhere] = 950.0+400.0*Rand()+50.0*game.skill;
	    game.comhere = true;
	}

	// If we need a super-commander, promote a Klingon
	if (same(game.quadrant, game.state.kscmdr)) {
	    game.quad[game.ks[1].x][game.ks[1].y] = IHS;
	    game.kpower[1] = 1175.0 + 400.0*Rand() + 125.0*game.skill;
	    game.iscate = (game.state.remkl > 1);
	    game.ishere = true;
	}
    }
    // Put in Romulans if needed
    for (i = game.klhere+1; i <= game.nenhere; i++) {
	w = dropin(IHR);
	game.ks[i] = w;
	game.kdist[i] = game.kavgd[i] = distance(game.sector, w);
	game.kpower[i] = Rand()*400.0 + 450.0 + 50.0*game.skill;
    }
    // If quadrant needs a starbase, put it in
    if (q->starbase)
	game.base = dropin(IHB);
	
    // If quadrant needs a planet, put it in
    if (q->planet != NOPLANET) {
	game.iplnet = q->planet;
	if (game.state.plnets[q->planet].inhabited == UNINHABITED)
	    game.plnet = dropin(IHP);
	else
	    game.plnet = dropin(IHW);
    }
    // Check for condition
    newcnd();
    // And finally the stars
    for (i = 1; i <= q->stars; i++) 
	dropin(IHSTAR);

    // Check for RNZ
    if (game.irhere > 0 && game.klhere == 0 && (q->planet == NOPLANET || game.state.plnets[q->planet].inhabited == UNINHABITED)) {
	game.neutz = true;
	if (!damaged(DRADIO)) {
	    skip(1);
	    prout(_("LT. Uhura- \"Captain, an urgent message."));
	    prout(_("  I'll put it on audio.\"  CLICK"));
	    skip(1);
	    prout(_("INTRUDER! YOU HAVE VIOLATED THE ROMULAN NEUTRAL ZONE."));
	    prout(_("LEAVE AT ONCE, OR YOU WILL BE DESTROYED!"));
	}
    }

    if (shutup==0) {
	// Put in THING if needed
	if (same(thing, game.quadrant)) {
	    w = dropin(IHQUEST);
	    thing = randplace(GALSIZE);
	    game.nenhere++;
	    iqhere = true;
	    game.ks[game.nenhere] = w;
	    game.kdist[game.nenhere] = game.kavgd[game.nenhere] =
		distance(game.sector, w);
	    game.kpower[game.nenhere] = Rand()*6000.0 +500.0 +250.0*game.skill;
	    if (!damaged(DSRSENS)) {
		skip(1);
		prout(_("Mr. Spock- \"Captain, this is most unusual."));
		prout(_("    Please examine your short-range scan.\""));
	    }
	}
    }

    // Decide if quadrant needs a Tholian
    if (game.options & OPTION_THOLIAN) {
	if ((game.skill < SKILL_GOOD && Rand() <= 0.02) ||   /* Lighten up if skill is low */
	    (game.skill == SKILL_GOOD && Rand() <= 0.05) ||
	    (game.skill > SKILL_GOOD && Rand() <= 0.08)
	    ) {
	    do {
		game.tholian.x = Rand() > 0.5 ? QUADSIZE : 1;
		game.tholian.y = Rand() > 0.5 ? QUADSIZE : 1;
	    } while (game.quad[game.tholian.x][game.tholian.y] != IHDOT);
	    game.quad[game.tholian.x][game.tholian.y] = IHT;
	    game.ithere = true;
	    game.nenhere++;
	    game.ks[game.nenhere] = game.tholian;
	    game.kdist[game.nenhere] = game.kavgd[game.nenhere] =
		distance(game.sector, game.tholian);
	    game.kpower[game.nenhere] = Rand()*400.0 +100.0 +25.0*game.skill;
	    /* Reserve unocupied corners */
	    if (game.quad[1][1]==IHDOT)
		game.quad[1][1] = 'X';
	    if (game.quad[1][QUADSIZE]==IHDOT)
		game.quad[1][QUADSIZE] = 'X';
	    if (game.quad[QUADSIZE][1]==IHDOT)
		game.quad[QUADSIZE][1] = 'X';
	    if (game.quad[QUADSIZE][QUADSIZE]==IHDOT)
		game.quad[QUADSIZE][QUADSIZE] = 'X';
	}
    }

    sortkl();

    // Put in a few black holes
    for (i = 1; i <= 3; i++)
	if (Rand() > 0.5) 
	    dropin(IHBLANK);

    // Take out X's in corners if Tholian present
    if (game.ithere) {
	if (game.quad[1][1]=='X')
	    game.quad[1][1] = IHDOT;
	if (game.quad[1][QUADSIZE]=='X')
	    game.quad[1][QUADSIZE] = IHDOT;
	if (game.quad[QUADSIZE][1]=='X')
	    game.quad[QUADSIZE][1] = IHDOT;
	if (game.quad[QUADSIZE][QUADSIZE]=='X')
	    game.quad[QUADSIZE][QUADSIZE] = IHDOT;
    }		
}

void sortkl(void) 
/* sort Klingons by distance from us */
{
    double t;
    int j, k;
    bool sw;

    // The author liked bubble sort. So we will use it. :-(

    if (game.nenhere-iqhere-game.ithere < 2)
	return;

    do {
	sw = false;
	for (j = 1; j < game.nenhere; j++)
	    if (game.kdist[j] > game.kdist[j+1]) {
		sw = true;
		t = game.kdist[j];
		game.kdist[j] = game.kdist[j+1];
		game.kdist[j+1] = t;
		t = game.kavgd[j];
		game.kavgd[j] = game.kavgd[j+1];
		game.kavgd[j+1] = t;
		k = game.ks[j].x;
		game.ks[j].x = game.ks[j+1].x;
		game.ks[j+1].x = k;
		k = game.ks[j].y;
		game.ks[j].y = game.ks[j+1].y;
		game.ks[j+1].y = k;
		t = game.kpower[j];
		game.kpower[j] = game.kpower[j+1];
		game.kpower[j+1] = t;
	    }
    } while (sw);
}

void setpassword(void)
/* set the self-destruct password */
{
    if (game.options & OPTION_PLAIN) {
	while (TRUE) {
	    chew();
	    proutn(_("Please type in a secret password- "));
	    scan();
	    strcpy(game.passwd, citem);
	    if (*game.passwd != 0)
		break;
	}
    } else {
	int i;
        for(i=0;i<3;i++)
	    game.passwd[i]=(char)(97+(int)(Rand()*25));
        game.passwd[3]=0;
    }
}
