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

void freeze(int boss) 
{
    FILE *fp;
    int key;
    if (boss) {
	strcpy(citem, "emsave.trk");
    }
    else {
	if ((key = scan()) == IHEOL) {
	    proutn("File name: ");
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
	proutn("Can't freeze game as file ");
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

void abandn(void) 
{
    int nb, l;

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
	prout("Remainder of ship's complement beam down");
	prout("to nearest habitable planet.");
	if (game.state.rembase==0) {
	    /* Oops! no place to go... */
	    finish(FABANDN);
	    return;
	}
	/* If at least one base left, give 'em the Faerie Queene */
	skip(1);
	game.icrystl = 0; /* crystals are lost */
	game.nprobes = 0; /* No probes */
	prout("You are captured by Klingons and released to");
	prout("the Federation in a prisoner-of-war exchange.");
	nb = Rand()*game.state.rembase+1;
	/* Set up quadrant and position FQ adjacient to base */
	if (game.quadx!=game.state.baseqx[nb] || game.quady!=game.state.baseqy[nb]) {
	    game.quadx = game.state.baseqx[nb];
	    game.quady = game.state.baseqy[nb];
	    game.sectx = game.secty = 5;
	    newqad(1);
	}
	for (;;) {
	    /* position next to base by trial and error */
	    game.quad[game.sectx][game.secty] = IHDOT;
	    for_sectors(l) {
		game.sectx = 3.0*Rand() - 1.0 + game.basex;
		game.secty = 3.0*Rand() - 1.0 + game.basey;
		if (VALID_SECTOR(game.sectx, game.secty) &&
		    game.quad[game.sectx][game.secty] == IHDOT) break;
	    }
	    if (l < QUADSIZE+1) break; /* found a spot */
	    game.sectx=QUADSIZE/2;
	    game.secty=QUADSIZE/2;
	    newqad(1);
	}
    }
    /* Get new commission */
    game.quad[game.sectx][game.secty] = game.ship = IHF;
    prout("Starfleet puts you in command of another ship,");
    prout("the Faerie Queene, which is antiquated but,");
    prout("still useable.");
    if (game.icrystl!=0) prout("The dilithium crystals have been moved.");
    game.imine=0;
    game.iscraft=0; /* Gallileo disappears */
    /* Resupply ship */
    game.condit=IHDOCKED;
    for (l = 0; l < NDEVICES; l++) 
	game.damage[l] = 0.0;
    game.damage[DSHUTTL] = -1;
    game.energy = game.inenrg = 3000.0;
    game.shield = game.inshld = 1250.0;
    game.torps = game.intorps = 6;
    game.lsupres=game.inlsr=3.0;
    game.shldup=0;
    game.warpfac=5.0;
    game.wfacsq=25.0;
    return;
}
	
void setup(int needprompt) 
{
    int i,j, krem, klumper;
    int ix, iy;
#ifdef DEBUG
    game.idebug = 0;
#endif
    //  Decide how many of everything
    if (choose(needprompt)) return; // frozen game
    // Prepare the Enterprise
    game.alldone = game.gamewon = 0;
    game.ship = IHE;
    game.energy = game.inenrg = 5000.0;
    game.shield = game.inshld = 2500.0;
    game.shldchg = game.shldup = 0;
    game.inlsr = 4.0;
    game.lsupres = 4.0;
    iran(GALSIZE, &game.quadx, &game.quady);
    iran(QUADSIZE, &game.sectx, &game.secty);
    game.torps = game.intorps = 10;
    game.nprobes = (int)(3.0*Rand() + 2.0);	/* Give them 2-4 of these wonders */
    game.warpfac = 5.0;
    game.wfacsq = game.warpfac * game.warpfac;
    for (i=0; i < NDEVICES; i++) 
	game.damage[i] = 0.0;
    // Set up assorted game parameters
    game.batx = game.baty = 0;
    game.state.date = game.indate = 100.0*(int)(31.0*Rand()+20.0);
    game.nkinks = game.nhelp = game.resting = game.casual = 0;
    game.isatb = game.iscate = game.imine = game.icrystl = game.icraft = game.state.nplankl = 0;
    game.state.starkl = game.state.basekl = 0;
    game.iscraft = 1;
    game.landed = -1;
    game.alive = 1;
    game.docfac = 0.25;
    for_quadrants(i)
	for_quadrants(j) {
	    game.state.galaxy[i][j].charted = 0;
	    game.state.galaxy[i][j].planet = NULL;
	    game.state.galaxy[i][j].romulans = 0;
	    game.state.galaxy[i][j].klingons = 0;
	    game.state.galaxy[i][j].starbase = 0;
	    game.state.galaxy[i][j].supernova = 0;
	}
    // Initialize times for extraneous events
    game.future[FSNOVA] = game.state.date + expran(0.5 * game.intime);
    game.future[FTBEAM] = game.state.date + expran(1.5 * (game.intime / game.state.remcom));
    game.future[FSNAP] = game.state.date + 1.0 + Rand(); // Force an early snapshot
    game.future[FBATTAK] = game.state.date + expran(0.3*game.intime);
    game.future[FCDBAS] = FOREVER;
    game.future[FSCMOVE] = game.state.nscrem ? game.state.date+0.2777 : FOREVER;
    game.future[FSCDBAS] = FOREVER;
    game.future[FDSPROB] = FOREVER;
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
	int contflag;
	do {
	    do iran(GALSIZE, &ix, &iy);
	    while (game.state.galaxy[ix][iy].starbase);
	    contflag = FALSE;
	    for (j = i-1; j > 0; j--) {
		/* Improved placement algorithm to spread out bases */
		double distq = square(ix-game.state.baseqx[j]) + square(iy-game.state.baseqy[j]);
		if (distq < 6.0*(BASEMAX+1-game.inbase) && Rand() < 0.75) {
		    contflag = TRUE;
#ifdef DEBUG
		    prout("DEBUG: Abandoning base #%d at %d-%d", i, ix, iy);
#endif
		    break;
		}
#ifdef DEBUG
		else if (distq < 6.0 * (BASEMAX+1-game.inbase)) {
		    prout("DEBUG: saving base #%d, close to #%d", i, j);
		}
#endif
	    }
	} while (contflag);
			
	game.state.baseqx[i] = ix;
	game.state.baseqy[i] = iy;
	game.state.galaxy[ix][iy].starbase = 1;
	game.state.chart[ix][iy].starbase = 1;
    }
    // Position ordinary Klingon Battle Cruisers
    krem = game.inkling;
    klumper = 0.25*game.skill*(9.0-game.length)+1.0;
    if (klumper > 9) klumper = 9; // Can't have more than 9 in quadrant
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
#ifdef DEBUG
    klumper = 1;
#endif
    for (i = 1; i <= game.incom; i++) {
	do {
	    do { /* IF debugging, put commanders by bases, always! */
#ifdef DEBUG
		if (game.idebug && klumper <= game.inbase) {
		    ix = game.state.baseqx[klumper];
		    iy = game.state.baseqy[klumper];
		    klumper++;
		}
		else
#endif
		    iran(GALSIZE, &ix, &iy);
	    }
	    while ((!game.state.galaxy[ix][iy].klingons && Rand() < 0.75)||
		   game.state.galaxy[ix][iy].supernova||
		   game.state.galaxy[ix][iy].klingons > 8);
	    // check for duplicate
	    for (j = 1; j < i; j++)
		if (game.state.cx[j]==ix && game.state.cy[j]==iy) break;
	} while (j < i);
	game.state.galaxy[ix][iy].klingons++;
	game.state.cx[i] = ix;
	game.state.cy[i] = iy;
    }
    // Locate planets in galaxy
    for (i = 0; i < game.inplan; i++) {
	do iran(GALSIZE, &ix, &iy); while (game.state.galaxy[ix][iy].planet);
	game.state.plnets[i].x = ix;
	game.state.plnets[i].y = iy;
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
	    game.state.galaxy[ix][iy].planet = game.state.plnets + i;
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
	game.state.isx = ix;
	game.state.isy = iy;
	game.state.galaxy[ix][iy].klingons++;
    }
    // Place thing (in tournament game, thingx == -1, don't want one!)
    if (thingx != -1) {
	iran(GALSIZE, &thingx, &thingy);
    }
    else {
	thingx = thingy = 0;
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
	proutn(cramlc(0, game.state.baseqx[i], game.state.baseqy[i]));
	proutn("  ");
    }
    skip(2);
    proutn("The Enterprise is currently in ");
    proutn(cramlc(quadrant, game.quadx, game.quady));
    proutn(" ");
    proutn(cramlc(sector, game.sectx, game.secty));
    skip(2);
    prout("Good Luck!");
    if (game.state.nscrem) prout("  YOU'LL NEED IT.");
    waitfor();
    newqad(0);
    if (game.nenhere-iqhere-game.ithere) game.shldup=1.0;
    if (game.neutz) attack(0);	// bad luck to start in a Romulan Neutral Zone
}

int choose(int needprompt) 
{
    while (TRUE) {
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
	    thingx = -1;
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
	    return TRUE;
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
	proutn("Choose your game options: ");
	scan();
    }
    if (isit("plain")) {
	// Approximates the UT FORTRAN version.
	game.options &=~ (OPTION_THOLIAN | OPTION_PLANETS | OPTION_THINGY | OPTION_PROBE | OPTION_RAMMING | OPTION_MVBADDY | OPTION_BLKHOLE | OPTION_BASE);
	game.options |= OPTION_PLAIN;
    } 
    else if (isit("almy")) {
	// Approximates Tom Almy's version.
	game.options &=~ (OPTION_THINGY | OPTION_BLKHOLE | OPTION_BASE);
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
#ifdef DEBUG
    if (strcmp(game.passwd, "debug")==0) game.idebug = 1;
#endif

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
    return FALSE;
}

void dropin(int iquad, int *ix, int *iy) 
{
    do iran(QUADSIZE, ix, iy);
    while (game.quad[*ix][*iy] != IHDOT);
    game.quad[*ix][*iy] = iquad;
}

void newcnd(void) 
{
    game.condit = IHGREEN;
    if (game.energy < 1000.0) game.condit = IHYELLOW;
    if (game.state.galaxy[game.quadx][game.quady].klingons || game.state.galaxy[game.quadx][game.quady].romulans)
	game.condit = IHRED;
    if (!game.alive) game.condit=IHDEAD;
}


void newqad(int shutup) 
{
    int i, j, ix, iy;
    planet *planhere;

    game.iattak = 1;
    game.justin = 1;
    game.basex = game.basey = 0;
    game.klhere = 0;
    game.comhere = 0;
    game.plnetx = game.plnety = 0;
    game.ishere = 0;
    game.irhere = 0;
    game.iplnet = 0;
    game.nenhere = 0;
    game.neutz = 0;
    game.inorbit = 0;
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
    // cope with supernova
    if (game.state.galaxy[game.quadx][game.quady].supernova)
	return;
    game.klhere = game.state.galaxy[game.quadx][game.quady].klingons;
    game.irhere = game.state.galaxy[game.quadx][game.quady].romulans;
    game.nenhere = game.klhere + game.irhere;

    // Position Starship
    game.quad[game.sectx][game.secty] = game.ship;

    if (game.state.galaxy[game.quadx][game.quady].klingons) {
	// Position ordinary Klingons
	for (i = 1; i <= game.klhere; i++) {
	    dropin(IHK, &ix, &iy);
	    game.kx[i] = ix;
	    game.ky[i] = iy;
	    game.kdist[i] = game.kavgd[i] = sqrt(square(game.sectx-ix) + square(game.secty-iy));
	    game.kpower[i] = Rand()*150.0 +300.0 +25.0*game.skill;
	}
	// If we need a commander, promote a Klingon
	for_commanders(i)
	    if (game.state.cx[i]==game.quadx && game.state.cy[i]==game.quady) break;
			
	if (i <= game.state.remcom) {
	    game.quad[ix][iy] = IHC;
	    game.kpower[game.klhere] = 950.0+400.0*Rand()+50.0*game.skill;
	    game.comhere = 1;
	}

	// If we need a super-commander, promote a Klingon
	if (game.quadx == game.state.isx && game.quady == game.state.isy) {
	    game.quad[game.kx[1]][game.ky[1]] = IHS;
	    game.kpower[1] = 1175.0 + 400.0*Rand() + 125.0*game.skill;
	    game.iscate = game.state.remkl>1;
	    game.ishere = 1;
	}
    }
    // Put in Romulans if needed
    for (i = game.klhere+1; i <= game.nenhere; i++) {
	dropin(IHR, &ix, &iy);
	game.kx[i] = ix;
	game.ky[i] = iy;
	game.kdist[i] = game.kavgd[i] = sqrt(square(game.sectx-ix) + square(game.secty-iy));
	game.kpower[i] = Rand()*400.0 + 450.0 + 50.0*game.skill;
    }
    // If quadrant needs a starbase, put it in
    if (game.state.galaxy[game.quadx][game.quady].starbase)
	dropin(IHB, &game.basex, &game.basey);
	
    // If quadrant needs a planet, put it in
    planhere = game.state.galaxy[game.quadx][game.quady].planet;
    if (planhere) {
	game.iplnet = planhere - game.state.plnets;
	dropin(IHP, &game.plnetx, &game.plnety);
    }
    // Check for game.condition
    newcnd();
    // And finally the stars
    for (i = 1; i <= game.state.galaxy[game.quadx][game.quady].stars; i++) 
	dropin(IHSTAR, &ix, &iy);

    // Check for RNZ
    if (game.irhere > 0 && game.klhere == 0 && (!planhere || planhere->inhabited == UNINHABITED)) {
	game.neutz = 1;
	if (game.damage[DRADIO] <= 0.0) {
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
	if (thingx == game.quadx && thingy == game.quady) {
	    dropin(IHQUEST, &ix, &iy);
	    iran(GALSIZE, &thingx, &thingy);
	    game.nenhere++;
	    iqhere=1;
	    game.kx[game.nenhere] = ix;
	    game.ky[game.nenhere] = iy;
	    game.kdist[game.nenhere] = game.kavgd[game.nenhere] =
		sqrt(square(game.sectx-ix) + square(game.secty-iy));
	    game.kpower[game.nenhere] = Rand()*6000.0 +500.0 +250.0*game.skill;
	    if (game.damage[DSRSENS] == 0.0) {
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
    #ifdef DEBUG
	    || strcmp(game.passwd, "tholianx")==0
    #endif
	    ) {
	    do {
		game.ithx = Rand() > 0.5 ? QUADSIZE : 1;
		game.ithy = Rand() > 0.5 ? QUADSIZE : 1;
	    } while (game.quad[game.ithx][game.ithy] != IHDOT);
	    game.quad[game.ithx][game.ithy] = IHT;
	    game.ithere = 1;
	    game.nenhere++;
	    game.kx[game.nenhere] = game.ithx;
	    game.ky[game.nenhere] = game.ithy;
	    game.kdist[game.nenhere] = game.kavgd[game.nenhere] =
		sqrt(square(game.sectx-game.ithx) + square(game.secty-game.ithy));
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
	    dropin(IHBLANK, &ix, &iy);

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
    int sw, j, k;

    // The author liked bubble sort. So we will use it. :-(

    if (game.nenhere-iqhere-game.ithere < 2) return;

    do {
	sw = FALSE;
	for (j = 1; j < game.nenhere; j++)
	    if (game.kdist[j] > game.kdist[j+1]) {
		sw = TRUE;
		t = game.kdist[j];
		game.kdist[j] = game.kdist[j+1];
		game.kdist[j+1] = t;
		t = game.kavgd[j];
		game.kavgd[j] = game.kavgd[j+1];
		game.kavgd[j+1] = t;
		k = game.kx[j];
		game.kx[j] = game.kx[j+1];
		game.kx[j+1] = k;
		k = game.ky[j];
		game.ky[j] = game.ky[j+1];
		game.ky[j+1] = k;
		t = game.kpower[j];
		game.kpower[j] = game.kpower[j+1];
		game.kpower[j+1] = t;
	    }
    } while (sw);
}
