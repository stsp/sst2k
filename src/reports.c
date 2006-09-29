#include "sst.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void attackreport(bool curt)
/* report status of bases under attack */
{
    if (!curt) {
	if (is_scheduled(FCDBAS)) {
	    prout(_("Starbase in %s is currently under Commander attack."),
		  cramlc(quadrant, game.battle));
	    prout(_("It can hold out until Stardate %d."),
		  (int)scheduled(FCDBAS));
	}
	else if (game.isatb == 1) {
	    prout(_("Starbase in %s is under Super-commander attack."),
		  cramlc(quadrant, game.state.kscmdr));
	    prout(_("It can hold out until Stardate %d."),
		  (int)scheduled(FSCDBAS));
	} else {
	    prout(_("No Starbase is currently under attack."));
	}
    } else {
        if (is_scheduled(FCDBAS))
	    proutn(_("Base in %i - %i attacked by C. Alive until %.1f"), game.battle.x, game.battle.y, scheduled(FCDBAS));
        if (game.isatb)
	    proutn(_("Base in %i - %i attacked by S. Alive until %.1f"), game.state.kscmdr.x, game.state.kscmdr.y, scheduled(FSCDBAS));
	clreol();
    }
}
	

void report(void)
/* report on general game status */
{
    char *s1,*s2,*s3;

    chew();
    s1 = (game.thawed?_("thawed "):"");
    switch (game.length) {
    case 1: s2=_("short"); break;
    case 2: s2=_("medium"); break;
    case 4: s2=_("long"); break;
    default: s2=_("unknown length"); break;
    }
    switch (game.skill) {
    case SKILL_NOVICE: s3=_("novice"); break;
    case SKILL_FAIR: s3=_("fair"); break;
    case SKILL_GOOD: s3=_("good"); break;
    case SKILL_EXPERT: s3=_("expert"); break;
    case SKILL_EMERITUS: s3=_("emeritus"); break;
    default: s3=_("skilled"); break;
    }
    skip(1);
    prout(_("You %s a %s%s %s game."),
	  game.alldone? _("were playing") : _("are playing"), s1, s2, s3);
    if (game.skill>SKILL_GOOD && game.thawed && !game.alldone)
	prout(_("No plaque is allowed."));
    if (game.tourn)
	prout(_("This is tournament game %d."), game.tourn);
    prout(_("Your secret password is \"%s\""),game.passwd);
    proutn(_("%d of %d Klingons have been killed"), KLINGKILLED, INKLINGTOT);
    if (NKILLC)
	prout(_(", including %d Commander%s."), NKILLC, NKILLC==1?"":_("s"));
    else if (NKILLK + NKILLSC > 0)
	prout(_(", but no Commanders."));
    else
	prout(".");
    if (game.skill > SKILL_FAIR)
	prout(_("The Super Commander has %sbeen destroyed."),
	      game.state.nscrem?_("not "):"");
    if (game.state.rembase != game.inbase) {
	proutn(_("There "));
	if (game.inbase-game.state.rembase==1)
	    proutn(_("has been 1 base"));
	else {
	    proutn(_("have been %d bases"), game.inbase-game.state.rembase);
	}
	prout(_(" destroyed, %d remaining."), game.state.rembase);
    }
    else
	prout(_("There are %d bases."), game.inbase);
    if (!damaged(DRADIO) || game.condition == docked || game.iseenit) {
	// Don't report this if not seen and
	// either the radio is dead or not at base!
	attackreport(false);
	game.iseenit = true;
    }
    if (game.casual) 
	prout(_("%d casualt%s suffered so far."),
	      game.casual, game.casual==1? "y" : "ies");
    if (game.nhelp)
	prout(_("There were %d call%s for help."),
	      game.nhelp, game.nhelp==1 ? "" : _("s"));
    if (game.ship == IHE) {
	proutn(_("You have "));
	if (game.nprobes)
	    proutn("%d", game.nprobes);
	else
	    proutn(_("no"));
	proutn(_(" deep space probe"));
	if (game.nprobes!=1)
	    proutn(_("s"));
	prout(".");
    }
    if ((!damaged(DRADIO) || game.condition == docked)
		&& is_scheduled(FDSPROB)) {
	if (game.isarmed) 
	    proutn(_("An armed deep space probe is in "));
	else
	    proutn(_("A deep space probe is in "));
	proutn(cramlc(quadrant, game.probec));
	prout(".");
    }
    if (game.icrystl) {
	if (game.cryprob <= .05)
	    prout(_("Dilithium crystals aboard ship... not yet used."));
	else {
	    int i=0;
	    double ai = 0.05;
	    while (game.cryprob > ai) {
		ai *= 2.0;
		i++;
	    }
	    prout(_("Dilithium crystals have been used %d time%s."),
		  i, i==1? "" : _("s"));
	}
    }
    skip(1);
}
	
void lrscan(void) 
/* long-range sensor scan */
{
    int x, y;
    if (damaged(DLRSENS)) {
	/* Now allow base's sensors if docked */
	if (game.condition != docked) {
	    prout(_("LONG-RANGE SENSORS DAMAGED."));
	    return;
	}
	prout(_("Starbase's long-range scan"));
    }
    else {
	prout(_("Long-range scan"));
    }
    for (x = game.quadrant.x-1; x <= game.quadrant.x+1; x++) {
	proutn(" ");
	for (y = game.quadrant.y-1; y <= game.quadrant.y+1; y++) {
	    if (!VALID_QUADRANT(x, y))
		proutn("  -1");
	    else {
		if (!damaged(DRADIO))
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

void damagereport(void) 
/* damage report */
{
    bool jdam = false;
    int i;
    chew();

    for (i = 0; i < NDEVICES; i++) {
	if (damaged(i)) {
	    if (!jdam) {
		prout(_("\tDEVICE\t\t\t-REPAIR TIMES-"));
		prout(_("\t\t\tIN FLIGHT\t\tDOCKED"));
		jdam = true;
	    }
	    prout("  %-26s\t%8.2f\t\t%8.2f", 
		  device[i],
		  game.damage[i]+0.05,
		  game.docfac*game.damage[i]+0.005);
	}
    }
    if (!jdam)
	prout(_("All devices functional."));
}

void rechart(void)
/* update the chart in the Enterprise's computer from galaxy data */
{
    int i, j;
    game.lastchart = game.state.date;
    for (i = 1; i <= GALSIZE; i++)
	for (j = 1; j <= GALSIZE; j++)
	    if (game.state.galaxy[i][j].charted) {
		game.state.chart[i][j].klingons = game.state.galaxy[i][j].klingons;
		game.state.chart[i][j].starbase = game.state.galaxy[i][j].starbase;
		game.state.chart[i][j].stars = game.state.galaxy[i][j].stars;
	    }
}

void chart(void)
/* display the star chart */ 
{
    int i,j;
    chew();

    if (!damaged(DRADIO))
	rechart();

    if (game.lastchart < game.state.date && game.condition == docked) {
	prout(_("Spock-  \"I revised the Star Chart from the starbase's records.\""));
	rechart();
    }

    prout(_("       STAR CHART FOR THE KNOWN GALAXY"));
    if (game.state.date > game.lastchart)
	prout(_("(Last surveillance update %d stardates ago)."),
	      (int)(game.state.date-game.lastchart));
    prout("      1    2    3    4    5    6    7    8");
    for (i = 1; i <= GALSIZE; i++) {
	proutn("%d |", i);
	for (j = 1; j <= GALSIZE; j++) {
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
	if (i<GALSIZE)
	    skip(1);
    }
}

static void sectscan(int goodScan, int i, int j) 
/* light up an individual dot in a sector */
{
    if (goodScan || (abs(i-game.sector.x)<= 1 && abs(j-game.sector.y) <= 1)){
	if ((game.quad[i][j]==IHMATER0)||(game.quad[i][j]==IHMATER1)||(game.quad[i][j]==IHMATER2)||(game.quad[i][j]==IHE)||(game.quad[i][j]==IHF)){
	    switch (game.condition) {
	    case red: textcolor(RED); break;
	    case green: textcolor(GREEN); break;
	    case yellow: textcolor(YELLOW); break;
	    case docked: textcolor(CYAN); break;
	    case dead: textcolor(BROWN);
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

void status(int req)
/* print status report lines */
{
#define RQ(n, a) if (!req || req == n) do { a } while(0)
    char *cp = NULL, s[256];
    int t, dam = 0;

    RQ(1,
	prstat(_("Stardate"), _("%.1f, Time Left %.2f"), game.state.date, game.state.remtime);
    );

    RQ(2,
	if (game.condition != docked)
	    newcnd();
	switch (game.condition) {
	    case red: cp = _("RED"); break;
	    case green: cp = _("GREEN"); break;
	    case yellow: cp = _("YELLOW"); break;
	    case docked: cp = _("DOCKED"); break;
	    case dead: cp = _("DEAD"); break;
	}
	for (t=0;t<NDEVICES;t++)
	    if (game.damage[t]>0) 
		dam++;
	prstat(_("Condition"), _("%s, %i DAMAGES"), cp, dam);
    );

    RQ(3,
	prstat(_("Position"), "%d - %d , %d - %d",
	      game.quadrant.x, game.quadrant.y, game.sector.x, game.sector.y);
    );

    RQ(4,
	if (damaged(DLIFSUP)) {
	    if (game.condition == docked)
		sprintf(s, _("DAMAGED, Base provides"));
	    else
		sprintf(s, _("DAMAGED, reserves=%4.2f"), game.lsupres);
	}
	else
	    sprintf(s, _("ACTIVE"));
	prstat(_("Life Support"), s);
    );

    RQ(5,
	prstat(_("Warp Factor"), "%.1f", game.warpfac);
    );

    RQ(6,
	prstat(_("Energy"), "%.2f%s", game.energy,
		(game.icrystl && (game.options & OPTION_SHOWME)) ? /* ESR */
		_(" (have crystals)") : "");
    );

    RQ(7,
	prstat(_("Torpedoes"), "%d", game.torps);
    );

    RQ(8,
	if (damaged(DSHIELD))
	    strcpy(s, _("DAMAGED,"));
	else if (game.shldup)
	    strcpy(s, _("UP,"));
	else
	    strcpy(s, _("DOWN,"));
	sprintf(s + strlen(s), _(" %d%% %.1f units"),
	       (int)((100.0*game.shield)/game.inshld + 0.5), game.shield);
	prstat(_("Shields"), s);
    );

    RQ(9,
        prstat(_("Klingons Left"), "%d", KLINGREM);
    );

    RQ(10,
	if (game.options & OPTION_WORLDS) {
	    int plnet = game.state.galaxy[game.quadrant.x][game.quadrant.y].planet;
	    if (plnet != NOPLANET && game.state.planets[plnet].inhabited != UNINHABITED)
		prstat(_("Major system"), "%s", systnames[plnet]);
	    else
		prout(_("Sector is uninhabited"));
	}
    );

    RQ(11,
	attackreport(!req);
    );

#undef RQ
}

void request(void)
{
    int req;
    static char requests[][3] =
	{"da","co","po","ls","wa","en","to","sh","kl","sy", "ti"};

    while (scan() == IHEOL)
	proutn(_("Information desired? "));
    chew();
    for (req = 0; req < ARRAY_SIZE(requests); req++)
	if (strncmp(citem, requests[req], min(2,strlen(citem)))==0)
	    break;
    if (req >= ARRAY_SIZE(requests)) {
	prout(_("UNRECOGNIZED REQUEST. Legal requests are:"));
	prout(("  date, condition, position, lsupport, warpfactor,"));
	prout(("  energy, torpedoes, shields, klingons, system, time."));
	return;
    }
    status(req + 1);
}
		
void srscan(void)
/* short-range scan */
{
    int i, j;
    int goodScan=true;
    if (damaged(DSRSENS)) {
	/* Allow base's sensors if docked */
	if (game.condition != docked) {
	    prout(_("   S.R. SENSORS DAMAGED!"));
	    goodScan=false;
	}
	else
	    prout(_("  [Using Base's sensors]"));
    }
    else
	prout(_("     Short-range scan"));
    if (goodScan && !damaged(DRADIO)) { 
	game.state.chart[game.quadrant.x][game.quadrant.y].klingons = game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons;
	game.state.chart[game.quadrant.x][game.quadrant.y].starbase = game.state.galaxy[game.quadrant.x][game.quadrant.y].starbase;
	game.state.chart[game.quadrant.x][game.quadrant.y].stars = game.state.galaxy[game.quadrant.x][game.quadrant.y].stars;
	game.state.galaxy[game.quadrant.x][game.quadrant.y].charted = true;
    }
    prout("    1 2 3 4 5 6 7 8 9 10");
    if (game.condition != docked)
	newcnd();
    for (i = 1; i <= QUADSIZE; i++) {
	proutn("%2d  ", i);
	for (j = 1; j <= QUADSIZE; j++) {
	    sectscan(goodScan, i, j);
	}
	skip(1);
    }
}
			
			
void eta(void)
/* use computer to get estimated time of arrival for a warp jump */
{
    coord w1, w2;
    bool wfl, prompt = false;
    double ttime, twarp, tpower;
    if (damaged(DCOMPTR)) {
	prout(_("COMPUTER DAMAGED, USE A POCKET CALCULATOR."));
	skip(1);
	return;
    }
    if (scan() != IHREAL) {
	prompt = true;
	chew();
	proutn(_("Destination quadrant and/or sector? "));
	if (scan()!=IHREAL) {
	    huh();
	    return;
	}
    }
    w1.y = aaitem +0.5;
    if (scan() != IHREAL) {
	huh();
	return;
    }
    w1.x = aaitem + 0.5;
    if (scan() == IHREAL) {
	w2.y = aaitem + 0.5;
	if (scan() != IHREAL) {
	    huh();
	    return;
	}
	w2.x = aaitem + 0.5;
    }
    else {
	if (game.quadrant.y>w1.x)
	    w2.x = 1;
	else
	    w2.x=QUADSIZE;
	if (game.quadrant.x>w1.y)
	    w2.y = 1;
	else
	    w2.y=QUADSIZE;
    }

    if (!VALID_QUADRANT(w1.x, w1.y) || !VALID_SECTOR(w2.x, w2.y)) {
	huh();
	return;
    }
    game.dist = sqrt(square(w1.y-game.quadrant.y+0.1*(w2.y-game.sector.y))+
		square(w1.x-game.quadrant.x+0.1*(w2.x-game.sector.x)));
    wfl = false;

    if (prompt)
	prout(_("Answer \"no\" if you don't know the value:"));
    for (;;) {
	chew();
	proutn(_("Time or arrival date? "));
	if (scan()==IHREAL) {
	    ttime = aaitem;
	    if (ttime > game.state.date)
		ttime -= game.state.date; // Actually a star date
	    if (ttime <= 1e-10 ||
		(twarp=(floor(sqrt((10.0*game.dist)/ttime)*10.0)+1.0)/10.0) > 10) {
		prout(_("We'll never make it, sir."));
		chew();
		return;
	    }
	    if (twarp < 1.0)
		twarp = 1.0;
	    break;
	}
	chew();
	proutn(_("Warp factor? "));
	if (scan()== IHREAL) {
	    wfl = true;
	    twarp = aaitem;
	    if (twarp<1.0 || twarp > 10.0) {
		huh();
		return;
	    }
	    break;
	}
	prout(_("Captain, certainly you can give me one of these."));
    }
    for (;;) {
	chew();
	ttime = (10.0*game.dist)/square(twarp);
	tpower = game.dist*twarp*twarp*twarp*(game.shldup+1);
	if (tpower >= game.energy) {
	    prout(_("Insufficient energy, sir."));
	    if (!game.shldup || tpower > game.energy*2.0) {
		if (!wfl)
		    return;
		proutn(_("New warp factor to try? "));
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
	    prout(_("But if you lower your shields,"));
	    proutn(_("remaining"));
	    tpower /= 2;
	}
	else
	    proutn(_("Remaining"));
	prout(_(" energy will be %.2f."), game.energy-tpower);
	if (wfl) {
	    prout(_("And we will arrive at stardate %.2f."),
		  game.state.date+ttime);
	}
	else if (twarp==1.0)
	    prout(_("Any warp speed is adequate."));
	else {
	    prout(_("Minimum warp needed is %.2f,"), twarp);
	    prout(_("and we will arrive at stardate %.2f."),
		  game.state.date+ttime);
	}
	if (game.state.remtime < ttime)
	    prout(_("Unfortunately, the Federation will be destroyed by then."));
	if (twarp > 6.0)
	    prout(_("You'll be taking risks at that speed, Captain"));
	if ((game.isatb==1 && same(game.state.kscmdr, w1) &&
	     scheduled(FSCDBAS)< ttime+game.state.date)||
	    (scheduled(FCDBAS)<ttime+game.state.date && same(game.battle, w1)))
	    prout(_("The starbase there will be destroyed by then."));
	proutn(_("New warp factor to try? "));
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

#if BSD_BUG_FOR_BUG
/*
 *	A visual scan is made in a particular direction of three sectors
 *	in the general direction specified.  This takes time, and
 *	Klingons can attack you, so it should be done only when sensors
 * 	are out.  Code swiped from BSD-Trek.  Not presently used, as we
 *	automatically display all adjacent sectors on the short-range
 *	scan even when short-range sensors are out.
 */

/* This struct[] has the delta x, delta y for particular directions */
coord visdelta[] =
{
    {-1,-1},
    {-1, 0},
    {-1, 1},
    {0,	 1},
    {1,	 1},
    {1,	 0},
    {1,	-1},
    {0,	-1},
    {-1,-1},
    {-1, 0},
    {-1, 1},
};

void visual(void)
{
    int		co, ix, iy;
    coord	*v;

    if (scan() != IHREAL) {
	chew();
	proutn(_("Direction? "));
	if (scan()!=IHREAL) {
	    huh();
	    return;
	}
    }
    if (aaitem < 0.0 || aaitem > 360.0)
	return;
    co = (aaitem + 22) / 45;
    v = &visdelta[co];
    ix = game.sector.x + v->x;
    iy = game.sector.y + v->y;
    if (ix < 0 || ix >= QUADSIZE || iy < 0 || iy >= QUADSIZE)
	co = '?';
    else
	co = game.quad[ix][iy];
    printf("%d,%d %c ", ix, iy, co);
    v++;
    ix = game.sector.x + v->x;
    iy = game.sector.y + v->y;
    if (ix < 0 || ix >= QUADSIZE || iy < 0 || iy >= QUADSIZE)
	co = '?';
    else
	co = game.quad[ix][iy];
    printf("%c ", co);
    v++;
    ix = game.sector.x + v->x;
    iy = game.sector.y + v->y;
    if (ix < 0 || ix >= QUADSIZE || iy < 0 || iy >= QUADSIZE)
	co = '?';
    else
	co = game.quad[ix][iy];
    printf("%c %d,%d\n", co, ix, iy);
    game.optime = 0.5;
    game.ididit = true;
}
#endif
