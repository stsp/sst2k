#include "sst.h"
#include <math.h>

void events(void) 
{
    int ictbeam=0, ipage=0, istract=0, line, i=0, j, k, l, ixhold=0, iyhold=0;
    double fintim = game.state.date + game.optime, datemin, xtime, repair, yank=0;
    int radio_was_broken;

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
	    prout("Lt. Uhura- \"Captain, the sub-space radio is working and");
	    prout("   surveillance reports are coming in.");
	    skip(1);
	    if (game.iseenit==0) {
		attakreport(0);
		game.iseenit = 1;
	    }
	    rechart();
	    prout("   The star chart is now up to date.\"");
	    skip(1);
	}
	/* Cause extraneous event LINE to occur */
	game.optime -= xtime;
	switch (line) {
	case FSNOVA: /* Supernova */
	    if (ipage==0) pause_game(1);
	    ipage=1;
	    snova(0,0);
	    game.future[FSNOVA] = game.state.date + expran(0.5*game.intime);
	    if (game.state.galaxy[game.quadx][game.quady].supernova) return;
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
		yank = square(game.state.isx-game.quadx) + square(game.state.isy-game.quady);
		/********* fall through to FTBEAM code ***********/
	    }
	    else return;
	case FTBEAM: /* Tractor beam */
	    if (line==FTBEAM) {
		if (game.state.remcom == 0) {
		    game.future[FTBEAM] = FOREVER;
		    break;
		}
		i = Rand()*game.state.remcom+1.0;
		yank = square(game.state.cx[i]-game.quadx) + square(game.state.cy[i]-game.quady);
		if (istract || game.condit == IHDOCKED || yank == 0) {
		    /* Drats! Have to reschedule */
		    game.future[FTBEAM] = game.state.date + game.optime +
			expran(1.5*game.intime/game.state.remcom);
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
	    prout(" caught in long range tractor beam--");
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
		    prout("Galileo, left on the planet surface, is captured");
		    prout("by aliens and made into a flying McDonald's.");
		    game.damage[DSHUTTL] = -10;
		    game.iscraft = -1;
		}
		else {
		    prout("Galileo, left on the planet surface, is well hidden.");
		}
	    }
	    if (line==0) {
		game.quadx = game.state.isx;
		game.quady = game.state.isy;
	    }
	    else {
		game.quadx = game.state.cx[i];
		game.quady = game.state.cy[i];
	    }
	    iran(QUADSIZE, &game.sectx, &game.secty);
	    crmshp();
	    proutn(" is pulled to ");
	    proutn(cramlc(quadrant, game.quadx, game.quady));
	    proutn(", ");
	    prout(cramlc(sector, game.sectx, game.secty));
	    if (game.resting) {
		prout("(Remainder of rest/repair period cancelled.)");
		game.resting = 0;
	    }
	    if (game.shldup==0) {
		if (game.damage[DSHIELD]==0 && game.shield > 0) {
		    doshield(2); /* Shldsup */
		    game.shldchg=0;
		}
		else prout("(Shields not currently useable.)");
	    }
	    newqad(0);
	    /* Adjust finish time to time of tractor beaming */
	    fintim = game.state.date+game.optime;
	    attack(0);
	    if (game.state.remcom <= 0) game.future[FTBEAM] = FOREVER;
	    else game.future[FTBEAM] = game.state.date+game.optime+expran(1.5*game.intime/game.state.remcom);
	    break;
	case FSNAP: /* Snapshot of the universe (for time warp) */
	    game.snapsht = game.state;
	    game.state.snap = 1;
	    game.future[FSNAP] = game.state.date + expran(0.5 * game.intime);
	    break;
	case FBATTAK: /* Commander attacks starbase */
	    if (game.state.remcom==0 || game.state.rembase==0) {
		/* no can do */
		game.future[FBATTAK] = game.future[FCDBAS] = FOREVER;
		break;
	    }
	    i = 0;
	    for_starbases(j) {
		for_commanders(k)
		    if (game.state.baseqx[j]==game.state.cx[k] && game.state.baseqy[j]==game.state.cy[k] &&
			(game.state.baseqx[j]!=game.quadx || game.state.baseqy[j]!=game.quady) &&
			(game.state.baseqx[j]!=game.state.isx || game.state.baseqy[j]!=game.state.isy)) {
			i = 1;
			break;
		    }
		if (i == 1) break;
	    }
	    if (j>game.state.rembase) {
		/* no match found -- try later */
		game.future[FBATTAK] = game.state.date + expran(0.3*game.intime);
		game.future[FCDBAS] = FOREVER;
		break;
	    }
	    /* commander + starbase combination found -- launch attack */
	    game.batx = game.state.baseqx[j];
	    game.baty = game.state.baseqy[j];
	    game.future[FCDBAS] = game.state.date+1.0+3.0*Rand();
	    if (game.isatb) /* extra time if SC already attacking */
		game.future[FCDBAS] += game.future[FSCDBAS]-game.state.date;
	    game.future[FBATTAK] = game.future[FCDBAS] +expran(0.3*game.intime);
	    game.iseenit = 0;
	    if (game.damage[DRADIO] != 0.0 &&
		game.condit != IHDOCKED) break; /* No warning :-( */
	    game.iseenit = 1;
	    if (ipage==0) pause_game(1);
	    ipage = 1;
	    skip(1);
	    proutn("Lt. Uhura-  \"Captain, the starbase in ");
	    prout(cramlc(quadrant, game.batx, game.baty));
	    prout("   reports that it is under attack and that it can");
	    proutn("   hold out only until stardate %d",
		   (int)game.future[FCDBAS]);
	    prout(".\"");
	    if (game.resting) {
		skip(1);
		proutn("Mr. Spock-  \"Captain, shall we cancel the rest period?\" ");
		if (ja()) {
		    game.resting = 0;
		    game.optime = 0.0;
		    return;
		}
	    }
	    break;
	case FSCDBAS: /* Supercommander destroys base */
	    game.future[FSCDBAS] = FOREVER;
	    game.isatb = 2;
	    if (!game.state.galaxy[game.state.isx][game.state.isy].starbase) 
		break; /* WAS RETURN! */
	    ixhold = game.batx;
	    iyhold = game.baty;
	    game.batx = game.state.isx;
	    game.baty = game.state.isy;
	case FCDBAS: /* Commander succeeds in destroying base */
	    if (line==FCDBAS) {
		game.future[FCDBAS] = FOREVER;
		/* find the lucky pair */
		for_commanders(i)
		    if (game.state.cx[i]==game.batx && game.state.cy[i]==game.baty) 
			break;
		if (i > game.state.remcom || game.state.rembase == 0 ||
		    !game.state.galaxy[game.batx][game.baty].starbase) {
		    /* No action to take after all */
		    game.batx = game.baty = 0;
		    break;
		}
	    }
	    /* Code merges here for any commander destroying base */
	    /* Not perfect, but will have to do */
	    /* Handle case where base is in same quadrant as starship */
	    if (game.batx==game.quadx && game.baty==game.quady) {
		game.state.chart[game.batx][game.baty].starbase = FALSE;
		game.quad[game.basex][game.basey]= IHDOT;
		game.basex=game.basey=0;
		newcnd();
		skip(1);
		prout("Spock-  \"Captain, I believe the starbase has been destroyegame.state.\"");
	    }
	    else if (game.state.rembase != 1 &&
		     (game.damage[DRADIO] <= 0.0 || game.condit == IHDOCKED)) {
		/* Get word via subspace radio */
		if (ipage==0) pause_game(1);
		ipage = 1;
		skip(1);
		prout("Lt. Uhura-  \"Captain, Starfleet Command reports that");
		proutn("   the starbase in ");
		proutn(cramlc(quadrant, game.batx, game.baty));
		prout(" has been destroyed by");
		if (game.isatb==2) prout("the Klingon Super-Commander");
		else prout("a Klingon Commander");
		game.state.chart[game.batx][game.baty].starbase = FALSE;
	    }
	    /* Remove Starbase from galaxy */
	    game.state.galaxy[game.batx][game.baty].starbase = FALSE;
	    for_starbases(i)
		if (game.state.baseqx[i]==game.batx && game.state.baseqy[i]==game.baty) {
		    game.state.baseqx[i]=game.state.baseqx[game.state.rembase];
		    game.state.baseqy[i]=game.state.baseqy[game.state.rembase];
		}
	    game.state.rembase--;
	    if (game.isatb == 2) {
		/* reinstate a commander's base attack */
		game.batx = ixhold;
		game.baty = iyhold;
		game.isatb = 0;
	    }
	    else {
		game.batx = game.baty = 0;
	    }
	    break;
	case FSCMOVE: /* Supercommander moves */
	    game.future[FSCMOVE] = game.state.date+0.2777;
	    if (game.ientesc+istract==0 &&
		game.isatb!=1 &&
		(game.iscate!=1 || game.justin==1)) scom(&ipage);
	    break;
	case FDSPROB: /* Move deep space probe */
	    game.future[FDSPROB] = game.state.date + 0.01;
	    game.probex += game.probeinx;
	    game.probey += game.probeiny;
	    i = (int)(game.probex/QUADSIZE +0.05);
	    j = (int)(game.probey/QUADSIZE + 0.05);
	    if (game.probecx != i || game.probecy != j) {
		game.probecx = i;
		game.probecy = j;
		if (!VALID_QUADRANT(i, j) ||
		    game.state.galaxy[game.probecx][game.probecy].supernova) {
		    // Left galaxy or ran into supernova
		    if (game.damage[DRADIO]==0.0 || game.condit == IHDOCKED) {
			if (ipage==0) pause_game(1);
			ipage = 1;
			skip(1);
			proutn("Lt. Uhura-  \"The deep space probe ");
			if (!VALID_QUADRANT(j, i))
			    proutn("has left the galaxy");
			else
			    proutn("is no longer transmitting");
			prout(".\"");
		    }
		    game.future[FDSPROB] = FOREVER;
		    break;
		}
		if (game.damage[DRADIO]==0.0   || game.condit == IHDOCKED) {
		    if (ipage==0) pause_game(1);
		    ipage = 1;
		    skip(1);
		    proutn("Lt. Uhura-  \"The deep space probe is now in ");
		    proutn(cramlc(quadrant, game.probecx, game.probecy));
		    prout(".\"");
		}
	    }
	    /* Update star chart if Radio is working or have access to
	       radio. */
	    if (game.damage[DRADIO] == 0.0 || game.condit == IHDOCKED) {
		game.state.chart[game.probecx][game.probecy].klingons = game.state.galaxy[game.probecx][game.probecy].klingons;
		game.state.chart[game.probecx][game.probecy].starbase = game.state.galaxy[game.probecx][game.probecy].starbase;
		game.state.chart[game.probecx][game.probecy].stars = game.state.galaxy[game.probecx][game.probecy].stars;
		game.state.galaxy[game.probecx][game.probecy].charted = TRUE;
	    }
	    game.proben--; // One less to travel
	    if (game.proben == 0 && game.isarmed &&
		game.state.galaxy[game.probecx][game.probecy].stars) {
		/* lets blow the sucker! */
		snova(1,0);
		game.future[FDSPROB] = FOREVER;
		if (game.state.galaxy[game.quadx][game.quady].supernova) 
		    return;
	    }
	    break;
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
	proutn("How long? ");
    }
    chew();
    if (key != IHREAL) {
	huh();
	return;
    }
    origTime = delay = aaitem;
    if (delay <= 0.0) return;
    if (delay >= game.state.remtime || game.nenhere != 0) {
	proutn("Are you sure? ");
	if (ja() == 0) return;
    }

    /* Alternate resting periods (events) with attacks */

    game.resting = 1;
    do {
	if (delay <= 0) game.resting = 0;
	if (game.resting == 0) {
	    prout("%d stardates left.", (int)game.state.remtime);
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
	(!game.state.galaxy[game.quadx][game.quady].supernova);

    game.resting = 0;
    game.optime = 0;
}

void nova(int ix, int iy) 
{
    static double course[] =
	{0.0, 10.5, 12.0, 1.5, 9.0, 0.0, 3.0, 7.5, 6.0, 4.5};
    int bot, top, top2, hits[QUADSIZE+1][3], kount, icx, icy, mm, nn, j;
    int iquad, iquad1, i, ll, newcx, newcy, ii, jj;
    if (Rand() < 0.05) {
	/* Wow! We've supernova'ed */
	snova(ix, iy);
	return;
    }

    /* handle initial nova */
    game.quad[ix][iy] = IHDOT;
    crmena(1, IHSTAR, 2, ix, iy);
    prout(" novas.");
    game.state.galaxy[game.quadx][game.quady].stars--;
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
		    ii = hits[mm][1]+nn-2;
		    jj = hits[mm][2]+j-2;
		    if (!VALID_SECTOR(jj, ii)) continue;
		    iquad = game.quad[ii][jj];
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
			    snova(ii,jj);
			    return;
			}
			top2++;
			hits[top2][1]=ii;
			hits[top2][2]=jj;
			game.state.galaxy[game.quadx][game.quady].stars -= 1;
			game.state.starkl++;
			crmena(1, IHSTAR, 2, ii, jj);
			prout(" novas.");
			game.quad[ii][jj] = IHDOT;
			break;
		    case IHP: /* Destroy planet */
			game.state.galaxy[game.quadx][game.quady].planet = NULL;
			game.state.nplankl++;
			crmena(1, IHP, 2, ii, jj);
			prout(" destroyed.");
			DESTROY(&game.state.plnets[game.iplnet]);
			game.iplnet = game.plnetx = game.plnety = 0;
			if (game.landed == 1) {
			    finish(FPNOVA);
			    return;
			}
			game.quad[ii][jj] = IHDOT;
			break;
		    case IHB: /* Destroy base */
			game.state.galaxy[game.quadx][game.quady].starbase = FALSE;
			for_starbases(i)
			    if (game.state.baseqx[i]==game.quadx && game.state.baseqy[i]==game.quady) 
				break;
			game.state.baseqx[i] = game.state.baseqx[game.state.rembase];
			game.state.baseqy[i] = game.state.baseqy[game.state.rembase];
			game.state.rembase--;
			game.basex = game.basey = 0;
			game.state.basekl++;
			newcnd();
			crmena(1, IHB, 2, ii, jj);
			prout(" destroyed.");
			game.quad[ii][jj] = IHDOT;
			break;
		    case IHE: /* Buffet ship */
		    case IHF:
			prout("***Starship buffeted by nova.");
			if (game.shldup) {
			    if (game.shield >= 2000.0) game.shield -= 2000.0;
			    else {
				double diff = 2000.0 - game.shield;
				game.energy -= diff;
				game.shield = 0.0;
				game.shldup = 0;
				prout("***Shields knocked out.");
				game.damage[DSHIELD] += 0.005*game.damfac*Rand()*diff;
			    }
			}
			else game.energy -= 2000.0;
			if (game.energy <= 0) {
			    finish(FNOVA);
			    return;
			}
			/* add in course nova contributes to kicking starship*/
			icx += game.sectx-hits[mm][1];
			icy += game.secty-hits[mm][2];
			kount++;
			break;
		    case IHK: /* kill klingon */
			deadkl(ii,jj,iquad, ii, jj);
			break;
		    case IHC: /* Damage/destroy big enemies */
		    case IHS:
		    case IHR:
			for_local_enemies(ll)
			    if (game.kx[ll]==ii && game.ky[ll]==jj) break;
			game.kpower[ll] -= 800.0; /* If firepower is lost, die */
			if (game.kpower[ll] <= 0.0) {
			    deadkl(ii, jj, iquad, ii, jj);
			    break;
			}
			newcx = ii + ii - hits[mm][1];
			newcy = jj + jj - hits[mm][2];
			crmena(1, iquad, 2, ii, jj);
			proutn(" damaged");
			if (!VALID_SECTOR(newcx, newcy)) {
			    /* can't leave quadrant */
			    skip(1);
			    break;
			}
			iquad1 = game.quad[newcx][newcy];
			if (iquad1 == IHBLANK) {
			    proutn(", blasted into ");
			    crmena(0, IHBLANK, 2, newcx, newcy);
			    skip(1);
			    deadkl(ii, jj, iquad, newcx, newcy);
			    break;
			}
			if (iquad1 != IHDOT) {
			    /* can't move into something else */
			    skip(1);
			    break;
			}
			proutn(", buffeted to ");
			proutn(cramlc(sector, newcx, newcy));
			game.quad[ii][jj] = IHDOT;
			game.quad[newcx][newcy] = iquad;
			game.kx[ll] = newcx;
			game.ky[ll] = newcy;
			game.kavgd[ll] = sqrt(square(game.sectx-newcx)+square(game.secty-newcy));
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
    prout("Force of nova displaces starship.");
    game.iattak=2;	/* Eliminates recursion problem */
    imove();
    game.optime = 10.0*game.dist/16.0;
    return;
}
	
	
void snova(int insx, int insy) 
{
    int comdead, nqx=0, nqy=0, nsx, nsy, num=0, kldead, iscdead;
    int nrmdead, npdead;
    int incipient=0;

    nsx = insy;
    nsy = insy;

    if (insy== 0) {
	if (insx == 1) {
	    /* NOVAMAX being used */
	    nqx = game.probecx;
	    nqy = game.probecy;
	}
	else {
	    int stars = 0;
	    /* Scheduled supernova -- select star */
	    /* logic changed here so that we won't favor quadrants in top
	       left of universe */
	    for_quadrants(nqx) {
		for_quadrants(nqy) {
		    stars += game.state.galaxy[nqx][nqy].stars;
		}
	    }
	    if (stars == 0) return; /* nothing to supernova exists */
	    num = Rand()*stars + 1;
	    for_quadrants(nqx) {
		for_quadrants(nqy) {
		    num -= game.state.galaxy[nqx][nqy].stars;
		    if (num <= 0) break;
		}
		if (num <=0) break;
	    }
#ifdef DEBUG
	    if (game.idebug) {
		proutn("Super nova here?");
		if (ja()==1) {
		    nqx = game.quadx;
		    nqy = game.quady;
		}
	    }
#endif
	}

	if (nqx != game.quady || nqy != game.quady || game.justin != 0) {
	    /* it isn't here, or we just entered (treat as inroute) */
	    if (game.damage[DRADIO] == 0.0 || game.condit == IHDOCKED) {
		skip(1);
		prout("Message from Starfleet Command       Stardate %.2f", game.state.date);
		prout("     Supernova in %s; caution advised.",
		      cramlc(quadrant, nqx, nqy));
	    }
	}
	else {
	    /* we are in the quadrant! */
	    incipient = 1;
	    num = Rand()* game.state.galaxy[nqx][nqy].stars + 1;
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
	skip(1);
	prouts("***RED ALERT!  RED ALERT!");
	skip(1);
	prout("***Incipient supernova detected at ", cramlc(sector, nsx, nsy));
	nqx = game.quadx;
	nqy = game.quady;
	if (square(nsx-game.sectx) + square(nsy-game.secty) <= 2.1) {
	    proutn("Emergency override attempts t");
	    prouts("***************");
	    skip(1);
	    stars();
	    game.alldone=1;
	}
    }
    /* destroy any Klingons in supernovaed quadrant */
    kldead = game.state.galaxy[nqx][nqy].klingons;
    game.state.galaxy[nqx][nqy].klingons = 0;
    comdead = iscdead = 0;
    if (nqx==game.state.isx && nqy == game.state.isy) {
	/* did in the Supercommander! */
	game.state.nscrem = game.state.isx = game.state.isy = game.isatb = game.iscate = 0;
	iscdead = 1;
	game.future[FSCMOVE] = game.future[FSCDBAS] = FOREVER;
    }
    if (game.state.remcom) {
	int maxloop = game.state.remcom, l;
	for (l = 1; l <= maxloop; l++) {
	    if (game.state.cx[l] == nqx && game.state.cy[l] == nqy) {
		game.state.cx[l] = game.state.cx[game.state.remcom];
		game.state.cy[l] = game.state.cy[game.state.remcom];
		game.state.cx[game.state.remcom] = game.state.cy[game.state.remcom] = 0;
		game.state.remcom--;
		kldead--;
		comdead++;
		if (game.state.remcom==0) game.future[FTBEAM] = FOREVER;
		break;
	    }
	}
    }
    game.state.remkl -= kldead;
    /* destroy Romulans and planets in supernovaed quadrant */
    nrmdead = game.state.galaxy[nqx][nqy].romulans;
    game.state.galaxy[nqx][nqy].romulans = 0;
    game.state.nromrem -= nrmdead;
    npdead = num - nrmdead*10;
    if (npdead) {
	int l;
	for (l = 0; l < game.inplan; l++)
	    if (game.state.plnets[l].x == nqx && game.state.plnets[l].y == nqy) {
		DESTROY(&game.state.plnets[l]);
	    }
    }
    /* Destroy any base in supernovaed quadrant */
    if (game.state.rembase) {
	int maxloop = game.state.rembase, l;
	for (l = 1; l <= maxloop; l++)
	    if (game.state.baseqx[l]==nqx && game.state.baseqy[l]==nqy) {
		game.state.baseqx[l] = game.state.baseqx[game.state.rembase];
		game.state.baseqy[l] = game.state.baseqy[game.state.rembase];
		game.state.baseqx[game.state.rembase] = game.state.baseqy[game.state.rembase] = 0;
		game.state.rembase--;
		break;
	    }
    }
    /* If starship caused supernova, tally up destruction */
    if (insx) {
	game.state.starkl += game.state.galaxy[nqx][nqy].stars;
	game.state.basekl += game.state.galaxy[nqx][nqy].starbase;
	game.state.nplankl += npdead;
    }
    /* mark supernova in galaxy and in star chart */
    if ((game.quadx == nqx && game.quady == nqy) ||
	game.damage[DRADIO] == 0 ||
	game.condit == IHDOCKED)
	game.state.galaxy[nqx][nqy].supernova = TRUE;
    /* If supernova destroys last klingons give special message */
    if (KLINGREM==0 && (nqx != game.quadx || nqy != game.quady)) {
	skip(2);
	if (insx == 0) prout("Lucky you!");
	proutn("A supernova in %s has just destroyed the last Klingons.",
	       cramlc(quadrant, nqx, nqy));
	finish(FWON);
	return;
    }
    /* if some Klingons remain, continue or die in supernova */
    if (game.alldone) finish(FSNOVAED);
    return;
}
		
				
