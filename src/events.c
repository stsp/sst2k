#include "sst.h"
#include <math.h>

void unschedule(int evtype)
/* remove an event from the schedule */
{
    game.future[evtype] = FOREVER;
}

int is_scheduled(int evtype)
/* is an event of specified type scheduled */
{
    return game.future[evtype] != FOREVER;
}

extern double scheduled(int evtype)
/* when will this event happen? */
{
    return game.future[evtype];
}

void schedule(int evtype, double offset)
/* schedule an event of specified type */
{
    game.future[evtype] = game.state.date + offset;
}

void postpone(int evtype, double offset)
/* poistpone a scheduled event */
{
    game.future[evtype] += offset;
}

void events(void) 
{
    int ictbeam=0, ipage=0, istract=0, line, i=0, j, k, l, ixhold=0, iyhold=0;
    double fintim = game.state.date + game.optime, datemin, xtime, repair, yank=0;
    int radio_was_broken;
    struct quadrant *pdest;

#ifdef DEBUG
    if (game.idebug) prout("EVENTS");
#endif

    radio_was_broken = (game.damage[DRADIO] != 0.0);

    for (;;) {
	/* Select earliest extraneous event, line==0 if no events */
	line = FSPY;
	if (game.alldone) return;
	datemin = fintim;
	for (l = 1; l < NEVENTS; l++)
	    if (game.future[l] < datemin) {
		line = l;
		datemin = game.future[l];
	    }
	xtime = datemin-game.state.date;
	game.state.date = datemin;
	/* Decrement Federation resources and recompute remaining time */
	game.state.remres -= (game.state.remkl+4*game.state.remcom)*xtime;
	game.state.remtime = game.state.remres/(game.state.remkl+4*game.state.remcom);
	if (game.state.remtime <=0) {
	    finish(FDEPLETE);
	    return;
	}
	/* Is life support adequate? */
	if (game.damage[DLIFSUP] && game.condit != IHDOCKED) {
	    if (game.lsupres < xtime && game.damage[DLIFSUP] > game.lsupres) {
		finish(FLIFESUP);
		return;
	    }
	    game.lsupres -= xtime;
	    if (game.damage[DLIFSUP] <= xtime) game.lsupres = game.inlsr;
	}
	/* Fix devices */
	repair = xtime;
	if (game.condit == IHDOCKED) repair /= game.docfac;
	/* Don't fix Deathray here */
	for (l=0; l<NDEVICES; l++)
	    if (game.damage[l] > 0.0 && l != DDRAY)
		game.damage[l] -= (game.damage[l]-repair > 0.0 ? repair : game.damage[l]);
	/* If radio repaired, update star chart and attack reports */
	if (radio_was_broken && game.damage[DRADIO] == 0.0) {
	    prout(_("Lt. Uhura- \"Captain, the sub-space radio is working and"));
	    prout(_("   surveillance reports are coming in."));
	    skip(1);
	    if (game.iseenit==0) {
		attakreport(0);
		game.iseenit = 1;
	    }
	    rechart();
	    prout(_("   The star chart is now up to date.\""));
	    skip(1);
	}
	/* Cause extraneous event LINE to occur */
	game.optime -= xtime;
	switch (line) {
	case FSNOVA: /* Supernova */
	    if (ipage==0) pause_game(1);
	    ipage=1;
	    snova(0,0);
	    schedule(FSNOVA, expran(0.5*game.intime));
	    if (game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova) return;
	    break;
	case FSPY: /* Check with spy to see if S.C. should tractor beam */
	    if (game.state.nscrem == 0 ||
		ictbeam+istract > 0 ||
		game.condit==IHDOCKED || game.isatb==1 || game.iscate==1) return;
	    if (game.ientesc ||
		(game.energy < 2000 && game.torps < 4 && game.shield < 1250) ||
		(game.damage[DPHASER]>0 && (game.damage[DPHOTON]>0 || game.torps < 4)) ||
		(game.damage[DSHIELD] > 0 &&
		 (game.energy < 2500 || game.damage[DPHASER] > 0) &&
		 (game.torps < 5 || game.damage[DPHOTON] > 0))) {
		/* Tractor-beam her! */
		istract=1;
		yank = square(game.state.kscmdr.x-game.quadrant.x) + square(game.state.kscmdr.y-game.quadrant.y);
		/********* fall through to FTBEAM code ***********/
	    }
	    else return;
	case FTBEAM: /* Tractor beam */
	    if (line==FTBEAM) {
		if (game.state.remcom == 0) {
		    unschedule(FTBEAM);
		    break;
		}
		i = Rand()*game.state.remcom+1.0;
		yank = square(game.state.kcmdr[i].x-game.quadrant.x) + square(game.state.kcmdr[i].y-game.quadrant.y);
		if (istract || game.condit == IHDOCKED || yank == 0) {
		    /* Drats! Have to reschedule */
		    schedule(FTBEAM, 
			     game.optime + expran(1.5*game.intime/game.state.remcom));
		    break;
		}
	    }
	    /* tractor beaming cases merge here */
	    yank = sqrt(yank);
	    if (ipage==0) pause_game(1);
	    ipage=1;
	    game.optime = (10.0/(7.5*7.5))*yank; /* 7.5 is yank rate (warp 7.5) */
	    ictbeam = 1;
	    skip(1);
	    proutn("***");
	    crmshp();
	    prout(_(" caught in long range tractor beam--"));
	    /* If Kirk & Co. screwing around on planet, handle */
	    atover(1); /* atover(1) is Grab */
	    if (game.alldone) return;
	    if (game.icraft == 1) { /* Caught in Galileo? */
		finish(FSTRACTOR);
		return;
	    }
	    /* Check to see if shuttle is aboard */
	    if (game.iscraft==0) {
		skip(1);
		if (Rand() > 0.5) {
		    prout(_("Galileo, left on the planet surface, is captured"));
		    prout(_("by aliens and made into a flying McDonald's."));
		    game.damage[DSHUTTL] = -10;
		    game.iscraft = -1;
		}
		else {
		    prout(_("Galileo, left on the planet surface, is well hidden."));
		}
	    }
	    if (line==0) {
		game.quadrant.x = game.state.kscmdr.x;
		game.quadrant.y = game.state.kscmdr.y;
	    }
	    else {
		game.quadrant.x = game.state.kcmdr[i].x;
		game.quadrant.y = game.state.kcmdr[i].y;
	    }
	    iran(QUADSIZE, &game.sector.x, &game.sector.y);
	    crmshp();
	    proutn(_(" is pulled to "));
	    proutn(cramlc(quadrant, game.quadrant));
	    proutn(", ");
	    prout(cramlc(sector, game.sector));
	    if (game.resting) {
		prout(_("(Remainder of rest/repair period cancelled.)"));
		game.resting = 0;
	    }
	    if (game.shldup==0) {
		if (game.damage[DSHIELD]==0 && game.shield > 0) {
		    doshield(2); /* Shldsup */
		    game.shldchg=0;
		}
		else prout(_("(Shields not currently useable.)"));
	    }
	    newqad(0);
	    /* Adjust finish time to time of tractor beaming */
	    fintim = game.state.date+game.optime;
	    attack(0);
	    if (game.state.remcom <= 0) unschedule(FTBEAM);
	    else schedule(FTBEAM, game.optime+expran(1.5*game.intime/game.state.remcom));
	    break;
	case FSNAP: /* Snapshot of the universe (for time warp) */
	    game.snapsht = game.state;
	    game.state.snap = 1;
	    schedule(FSNAP, expran(0.5 * game.intime));
	    break;
	case FBATTAK: /* Commander attacks starbase */
	    if (game.state.remcom==0 || game.state.rembase==0) {
		/* no can do */
		unschedule(FBATTAK);
		unschedule(FCDBAS);
		break;
	    }
	    i = 0;
	    for_starbases(j) {
		for_commanders(k)
		    if (game.state.baseq[j].x==game.state.kcmdr[k].x && game.state.baseq[j].y==game.state.kcmdr[k].y &&
			(game.state.baseq[j].x!=game.quadrant.x || game.state.baseq[j].y!=game.quadrant.y) &&
			(game.state.baseq[j].x!=game.state.kscmdr.x || game.state.baseq[j].y!=game.state.kscmdr.y)) {
			i = 1;
			break;
		    }
		if (i == 1) break;
	    }
	    if (j>game.state.rembase) {
		/* no match found -- try later */
		schedule(FBATTAK, expran(0.3*game.intime));
		unschedule(FCDBAS);
		break;
	    }
	    /* commander + starbase combination found -- launch attack */
	    game.battle.x = game.state.baseq[j].x;
	    game.battle.y = game.state.baseq[j].y;
	    schedule(FCDBAS, 1.0+3.0*Rand());
	    if (game.isatb) /* extra time if SC already attacking */
		postpone(FCDBAS, scheduled(FSCDBAS)-game.state.date);
	    game.future[FBATTAK] = game.future[FCDBAS] +expran(0.3*game.intime);
	    game.iseenit = 0;
	    if (game.damage[DRADIO] != 0.0 &&
		game.condit != IHDOCKED) break; /* No warning :-( */
	    game.iseenit = 1;
	    if (ipage==0) pause_game(1);
	    ipage = 1;
	    skip(1);
	    proutn(_("Lt. Uhura-  \"Captain, the starbase in "));
	    prout(cramlc(quadrant, game.battle));
	    prout(_("   reports that it is under attack and that it can"));
	    proutn(_("   hold out only until stardate %d"),
		   (int)scheduled(FCDBAS));
	    prout(".\"");
	    if (game.resting) {
		skip(1);
		proutn(_("Mr. Spock-  \"Captain, shall we cancel the rest period?\""));
		if (ja()) {
		    game.resting = 0;
		    game.optime = 0.0;
		    return;
		}
	    }
	    break;
	case FSCDBAS: /* Supercommander destroys base */
	    unschedule(FSCDBAS);
	    game.isatb = 2;
	    if (!game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].starbase) 
		break; /* WAS RETURN! */
	    ixhold = game.battle.x;
	    iyhold = game.battle.y;
	    game.battle.x = game.state.kscmdr.x;
	    game.battle.y = game.state.kscmdr.y;
	case FCDBAS: /* Commander succeeds in destroying base */
	    if (line==FCDBAS) {
		unschedule(FCDBAS);
		/* find the lucky pair */
		for_commanders(i)
		    if (game.state.kcmdr[i].x==game.battle.x && game.state.kcmdr[i].y==game.battle.y) 
			break;
		if (i > game.state.remcom || game.state.rembase == 0 ||
		    !game.state.galaxy[game.battle.x][game.battle.y].starbase) {
		    /* No action to take after all */
		    game.battle.x = game.battle.y = 0;
		    break;
		}
	    }
	    /* Code merges here for any commander destroying base */
	    /* Not perfect, but will have to do */
	    /* Handle case where base is in same quadrant as starship */
	    if (game.battle.x==game.quadrant.x && game.battle.y==game.quadrant.y) {
		game.state.chart[game.battle.x][game.battle.y].starbase = false;
		game.quad[game.base.x][game.base.y] = IHDOT;
		game.base.x=game.base.y=0;
		newcnd();
		skip(1);
		prout(_("Spock-  \"Captain, I believe the starbase has been destroyed.\""));
	    }
	    else if (game.state.rembase != 1 &&
		     (game.damage[DRADIO] <= 0.0 || game.condit == IHDOCKED)) {
		/* Get word via subspace radio */
		if (ipage==0) pause_game(1);
		ipage = 1;
		skip(1);
		prout(_("Lt. Uhura-  \"Captain, Starfleet Command reports that"));
		proutn(_("   the starbase in "));
		proutn(cramlc(quadrant, game.battle));
		prout(_(" has been destroyed by"));
		if (game.isatb==2) prout(_("the Klingon Super-Commander"));
		else prout(_("a Klingon Commander"));
		game.state.chart[game.battle.x][game.battle.y].starbase = false;
	    }
	    /* Remove Starbase from galaxy */
	    game.state.galaxy[game.battle.x][game.battle.y].starbase = false;
	    for_starbases(i)
		if (game.state.baseq[i].x==game.battle.x && game.state.baseq[i].y==game.battle.y) {
		    game.state.baseq[i].x=game.state.baseq[game.state.rembase].x;
		    game.state.baseq[i].y=game.state.baseq[game.state.rembase].y;
		}
	    game.state.rembase--;
	    if (game.isatb == 2) {
		/* reinstate a commander's base attack */
		game.battle.x = ixhold;
		game.battle.y = iyhold;
		game.isatb = 0;
	    }
	    else {
		game.battle.x = game.battle.y = 0;
	    }
	    break;
	case FSCMOVE: /* Supercommander moves */
	    schedule(FSCMOVE, 0.2777);
	    if (game.ientesc+istract==0 &&
		game.isatb!=1 &&
		(game.iscate!=1 || game.justin==1)) scom(&ipage);
	    break;
	case FDSPROB: /* Move deep space probe */
	    schedule(FDSPROB, 0.01);
	    game.probex += game.probeinx;
	    game.probey += game.probeiny;
	    i = (int)(game.probex/QUADSIZE +0.05);
	    j = (int)(game.probey/QUADSIZE + 0.05);
	    if (game.probec.x != i || game.probec.y != j) {
		game.probec.x = i;
		game.probec.y = j;
		if (!VALID_QUADRANT(i, j) ||
		    game.state.galaxy[game.probec.x][game.probec.y].supernova) {
		    // Left galaxy or ran into supernova
		    if (game.damage[DRADIO]==0.0 || game.condit == IHDOCKED) {
			if (ipage==0) pause_game(1);
			ipage = 1;
			skip(1);
			proutn(_("Lt. Uhura-  \"The deep space probe "));
			if (!VALID_QUADRANT(j, i))
			    proutn(_("has left the galaxy"));
			else
			    proutn(_("is no longer transmitting"));
			prout(".\"");
		    }
		    unschedule(FDSPROB);
		    break;
		}
		if (game.damage[DRADIO]==0.0   || game.condit == IHDOCKED) {
		    if (ipage==0) pause_game(1);
		    ipage = 1;
		    skip(1);
		    proutn(_("Lt. Uhura-  \"The deep space probe is now in "));
		    proutn(cramlc(quadrant, game.probec));
		    prout(".\"");
		}
	    }
	    pdest = &game.state.galaxy[game.probec.x][game.probec.y];
	    /* Update star chart if Radio is working or have access to
	       radio. */
	    if (game.damage[DRADIO] == 0.0 || game.condit == IHDOCKED) {
		struct page *chp = &game.state.chart[game.probec.x][game.probec.y];

		chp->klingons = pdest->klingons;
		chp->starbase = pdest->starbase;
		chp->stars = pdest->stars;
		pdest->charted = true;
	    }
	    game.proben--; // One less to travel
	    if (game.proben == 0 && game.isarmed && pdest->stars) {
		/* lets blow the sucker! */
		snova(1,0);
		unschedule(FDSPROB);
		if (game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova) 
		    return;
	    }
	    break;
#ifdef EXPERIMENTAL
	case FDISTR: /* inhabited system issues distress call */
	    /* in BSD Trek this is a straight 1 stardate ahead */ 
	    schedule(FDISTR, 1.0 + Rand());
	    /* if we already have too many, throw this one away */
	    if (game.ndistr >= MAXDISTR)
		break;
	    /* try a whole bunch of times to find something suitable */
	    for (i = 0; i < 100; i++) {
		struct quadrant *q;
		iran(GALSIZE, &ix, &iy);
		q = &game.state.galaxy[game.quadrant.x][game.quadrant.y];
		/* need a quadrant which is not the current one,
		   which has some stars which are inhabited and
		   not already under attack, which is not
		   supernova'ed, and which has some Klingons in it */
		if (!((ix == game.quadrant.x && iy == game.quadrant.y) || q->stars<=0 ||
		      (q->qsystemname & Q_DISTRESSED) ||
		      (q->qsystemname & Q_SYSTEM) == 0 || q->klings <= 0))
		    break;
	    }
	    if (i >= 100)
		/* can't seem to find one; ignore this call */
		break;

	    /* got one!!  Schedule its enslavement */
	    game.ndistr++;
	    e = xsched(E_ENSLV, 1, ix, iy, q->qsystemname);
	    q->qsystemname = (e - Event) | Q_DISTRESSED;

	    /* tell the captain about it if we can */
	    if (game.damage[DRADIO] == 0.0)
	    {
		printf("\nUhura: Captain, starsystem %s in quadrant %d,%d is under attack\n",
		       Systemname[e->systemname], ix, iy);
		restcancel++;
	    }
	    else
		/* if we can't tell him, make it invisible */
		e->evcode |= E_HIDDEN;
	    break;
      case FENSLV:		/* starsystem is enslaved */
	    unschedule(e);
	    /* see if current distress call still active */
	    q = &Quad[e->x][e->y];
	    if (q->klings <= 0)
	    {
		/* no Klingons, clean up */
		/* restore the system name */
		q->qsystemname = e->systemname;
		break;
	    }

	    /* play stork and schedule the first baby */
	    e = schedule(E_REPRO, Param.eventdly[E_REPRO] * franf(), e->x, e->y, e->systemname);

	    /* report the disaster if we can */
	    if (game.damage[DRADIO] == 0.0)
	    {
		printf("\nUhura:  We've lost contact with starsystem %s\n",
		       Systemname[e->systemname]);
		printf("  in quadrant %d,%d.\n", e->x, e->y);
	    }
	    else
		e->evcode |= E_HIDDEN;
	    break;
      case FREPRO:		/* Klingon reproduces */
	    /* see if distress call is still active */
	    q = &Quad[e->x][e->y];
	    if (q->klings <= 0)
	    {
		unschedule(e);
		q->qsystemname = e->systemname;
		break;
	    }
	    xresched(e, E_REPRO, 1);
	    /* reproduce one Klingon */
	    ix = e->x;
	    iy = e->y;
	    if (Now.klings == 127)
		break;		/* full right now */
	    if (q->klings >= MAXKLQUAD)
	    {
		/* this quadrant not ok, pick an adjacent one */
		for (i = ix - 1; i <= ix + 1; i++)
		{
		    if (!VALID_QUADRANT(i))
			continue;
		    for (j = iy - 1; j <= iy + 1; j++)
		    {
			if (!VALID_QUADRANT(j))
			    continue;
			q = &Quad[i][j];
			/* check for this quad ok (not full & no snova) */
			if (q->klings >= MAXKLQUAD || q->stars < 0)
			    continue;
			break;
		    }
		    if (j <= iy + 1)
			break;
		}
		if (j > iy + 1)
		    /* cannot create another yet */
		    break;
		ix = i;
		iy = j;
	    }
	    /* deliver the child */
	    game.remkl++;
	    if (ix == game.quadrant.x && iy == game.quadrant.y)
		newkling(++game.klhere, &ixhold, &iyhold);

	    /* recompute time left */
	    game.state.remtime = game.state.remres/(game.state.remkl+4*game.state.remcom);
	    break;
#endif /* EXPERIMENTAL */
	}
    }
}

				
void wait(void) 
{
    int key;
    double temp, delay, origTime;

    game.ididit = 0;
    for (;;) {
	key = scan();
	if (key  != IHEOL) break;
	proutn(_("How long? "));
    }
    chew();
    if (key != IHREAL) {
	huh();
	return;
    }
    origTime = delay = aaitem;
    if (delay <= 0.0) return;
    if (delay >= game.state.remtime || game.nenhere != 0) {
	proutn(_("Are you sure? "));
	if (ja() == 0) return;
    }

    /* Alternate resting periods (events) with attacks */

    game.resting = 1;
    do {
	if (delay <= 0) game.resting = 0;
	if (game.resting == 0) {
	    prout(_("%d stardates left."), (int)game.state.remtime);
	    return;
	}
	temp = game.optime = delay;

	if (game.nenhere) {
	    double rtime = 1.0 + Rand();
	    if (rtime < temp) temp = rtime;
	    game.optime = temp;
	}
	if (game.optime < delay) attack(0);
	if (game.alldone) return;
	events();
	game.ididit = 1;
	if (game.alldone) return;
	delay -= temp;
	/* Repair Deathray if long rest at starbase */
	if (origTime-delay >= 9.99 && game.condit == IHDOCKED)
	    game.damage[DDRAY] = 0.0;
    } while 
	// leave if quadrant supernovas
	(!game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova);

    game.resting = 0;
    game.optime = 0;
}

void nova(int ix, int iy) 
{
    static double course[] =
	{0.0, 10.5, 12.0, 1.5, 9.0, 0.0, 3.0, 7.5, 6.0, 4.5};
    int bot, top, top2, hits[QUADSIZE+1][3], kount, icx, icy, mm, nn, j;
    int iquad, iquad1, i, ll;
    coord newc, nov, scratch;

    nov.x = ix; nov.y = iy;
    if (Rand() < 0.05) {
	/* Wow! We've supernova'ed */
	snova(ix, iy);
	return;
    }

    /* handle initial nova */
    game.quad[ix][iy] = IHDOT;
    crmena(1, IHSTAR, 2, nov);
    prout(_(" novas."));
    game.state.galaxy[game.quadrant.x][game.quadrant.y].stars--;
    game.state.starkl++;
	
    /* Set up stack to recursively trigger adjacent stars */
    bot = top = top2 = 1;
    kount = 0;
    icx = icy = 0;
    hits[1][1] = ix;
    hits[1][2] = iy;
    while (1) {
	for (mm = bot; mm <= top; mm++) 
	    for (nn = 1; nn <= 3; nn++)  /* nn,j represents coordinates around current */
		for (j = 1; j <= 3; j++) {
		    if (j==2 && nn== 2) continue;
		    scratch.x = hits[mm][1]+nn-2;
		    scratch.y = hits[mm][2]+j-2;
		    if (!VALID_SECTOR(scratch.y, scratch.x)) continue;
		    iquad = game.quad[scratch.x][scratch.y];
		    switch (iquad) {
		    // case IHDOT:	/* Empty space ends reaction
		    // case IHQUEST:
		    // case IHBLANK:
		    // case IHT:
		    // case IHWEB:
		    default:
			break;
		    case IHSTAR: /* Affect another star */
			if (Rand() < 0.05) {
			    /* This star supernovas */
			    snova(scratch.x,scratch.y);
			    return;
			}
			top2++;
			hits[top2][1]=scratch.x;
			hits[top2][2]=scratch.y;
			game.state.galaxy[game.quadrant.x][game.quadrant.y].stars -= 1;
			game.state.starkl++;
			crmena(1, IHSTAR, 2, scratch);
			prout(_(" novas."));
			game.quad[scratch.x][scratch.y] = IHDOT;
			break;
		    case IHP: /* Destroy planet */
			game.state.galaxy[game.quadrant.x][game.quadrant.y].planet = NULL;
			game.state.nplankl++;
			crmena(1, IHP, 2, scratch);
			prout(_(" destroyed."));
			DESTROY(&game.state.plnets[game.iplnet]);
			game.iplnet = game.plnet.x = game.plnet.y = 0;
			if (game.landed == 1) {
			    finish(FPNOVA);
			    return;
			}
			game.quad[scratch.x][scratch.y] = IHDOT;
			break;
		    case IHB: /* Destroy base */
			game.state.galaxy[game.quadrant.x][game.quadrant.y].starbase = false;
			for_starbases(i)
			    if (game.state.baseq[i].x==game.quadrant.x && game.state.baseq[i].y==game.quadrant.y) 
				break;
			game.state.baseq[i] = game.state.baseq[game.state.rembase];
			game.state.rembase--;
			game.base.x = game.base.y = 0;
			game.state.basekl++;
			newcnd();
			crmena(1, IHB, 2, scratch);
			prout(_(" destroyed."));
			game.quad[scratch.x][scratch.y] = IHDOT;
			break;
		    case IHE: /* Buffet ship */
		    case IHF:
			prout(_("***Starship buffeted by nova."));
			if (game.shldup) {
			    if (game.shield >= 2000.0) game.shield -= 2000.0;
			    else {
				double diff = 2000.0 - game.shield;
				game.energy -= diff;
				game.shield = 0.0;
				game.shldup = 0;
				prout(_("***Shields knocked out."));
				game.damage[DSHIELD] += 0.005*game.damfac*Rand()*diff;
			    }
			}
			else game.energy -= 2000.0;
			if (game.energy <= 0) {
			    finish(FNOVA);
			    return;
			}
			/* add in course nova contributes to kicking starship*/
			icx += game.sector.x-hits[mm][1];
			icy += game.sector.y-hits[mm][2];
			kount++;
			break;
		    case IHK: /* kill klingon */
			deadkl(scratch,iquad, scratch.x, scratch.y);
			break;
		    case IHC: /* Damage/destroy big enemies */
		    case IHS:
		    case IHR:
			for_local_enemies(ll)
			    if (game.ks[ll].x==scratch.x && game.ks[ll].y==scratch.y) break;
			game.kpower[ll] -= 800.0; /* If firepower is lost, die */
			if (game.kpower[ll] <= 0.0) {
			    deadkl(scratch, iquad, scratch.x, scratch.y);
			    break;
			}
			newc.x = scratch.x + scratch.x - hits[mm][1];
			newc.y = scratch.y + scratch.y - hits[mm][2];
			crmena(1, iquad, 2, scratch);
			proutn(_(" damaged"));
			if (!VALID_SECTOR(newc.x, newc.y)) {
			    /* can't leave quadrant */
			    skip(1);
			    break;
			}
			iquad1 = game.quad[newc.x][newc.y];
			if (iquad1 == IHBLANK) {
			    proutn(_(", blasted into "));
			    crmena(0, IHBLANK, 2, newc);
			    skip(1);
			    deadkl(scratch, iquad, newc.x, newc.y);
			    break;
			}
			if (iquad1 != IHDOT) {
			    /* can't move into something else */
			    skip(1);
			    break;
			}
			proutn(_(", buffeted to "));
			proutn(cramlc(sector, newc));
			game.quad[scratch.x][scratch.y] = IHDOT;
			game.quad[newc.x][newc.y] = iquad;
			game.ks[ll].x = newc.x;
			game.ks[ll].y = newc.y;
			game.kavgd[ll] = sqrt(square(game.sector.x-newc.x)+square(game.sector.y-newc.y));
			game.kdist[ll] = game.kavgd[ll];
			skip(1);
			break;
		    }
		}
	if (top == top2) 
	    break;
	bot = top + 1;
	top = top2;
    }
    if (kount==0) 
	return;

    /* Starship affected by nova -- kick it away. */
    game.dist = kount*0.1;
    if (icx) icx = (icx < 0 ? -1 : 1);
    if (icy) icy = (icy < 0 ? -1 : 1);
    game.direc = course[3*(icx+1)+icy+2];
    if (game.direc == 0.0) game.dist = 0.0;
    if (game.dist == 0.0) return;
    game.optime = 10.0*game.dist/16.0;
    skip(1);
    prout(_("Force of nova displaces starship."));
    game.iattak=2;	/* Eliminates recursion problem */
    imove();
    game.optime = 10.0*game.dist/16.0;
    return;
}
	
	
void snova(int insx, int insy) 
{
    int comdead, nsx, nsy, num=0, kldead, iscdead;
    int nrmdead, npdead;
    int incipient=0;
    coord nq;

    nq.x = nq.y = 0;
    nsx = insy;
    nsy = insy;

    if (insy== 0) {
	if (insx == 1)
	    /* NOVAMAX being used */
	    nq = game.probec;
	else {
	    int stars = 0;
	    /* Scheduled supernova -- select star */
	    /* logic changed here so that we won't favor quadrants in top
	       left of universe */
	    for_quadrants(nq.x) {
		for_quadrants(nq.y) {
		    stars += game.state.galaxy[nq.x][nq.y].stars;
		}
	    }
	    if (stars == 0) return; /* nothing to supernova exists */
	    num = Rand()*stars + 1;
	    for_quadrants(nq.x) {
		for_quadrants(nq.y) {
		    num -= game.state.galaxy[nq.x][nq.y].stars;
		    if (num <= 0) break;
		}
		if (num <=0) break;
	    }
#ifdef DEBUG
	    if (game.idebug) {
		proutn("Super nova here?");
		if (ja()==1) {
		    nq.x = game.quadrant.x;
		    nq.y = game.quadrant.y;
		}
	    }
#endif
	}

	if (nq.x != game.quadrant.y || nq.y != game.quadrant.y || game.justin != 0) {
	    /* it isn't here, or we just entered (treat as inroute) */
	    if (game.damage[DRADIO] == 0.0 || game.condit == IHDOCKED) {
		skip(1);
		prout(_("Message from Starfleet Command       Stardate %.2f"), game.state.date);
		prout(_("     Supernova in %s; caution advised."),
		      cramlc(quadrant, nq));
	    }
	}
	else {
	    /* we are in the quadrant! */
	    incipient = 1;
	    num = Rand()* game.state.galaxy[nq.x][nq.y].stars + 1;
	    for_sectors(nsx) {
		for_sectors(nsy) {
		    if (game.quad[nsx][nsy]==IHSTAR) {
			num--;
			if (num==0) break;
		    }
		}
		if (num==0) break;
	    }
	}
    }
    else {
	incipient = 1;
    }

    if (incipient) {
	coord nd;
	skip(1);
	prouts(_("***RED ALERT!  RED ALERT!"));
	skip(1);
	nd.x = nsx; nd.y = nsy;
	prout(_("***Incipient supernova detected at "), cramlc(sector, nd));
	nq = game.quadrant;
	if (square(nsx-game.sector.x) + square(nsy-game.sector.y) <= 2.1) {
	    proutn(_("Emergency override attempts t"));
	    prouts("***************");
	    skip(1);
	    stars();
	    game.alldone=1;
	}
    }
    /* destroy any Klingons in supernovaed quadrant */
    kldead = game.state.galaxy[nq.x][nq.y].klingons;
    game.state.galaxy[nq.x][nq.y].klingons = 0;
    comdead = iscdead = 0;
    if (same(nq, game.state.kscmdr)) {
	/* did in the Supercommander! */
	game.state.nscrem = game.state.kscmdr.x = game.state.kscmdr.y = game.isatb = game.iscate = 0;
	iscdead = 1;
	unschedule(FSCMOVE);
	unschedule(FSCDBAS);
    }
    if (game.state.remcom) {
	int maxloop = game.state.remcom, l;
	for (l = 1; l <= maxloop; l++) {
	    if (same(game.state.kcmdr[l], nq)) {
		game.state.kcmdr[l] = game.state.kcmdr[game.state.remcom];
		game.state.kcmdr[game.state.remcom].x = game.state.kcmdr[game.state.remcom].y = 0;
		game.state.remcom--;
		kldead--;
		comdead++;
		if (game.state.remcom==0) unschedule(FTBEAM);
		break;
	    }
	}
    }
    game.state.remkl -= kldead;
    /* destroy Romulans and planets in supernovaed quadrant */
    nrmdead = game.state.galaxy[nq.x][nq.y].romulans;
    game.state.galaxy[nq.x][nq.y].romulans = 0;
    game.state.nromrem -= nrmdead;
    npdead = num - nrmdead*10;
    if (npdead) {
	int l;
	for (l = 0; l < game.inplan; l++)
	    if (same(game.state.plnets[l].w, nq)) {
		DESTROY(&game.state.plnets[l]);
	    }
    }
    /* Destroy any base in supernovaed quadrant */
    if (game.state.rembase) {
	int maxloop = game.state.rembase, l;
	for (l = 1; l <= maxloop; l++)
	    if (same(game.state.baseq[l], nq)) {
		game.state.baseq[l] = game.state.baseq[game.state.rembase];
		game.state.baseq[game.state.rembase].x = game.state.baseq[game.state.rembase].y = 0;
		game.state.rembase--;
		break;
	    }
    }
    /* If starship caused supernova, tally up destruction */
    if (insx) {
	game.state.starkl += game.state.galaxy[nq.x][nq.y].stars;
	game.state.basekl += game.state.galaxy[nq.x][nq.y].starbase;
	game.state.nplankl += npdead;
    }
    /* mark supernova in galaxy and in star chart */
    if ((game.quadrant.x == nq.x && game.quadrant.y == nq.y) ||
	game.damage[DRADIO] == 0 ||
	game.condit == IHDOCKED)
	game.state.galaxy[nq.x][nq.y].supernova = true;
    /* If supernova destroys last klingons give special message */
    if (KLINGREM==0 && (nq.x != game.quadrant.x || nq.y != game.quadrant.y)) {
	skip(2);
	if (insx == 0) prout(_("Lucky you!"));
	proutn(_("A supernova in %s has just destroyed the last Klingons."),
	       cramlc(quadrant, nq));
	finish(FWON);
	return;
    }
    /* if some Klingons remain, continue or die in supernova */
    if (game.alldone) finish(FSNOVAED);
    return;
}
		
				
