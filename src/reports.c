#include "sst.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void attakreport(int curt) 
{
    if (!curt) {
	if (is_scheduled(FCDBAS)) {
	    prout("Starbase in %s is currently under Commander attack.",
		  cramlc(quadrant, game.battle));
	    prout("It can hold out until Stardate %d.", 
		  (int)scheduled(FCDBAS));
	}
	if (game.isatb == 1) {
	    prout("Starbase in %s is under Super-commander attack.",
		  cramlc(quadrant, game.state.kscmdr));
	    prout("It can hold out until Stardate %d.", 
		  (int)scheduled(FSCDBAS));
	}
    } else {
        if (is_scheduled(FCDBAS))
	    proutn("Base in %i - %i attacked by C. Alive until %.1f", game.battle.x, game.battle.y, scheduled(FCDBAS));
        if (game.isatb)
	    proutn("Base in %i - %i attacked by S. Alive until %.1f", game.state.kscmdr.x, game.state.kscmdr.y, scheduled(FSCDBAS));
    }
    clreol();
}
	

void report(void) 
{
    char *s1,*s2,*s3;

    chew();
    s1 = (game.thawed?"game.thawed ":"");
    switch (game.length) {
    case 1: s2="short"; break;
    case 2: s2="medium"; break;
    case 4: s2="long"; break;
    default: s2="unknown length"; break;
    }
    switch (game.skill) {
    case SKILL_NOVICE: s3="novice"; break;
    case SKILL_FAIR: s3="fair"; break;
    case SKILL_GOOD: s3="good"; break;
    case SKILL_EXPERT: s3="expert"; break;
    case SKILL_EMERITUS: s3="emeritus"; break;
    default: s3="skilled"; break;
    }
    prout("");
    prout("You %s playing a %s%s %s game.",
	  game.alldone? "were": "are now", s1, s2, s3);
    if (game.skill>SKILL_GOOD && game.thawed && !game.alldone) prout("No plaque is allowed.");
    if (game.tourn) prout("This is tournament game %d.", game.tourn);
    prout("Your secret password is \"%s\"",game.passwd);
    proutn("%d of %d Klingons have been killed", KLINGKILLED, INKLINGTOT);
    if (NKILLC) prout(", including %d Commander%s.", NKILLC, NKILLC==1?"":"s");
    else if (NKILLK + NKILLSC > 0) prout(", but no Commanders.");
    else prout(".");
    if (game.skill > SKILL_FAIR) prout("The Super Commander has %sbeen destroyed.",
				  game.state.nscrem?"not ":"");
    if (game.state.rembase != game.inbase) {
	proutn("There ");
	if (game.inbase-game.state.rembase==1) proutn("has been 1 base");
	else {
	    proutn("have been %d bases", game.inbase-game.state.rembase);
	}
	prout(" destroyed, %d remaining.", game.state.rembase);
    }
    else prout("There are %d bases.", game.inbase);
    if (game.damage[DRADIO] == 0.0 || game.condit == IHDOCKED || game.iseenit) {
	/* Don't report this if not seen and
	   either the radio is dead or not at base! */
	attakreport(0);
	game.iseenit = 1;
    }
    if (game.casual) prout("%d casualt%s suffered so far.",
		      game.casual, game.casual==1? "y" : "ies");
    if (game.nhelp) prout("There were %d call%s for help.",
		     game.nhelp, game.nhelp==1 ? "" : "s");
    if (game.ship == IHE) {
	proutn("You have ");
	if (game.nprobes) proutn("%d", game.nprobes);
	else proutn("no");
	proutn(" deep space probe");
	if (game.nprobes!=1) proutn("s");
	prout(".");
    }
    if ((game.damage[DRADIO] == 0.0 || game.condit == IHDOCKED)
		&& is_scheduled(FDSPROB)) {
	if (game.isarmed) 
	    proutn("An armed deep space probe is in");
	else
	    proutn("A deep space probe is in");
	proutn(cramlc(quadrant, game.probec));
	prout(".");
    }
    if (game.icrystl) {
	if (game.cryprob <= .05)
	    prout("Dilithium crystals aboard ship... not yet used.");
	else {
	    int i=0;
	    double ai = 0.05;
	    while (game.cryprob > ai) {
		ai *= 2.0;
		i++;
	    }
	    prout("Dilithium crystals have been used %d time%s.",
		  i, i==1? "" : "s");
	}
    }
    skip(1);
}
	
void lrscan(void) 
{
    int x, y;
    chew();
    if (game.damage[DLRSENS] != 0.0) {
	/* Now allow base's sensors if docked */
	if (game.condit != IHDOCKED) {
	    prout("LONG-RANGE SENSORS DAMAGED.");
	    return;
	}
	prout("Starbase's long-range scan");
    }
    else {
	prout("Long-range scan");
    }
    for (x = game.quadrant.x-1; x <= game.quadrant.x+1; x++) {
	proutn(" ");
	for (y = game.quadrant.y-1; y <= game.quadrant.y+1; y++) {
	    if (!VALID_QUADRANT(x, y))
		proutn("  -1");
	    else {
		if (!game.damage[DRADIO])
		    game.state.galaxy[x][y].charted = true;
		game.state.chart[x][y].klingons = game.state.galaxy[x][y].klingons;
		game.state.chart[x][y].starbase = game.state.galaxy[x][y].starbase;
		game.state.chart[x][y].stars = game.state.galaxy[x][y].stars;
		if (game.state.galaxy[x][y].supernova) 
		    proutn(" ***");
		else
		    proutn(" %3d", game.state.chart[x][y].klingons*100 + game.state.chart[x][y].starbase * 10 + game.state.chart[x][y].stars);
	    }
	}
	prout(" ");
    }
}

void dreprt(void) 
{
    bool jdam = false;
    int i;
    chew();

    for (i = 0; i < NDEVICES; i++) {
	if (game.damage[i] > 0.0) {
	    if (!jdam) {
		prout("DEVICE            -REPAIR TIMES-");
		prout("                IN FLIGHT   DOCKED");
		jdam = true;
	    }
	    prout("  %16s %8.2f  %8.2f", 
		  device[i],
		  game.damage[i]+0.05,
		  game.docfac*game.damage[i]+0.005);
	}
    }
    if (!jdam) prout("All devices functional.");
}

void rechart(void)
/* update the chart in the Enterprise's computer from galaxy data */
{
    int i, j;
    game.lastchart = game.state.date;
    for_quadrants(i)
	for_quadrants(j) 
	    if (game.state.galaxy[i][j].charted) {
		game.state.chart[i][j].klingons = game.state.galaxy[i][j].klingons;
		game.state.chart[i][j].starbase = game.state.galaxy[i][j].starbase;
		game.state.chart[i][j].stars = game.state.galaxy[i][j].stars;
	    }
}

void chart(int nn) 
{
    int i,j;
    chew();

    if (game.damage[DRADIO] == 0.0)
	rechart();

    if (game.lastchart < game.state.date && game.condit == IHDOCKED) {
	prout("Spock-  \"I revised the Star Chart from the starbase's records.\"");
	rechart();
    }

    if (nn == 0) prout("       STAR CHART FOR THE KNOWN GALAXY");
    if (game.state.date > game.lastchart)
	prout("(Last surveillance update %d stardates ago).",
	      (int)(game.state.date-game.lastchart));
    prout("      1    2    3    4    5    6    7    8");
    for_quadrants(i) {
	proutn("%d |", i);
	for_quadrants(j) {
	    char buf[4];
	    if ((game.options & OPTION_SHOWME) && i == game.quadrant.x && j == game.quadrant.y)
		proutn("<");
	    else
		proutn(" ");
	    if (game.state.galaxy[i][j].supernova)
		strcpy(buf, "***");
	    else if (!game.state.galaxy[i][j].charted && game.state.galaxy[i][j].starbase)
		strcpy(buf, ".1.");
	    else if (game.state.galaxy[i][j].charted)
		sprintf(buf, "%3d", game.state.chart[i][j].klingons*100 + game.state.chart[i][j].starbase * 10 + game.state.chart[i][j].stars);
	    else
		strcpy(buf, "...");
	    proutn(buf);
	    if ((game.options & OPTION_SHOWME) && i == game.quadrant.x && j == game.quadrant.y)
		proutn(">");
	    else
		proutn(" ");
	}
	proutn("  |");
	if (i<GALSIZE) skip(1);
    }
    prout("");
}

static void sectscan(int goodScan, int i, int j) 
{
    if (goodScan || (abs(i-game.sector.x)<= 1 && abs(j-game.sector.y) <= 1)){
	if ((game.quad[i][j]==IHMATER0)||(game.quad[i][j]==IHMATER1)||(game.quad[i][j]==IHMATER2)||(game.quad[i][j]==IHE)||(game.quad[i][j]==IHF)){
	    switch (game.condit) {
	    case IHRED: textcolor(RED); break;
	    case IHGREEN: textcolor(GREEN); break;
	    case IHYELLOW: textcolor(YELLOW); break;
	    case IHDOCKED: textcolor(CYAN); break;
	    case IHDEAD: textcolor(BROWN);
	    }
	    if (game.quad[i][j] != game.ship) 
		highvideo();
	}
	proutn("%c ",game.quad[i][j]);
	textcolor(DEFAULT);
    }
    else
	proutn("- ");
}

static void status(int req) 
{
    char *cp = NULL;
    int t, dam = 0;
    switch (req) {
    case 1:
	proutn("Stardate      %.1f, Time Left %.2f", game.state.date, game.state.remtime);
	break;
    case 2:
	if (game.condit != IHDOCKED) newcnd();
	switch (game.condit) {
	case IHRED: cp = "RED"; break;
	case IHGREEN: cp = "GREEN"; break;
	case IHYELLOW: cp = "YELLOW"; break;
	case IHDOCKED: cp = "DOCKED"; break;
	case IHDEAD: cp="DEAD"; break;
	}
	for (t=0;t<NDEVICES;t++)
	    if (game.damage[t]>0) 
		dam++;
	proutn("Condition     %s, %i DAMAGES", cp, dam);
	break;
    case 3:
	proutn("Position      %d - %d , %d - %d",
	       game.quadrant.x, game.quadrant.y, game.sector.x, game.sector.y);
	break;
    case 4:
	proutn("Life Support  ");
	if (game.damage[DLIFSUP] != 0.0) {
	    if (game.condit == IHDOCKED)
		proutn("DAMAGED, Base provides");
	    else
		proutn("DAMAGED, reserves=%4.2f", game.lsupres);
	}
	else
	    proutn("ACTIVE");
	break;
    case 5:
	proutn("Warp Factor   %.1f", game.warpfac);
	break;
    case 6:
	proutn("Energy        %.2f", game.energy);
	if (game.icrystl && (game.options & OPTION_SHOWME))	/* ESR */
	    proutn(" (have crystals)");
	break;
    case 7:
	proutn("Torpedoes     %d", game.torps);
	break;
    case 8:
	proutn("Shields       ");
	if (game.damage[DSHIELD] != 0)
	    proutn("DAMAGED,");
	else if (game.shldup)
	    proutn("UP,");
	else
	    proutn("DOWN,");
	proutn(" %d%% %.1f units",
	       (int)((100.0*game.shield)/game.inshld + 0.5), game.shield);
	break;
    case 9:
	proutn("Klingons Left %d", KLINGREM);
	break;
    case 10:
	if (game.options & OPTION_WORLDS) {
	    int here = game.state.galaxy[game.quadrant.x][game.quadrant.y].planet;
	    if (here != NOPLANET && game.state.plnets[here].inhabited != UNINHABITED)
		proutn("Major system  %s", systemname(here));
	    else
		proutn("Sector is uninhabited");
	}

	break;
    case 11:
	attakreport(1);
	break;
    }
}
		
int srscan(int l) 
{
    /* the "sy" request is undocumented */
    static char requests[][3] =
	{"","da","co","po","ls","wa","en","to","sh","kl","sy", "ti"};
    
    int i, j, jj, req=0;
    int goodScan=true, leftside=true, rightside=true, nn=false; 
    switch (l) {
    case SCAN_FULL: // SRSCAN
	if (game.damage[DSRSENS] != 0) {
	    /* Allow base's sensors if docked */
	    if (game.condit != IHDOCKED) {
		prout("   S.R. SENSORS DAMAGED!");
		goodScan=false;
	    }
	    else
		prout("  [Using Base's sensors]");
	}
	else prout("     Short-range scan");
	if (goodScan && !game.damage[DRADIO]) { 
	    game.state.chart[game.quadrant.x][game.quadrant.y].klingons = game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons;
	    game.state.chart[game.quadrant.x][game.quadrant.y].starbase = game.state.galaxy[game.quadrant.x][game.quadrant.y].starbase;
	    game.state.chart[game.quadrant.x][game.quadrant.y].stars = game.state.galaxy[game.quadrant.x][game.quadrant.y].stars;
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].charted = true;
	}
	scan();
	if (isit("chart")) nn = true;
	if (isit("no")) rightside = false;
	chew();
	prout("    1 2 3 4 5 6 7 8 9 10");
	break;
    case SCAN_REQUEST:
	while (scan() == IHEOL)
	    proutn("Information desired? ");
	chew();
	for (req = 1; req <= sizeof(requests)/sizeof(requests[0]); req++)
	    if (strncmp(citem,requests[req],min(2,strlen(citem)))==0)
		break;
	if (req > sizeof(requests)/sizeof(requests[0])) {
	    prout("UNRECOGNIZED REQUEST. Legal requests are:");
	    prout("  date, condition, position, lsupport, warpfactor,");
	    prout("  energy, torpedoes, shields, klingons, time, system, bases.");
	    return false;
	}
	// no break
    case SCAN_STATUS: // STATUS
	chew();
	leftside = false;
	skip(1);
	// no break
    case SCAN_NO_LEFTSIDE: // REQUEST
	leftside=false;
	break;
    }
    if (game.condit != IHDOCKED) newcnd();
    for (i = 1; i <= max(QUADSIZE, sizeof(requests)/sizeof(requests[0])); i++) {
	jj = (req!=0 ? req : i);
	if (leftside && i <= QUADSIZE) {
	    proutn("%2d  ", i);
	    for_sectors(j) {
		sectscan(goodScan, i, j);
	    }
	}
	if (rightside)
	    status(jj);
	if (i<sizeof(requests)/sizeof(requests[0])) skip(1);
	if (req!=0) return(goodScan);
    }
    prout("");
    if (nn) chart(1);
    return(goodScan);
}
			
			
void eta(void)
{
    int ix1, ix2, iy1, iy2;
    bool wfl, prompt = false;
    double ttime, twarp, tpower;
    if (game.damage[DCOMPTR] != 0.0) {
	prout("COMPUTER DAMAGED, USE A POCKET CALCULATOR.");
	skip(1);
	return;
    }
    if (scan() != IHREAL) {
	prompt = true;
	chew();
	proutn("Destination quadrant and/or sector? ");
	if (scan()!=IHREAL) {
	    huh();
	    return;
	}
    }
    iy1 = aaitem +0.5;
    if (scan() != IHREAL) {
	huh();
	return;
    }
    ix1 = aaitem + 0.5;
    if (scan() == IHREAL) {
	iy2 = aaitem + 0.5;
	if (scan() != IHREAL) {
	    huh();
	    return;
	}
	ix2 = aaitem + 0.5;
    }
    else {
	if (game.quadrant.y>ix1) ix2 = 1;
	else ix2=QUADSIZE;
	if (game.quadrant.x>iy1) iy2 = 1;
	else iy2=QUADSIZE;
    }

    if (!VALID_QUADRANT(ix1, iy1) || !VALID_SECTOR(ix2, iy2)) {
	huh();
	return;
    }
    game.dist = sqrt(square(iy1-game.quadrant.x+0.1*(iy2-game.sector.x))+
		square(ix1-game.quadrant.y+0.1*(ix2-game.sector.y)));
    wfl = false;

    if (prompt) prout("Answer \"no\" if you don't know the value:");
    for (;;) {
	chew();
	proutn("Time or arrival date? ");
	if (scan()==IHREAL) {
	    ttime = aaitem;
	    if (ttime > game.state.date) ttime -= game.state.date; // Actually a star date
	    if (ttime <= 1e-10 ||
		(twarp=(floor(sqrt((10.0*game.dist)/ttime)*10.0)+1.0)/10.0) > 10) {
		prout("We'll never make it, sir.");
		chew();
		return;
	    }
	    if (twarp < 1.0) twarp = 1.0;
	    break;
	}
	chew();
	proutn("Warp factor? ");
	if (scan()== IHREAL) {
	    wfl = true;
	    twarp = aaitem;
	    if (twarp<1.0 || twarp > 10.0) {
		huh();
		return;
	    }
	    break;
	}
	prout("Captain, certainly you can give me one of these.");
    }
    for (;;) {
	chew();
	ttime = (10.0*game.dist)/square(twarp);
	tpower = game.dist*twarp*twarp*twarp*(game.shldup+1);
	if (tpower >= game.energy) {
	    prout("Insufficient energy, sir.");
	    if (game.shldup==0 || tpower > game.energy*2.0) {
		if (!wfl) return;
		proutn("New warp factor to try? ");
		if (scan() == IHREAL) {
		    wfl = true;
		    twarp = aaitem;
		    if (twarp<1.0 || twarp > 10.0) {
			huh();
			return;
		    }
		    continue;
		}
		else {
		    chew();
		    skip(1);
		    return;
		}
	    }
	    prout("But if you lower your shields,");
	    proutn("remaining");
	    tpower /= 2;
	}
	else
	    proutn("Remaining");
	prout(" game.energy will be %.2f.", game.energy-tpower);
	if (wfl) {
	    prout("And we will arrive at stardate %.2f.",
		  game.state.date+ttime);
	}
	else if (twarp==1.0)
	    prout("Any warp speed is adequate.");
	else {
	    prout("Minimum warp needed is %.2f,", twarp);
	    prout("and we will arrive at stardate %.2f.",
		  game.state.date+ttime);
	}
	if (game.state.remtime < ttime)
	    prout("Unfortunately, the Federation will be destroyed by then.");
	if (twarp > 6.0)
	    prout("You'll be taking risks at that speed, Captain");
	if ((game.isatb==1 && game.state.kscmdr.y == iy1 && game.state.kscmdr.x == ix1 &&
	     scheduled(FSCDBAS)< ttime+game.state.date)||
	    (scheduled(FCDBAS)<ttime+game.state.date && game.battle.y==iy1 && game.battle.x == ix1))
	    prout("The starbase there will be destroyed by then.");
	proutn("New warp factor to try? ");
	if (scan() == IHREAL) {
	    wfl = true;
	    twarp = aaitem;
	    if (twarp<1.0 || twarp > 10.0) {
		huh();
		return;
	    }
	}
	else {
	    chew();
	    skip(1);
	    return;
	}
    }
			
}
