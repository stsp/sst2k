#include "sst.h"
#include <math.h>

void events(void) 
{
    int ictbeam=0, ipage=0, istract=0, line, i=0, j, k, l, ixhold=0, iyhold=0;
    double fintim = game.state.date + Time, datemin, xtime, repair, yank=0;

#ifdef DEBUG
    if (idebug) prout("EVENTS");
#endif

    if (stdamtim == 1e30 && game.damage[DRADIO] != 0.0) {
	/* chart will no longer be updated because radio is dead */
	stdamtim = game.state.date;
	for (i=1; i <= GALSIZE ; i++)
	    for (j=1; j <= GALSIZE; j++)
		if (game.starch[i][j] == 1) 
		    game.starch[i][j] = game.state.galaxy[i][j]+SUPERNOVA_PLACE;
    }

    for (;;) {
	/* Select earliest extraneous event, line==0 if no events */
	line = FSPY;
	if (alldone) return;
	datemin = fintim;
	for (l=1; l<=NEVENTS; l++)
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
	if (game.damage[DLIFSUP] && condit != IHDOCKED) {
	    if (lsupres < xtime && game.damage[DLIFSUP] > lsupres) {
		finish(FLIFESUP);
		return;
	    }
	    lsupres -= xtime;
	    if (game.damage[DLIFSUP] <= xtime) lsupres = inlsr;
	}
	/* Fix devices */
	repair = xtime;
	if (condit == IHDOCKED) repair /= docfac;
	/* Don't fix Deathray here */
	for (l=1; l<=NDEVICES; l++)
	    if (game.damage[l] > 0.0 && l != DDRAY)
		game.damage[l] -= (game.damage[l]-repair > 0.0 ? repair : game.damage[l]);
	/* If radio repaired, update star chart and attack reports */
	if (stdamtim != 1e30 && game.damage[DRADIO] == 0.0) {
	    stdamtim = 1e30;
	    prout("Lt. Uhura- \"Captain, the sub-space radio is working and");
	    prout("   surveillance reports are coming in.");
	    skip(1);
	    for (i=1; i <= GALSIZE ; i++)
		for (j=1; j <= GALSIZE; j++)
		    if (game.starch[i][j] >= SUPERNOVA_PLACE) game.starch[i][j] = 1;
	    if (iseenit==0) {
		attakreport(0);
		iseenit = 1;
	    }
	    skip(1);
	    prout("   The star chart is now up to date.\"");
	    skip(1);
	}
	/* Cause extraneous event LINE to occur */
	Time -= xtime;
	switch (line) {
	case FSNOVA: /* Supernova */
	    if (ipage==0) pause_game(1);
	    ipage=1;
	    snova(0,0);
	    game.future[FSNOVA] = game.state.date + expran(0.5*intime);
	    if (game.state.galaxy[quadx][quady] == SUPERNOVA_PLACE) return;
	    break;
	case FSPY: /* Check with spy to see if S.C. should tractor beam */
	    if (game.state.nscrem == 0 ||
		ictbeam+istract > 0 ||
		condit==IHDOCKED || isatb==1 || iscate==1) return;
	    if (ientesc ||
		(energy < 2000 && torps < 4 && shield < 1250) ||
		(game.damage[DPHASER]>0 && (game.damage[DPHOTON]>0 || torps < 4)) ||
		(game.damage[DSHIELD] > 0 &&
		 (energy < 2500 || game.damage[DPHASER] > 0) &&
		 (torps < 5 || game.damage[DPHOTON] > 0))) {
		/* Tractor-beam her! */
		istract=1;
		yank = square(game.state.isx-quadx) + square(game.state.isy-quady);
		/********* fall through to FTBEAM code ***********/
	    }
	    else return;
	case FTBEAM: /* Tractor beam */
	    if (line==FTBEAM) {
		if (game.state.remcom == 0) {
		    game.future[FTBEAM] = 1e30;
		    break;
		}
		i = Rand()*game.state.remcom+1.0;
		yank = square(game.state.cx[i]-quadx) + square(game.state.cy[i]-quady);
		if (istract || condit == IHDOCKED || yank == 0) {
		    /* Drats! Have to reschedule */
		    game.future[FTBEAM] = game.state.date + Time +
			expran(1.5*intime/game.state.remcom);
		    break;
		}
	    }
	    /* tractor beaming cases merge here */
	    yank = sqrt(yank);
	    if (ipage==0) pause_game(1);
	    ipage=1;
	    Time = (10.0/(7.5*7.5))*yank; /* 7.5 is yank rate (warp 7.5) */
	    ictbeam = 1;
	    skip(1);
	    proutn("***");
	    crmshp();
	    prout(" caught in long range tractor beam--");
	    /* If Kirk & Co. screwing around on planet, handle */
	    atover(1); /* atover(1) is Grab */
	    if (alldone) return;
	    if (icraft == 1) { /* Caught in Galileo? */
		finish(FSTRACTOR);
		return;
	    }
	    /* Check to see if shuttle is aboard */
	    if (iscraft==0) {
		skip(1);
		if (Rand() > 0.5) {
		    prout("Galileo, left on the planet surface, is captured");
		    prout("by aliens and made into a flying McDonald's.");
		    game.damage[DSHUTTL] = -10;
		    iscraft = -1;
		}
		else {
		    prout("Galileo, left on the planet surface, is well hidden.");
		}
	    }
	    if (line==0) {
		quadx = game.state.isx;
		quady = game.state.isy;
	    }
	    else {
		quadx = game.state.cx[i];
		quady = game.state.cy[i];
	    }
	    iran(QUADSIZE, &sectx, &secty);
	    crmshp();
	    proutn(" is pulled to ");
	    proutn(cramlc(quadrant, quadx, quady));
	    proutn(", ");
	    prout(cramlc(sector, sectx, secty));
	    if (resting) {
		prout("(Remainder of rest/repair period cancelled.)");
		resting = 0;
	    }
	    if (shldup==0) {
		if (game.damage[DSHIELD]==0 && shield > 0) {
		    doshield(2); /* Shldsup */
		    shldchg=0;
		}
		else prout("(Shields not currently useable.)");
	    }
	    newqad(0);
	    /* Adjust finish time to time of tractor beaming */
	    fintim = game.state.date+Time;
	    attack(0);
	    if (game.state.remcom <= 0) game.future[FTBEAM] = 1e30;
	    else game.future[FTBEAM] = game.state.date+Time+expran(1.5*intime/game.state.remcom);
	    break;
	case FSNAP: /* Snapshot of the universe (for time warp) */
	    game.snapsht = game.state;
	    game.state.snap = 1;
	    game.future[FSNAP] = game.state.date + expran(0.5 * intime);
	    break;
	case FBATTAK: /* Commander attacks starbase */
	    if (game.state.remcom==0 || game.state.rembase==0) {
		/* no can do */
		game.future[FBATTAK] = game.future[FCDBAS] = 1e30;
		break;
	    }
	    i = 0;
	    for (j=1; j<=game.state.rembase; j++) {
		for (k=1; k<=game.state.remcom; k++)
		    if (game.state.baseqx[j]==game.state.cx[k] && game.state.baseqy[j]==game.state.cy[k] &&
			(game.state.baseqx[j]!=quadx || game.state.baseqy[j]!=quady) &&
			(game.state.baseqx[j]!=game.state.isx || game.state.baseqy[j]!=game.state.isy)) {
			i = 1;
			break;
		    }
		if (i == 1) break;
	    }
	    if (j>game.state.rembase) {
		/* no match found -- try later */
		game.future[FBATTAK] = game.state.date + expran(0.3*intime);
		game.future[FCDBAS] = 1e30;
		break;
	    }
	    /* commander + starbase combination found -- launch attack */
	    batx = game.state.baseqx[j];
	    baty = game.state.baseqy[j];
	    game.future[FCDBAS] = game.state.date+1.0+3.0*Rand();
	    if (isatb) /* extra time if SC already attacking */
		game.future[FCDBAS] += game.future[FSCDBAS]-game.state.date;
	    game.future[FBATTAK] = game.future[FCDBAS] +expran(0.3*intime);
	    iseenit = 0;
	    if (game.damage[DRADIO] != 0.0 &&
		condit != IHDOCKED) break; /* No warning :-( */
	    iseenit = 1;
	    if (ipage==0) pause_game(1);
	    ipage = 1;
	    skip(1);
	    proutn("Lt. Uhura-  \"Captain, the starbase in ");
	    prout(cramlc(quadrant, batx, baty));
	    prout("   reports that it is under attack and that it can");
	    proutn("   hold out only until stardate %d",
		   (int)game.future[FCDBAS]);
	    prout(".\"");
	    if (resting) {
		skip(1);
		proutn("Mr. Spock-  \"Captain, shall we cancel the rest period?\" ");
		if (ja()) {
		    resting = 0;
		    Time = 0.0;
		    return;
		}
	    }
	    break;
	case FSCDBAS: /* Supercommander destroys base */
	    game.future[FSCDBAS] = 1e30;
	    isatb = 2;
	    if (!BASES(game.state.galaxy[game.state.isx][game.state.isy])) 
		break; /* WAS RETURN! */
	    ixhold = batx;
	    iyhold = baty;
	    batx = game.state.isx;
	    baty = game.state.isy;
	case FCDBAS: /* Commander succeeds in destroying base */
	    if (line==FCDBAS) {
		game.future[FCDBAS] = 1e30;
		/* find the lucky pair */
		for (i = 1; i <= game.state.remcom; i++)
		    if (game.state.cx[i]==batx && game.state.cy[i]==baty) break;
		if (i > game.state.remcom || game.state.rembase == 0 ||
		    !BASES(game.state.galaxy[batx][baty])) {
		    /* No action to take after all */
		    batx = baty = 0;
		    break;
		}
	    }
	    /* Code merges here for any commander destroying base */
	    /* Not perfect, but will have to do */
	    if (game.starch[batx][baty] == -1) game.starch[batx][baty] = 0;
	    /* Handle case where base is in same quadrant as starship */
	    if (batx==quadx && baty==quady) {
		if (game.starch[batx][baty] >= SUPERNOVA_PLACE) game.starch[batx][baty] -= BASE_PLACE;
		game.quad[basex][basey]= IHDOT;
		basex=basey=0;
		newcnd();
		skip(1);
		prout("Spock-  \"Captain, I believe the starbase has been destroyegame.state.\"");
	    }
	    else if (game.state.rembase != 1 &&
		     (game.damage[DRADIO] <= 0.0 || condit == IHDOCKED)) {
		/* Get word via subspace radio */
		if (ipage==0) pause_game(1);
		ipage = 1;
		skip(1);
		prout("Lt. Uhura-  \"Captain, Starfleet Command reports that");
		proutn("   the starbase in ");
		proutn(cramlc(quadrant, batx, baty));
		prout(" has been destroyed by");
		if (isatb==2) prout("the Klingon Super-Commander");
		else prout("a Klingon Commander");
	    }
	    /* Remove Starbase from galaxy */
	    game.state.galaxy[batx][baty] -= BASE_PLACE;
	    for (i=1; i <= game.state.rembase; i++)
		if (game.state.baseqx[i]==batx && game.state.baseqy[i]==baty) {
		    game.state.baseqx[i]=game.state.baseqx[game.state.rembase];
		    game.state.baseqy[i]=game.state.baseqy[game.state.rembase];
		}
	    game.state.rembase--;
	    if (isatb == 2) {
		/* reinstate a commander's base attack */
		batx = ixhold;
		baty = iyhold;
		isatb = 0;
	    }
	    else {
		batx = baty = 0;
	    }
	    break;
	case FSCMOVE: /* Supercommander moves */
	    game.future[FSCMOVE] = game.state.date+0.2777;
	    if (ientesc+istract==0 &&
		isatb!=1 &&
		(iscate!=1 || justin==1)) scom(&ipage);
	    break;
	case FDSPROB: /* Move deep space probe */
	    game.future[FDSPROB] = game.state.date + 0.01;
	    probex += probeinx;
	    probey += probeiny;
	    i = (int)(probex/QUADSIZE +0.05);
	    j = (int)(probey/QUADSIZE + 0.05);
	    if (probecx != i || probecy != j) {
		probecx = i;
		probecy = j;
		if (i < 1 || i > GALSIZE || j < 1 || j > GALSIZE ||
		    game.state.galaxy[probecx][probecy] == SUPERNOVA_PLACE) {
		    // Left galaxy or ran into supernova
		    if (game.damage[DRADIO]==0.0 || condit == IHDOCKED) {
			if (ipage==0) pause_game(1);
			ipage = 1;
			skip(1);
			proutn("Lt. Uhura-  \"The deep space probe ");
			if (i < 1 ||i > GALSIZE || j < 1 || j > GALSIZE)
			    proutn("has left the galaxy");
			else
			    proutn("is no longer transmitting");
			prout(".\"");
		    }
		    game.future[FDSPROB] = 1e30;
		    break;
		}
		if (game.damage[DRADIO]==0.0   || condit == IHDOCKED) {
		    if (ipage==0) pause_game(1);
		    ipage = 1;
		    skip(1);
		    proutn("Lt. Uhura-  \"The deep space probe is now in ");
		    proutn(cramlc(quadrant, probecx, probecy));
		    prout(".\"");
		}
	    }
	    /* Update star chart if Radio is working or have access to
	       radio. */
	    if (game.damage[DRADIO] == 0.0 || condit == IHDOCKED)
		game.starch[probecx][probecy] = game.damage[DRADIO] > 0.0 ?
		    game.state.galaxy[probecx][probecy]+SUPERNOVA_PLACE : 1;
	    proben--; // One less to travel
	    if (proben == 0 && isarmed &&
		STARS(game.state.galaxy[probecx][probecy])) {
		/* lets blow the sucker! */
		snova(1,0);
		game.future[FDSPROB] = 1e30;
		if (game.state.galaxy[quadx][quady] == SUPERNOVA_PLACE) return;
	    }
	    break;
	}
    }
}

				
void wait(void) 
{
    int key;
    double temp, delay, origTime;

    ididit = 0;
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
    if (delay >= game.state.remtime || nenhere != 0) {
	proutn("Are you sure? ");
	if (ja() == 0) return;
    }

    /* Alternate resting periods (events) with attacks */

    resting = 1;
    do {
	if (delay <= 0) resting = 0;
	if (resting == 0) {
	    prout("%d stardates left.", (int)game.state.remtime);
	    return;
	}
	temp = Time = delay;

	if (nenhere) {
	    double rtime = 1.0 + Rand();
	    if (rtime < temp) temp = rtime;
	    Time = temp;
	}
	if (Time < delay) attack(0);
	if (alldone) return;
	events();
	ididit = 1;
	if (alldone) return;
	delay -= temp;
	/* Repair Deathray if long rest at starbase */
	if (origTime-delay >= 9.99 && condit == IHDOCKED)
	    game.damage[DDRAY] = 0.0;
    } while 
	(game.state.galaxy[quadx][quady] != SUPERNOVA_PLACE); // leave if quadrant supernovas

    resting = 0;
    Time = 0;
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
    game.state.galaxy[quadx][quady] -= 1;
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
		    if (ii < 1 || ii > QUADSIZE || jj < 1 || jj > QUADSIZE) continue;
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
			game.state.galaxy[quadx][quady] -= 1;
			game.state.starkl++;
			crmena(1, IHSTAR, 2, ii, jj);
			prout(" novas.");
			game.quad[ii][jj] = IHDOT;
			break;
		    case IHP: /* Destroy planet */
			game.state.newstuf[quadx][quady] -= 1;
			game.state.nplankl++;
			crmena(1, IHP, 2, ii, jj);
			prout(" destroyed.");
			DESTROY(&game.state.plnets[iplnet]);
			iplnet = plnetx = plnety = 0;
			if (landed == 1) {
			    finish(FPNOVA);
			    return;
			}
			game.quad[ii][jj] = IHDOT;
			break;
		    case IHB: /* Destroy base */
			game.state.galaxy[quadx][quady] -= BASE_PLACE;
			for (i = 1; i <= game.state.rembase; i++)
			    if (game.state.baseqx[i]==quadx && game.state.baseqy[i]==quady) break;
			game.state.baseqx[i] = game.state.baseqx[game.state.rembase];
			game.state.baseqy[i] = game.state.baseqy[game.state.rembase];
			game.state.rembase--;
			basex = basey = 0;
			game.state.basekl++;
			newcnd();
			crmena(1, IHB, 2, ii, jj);
			prout(" destroyed.");
			game.quad[ii][jj] = IHDOT;
			break;
		    case IHE: /* Buffet ship */
		    case IHF:
			prout("***Starship buffeted by nova.");
			if (shldup) {
			    if (shield >= 2000.0) shield -= 2000.0;
			    else {
				double diff = 2000.0 - shield;
				energy -= diff;
				shield = 0.0;
				shldup = 0;
				prout("***Shields knocked out.");
				game.damage[DSHIELD] += 0.005*damfac*Rand()*diff;
			    }
			}
			else energy -= 2000.0;
			if (energy <= 0) {
			    finish(FNOVA);
			    return;
			}
			/* add in course nova contributes to kicking starship*/
			icx += sectx-hits[mm][1];
			icy += secty-hits[mm][2];
			kount++;
			break;
		    case IHK: /* kill klingon */
			deadkl(ii,jj,iquad, ii, jj);
			break;
		    case IHC: /* Damage/destroy big enemies */
		    case IHS:
		    case IHR:
			for (ll = 1; ll <= nenhere; ll++)
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
			if (newcx<1 || newcx>QUADSIZE || newcy<1 || newcy>QUADSIZE) {
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
			game.kavgd[ll] = sqrt(square(sectx-newcx)+square(secty-newcy));
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
    dist = kount*0.1;
    if (icx) icx = (icx < 0 ? -1 : 1);
    if (icy) icy = (icy < 0 ? -1 : 1);
    direc = course[3*(icx+1)+icy+2];
    if (direc == 0.0) dist = 0.0;
    if (dist == 0.0) return;
    Time = 10.0*dist/16.0;
    skip(1);
    prout("Force of nova displaces starship.");
    iattak=2;	/* Eliminates recursion problem */
    imove();
    Time = 10.0*dist/16.0;
    return;
}
	
	
void snova(int insx, int insy) 
{
    int comdead, nqx=0, nqy=0, nsx, nsy, num, kldead, iscdead;
    int nrmdead, npdead;
    int incipient=0;

    nsx = insy;
    nsy = insy;

    if (insy== 0) {
	if (insx == 1) {
	    /* NOVAMAX being used */
	    nqx = probecx;
	    nqy = probecy;
	}
	else {
	    int stars = 0;
	    /* Scheduled supernova -- select star */
	    /* logic changed here so that we won't favor quadrants in top
	       left of universe */
	    for (nqx = 1; nqx<=GALSIZE; nqx++) {
		for (nqy = 1; nqy<=GALSIZE; nqy++) {
		    stars += STARS(game.state.galaxy[nqx][nqy]);
		}
	    }
	    if (stars == 0) return; /* nothing to supernova exists */
	    num = Rand()*stars + 1;
	    for (nqx = 1; nqx<=GALSIZE; nqx++) {
		for (nqy = 1; nqy<=GALSIZE; nqy++) {
		    num -= STARS(game.state.galaxy[nqx][nqy]);
		    if (num <= 0) break;
		}
		if (num <=0) break;
	    }
#ifdef DEBUG
	    if (idebug) {
		proutn("Super nova here?");
		if (ja()==1) {
		    nqx = quadx;
		    nqy = quady;
		}
	    }
#endif
	}

	if (nqx != quady || nqy != quady || justin != 0) {
	    /* it isn't here, or we just entered (treat as inroute) */
	    if (game.damage[DRADIO] == 0.0 || condit == IHDOCKED) {
		skip(1);
		prout("Message from Starfleet Command       Stardate %.2f", game.state.date);
		prout("     Supernova in %s; caution advised.",
		      cramlc(quadrant, nqx, nqy));
	    }
	}
	else {
	    /* we are in the quadrant! */
	    incipient = 1;
	    num = Rand()* STARS(game.state.galaxy[nqx][nqy]) + 1;
	    for (nsx=1; nsx < QUADSIZE; nsx++) {
		for (nsy=1; nsy < QUADSIZE; nsy++) {
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
	nqx = quadx;
	nqy = quady;
	if (square(nsx-sectx) + square(nsy-secty) <= 2.1) {
	    proutn("Emergency override attempts t");
	    prouts("***************");
	    skip(1);
	    stars();
	    alldone=1;
	}
    }
    /* destroy any Klingons in supernovaed quadrant */
    num=game.state.galaxy[nqx][nqy];
    kldead = num/100;
    comdead = iscdead = 0;
    if (nqx==game.state.isx && nqy == game.state.isy) {
	/* did in the Supercommander! */
	game.state.nscrem = game.state.isx = game.state.isy = isatb = iscate = 0;
	iscdead = 1;
	game.future[FSCMOVE] = game.future[FSCDBAS] = 1e30;
    }
    game.state.remkl -= kldead;
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
		if (game.state.remcom==0) game.future[FTBEAM] = 1e30;
		break;
	    }
	}
    }
    /* destroy Romulans and planets in supernovaed quadrant */
    num = game.state.newstuf[nqx][nqy];
    game.state.newstuf[nqx][nqy] = 0;
    nrmdead = num/10;
    game.state.nromrem -= nrmdead;
    npdead = num - nrmdead*10;
    if (npdead) {
	int l;
	for (l = 0; l < inplan; l++)
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
	num = game.state.galaxy[nqx][nqy] % 100;
	game.state.starkl += num % 10;
	game.state.basekl += num/10;
	game.state.killk += kldead;
	game.state.killc += comdead;
	game.state.nromkl += nrmdead;
	game.state.nplankl += npdead;
	game.state.nsckill += iscdead;
    }
    /* mark supernova in galaxy and in star chart */
    if ((quadx == nqx && quady == nqy) ||
	game.damage[DRADIO] == 0 ||
	condit == IHDOCKED)
	game.starch[nqx][nqy] = 1;
    game.state.galaxy[nqx][nqy] = SUPERNOVA_PLACE;
    /* If supernova destroys last klingons give special message */
    if (game.state.remkl==0 && (nqx != quadx || nqy != quady)) {
	skip(2);
	if (insx == 0) prout("Lucky you!");
	proutn("A supernova in %s has just destroyed the last Klingons.",
	       cramlc(quadrant, nqx, nqy));
	finish(FWON);
	return;
    }
    /* if some Klingons remain, continue or die in supernova */
    if (alldone) finish(FSNOVAED);
    return;
}
		
				
