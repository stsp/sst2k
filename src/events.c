/*
 * events.c -- event-queue handling
 *
 * This isn't a real event queue a la BSD Trek yet -- you can only have one 
 * event of each type active at any given time.  Mostly these means we can 
 * only have one FDISTR/FENSLV/FREPRO sequence going at any given time;
 * BSD Trek, from which we swiped the idea, can have up to 5.
 */
#include "sst.h"
#include <math.h>

event *unschedule(int evtype)
/* remove an event from the schedule */
{
    game.future[evtype].date = FOREVER;
    return &game.future[evtype];
}

int is_scheduled(int evtype)
/* is an event of specified type scheduled */
{
    return game.future[evtype].date != FOREVER;
}

extern double scheduled(int evtype)
/* when will this event happen? */
{
    return game.future[evtype].date;
}

event *schedule(int evtype, double offset)
/* schedule an event of specified type */
{
    game.future[evtype].date = game.state.date + offset;
    return &game.future[evtype];
}

void postpone(int evtype, double offset)
/* poistpone a scheduled event */
{
    game.future[evtype].date += offset;
}

static bool cancelrest(void)
/* rest period is interrupted by event */
{
    if (game.resting) {
	skip(1);
	proutn(_("Mr. Spock-  \"Captain, shall we cancel the rest period?\""));
	if (ja() == true) {
	    game.resting = false;
	    game.optime = 0.0;
	    return true;
	}
    }

    return false;
}

void events(void) 
/* run through the event queue looking for things to do */
{
    int evcode, i=0, j, k, l;
    double fintim = game.state.date + game.optime, datemin, xtime, repair, yank=0;
    bool radio_was_broken, ictbeam = false, ipage = false, istract = false;
    struct quadrant *pdest, *q;
    coord w, hold;
    event *ev, *ev2;

    if (idebug) {
	prout("=== EVENTS from %.2f to %.2f:", game.state.date, fintim);
	for (i = 1; i < NEVENTS; i++) {
	    switch (i) {
	    case FSNOVA:  proutn("=== Supernova       "); break;
	    case FTBEAM:  proutn("=== T Beam          "); break;
	    case FSNAP:   proutn("=== Snapshot        "); break;
	    case FBATTAK: proutn("=== Base Attack     "); break;
	    case FCDBAS:  proutn("=== Base Destroy    "); break;
	    case FSCMOVE: proutn("=== SC Move         "); break;
	    case FSCDBAS: proutn("=== SC Base Destroy "); break;
	    case FDSPROB: proutn("=== Probe Move      "); break;
	    case FDISTR:  proutn("=== Distress Call   "); break;
	    case FENSLV:  proutn("=== Enlavement      "); break;
	    case FREPRO:  proutn("=== Klingon Build   "); break;
	    }
	    if (is_scheduled(i))
		prout("%.2f", scheduled(i));
	    else
		prout("never");

	}
    }

    radio_was_broken = damaged(DRADIO);

    hold.x = hold.y = 0;
    for (;;) {
	/* Select earliest extraneous event, evcode==0 if no events */
	evcode = FSPY;
	if (game.alldone) return;
	datemin = fintim;
	for (l = 1; l < NEVENTS; l++)
	    if (game.future[l].date < datemin) {
		evcode = l;
		if (idebug)
		    prout("== Event %d fires", evcode);
		datemin = game.future[l].date;
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
	/* Any crew left alive? */
	if (game.state.crew <=0) {
	    finish(FCREW);
	    return;
	}
	/* Is life support adequate? */
	if (damaged(DLIFSUP) && game.condition != docked) {
	    if (game.lsupres < xtime && game.damage[DLIFSUP] > game.lsupres) {
		finish(FLIFESUP);
		return;
	    }
	    game.lsupres -= xtime;
	    if (game.damage[DLIFSUP] <= xtime) game.lsupres = game.inlsr;
	}
	/* Fix devices */
	repair = xtime;
	if (game.condition == docked) repair /= game.docfac;
	/* Don't fix Deathray here */
	for (l=0; l<NDEVICES; l++)
	    if (game.damage[l] > 0.0 && l != DDRAY)
		game.damage[l] -= (game.damage[l]-repair > 0.0 ? repair : game.damage[l]);
	/* If radio repaired, update star chart and attack reports */
	if (radio_was_broken && !damaged(DRADIO)) {
	    prout(_("Lt. Uhura- \"Captain, the sub-space radio is working and"));
	    prout(_("   surveillance reports are coming in."));
	    skip(1);
	    if (!game.iseenit) {
		attakreport(false);
		game.iseenit = true;
	    }
	    rechart();
	    prout(_("   The star chart is now up to date.\""));
	    skip(1);
	}
	/* Cause extraneous event EVCODE to occur */
	game.optime -= xtime;
	switch (evcode) {
	case FSNOVA: /* Supernova */
	    if (!ipage) pause_game(true);
	    ipage=true;
	    snova(false, NULL);
	    schedule(FSNOVA, expran(0.5*game.intime));
	    if (game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova) return;
	    break;
	case FSPY: /* Check with spy to see if S.C. should tractor beam */
	    if (game.state.nscrem == 0 ||
		ictbeam || istract ||
		game.condition==docked || game.isatb==1 || game.iscate) return;
	    if (game.ientesc ||
		(game.energy < 2000 && game.torps < 4 && game.shield < 1250) ||
		(damaged(DPHASER) && (damaged(DPHOTON) || game.torps < 4)) ||
		(damaged(DSHIELD) &&
		 (game.energy < 2500 || damaged(DPHASER)) &&
		 (game.torps < 5 || damaged(DPHOTON)))) {
		/* Tractor-beam her! */
		istract = true;
		yank = distance(game.state.kscmdr, game.quadrant);
		/********* fall through to FTBEAM code ***********/
	    }
	    else return;
	case FTBEAM: /* Tractor beam */
	    if (evcode==FTBEAM) {
		if (game.state.remcom == 0) {
		    unschedule(FTBEAM);
		    break;
		}
		i = Rand()*game.state.remcom+1.0;
		yank = square(game.state.kcmdr[i].x-game.quadrant.x) + square(game.state.kcmdr[i].y-game.quadrant.y);
		if (istract || game.condition == docked || yank == 0) {
		    /* Drats! Have to reschedule */
		    schedule(FTBEAM, 
			     game.optime + expran(1.5*game.intime/game.state.remcom));
		    break;
		}
	    }
	    /* tractor beaming cases merge here */
	    yank = sqrt(yank);
	    if (!ipage) pause_game(true);
	    ipage=true;
	    game.optime = (10.0/(7.5*7.5))*yank; /* 7.5 is yank rate (warp 7.5) */
	    ictbeam = true;
	    skip(1);
	    proutn("***");
	    crmshp();
	    prout(_(" caught in long range tractor beam--"));
	    /* If Kirk & Co. screwing around on planet, handle */
	    atover(true); /* atover(true) is Grab */
	    if (game.alldone) return;
	    if (game.icraft) { /* Caught in Galileo? */
		finish(FSTRACTOR);
		return;
	    }
	    /* Check to see if shuttle is aboard */
	    if (game.iscraft == offship) {
		skip(1);
		if (Rand() > 0.5) {
		    prout(_("Galileo, left on the planet surface, is captured"));
		    prout(_("by aliens and made into a flying McDonald's."));
		    game.damage[DSHUTTL] = -10;
		    game.iscraft = removed;
		}
		else {
		    prout(_("Galileo, left on the planet surface, is well hidden."));
		}
	    }
	    if (evcode==0)
		game.quadrant = game.state.kscmdr;
	    else
		game.quadrant = game.state.kcmdr[i];
	    game.sector = randplace(QUADSIZE);
	    crmshp();
	    proutn(_(" is pulled to "));
	    proutn(cramlc(quadrant, game.quadrant));
	    proutn(", ");
	    prout(cramlc(sector, game.sector));
	    if (game.resting) {
		prout(_("(Remainder of rest/repair period cancelled.)"));
		game.resting = false;
	    }
	    if (!game.shldup) {
		if (!damaged(DSHIELD) && game.shield > 0) {
		    doshield(true); /* raise shields */
		    game.shldchg=false;
		}
		else prout(_("(Shields not currently useable.)"));
	    }
	    newqad(false);
	    /* Adjust finish time to time of tractor beaming */
	    fintim = game.state.date+game.optime;
	    attack(0);
	    if (game.state.remcom <= 0) unschedule(FTBEAM);
	    else schedule(FTBEAM, game.optime+expran(1.5*game.intime/game.state.remcom));
	    break;
	case FSNAP: /* Snapshot of the universe (for time warp) */
	    game.snapsht = game.state;
	    game.state.snap = true;
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
		    if (same(game.state.baseq[j], game.state.kcmdr[k]) &&
			!same(game.state.baseq[j], game.quadrant) &&
			!same(game.state.baseq[j], game.state.kscmdr)) {
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
	    game.battle = game.state.baseq[j];
	    schedule(FCDBAS, 1.0+3.0*Rand());
	    if (game.isatb) /* extra time if SC already attacking */
		postpone(FCDBAS, scheduled(FSCDBAS)-game.state.date);
	    game.future[FBATTAK].date = game.future[FCDBAS].date + expran(0.3*game.intime);
	    game.iseenit = false;
	    if (!damaged(DRADIO) && game.condition != docked) 
		break; /* No warning :-( */
	    game.iseenit = true;
	    if (!ipage) pause_game(true);
	    ipage = true;
	    skip(1);
	    proutn(_("Lt. Uhura-  \"Captain, the starbase in "));
	    prout(cramlc(quadrant, game.battle));
	    prout(_("   reports that it is under attack and that it can"));
	    proutn(_("   hold out only until stardate %d"),
		   (int)scheduled(FCDBAS));
	    prout(".\"");
	    if (cancelrest())
		return;
	    break;
	case FSCDBAS: /* Supercommander destroys base */
	    unschedule(FSCDBAS);
	    game.isatb = 2;
	    if (!game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].starbase) 
		break; /* WAS RETURN! */
	    hold = game.battle;
	    game.battle = game.state.kscmdr;
	    /* FALL THROUGH */
	case FCDBAS: /* Commander succeeds in destroying base */
	    if (evcode==FCDBAS) {
		unschedule(FCDBAS);
		/* find the lucky pair */
		for_commanders(i)
		    if (same(game.state.kcmdr[i], game.battle)) 
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
	    if (same(game.battle, game.quadrant)) {
		game.state.chart[game.battle.x][game.battle.y].starbase = false;
		game.quad[game.base.x][game.base.y] = IHDOT;
		game.base.x=game.base.y=0;
		newcnd();
		skip(1);
		prout(_("Spock-  \"Captain, I believe the starbase has been destroyed.\""));
	    }
	    else if (game.state.rembase != 1 &&
		     (!damaged(DRADIO) || game.condition == docked)) {
		/* Get word via subspace radio */
		if (!ipage) pause_game(true);
		ipage = true;
		skip(1);
		prout(_("Lt. Uhura-  \"Captain, Starfleet Command reports that"));
		proutn(_("   the starbase in "));
		proutn(cramlc(quadrant, game.battle));
		prout(_(" has been destroyed by"));
		if (game.isatb == 2) 
		    prout(_("the Klingon Super-Commander"));
		else prout(_("a Klingon Commander"));
		game.state.chart[game.battle.x][game.battle.y].starbase = false;
	    }
	    /* Remove Starbase from galaxy */
	    game.state.galaxy[game.battle.x][game.battle.y].starbase = false;
	    for_starbases(i)
		if (same(game.state.baseq[i], game.battle))
		    game.state.baseq[i] = game.state.baseq[game.state.rembase];
	    game.state.rembase--;
	    if (game.isatb == 2) {
		/* reinstate a commander's base attack */
		game.battle = hold;
		game.isatb = 0;
	    }
	    else {
		game.battle.x = game.battle.y = 0;
	    }
	    break;
	case FSCMOVE: /* Supercommander moves */
	    schedule(FSCMOVE, 0.2777);
	    if (!game.ientesc && !istract && game.isatb != 1 &&
			(!game.iscate || !game.justin)) 
		scom(&ipage);
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
		    if (!damaged(DRADIO) || game.condition == docked) {
			if (!ipage) pause_game(true);
			ipage = true;
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
		if (!damaged(DRADIO) || game.condition == docked) {
		    if (!ipage) pause_game(true);
		    ipage = true;
		    skip(1);
		    proutn(_("Lt. Uhura-  \"The deep space probe is now in "));
		    proutn(cramlc(quadrant, game.probec));
		    prout(".\"");
		}
	    }
	    pdest = &game.state.galaxy[game.probec.x][game.probec.y];
	    /* Update star chart if Radio is working or have access to
	       radio. */
	    if (!damaged(DRADIO) || game.condition == docked) {
		struct page *chp = &game.state.chart[game.probec.x][game.probec.y];

		chp->klingons = pdest->klingons;
		chp->starbase = pdest->starbase;
		chp->stars = pdest->stars;
		pdest->charted = true;
	    }
	    game.proben--; // One less to travel
	    if (game.proben == 0 && game.isarmed && pdest->stars) {
		/* lets blow the sucker! */
		snova(true, &game.probec);
		unschedule(FDSPROB);
		if (game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova) 
		    return;
	    }
	    break;
	case FDISTR: /* inhabited system issues distress call */
	    unschedule(FDISTR);
	    /* try a whole bunch of times to find something suitable */
	    i = 100;
	    do {
		/* need a quadrant which is not the current one,
		   which has some stars which are inhabited and
		   not already under attack, which is not
		   supernova'ed, and which has some Klingons in it */
		w = randplace(GALSIZE);
		q = &game.state.galaxy[w.x][w.y];
	    } while (--i &&
		     (same(game.quadrant, w) || q->planet == NOPLANET ||
		      q->supernova || q->status!=secure || q->klingons<=0));
	    if (i == 0) {
		/* can't seem to find one; ignore this call */
		if (idebug)
		    prout("=== Couldn't find location for distress event.");
		break;
	    }

	    /* got one!!  Schedule its enslavement */
	    ev = schedule(FENSLV, expran(game.intime));
	    ev->quadrant = w;
	    q->status = distressed;

	    /* tell the captain about it if we can */
	    if (!damaged(DRADIO) || game.condition == docked)
	    {
		prout("Uhura- Captain, %s in %s reports it is under attack",
		      systemname(q->planet), cramlc(quadrant, w));
		prout("by a Klingon invasion fleet.");
		if (cancelrest())
		    return;
	    }
	    break;
	case FENSLV:		/* starsystem is enslaved */
	    ev = unschedule(FENSLV);
	    /* see if current distress call still active */
	    q = &game.state.galaxy[ev->quadrant.x][ev->quadrant.y];
	    if (q->klingons <= 0) {
		q->status = secure;
		break;
	    }
	    q->status = enslaved;

	    /* play stork and schedule the first baby */
	    ev2 = schedule(FREPRO, expran(2.0 * game.intime));
	    ev2->quadrant = ev->quadrant;

	    /* report the disaster if we can */
	    if (!damaged(DRADIO) || game.condition == docked)
	    {
		prout("Uhura- We've lost contact with starsystem %s",
		      systemname(q->planet));
		prout("in %s.\n", cramlc(quadrant, ev->quadrant));
	    }
	    break;
	case FREPRO:		/* Klingon reproduces */
	    /*
	     * If we ever switch to a real event queue, we'll need to
	     * explicitly retrieve and restore the x and y.
	     */
	    ev = schedule(FREPRO, expran(1.0 * game.intime));
	    /* see if current distress call still active */
	    q = &game.state.galaxy[ev->quadrant.x][ev->quadrant.y];
	    if (q->klingons <= 0) {
		q->status = secure;
		break;
	    }
	    if (game.state.remkl >=MAXKLGAME)
		break;		/* full right now */
	    /* reproduce one Klingon */
	    w = ev->quadrant;
	    if (game.klhere >= MAXKLQUAD) {
		/* this quadrant not ok, pick an adjacent one */
		for (i = w.x - 1; i <= w.x + 1; i++)
		{
		    for (j = w.y - 1; j <= w.y + 1; j++)
		    {
			if (!VALID_QUADRANT(i, j))
			    continue;
			q = &game.state.galaxy[w.x][w.y];
			/* check for this quad ok (not full & no snova) */
			if (q->klingons >= MAXKLQUAD || q->supernova)
			    continue;
			goto foundit;
		    }
		}
		break;	/* search for eligible quadrant failed */
	    foundit:
		w.x = i;
		w.y = j;
	    }

	    /* deliver the child */
	    game.state.remkl++;
	    q->klingons++;
	    if (same(game.quadrant, w))
		newkling(++game.klhere);

	    /* recompute time left */
	    game.state.remtime = game.state.remres/(game.state.remkl+4*game.state.remcom);
	    /* report the disaster if we can */
	    if (!damaged(DRADIO) || game.condition == docked)
	    {
		if (same(game.quadrant, w)) {
		    prout("Spock- sensors indicate the Klingons have");
		    prout("launched a warship from %s.",systemname(q->planet));
		} else {
		    prout("Uhura- Starfleet reports increased Klingon activity");
		    if (q->planet != NOPLANET)
			proutn("near %s", systemname(q->planet));
		    prout("in %s.\n", cramlc(quadrant, w));
		}
	    }
	    break;
	}
    }
}

				
void wait(void) 
/* wait on events */
{
    int key;
    double temp, delay, origTime;

    game.ididit = false;
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
	if (ja() == false) return;
    }

    /* Alternate resting periods (events) with attacks */

    game.resting = true;
    do {
	if (delay <= 0) game.resting = false;
	if (!game.resting) {
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
	game.ididit = true;
	if (game.alldone) return;
	delay -= temp;
	/* Repair Deathray if long rest at starbase */
	if (origTime-delay >= 9.99 && game.condition == docked)
	    game.damage[DDRAY] = 0.0;
    } while 
	// leave if quadrant supernovas
	(!game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova);

    game.resting = false;
    game.optime = 0;
}

/*
 *	A nova occurs.  It is the result of having a star hit with a
 *	photon torpedo, or possibly of a probe warhead going off.
 *	Stars that go nova cause stars which surround them to undergo
 *	the same probabilistic process.  Klingons next to them are
 *	destroyed.  And if the starship is next to it, it gets zapped.
 *	If the zap is too much, it gets destroyed.
 */
void nova(coord nov) 
/* star goes nova */
{
    static double course[] =
	{0.0, 10.5, 12.0, 1.5, 9.0, 0.0, 3.0, 7.5, 6.0, 4.5};
    int bot, top, top2, hits[QUADSIZE+1][3], kount, icx, icy, mm, nn, j;
    int iquad, iquad1, i, ll;
    coord newc, scratch;

    if (Rand() < 0.05) {
	/* Wow! We've supernova'ed */
	snova(false, &nov);
	return;
    }

    /* handle initial nova */
    game.quad[nov.x][nov.y] = IHDOT;
    crmena(false, IHSTAR, sector, nov);
    prout(_(" novas."));
    game.state.galaxy[game.quadrant.x][game.quadrant.y].stars--;
    game.state.starkl++;
	
    /* Set up stack to recursively trigger adjacent stars */
    bot = top = top2 = 1;
    kount = 0;
    icx = icy = 0;
    hits[1][1] = nov.x;
    hits[1][2] = nov.y;
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
			    snova(false, &scratch);
			    return;
			}
			top2++;
			hits[top2][1]=scratch.x;
			hits[top2][2]=scratch.y;
			game.state.galaxy[game.quadrant.x][game.quadrant.y].stars -= 1;
			game.state.starkl++;
			crmena(true, IHSTAR, sector, scratch);
			prout(_(" novas."));
			game.quad[scratch.x][scratch.y] = IHDOT;
			break;
		    case IHP: /* Destroy planet */
			game.state.galaxy[game.quadrant.x][game.quadrant.y].planet = NOPLANET;
			game.state.nplankl++;
			crmena(true, IHP, sector, scratch);
			prout(_(" destroyed."));
			DESTROY(&game.state.plnets[game.iplnet]);
			game.iplnet = game.plnet.x = game.plnet.y = 0;
			if (game.landed) {
			    finish(FPNOVA);
			    return;
			}
			game.quad[scratch.x][scratch.y] = IHDOT;
			break;
		    case IHB: /* Destroy base */
			game.state.galaxy[game.quadrant.x][game.quadrant.y].starbase = false;
			for_starbases(i)
			    if (same(game.state.baseq[i], game.quadrant)) 
				break;
			game.state.baseq[i] = game.state.baseq[game.state.rembase];
			game.state.rembase--;
			game.base.x = game.base.y = 0;
			game.state.basekl++;
			newcnd();
			crmena(true, IHB, sector, scratch);
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
				game.shldup = false;
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
			deadkl(scratch,iquad, scratch);
			break;
		    case IHC: /* Damage/destroy big enemies */
		    case IHS:
		    case IHR:
			for_local_enemies(ll)
			    if (same(game.ks[ll], scratch)) break;
			game.kpower[ll] -= 800.0; /* If firepower is lost, die */
			if (game.kpower[ll] <= 0.0) {
			    deadkl(scratch, iquad, scratch);
			    break;
			}
			newc.x = scratch.x + scratch.x - hits[mm][1];
			newc.y = scratch.y + scratch.y - hits[mm][2];
			crmena(true, iquad, sector, scratch);
			proutn(_(" damaged"));
			if (!VALID_SECTOR(newc.x, newc.y)) {
			    /* can't leave quadrant */
			    skip(1);
			    break;
			}
			iquad1 = game.quad[newc.x][newc.y];
			if (iquad1 == IHBLANK) {
			    proutn(_(", blasted into "));
			    crmena(false, IHBLANK, sector, newc);
			    skip(1);
			    deadkl(scratch, iquad, newc);
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
			game.ks[ll] = newc;
			game.kdist[ll] = game.kavgd[ll] = distance(game.sector, newc);
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
	
	
void snova(bool induced, coord *w) 
/* star goes supernova */
{
    int num = 0, nrmdead, npdead, kldead;
    coord nq;

    if (w != NULL) 
	nq = *w;
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
	if (idebug) {
	    proutn("=== Super nova here?");
	    if (ja() == true)
		nq = game.quadrant;
	}
    }

    if (!same(nq, game.quadrant) || game.justin) {
	/* it isn't here, or we just entered (treat as enroute) */
	if (!damaged(DRADIO) || game.condition == docked) {
	    skip(1);
	    prout(_("Message from Starfleet Command       Stardate %.2f"), game.state.date);
	    prout(_("     Supernova in %s; caution advised."),
		  cramlc(quadrant, nq));
	}
    }
    else {
	coord ns;
	/* we are in the quadrant! */
	num = Rand()* game.state.galaxy[nq.x][nq.y].stars + 1;
	for_sectors(ns.x) {
	    for_sectors(ns.y) {
		if (game.quad[ns.x][ns.y]==IHSTAR) {
		    num--;
		    if (num==0) break;
		}
	    }
	    if (num==0) break;
	}

	skip(1);
	prouts(_("***RED ALERT!  RED ALERT!"));
	skip(1);
	prout(_("***Incipient supernova detected at "), cramlc(sector, ns));
	if (square(ns.x-game.sector.x) + square(ns.y-game.sector.y) <= 2.1) {
	    proutn(_("Emergency override attempts t"));
	    prouts("***************");
	    skip(1);
	    stars();
	    game.alldone = true;
	}
    }

    /* destroy any Klingons in supernovaed quadrant */
    kldead = game.state.galaxy[nq.x][nq.y].klingons;
    game.state.galaxy[nq.x][nq.y].klingons = 0;
    if (same(nq, game.state.kscmdr)) {
	/* did in the Supercommander! */
	game.state.nscrem = game.state.kscmdr.x = game.state.kscmdr.y = game.isatb =  0;
	game.iscate = false;
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
	int loop;
	for (loop = 0; loop < game.inplan; loop++)
	    if (same(game.state.plnets[loop].w, nq)) {
		DESTROY(&game.state.plnets[loop]);
	    }
    }
    /* Destroy any base in supernovaed quadrant */
    if (game.state.rembase) {
	int maxloop = game.state.rembase, loop;
	for (loop = 1; loop <= maxloop; loop++)
	    if (same(game.state.baseq[loop], nq)) {
		game.state.baseq[loop] = game.state.baseq[game.state.rembase];
		game.state.baseq[game.state.rembase].x = game.state.baseq[game.state.rembase].y = 0;
		game.state.rembase--;
		break;
	    }
    }
    /* If starship caused supernova, tally up destruction */
    if (induced) {
	game.state.starkl += game.state.galaxy[nq.x][nq.y].stars;
	game.state.basekl += game.state.galaxy[nq.x][nq.y].starbase;
	game.state.nplankl += npdead;
    }
    /* mark supernova in galaxy and in star chart */
    if (same(game.quadrant, nq) || !damaged(DRADIO) || game.condition == docked)
	game.state.galaxy[nq.x][nq.y].supernova = true;
    /* If supernova destroys last Klingons give special message */
    if (KLINGREM==0 && !same(nq, game.quadrant)) {
	skip(2);
	if (!induced) prout(_("Lucky you!"));
	proutn(_("A supernova in %s has just destroyed the last Klingons."),
	       cramlc(quadrant, nq));
	finish(FWON);
	return;
    }
    /* if some Klingons remain, continue or die in supernova */
    if (game.alldone) finish(FSNOVAED);
    return;
}
