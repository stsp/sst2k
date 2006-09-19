#include <time.h>
#include <sys/stat.h>
#include "sst.h"

static long filelength(int fd) {
struct stat buf;
    fstat(fd, &buf);
    return buf.st_size;
}

void prelim(void) 
{
    skip(2);
    prout("-SUPER- STAR TREK");
    skip(1);
#ifdef __HISTORICAL__
    prout("Latest update-21 Sept 78");
    skip(1);
#endif /* __HISTORICAL__ */
}

void freeze(bool boss) 
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

    /* I hope that's enough! */
}


int thaw(void) 
{
    FILE *fp;
    int key;

    game.passwd[0] = '\0';
    if ((key = scan()) == IHEOL) {
	proutn("File name: ");
	key = scan();
    }
    if (key != IHALPHA) {
	huh();
	return 1;
    }
    chew();
    if (strchr(citem, '.') == NULL) {
	strcat(citem, ".trk");
    }
    if ((fp = fopen(citem, "rb")) == NULL) {
	proutn("Can't find game file ");
	proutn(citem);
	skip(1);
	return 1;
    }
    fread(&game, sizeof(game), 1, fp);
    if (feof(fp) || ftell(fp) != filelength(fileno(fp)) || strcmp(game.magic, SSTMAGIC)) {
	prout("Game file format is bad, should begin with " SSTMAGIC);
	skip(1);
	fclose(fp);
	return 1;
    }

    fclose(fp);

    return 0;
}

/*
**  Abandon Ship
**
**	The ship is abandoned.  If your current ship is the Faire
**	Queene, or if your shuttlecraft is dead, you're out of
**	luck.  You need the shuttlecraft in order for the captain
**	(that's you!!) to escape.
**
**	Your crew can beam to an inhabited starsystem in the
**	quadrant, if there is one and if the transporter is working.
**	If there is no inhabited starsystem, or if the transporter
**	is out, they are left to die in outer space.
**
**	If there are no starbases left, you are captured by the
**	Klingons, who torture you mercilessly.  However, if there
**	is at least one starbase, you are returned to the
**	Federation in a prisoner of war exchange.  Of course, this
**	can't happen unless you have taken some prisoners.
**
*/

void abandn(void) 
{
    int nb, l;
    struct quadrant *q;

    chew();
    if (game.condit==IHDOCKED) {
	if (game.ship!=IHE) {
	    prout("You cannot abandon Ye Faerie Queene.");
	    return;
	}
    }
    else {
	/* Must take shuttle craft to exit */
	if (game.damage[DSHUTTL]==-1) {
	    prout("Ye Faerie Queene has no shuttle craft.");
	    return;
	}
	if (game.damage[DSHUTTL]<0) {
	    prout("Shuttle craft now serving Big Macs.");
	    return;
	}
	if (game.damage[DSHUTTL]>0) {
	    prout("Shuttle craft damaged.");
	    return;
	}
	if (game.landed==1) {
	    prout("You must be aboard the Enterprise.");
	    return;
	}
	if (game.iscraft!=1) {
	    prout("Shuttle craft not currently available.");
	    return;
	}
	/* Print abandon ship messages */
	skip(1);
	prouts("***ABANDON SHIP!  ABANDON SHIP!");
	skip(1);
	prouts("***ALL HANDS ABANDON SHIP!");
	skip(2);
	prout("Captain and crew escape in shuttle craft.");
	if (game.state.rembase==0) {
	    /* Oops! no place to go... */
	    finish(FABANDN);
	    return;
	}
	q = &game.state.galaxy[game.quadrant.x][game.quadrant.y];
	/* Dispose of crew */
	if (!(game.options & OPTION_WORLDS) && !damaged(DTRANSP)) {
	    prout("Remainder of ship's complement beam down");
	    prout("to nearest habitable planet.");
	} else if (q->planet != NOPLANET && !damaged(DTRANSP)) {
	    prout("Remainder of ship's complement beam down");
	    prout("to %s.", systemname(q->planet));
	} else {
	    prout("Entire crew of %d left to die in outer space.");
	    game.casual += game.state.crew;
	    game.abandoned += game.state.crew;
	}

	/* If at least one base left, give 'em the Faerie Queene */
	skip(1);
	game.icrystl = 0; /* crystals are lost */
	game.nprobes = 0; /* No probes */
	prout("You are captured by Klingons and released to");
	prout("the Federation in a prisoner-of-war exchange.");
	nb = Rand()*game.state.rembase+1;
	/* Set up quadrant and position FQ adjacient to base */
	if (!same(game.quadrant, game.state.baseq[nb])) {
	    game.quadrant = game.state.baseq[nb];
	    game.sector.x = game.sector.y = 5;
	    newqad(true);
	}
	for (;;) {
	    /* position next to base by trial and error */
	    game.quad[game.sector.x][game.sector.y] = IHDOT;
	    for_sectors(l) {
		game.sector.x = 3.0*Rand() - 1.0 + game.base.x;
		game.sector.y = 3.0*Rand() - 1.0 + game.base.y;
		if (VALID_SECTOR(game.sector.x, game.sector.y) &&
		    game.quad[game.sector.x][game.sector.y] == IHDOT) break;
	    }
	    if (l < QUADSIZE+1) break; /* found a spot */
	    game.sector.x=QUADSIZE/2;
	    game.sector.y=QUADSIZE/2;
	    newqad(true);
	}
    }
    /* Get new commission */
    game.quad[game.sector.x][game.sector.y] = game.ship = IHF;
    game.state.crew = FULLCREW;
    prout("Starfleet puts you in command of another ship,");
    prout("the Faerie Queene, which is antiquated but,");
    prout("still useable.");
    if (game.icrystl!=0) prout("The dilithium crystals have been moved.");
    game.imine = false;
    game.iscraft=0; /* Galileo disappears */
    /* Resupply ship */
    game.condit=IHDOCKED;
    for (l = 0; l < NDEVICES; l++) 
	game.damage[l] = 0.0;
    game.damage[DSHUTTL] = -1;
    game.energy = game.inenrg = 3000.0;
    game.shield = game.inshld = 1250.0;
    game.torps = game.intorps = 6;
    game.lsupres=game.inlsr=3.0;
    game.shldup=false;
    game.warpfac=5.0;
    game.wfacsq=25.0;
    return;
}
	
void setup(bool needprompt) 
{
    int i,j, krem, klumper;
    int ix, iy;
    //  Decide how many of everything
    if (choose(needprompt)) return; // frozen game
    // Prepare the Enterprise
    game.alldone = game.gamewon = false;
    game.ship = IHE;
    game.state.crew = FULLCREW;
    game.energy = game.inenrg = 5000.0;
    game.shield = game.inshld = 2500.0;
    game.shldchg = 0;
    game.shldup = false;
    game.inlsr = 4.0;
    game.lsupres = 4.0;
    iran(GALSIZE, &game.quadrant.x, &game.quadrant.y);
    iran(QUADSIZE, &game.sector.x, &game.sector.y);
    game.torps = game.intorps = 10;
    game.nprobes = (int)(3.0*Rand() + 2.0);	/* Give them 2-4 of these wonders */
    game.warpfac = 5.0;
    game.wfacsq = game.warpfac * game.warpfac;
    for (i=0; i < NDEVICES; i++) 
	game.damage[i] = 0.0;
    // Set up assorted game parameters
    game.battle.x = game.battle.y = 0;
    game.state.date = game.indate = 100.0*(int)(31.0*Rand()+20.0);
    game.nkinks = game.nhelp = game.casual = game.abandoned = 0;
    game.resting = game.imine = false;
    game.isatb = game.iscate = game.icrystl = game.icraft = game.state.nplankl = 0;
    game.state.starkl = game.state.basekl = 0;
    game.iscraft = 1;
    game.landed = -1;
    game.alive = 1;
    game.docfac = 0.25;
    for_quadrants(i)
	for_quadrants(j) {
	struct quadrant *quad = &game.state.galaxy[i][j];
	    quad->charted = 0;
	    quad->planet = NOPLANET;
	    quad->romulans = 0;
	    quad->klingons = 0;
	    quad->starbase = 0;
	    quad->supernova = 0;
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
	    do iran(GALSIZE, &ix, &iy);
	    while (game.state.galaxy[ix][iy].starbase);
	    contflag = false;
	    for (j = i-1; j > 0; j--) {
		/* Improved placement algorithm to spread out bases */
		double distq = square(ix-game.state.baseq[j].x) + square(iy-game.state.baseq[j].y);
		if (distq < 6.0*(BASEMAX+1-game.inbase) && Rand() < 0.75) {
		    contflag = true;
		    if (idebug)
			prout("=== Abandoning base #%d at %d-%d", i, ix, iy);
		    break;
		}
		else if (distq < 6.0 * (BASEMAX+1-game.inbase)) {
		    if (idebug)
			prout("=== Saving base #%d, close to #%d", i, j);
		}
	    }
	} while (contflag);
			
	game.state.baseq[i].x = ix;
	game.state.baseq[i].y = iy;
	game.state.galaxy[ix][iy].starbase = 1;
	game.state.chart[ix][iy].starbase = 1;
    }
    // Position ordinary Klingon Battle Cruisers
    krem = game.inkling;
    klumper = 0.25*game.skill*(9.0-game.length)+1.0;
    if (klumper > MAXKLQUAD) 
	klumper = MAXKLQUAD;
    do {
	double r = Rand();
	int klump = (1.0 - r*r)*klumper;
	if (klump > krem) klump = krem;
	krem -= klump;
	do iran(GALSIZE,&ix,&iy);
	while (game.state.galaxy[ix][iy].supernova ||
		game.state.galaxy[ix][iy].klingons + klump > 9);
	game.state.galaxy[ix][iy].klingons += klump;
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
		    ix = game.state.baseq[klumper].x;
		    iy = game.state.baseq[klumper].y;
		    klumper++;
		}
		else
#endif /* ODEBUG */
		    iran(GALSIZE, &ix, &iy);
	    }
	    while ((!game.state.galaxy[ix][iy].klingons && Rand() < 0.75)||
		   game.state.galaxy[ix][iy].supernova||
		   game.state.galaxy[ix][iy].klingons > 8);
	    // check for duplicate
	    for (j = 1; j < i; j++)
		if (game.state.kcmdr[j].x==ix && game.state.kcmdr[j].y==iy) break;
	} while (j < i);
	game.state.galaxy[ix][iy].klingons++;
	game.state.kcmdr[i].x = ix;
	game.state.kcmdr[i].y = iy;
    }
    // Locate planets in galaxy
    for (i = 0; i < game.inplan; i++) {
	do iran(GALSIZE, &ix, &iy); while (game.state.galaxy[ix][iy].planet != NOPLANET);
	game.state.plnets[i].w.x = ix;
	game.state.plnets[i].w.y = iy;
	if (i < NINHAB) {
	    game.state.plnets[i].pclass = M;	// All inhabited planets are class M
	    game.state.plnets[i].crystals = 0;
	    game.state.plnets[i].known = known;
	    game.state.plnets[i].inhabited = i;
	} else {
	    game.state.plnets[i].pclass = Rand()*3.0; // Planet class M N or O
	    game.state.plnets[i].crystals = 1.5*Rand();		// 1 in 3 chance of crystals
	    game.state.plnets[i].known = unknown;
	    game.state.plnets[i].inhabited = UNINHABITED;
	}
	if ((game.options & OPTION_WORLDS) || i >= NINHAB)
	    game.state.galaxy[ix][iy].planet = i;
    }
    // Locate Romulans
    for (i = 1; i <= game.state.nromrem; i++) {
	iran(GALSIZE, &ix, &iy);
	game.state.galaxy[ix][iy].romulans = 1;
    }
    // Locate the Super Commander
    if (game.state.nscrem > 0) {
	do iran(GALSIZE, &ix, &iy);
	while (game.state.galaxy[ix][iy].supernova || game.state.galaxy[ix][iy].klingons > 8);
	game.state.kscmdr.x = ix;
	game.state.kscmdr.y = iy;
	game.state.galaxy[ix][iy].klingons++;
    }
    // Place thing (in tournament game, thingx == -1, don't want one!)
    if (thing.x != -1) {
	iran(GALSIZE, &thing.x, &thing.y);
    }
    else {
	thing.x = thing.y = 0;
    }

//	idate = date;
    skip(2);
    game.state.snap = 0;
		
    if (game.skill == SKILL_NOVICE) {
	prout("It is stardate %d. The Federation is being attacked by",
	      (int)game.state.date);
	prout("a deadly Klingon invasion force. As captain of the United");
	prout("Starship U.S.S. Enterprise, it is your mission to seek out");
	prout("and destroy this invasion force of %d battle cruisers.",
	      INKLINGTOT);
	prout("You have an initial allotment of %d stardates to complete", (int)game.intime);
	prout("your mission.  As you proceed you may be given more time.");
	prout("");
	prout("You will have %d supporting starbases.", game.inbase);
	proutn("Starbase locations-  ");
    }
    else {
	prout("Stardate %d.", (int)game.state.date);
	prout("");
	prout("%d Klingons.", INKLINGTOT);
	prout("An unknown number of Romulans.");
	if (game.state.nscrem) prout("and one (GULP) Super-Commander.");
	prout("%d stardates.",(int)game.intime);
	proutn("%d starbases in ", game.inbase);
    }
    for (i = 1; i <= game.inbase; i++) {
	proutn(cramlc(0, game.state.baseq[i]));
	proutn("  ");
    }
    skip(2);
    proutn("The Enterprise is currently in ");
    proutn(cramlc(quadrant, game.quadrant));
    proutn(" ");
    proutn(cramlc(sector, game.sector));
    skip(2);
    prout("Good Luck!");
    if (game.state.nscrem) prout("  YOU'LL NEED IT.");
    waitfor();
    newqad(false);
    if (game.nenhere-iqhere-game.ithere) game.shldup = true;
    if (game.neutz) attack(0);	// bad luck to start in a Romulan Neutral Zone
}

bool choose(bool needprompt) 
{
    for(;;) {
	game.tourn = 0;
	game.thawed = 0;
	game.skill = SKILL_NONE;
	game.length = 0;
	if (needprompt) /* Can start with command line options */
	    proutn("Would you like a regular, tournament, or saved game? ");
	scan();
	if (strlen(citem)==0) continue; // Try again
	if (isit("tournament")) {
	    while (scan() == IHEOL) {
		proutn("Type in tournament number-");
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
	    if (thaw()) continue;
	    chew();
	    if (*game.passwd==0) continue;
	    if (!game.alldone) game.thawed = 1; // No plaque if not finished
	    report();
	    waitfor();
	    return true;
	}
	if (isit("regular")) break;
	proutn("What is \"");
	proutn(citem);
	prout("\"?");
	chew();
    }
    while (game.length==0 || game.skill==SKILL_NONE) {
	if (scan() == IHALPHA) {
	    if (isit("short")) game.length = 1;
	    else if (isit("medium")) game.length = 2;
	    else if (isit("long")) game.length = 4;
	    else if (isit("novice")) game.skill = SKILL_NOVICE;
	    else if (isit("fair")) game.skill = SKILL_FAIR;
	    else if (isit("good")) game.skill = SKILL_GOOD;
	    else if (isit("expert")) game.skill = SKILL_EXPERT;
	    else if (isit("emeritus")) game.skill = SKILL_EMERITUS;
	    else {
		proutn("What is \"");
		proutn(citem);
		prout("\"?");
	    }
	}
	else {
	    chew();
	    if (game.length==0) proutn("Would you like a Short, Medium, or Long game? ");
	    else if (game.skill == SKILL_NONE) proutn("Are you a Novice, Fair, Good, Expert, or Emeritus player? ");
	}
    }
    // Choose game options -- added by ESR for SST2K
    if (scan() != IHALPHA) {
	chew();
	proutn("Choose your game style (or just press enter): ");
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
	    proutn("What is \"");
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

void dropin(int iquad, coord *w) 
{
    do iran(QUADSIZE, &w->x, &w->y);
    while (game.quad[w->x][w->y] != IHDOT);
    game.quad[w->x][w->y] = iquad;
}

void newcnd(void) 
{
    game.condit = IHGREEN;
    if (game.energy < 1000.0) game.condit = IHYELLOW;
    if (game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons || game.state.galaxy[game.quadrant.x][game.quadrant.y].romulans)
	game.condit = IHRED;
    if (!game.alive) game.condit=IHDEAD;
}

void newkling(int i, coord *pi)
/* drop new Klingon into current quadrant */
{
    dropin(IHK, pi);
    game.ks[i] = *pi;
    game.kdist[i] = game.kavgd[i] = sqrt(square(game.sector.x-pi->x) + square(game.sector.y-pi->y));
    game.kpower[i] = Rand()*150.0 +300.0 +25.0*game.skill;
}

void newqad(bool shutup) 
{
    int i, j;
    coord w;
    struct quadrant *here;

    game.iattak = 1;
    game.justin = true;
    game.base.x = game.base.y = 0;
    game.klhere = 0;
    game.comhere = 0;
    game.plnet.x = game.plnet.y = 0;
    game.ishere = 0;
    game.irhere = 0;
    game.iplnet = 0;
    game.nenhere = 0;
    game.neutz = false;
    game.inorbit = false;
    game.landed = -1;
    game.ientesc = 0;
    game.ithere = 0;
    iqhere=0;
    iqengry=0;
    game.iseenit = 0;
    if (game.iscate) {
	// Attempt to escape Super-commander, so tbeam back!
	game.iscate = 0;
	game.ientesc = 1;
    }
    // Clear quadrant
    for_sectors(i)
	for_sectors(j) 
	    game.quad[i][j] = IHDOT;
    here = &game.state.galaxy[game.quadrant.x][game.quadrant.y];
    // cope with supernova
    if (here->supernova)
	return;
    game.klhere = here->klingons;
    game.irhere = here->romulans;
    game.nenhere = game.klhere + game.irhere;

    // Position Starship
    game.quad[game.sector.x][game.sector.y] = game.ship;

    if (here->klingons) {
	// Position ordinary Klingons
	for (i = 1; i <= game.klhere; i++)
	    newkling(i, &w);
	// If we need a commander, promote a Klingon
	for_commanders(i)
	    if (game.state.kcmdr[i].x==game.quadrant.x && game.state.kcmdr[i].y==game.quadrant.y) break;
			
	if (i <= game.state.remcom) {
	    game.quad[w.x][w.y] = IHC;
	    game.kpower[game.klhere] = 950.0+400.0*Rand()+50.0*game.skill;
	    game.comhere = 1;
	}

	// If we need a super-commander, promote a Klingon
	if (game.quadrant.x == game.state.kscmdr.x && game.quadrant.y == game.state.kscmdr.y) {
	    game.quad[game.ks[1].x][game.ks[1].y] = IHS;
	    game.kpower[1] = 1175.0 + 400.0*Rand() + 125.0*game.skill;
	    game.iscate = game.state.remkl>1;
	    game.ishere = 1;
	}
    }
    // Put in Romulans if needed
    for (i = game.klhere+1; i <= game.nenhere; i++) {
	dropin(IHR, &w);
	game.ks[i] = w;
	game.kdist[i] = game.kavgd[i] = sqrt(square(game.sector.x-w.x) + square(game.sector.y-w.y));
	game.kpower[i] = Rand()*400.0 + 450.0 + 50.0*game.skill;
    }
    // If quadrant needs a starbase, put it in
    if (here->starbase)
	dropin(IHB, &game.base);
	
    // If quadrant needs a planet, put it in
    if (here->planet != NOPLANET) {
	game.iplnet = here->planet;
	if (game.state.plnets[here->planet].inhabited == UNINHABITED)
	    dropin(IHP, &game.plnet);
	else
	    dropin(IHW, &game.plnet);
    }
    // Check for game.condition
    newcnd();
    // And finally the stars
    for (i = 1; i <= here->stars; i++) 
	dropin(IHSTAR, &w);

    // Check for RNZ
    if (game.irhere > 0 && game.klhere == 0 && (here->planet == NOPLANET || game.state.plnets[here->planet].inhabited == UNINHABITED)) {
	game.neutz = 1;
	if (!damaged(DRADIO)) {
	    skip(1);
	    prout("LT. Uhura- \"Captain, an urgent message.");
	    prout("  I'll put it on audio.\"  CLICK");
	    skip(1);
	    prout("INTRUDER! YOU HAVE VIOLATED THE ROMULAN NEUTRAL ZONE.");
	    prout("LEAVE AT ONCE, OR YOU WILL BE DESTROYED!");
	}
    }

    if (shutup==0) {
	// Put in THING if needed
	if (same(thing, game.quadrant)) {
	    dropin(IHQUEST, &w);
	    iran(GALSIZE, &thing.x, &thing.y);
	    game.nenhere++;
	    iqhere=1;
	    game.ks[game.nenhere] = w;
	    game.kdist[game.nenhere] = game.kavgd[game.nenhere] =
		sqrt(square(game.sector.x-w.x) + square(game.sector.y-w.y));
	    game.kpower[game.nenhere] = Rand()*6000.0 +500.0 +250.0*game.skill;
	    if (!damaged(DSRSENS)) {
		skip(1);
		prout("MR. SPOCK- \"Captain, this is most unusual.");
		prout("    Please examine your short-range scan.\"");
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
	    game.ithere = 1;
	    game.nenhere++;
	    game.ks[game.nenhere].x = game.tholian.x;
	    game.ks[game.nenhere].y = game.tholian.y;
	    game.kdist[game.nenhere] = game.kavgd[game.nenhere] =
		sqrt(square(game.sector.x-game.tholian.x) + square(game.sector.y-game.tholian.y));
	    game.kpower[game.nenhere] = Rand()*400.0 +100.0 +25.0*game.skill;
	    /* Reserve unocupied corners */
	    if (game.quad[1][1]==IHDOT) game.quad[1][1] = 'X';
	    if (game.quad[1][QUADSIZE]==IHDOT) game.quad[1][QUADSIZE] = 'X';
	    if (game.quad[QUADSIZE][1]==IHDOT) game.quad[QUADSIZE][1] = 'X';
	    if (game.quad[QUADSIZE][QUADSIZE]==IHDOT) game.quad[QUADSIZE][QUADSIZE] = 'X';
	}
    }

    sortkl();

    // Put in a few black holes
    for (i = 1; i <= 3; i++)
	if (Rand() > 0.5) 
	    dropin(IHBLANK, &w);

    // Take out X's in corners if Tholian present
    if (game.ithere) {
	if (game.quad[1][1]=='X') game.quad[1][1] = IHDOT;
	if (game.quad[1][QUADSIZE]=='X') game.quad[1][QUADSIZE] = IHDOT;
	if (game.quad[QUADSIZE][1]=='X') game.quad[QUADSIZE][1] = IHDOT;
	if (game.quad[QUADSIZE][QUADSIZE]=='X') game.quad[QUADSIZE][QUADSIZE] = IHDOT;
    }		
}

void sortkl(void) 
{
    double t;
    int j, k;
    bool sw;

    // The author liked bubble sort. So we will use it. :-(

    if (game.nenhere-iqhere-game.ithere < 2) return;

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
{
    if (!(game.options & OPTION_CURSES)) {
	while (TRUE) {
	    scan();
	    strcpy(game.passwd, citem);
	    chew();
	    if (*game.passwd != 0) break;
	    proutn(_("Please type in a secret password-"));
	}
    } else {
	int i;
        for(i=0;i<3;i++) game.passwd[i]=(char)(97+(int)(Rand()*25));
        game.passwd[3]=0;
    }
}
