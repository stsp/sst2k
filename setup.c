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
    if (condit==IHDOCKED) {
	if (ship!=IHE) {
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
	    prout("Shuttle craft now serving Big Mac's.");
	    return;
	}
	if (game.damage[DSHUTTL]>0) {
	    prout("Shuttle craft damaged.");
	    return;
	}
	if (landed==1) {
	    prout("You must be aboard the Enterprise.");
	    return;
	}
	if (iscraft!=1) {
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
	    /* Ops! no place to go... */
	    finish(FABANDN);
	    return;
	}
	/* If at least one base left, give 'em the Faerie Queene */
	skip(1);
	icrystl = 0; /* crystals are lost */
	nprobes = 0; /* No probes */
	prout("You are captured by Klingons and released to");
	prout("the Federation in a prisoner-of-war exchange.");
	nb = Rand()*game.state.rembase+1;
	/* Set up quadrant and position FQ adjacient to base */
	if (quadx!=game.state.baseqx[nb] || quady!=game.state.baseqy[nb]) {
	    quadx = game.state.baseqx[nb];
	    quady = game.state.baseqy[nb];
	    sectx = secty = 5;
	    newqad(1);
	}
	for (;;) {
	    /* position next to base by trial and error */
	    game.quad[sectx][secty] = IHDOT;
	    for (l = 1; l <= QUADSIZE; l++) {
		sectx = 3.0*Rand() - 1.0 + basex;
		secty = 3.0*Rand() - 1.0 + basey;
		if (sectx >= 1 && sectx <= QUADSIZE &&
		    secty >= 1 && secty <= QUADSIZE &&
		    game.quad[sectx][secty] == IHDOT) break;
	    }
	    if (l < QUADSIZE+1) break; /* found a spot */
	    sectx=QUADSIZE/2;
	    secty=QUADSIZE/2;
	    newqad(1);
	}
    }
    /* Get new commission */
    game.quad[sectx][secty] = ship = IHF;
    prout("Starfleet puts you in command of another ship,");
    prout("the Faerie Queene, which is antiquated but,");
    prout("still useable.");
    if (icrystl!=0) prout("The dilithium crystals have been moved.");
    imine=0;
    iscraft=0; /* Gallileo disappears */
    /* Resupply ship */
    condit=IHDOCKED;
    for (l = 0; l < NDEVICES; l++) 
	game.damage[l] = 0.0;
    game.damage[DSHUTTL] = -1;
    energy = inenrg = 3000.0;
    shield = inshld = 1250.0;
    torps = intorps = 6;
    lsupres=inlsr=3.0;
    shldup=0;
    warpfac=5.0;
    wfacsq=25.0;
    return;
}
	
void setup(int needprompt) 
{
    int i,j, krem, klumper;
    int ix, iy;
#ifdef DEBUG
    idebug = 0;
#endif
    //  Decide how many of everything
    if (choose(needprompt)) return; // frozen game
    // Prepare the Enterprise
    alldone = gamewon = 0;
    ship = IHE;
    energy = inenrg = 5000.0;
    shield = inshld = 2500.0;
    shldchg = shldup = 0;
    inlsr = 4.0;
    lsupres = 4.0;
    iran(GALSIZE, &quadx, &quady);
    iran(QUADSIZE, &sectx, &secty);
    torps = intorps = 10;
    nprobes = (int)(3.0*Rand() + 2.0);	/* Give them 2-4 of these wonders */
    warpfac = 5.0;
    wfacsq = warpfac * warpfac;
    for (i=0; i < NDEVICES; i++) 
	game.damage[i] = 0.0;
    // Set up assorted game parameters
    batx = baty = 0;
    game.state.date = indate = 100.0*(int)(31.0*Rand()+20.0);
    game.state.killk = game.state.killc = nkinks = nhelp = resting = casual = game.state.nromkl = 0;
    isatb = iscate = imine = icrystl = icraft = game.state.nsckill = game.state.nplankl = 0;
    game.state.starkl = game.state.basekl = 0;
    iscraft = 1;
    landed = -1;
    alive = 1;
    docfac = 0.25;
    for (i = 1; i <= GALSIZE; i++)
	for (j = 1; j <= GALSIZE; j++) {
	    game.state.galaxy[i][j].charted = 0;
	    game.state.galaxy[i][j].planets = 0;
	    game.state.galaxy[i][j].romulans = 0;
	}
    // Initialize times for extraneous events
    game.future[FSNOVA] = game.state.date + expran(0.5 * intime);
    game.future[FTBEAM] = game.state.date + expran(1.5 * (intime / game.state.remcom));
    game.future[FSNAP] = game.state.date + 1.0 + Rand(); // Force an early snapshot
    game.future[FBATTAK] = game.state.date + expran(0.3*intime);
    game.future[FCDBAS] = 1e30;
    game.future[FSCMOVE] = game.state.nscrem ? game.state.date+0.2777 : 1e30;
    game.future[FSCDBAS] = 1e30;
    game.future[FDSPROB] = 1e30;
    // Starchart is functional
    stdamtim = 1e30;
    // Put stars in the galaxy
    instar = 0;
    for (i=1; i<=GALSIZE; i++)
	for (j=1; j<=GALSIZE; j++) {
	    int k = Rand()*9.0 + 1.0;
	    instar += k;
	    game.state.galaxy[i][j].stars = k;
	}
    // Locate star bases in galaxy
    for (i = 1; i <= inbase; i++) {
	int contflag;
	do {
	    do iran(GALSIZE, &ix, &iy);
	    while (game.state.galaxy[ix][iy].starbase);
	    contflag = FALSE;
	    for (j = i-1; j > 0; j--) {
		/* Improved placement algorithm to spread out bases */
		double distq = square(ix-game.state.baseqx[j]) + square(iy-game.state.baseqy[j]);
		if (distq < 6.0*(BASEMAX-inbase) && Rand() < 0.75) {
		    contflag = TRUE;
#ifdef DEBUG
		    proutn("DEBUG: Abandoning base #%d at %d-%d\n", i, ix, iy);
#endif
		    break;
		}
#ifdef DEBUG
		else if (distq < 6.0 * (BASEMAX-inbase)) {
		    proutn("DEBUG: saving base #%d, close to #%d\n", i, j);
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
    krem = inkling - incom - game.state.nscrem;
    klumper = 0.25*skill*(9.0-length)+1.0;
    if (klumper > 9) klumper = 9; // Can't have more than 9 in quadrant
    do {
	double r = Rand();
	int klump = (1.0 - r*r)*klumper;
	if (klump > krem) klump = krem;
	krem -= klump;
	do iran(GALSIZE,&ix,&iy); while (game.state.galaxy[ix][iy].supernova);
	game.state.galaxy[ix][iy].klingons += klump;
    } while (krem > 0);
    // Position Klingon Commander Ships
#ifdef DEBUG
    klumper = 1;
#endif
    for (i = 1; i <= incom; i++) {
	do {
	    do { /* IF debugging, put commanders by bases, always! */
#ifdef DEBUG
		if (idebug && klumper <= inbase) {
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
    for (i = 0; i < inplan; i++) {
	do iran(GALSIZE, &ix, &iy); while (game.state.galaxy[ix][iy].planets);
	game.state.galaxy[ix][iy].planets = 1;
	game.state.plnets[i].x = ix;
	game.state.plnets[i].y = iy;
	game.state.plnets[i].pclass = Rand()*3.0; // Planet class M N or O
	game.state.plnets[i].crystals = 1.5*Rand();		// 1 in 3 chance of crystals
	game.state.plnets[i].known = unknown;
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
		
    if (skill == SKILL_NOVICE) {
	prout("It is stardate %d. The Federation is being attacked by",
	      (int)game.state.date);
	prout("a deadly Klingon invasion force. As captain of the United");
	prout("Starship U.S.S. Enterprise, it is your mission to seek out");
	prout("and destroy this invasion force of %d battle cruisers.",
	      inkling);
	prout("You have an initial allotment of %d stardates to complete", (int)intime);
	prout("your mission.  As you proceed you may be given more time.");
	prout("");
	prout("You will have %d supporting starbases.", inbase);
	proutn("Starbase locations-  ");
    }
    else {
	prout("Stardate %d.", (int)game.state.date);
	prout("");
	prout("%d Klingons.", inkling);
	prout("An unknown number of Romulans.");
	if (game.state.nscrem) prout("and one (GULP) Super-Commander.");
	prout("%d stardates.",(int)intime);
	proutn("%d starbases in ", inbase);
    }
    for (i = 1; i <= inbase; i++) {
	proutn(cramlc(0, game.state.baseqx[i], game.state.baseqy[i]));
	proutn("  ");
    }
    skip(2);
    proutn("The Enterprise is currently in ");
    proutn(cramlc(quadrant, quadx, quady));
    proutn(" ");
    proutn(cramlc(sector, sectx, secty));
    skip(2);
    prout("Good Luck!");
    if (game.state.nscrem) prout("  YOU'LL NEED IT.");
    waitfor();
    newqad(0);
    if (nenhere-iqhere-ithere) shldup=1.0;
    if (neutz) attack(0);	// bad luck to start in a Romulan Neutral Zone
}

void randomize(void) 
{
    srand((int)time(NULL));
}

int choose(int needprompt) 
{
    while (TRUE) {
	tourn = 0;
	thawed = 0;
	skill = SKILL_NONE;
	length = 0;
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
	    tourn = (int)aaitem;
	    thingx = -1;
	    srand((unsigned int)(int)aaitem);
	    break;
	}
	if (isit("saved") || isit("frozen")) {
	    if (thaw()) continue;
	    chew();
	    if (*game.passwd==0) continue;
	    if (!alldone) thawed = 1; // No plaque if not finished
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
    while (length==0 || skill==SKILL_NONE) {
	if (scan() == IHALPHA) {
	    if (isit("short")) length = 1;
	    else if (isit("medium")) length = 2;
	    else if (isit("long")) length = 4;
	    else if (isit("novice")) skill = SKILL_NOVICE;
	    else if (isit("fair")) skill = SKILL_FAIR;
	    else if (isit("good")) skill = SKILL_GOOD;
	    else if (isit("expert")) skill = SKILL_EXPERT;
	    else if (isit("emeritus")) skill = SKILL_EMERITUS;
	    else {
		proutn("What is \"");
		proutn(citem);
		prout("\"?");
	    }
	}
	else {
	    chew();
	    if (length==0) proutn("Would you like a Short, Medium, or Long game? ");
	    else if (skill == SKILL_NONE) proutn("Are you a Novice, Fair, Good, Expert, or Emeritus player? ");
	}
    }
    setpassword();
#ifdef DEBUG
    if (strcmp(game.passwd, "debug")==0) idebug = 1;
#endif

    // Use parameters to generate initial values of things
    damfac = 0.5 * skill;
    game.state.rembase = 3.0*Rand()+2.0;
    inbase = game.state.rembase;
    if (game.options & OPTION_PLANETS)
	inplan = (PLNETMAX/2) + (PLNETMAX/2+1)*Rand();
    game.state.nromrem = (2.0+Rand())*skill;
    game.state.nscrem = (skill > SKILL_FAIR ? 1 : 0);
    game.state.remtime = 7.0 * length;
    intime = game.state.remtime;
    game.state.remkl = 2.0*intime*((skill+1 - 2*Rand())*skill*0.1+.15);
    inkling = game.state.remkl;
    incom = skill + 0.0625*inkling*Rand();
    game.state.remcom= min(10, incom);
    incom = game.state.remcom;
    game.state.remres = (inkling+4*incom)*intime;
    inresor = game.state.remres;
    if (inkling > 50) {
	inbase = (game.state.rembase += 1);
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
    condit = IHGREEN;
    if (energy < 1000.0) condit = IHYELLOW;
    if (game.state.galaxy[quadx][quady].klingons || game.state.galaxy[quadx][quady].romulans)
	condit = IHRED;
    if (!alive) condit=IHDEAD;
}


void newqad(int shutup) 
{
    int i, j, ix, iy, nplan;

    iattak = 1;
    justin = 1;
    basex = basey = 0;
    klhere = 0;
    comhere = 0;
    plnetx = plnety = 0;
    ishere = 0;
    irhere = 0;
    iplnet = 0;
    nenhere = 0;
    neutz = 0;
    inorbit = 0;
    landed = -1;
    ientesc = 0;
    ithere = 0;
    iqhere=0;
    iqengry=0;
    iseenit = 0;
    if (iscate) {
	// Attempt to escape Super-commander, so tbeam back!
	iscate = 0;
	ientesc = 1;
    }
    // Clear quadrant
    for (i=1; i <= QUADSIZE; i++)
	for (j=1; j <= QUADSIZE; j++) 
	    game.quad[i][j] = IHDOT;
    // cope with supernova
    if (game.state.galaxy[quadx][quady].supernova)
	return;
    klhere = game.state.galaxy[quadx][quady].klingons;
    irhere = game.state.galaxy[quadx][quady].romulans;
    nplan  = game.state.galaxy[quadx][quady].planets;
    nenhere = klhere + irhere;

    // Position Starship
    game.quad[sectx][secty] = ship;

    if (game.state.galaxy[quadx][quady].klingons) {
	// Position ordinary Klingons
	for (i = 1; i <= klhere; i++) {
	    dropin(IHK, &ix, &iy);
	    game.kx[i] = ix;
	    game.ky[i] = iy;
	    game.kdist[i] = game.kavgd[i] = sqrt(square(sectx-ix) + square(secty-iy));
	    game.kpower[i] = Rand()*150.0 +300.0 +25.0*skill;
	}
	// If we need a commander, promote a Klingon
	for (i = 1; i <= game.state.remcom ; i++) 
	    if (game.state.cx[i]==quadx && game.state.cy[i]==quady) break;
			
	if (i <= game.state.remcom) {
	    game.quad[ix][iy] = IHC;
	    game.kpower[klhere] = 950.0+400.0*Rand()+50.0*skill;
	    comhere = 1;
	}

	// If we need a super-commander, promote a Klingon
	if (quadx == game.state.isx && quady == game.state.isy) {
	    game.quad[game.kx[1]][game.ky[1]] = IHS;
	    game.kpower[1] = 1175.0 + 400.0*Rand() + 125.0*skill;
	    iscate = game.state.remkl>1;
	    ishere = 1;
	}
    }
    // Put in Romulans if needed
    for (i = klhere+1; i <= nenhere; i++) {
	dropin(IHR, &ix, &iy);
	game.kx[i] = ix;
	game.ky[i] = iy;
	game.kdist[i] = game.kavgd[i] = sqrt(square(sectx-ix) + square(secty-iy));
	game.kpower[i] = Rand()*400.0 + 450.0 + 50.0*skill;
    }
    // If quadrant needs a starbase, put it in
    if (game.state.galaxy[quadx][quady].starbase)
	dropin(IHB, &basex, &basey);
	
    if (nplan) {
	// If quadrant needs a planet, put it in
	for (i=0; i < inplan; i++)
	    if (game.state.plnets[i].x == quadx && game.state.plnets[i].y == quady) break;
	if (i < inplan) {
	    iplnet = i;
	    dropin(IHP, &plnetx, &plnety);
	}
    }
    // Check for condition
    newcnd();
    // And finally the stars
    for (i = 1; i <= game.state.galaxy[quadx][quady].stars; i++) 
	dropin(IHSTAR, &ix, &iy);

    // Check for RNZ
    if (irhere > 0 && klhere == 0) {
	neutz = 1;
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
	if (thingx == quadx && thingy == quady) {
	    dropin(IHQUEST, &ix, &iy);
	    iran(GALSIZE, &thingx, &thingy);
	    nenhere++;
	    iqhere=1;
	    game.kx[nenhere] = ix;
	    game.ky[nenhere] = iy;
	    game.kdist[nenhere] = game.kavgd[nenhere] =
		sqrt(square(sectx-ix) + square(secty-iy));
	    game.kpower[nenhere] = Rand()*6000.0 +500.0 +250.0*skill;
	    if (game.damage[DSRSENS] == 0.0) {
		skip(1);
		prout("MR. SPOCK- \"Captain, this is most unusual.");
		prout("    Please examine your short-range scan.\"");
	    }
	}
    }

    // Decide if quadrant needs a Tholian
    if (game.options & OPTION_THOLIAN) {
	if ((skill < SKILL_GOOD && Rand() <= 0.02) ||   /* Lighten up if skill is low */
	    (skill == SKILL_GOOD && Rand() <= 0.05) ||
	    (skill > SKILL_GOOD && Rand() <= 0.08)
    #ifdef DEBUG
	    || strcmp(game.passwd, "tholianx")==0
    #endif
	    ) {
	    do {
		ithx = Rand() > 0.5 ? QUADSIZE : 1;
		ithy = Rand() > 0.5 ? QUADSIZE : 1;
	    } while (game.quad[ithx][ithy] != IHDOT);
	    game.quad[ithx][ithy] = IHT;
	    ithere = 1;
	    nenhere++;
	    game.kx[nenhere] = ithx;
	    game.ky[nenhere] = ithy;
	    game.kdist[nenhere] = game.kavgd[nenhere] =
		sqrt(square(sectx-ithx) + square(secty-ithy));
	    game.kpower[nenhere] = Rand()*400.0 +100.0 +25.0*skill;
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
    if (ithere) {
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

    if (nenhere-iqhere-ithere < 2) return;

    do {
	sw = FALSE;
	for (j = 1; j < nenhere; j++)
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
