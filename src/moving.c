#include <unistd.h>
#include "sstlinux.h"
#include "sst.h"

static void getcd(int, int);

void imove(void) 
{
    double angle, deltax, deltay, bigger, x, y,
        finald, finalx, finaly, stopegy, probf;
    int trbeam = 0, n, l, kink, kinks, iquad;
    coord w;

    w.x = w.y = 0;
    if (game.inorbit) {
	prout("Helmsman Sulu- \"Leaving standard orbit.\"");
	game.inorbit = FALSE;
    }

    angle = ((15.0 - game.direc) * 0.5235988);
    deltax = -sin(angle);
    deltay = cos(angle);
    if (fabs(deltax) > fabs(deltay))
	bigger = fabs(deltax);
    else
	bigger = fabs(deltay);
		
    deltay /= bigger;
    deltax /= bigger;

    /* If tractor beam is to occur, don't move full distance */
    if (game.state.date+game.optime >= scheduled(FTBEAM)) {
	trbeam = 1;
	game.condit = IHRED;
	game.dist = game.dist*(scheduled(FTBEAM)-game.state.date)/game.optime + 0.1;
	game.optime = scheduled(FTBEAM) - game.state.date + 1e-5;
    }
    /* Move within the quadrant */
    game.quad[game.sector.x][game.sector.y] = IHDOT;
    x = game.sector.x;
    y = game.sector.y;
    n = 10.0*game.dist*bigger+0.5;

    if (n > 0) {
	for (l = 1; l <= n; l++) {
	    w.x = (x += deltax) + 0.5;
	    w.y = (y += deltay) + 0.5;
	    if (!VALID_SECTOR(w.x, w.y)) {
		/* Leaving quadrant -- allow final enemy attack */
		/* Don't do it if being pushed by Nova */
		if (game.nenhere != 0 && game.iattak != 2) {
		    newcnd();
		    for_local_enemies(l) {
			finald = sqrt((w.x-game.ks[l].x)*(double)(w.x-game.ks[l].x) +
				      (w.y-game.ks[l].y)*(double)(w.y-game.ks[l].y));
			game.kavgd[l] = 0.5 * (finald+game.kdist[l]);
		    }
		    /*
		     * Stas Sergeev added the game.condition
		     * that attacks only happen if Klingons
		     * are present and your skill is good.
		     */
		    if (game.skill > SKILL_GOOD && game.klhere > 0 && !game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova)
			attack(0);
		    if (game.alldone) return;
		}
		/* compute final position -- new quadrant and sector */
		x = QUADSIZE*(game.quadrant.x-1)+game.sector.x;
		y = QUADSIZE*(game.quadrant.y-1)+game.sector.y;
		w.x = x+10.0*game.dist*bigger*deltax+0.5;
		w.y = y+10.0*game.dist*bigger*deltay+0.5;
		/* check for edge of galaxy */
		kinks = 0;
		do {
		    kink = 0;
		    if (w.x <= 0) {
			w.x = -w.x + 1;
			kink = 1;
		    }
		    if (w.y <= 0) {
			w.y = -w.y + 1;
			kink = 1;
		    }
		    if (w.x > GALSIZE*QUADSIZE) {
			w.x = (GALSIZE*QUADSIZE*2)+1 - w.x;
			kink = 1;
		    }
		    if (w.y > GALSIZE*QUADSIZE) {
			w.y = (GALSIZE*QUADSIZE*2)+1 - w.y;
			kink = 1;
		    }
		    if (kink) kinks = 1;
		} while (kink);

		if (kinks) {
		    game.nkinks += 1;
		    if (game.nkinks == 3) {
			/* Three strikes -- you're out! */
			finish(FNEG3);
			return;
		    }
		    skip(1);
		    prout("YOU HAVE ATTEMPTED TO CROSS THE NEGATIVE ENERGY BARRIER");
		    prout("AT THE EDGE OF THE GALAXY.  THE THIRD TIME YOU TRY THIS,");
		    prout("YOU WILL BE DESTROYED.");
		}
		/* Compute final position in new quadrant */
		if (trbeam) return; /* Don't bother if we are to be beamed */
		game.quadrant.x = (w.x+(QUADSIZE-1))/QUADSIZE;
		game.quadrant.y = (w.y+(QUADSIZE-1))/QUADSIZE;
		game.sector.x = w.x - QUADSIZE*(game.quadrant.x-1);
		game.sector.y = w.y - QUADSIZE*(game.quadrant.y-1);
		skip(1);
		prout("Entering %s.", cramlc(quadrant, game.quadrant));
		game.quad[game.sector.x][game.sector.y] = game.ship;
		newqad(0);
		if (game.skill>SKILL_NOVICE) attack(0);
		return;
	    }
	    iquad = game.quad[w.x][w.y];
	    if (iquad != IHDOT) {
		/* object encountered in flight path */
		stopegy = 50.0*game.dist/game.optime;
		game.dist=0.1*sqrt((game.sector.x-w.x)*(double)(game.sector.x-w.x) +
			      (game.sector.y-w.y)*(double)(game.sector.y-w.y));
		switch (iquad) {
		case IHT: /* Ram a Tholian */
		case IHK: /* Ram enemy ship */
		case IHC:
		case IHS:
		case IHR:
		case IHQUEST:
		    game.sector.x = w.x;
		    game.sector.y = w.y;
		    ram(0, iquad, game.sector);
		    finalx = game.sector.x;
		    finaly = game.sector.y;
		    break;
		case IHBLANK:
		    skip(1);
		    prouts("***RED ALERT!  RED ALERT!");
		    skip(1);
		    proutn("***");
		    crmshp();
		    proutn(" pulled into black hole at ");
		    prout(cramlc(sector, w));
		    /*
		     * Getting pulled into a black hole was certain
		     * death in Almy's original.  Stas Sergeev added a
		     * possibility that you'll get timewarped instead.
		     */
		    n=0;
		    for (l=0;l<NDEVICES;l++)
			if (game.damage[l]>0) 
			    n++;
		    probf=pow(1.4,(game.energy+game.shield)/5000.0-1.0)*pow(1.3,1.0/(n+1)-1.0);
		    if ((game.options & OPTION_BLKHOLE) && Rand()>probf) 
			timwrp();
		    else 
			finish(FHOLE);
		    return;
		default:
		    /* something else */
		    skip(1);
		    crmshp();
		    if (iquad == IHWEB)
			proutn(" encounters Tholian web at ");
		    else
			proutn(" blocked by object at ");
		    proutn(cramlc(sector, w));
		    prout(";");
		    proutn("Emergency stop required ");
		    prout("%2d units of energy.", (int)stopegy);
		    game.energy -= stopegy;
		    finalx = x-deltax+0.5;
		    game.sector.x = finalx;
		    finaly = y-deltay+0.5;
		    game.sector.y = finaly;
		    if (game.energy <= 0) {
			finish(FNRG);
			return;
		    }
		    break;
		}
		goto no_quad_change;	/* sorry! */
	    }
	}
	game.dist = 0.1*sqrt((game.sector.x-w.x)*(double)(game.sector.x-w.x) +
			(game.sector.y-w.y)*(double)(game.sector.y-w.y));
	game.sector.x = w.x;
	game.sector.y = w.y;
    }
    finalx = game.sector.x;
    finaly = game.sector.y;
no_quad_change:
    /* No quadrant change -- compute new avg enemy distances */
    game.quad[game.sector.x][game.sector.y] = game.ship;
    if (game.nenhere) {
	for_local_enemies(l) {
	    finald = sqrt((w.x-game.ks[l].x)*(double)(w.x-game.ks[l].x) +
			  (w.y-game.ks[l].y)*(double)(w.y-game.ks[l].y));
	    game.kavgd[l] = 0.5 * (finald+game.kdist[l]);
	    game.kdist[l] = finald;
	}
	sortkl();
	if (!game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova && game.iattak == 0)
	    attack(0);
	for_local_enemies(l) game.kavgd[l] = game.kdist[l];
    }
    newcnd();
    game.iattak = 0;
    drawmaps(0);
    setwnd(message_window);
    return;
}

void dock(int l) 
{
    chew();
    if (game.condit == IHDOCKED && l) {
	prout("Already docked.");
	return;
    }
    if (game.inorbit) {
	prout("You must first leave standard orbit.");
	return;
    }
    if (game.base.x==0 || abs(game.sector.x-game.base.x) > 1 || abs(game.sector.y-game.base.y) > 1) {
	crmshp();
	prout(" not adjacent to base.");
	return;
    }
    game.condit = IHDOCKED;
    if (l) prout("Docked.");
    game.ididit=1;
    if (game.energy < game.inenrg) game.energy = game.inenrg;
    game.shield = game.inshld;
    game.torps = game.intorps;
    game.lsupres = game.inlsr;
    if (game.damage[DRADIO] == 0.0 &&
	(is_scheduled(FCDBAS) || game.isatb == 1) && game.iseenit == 0) {
	/* get attack report from base */
	prout("Lt. Uhura- \"Captain, an important message from the starbase:\"");
	attakreport(0);
	game.iseenit = 1;
    }
}

static void getcd(int isprobe, int akey) {
	/* This program originally required input in terms of a (clock)
	   direction and distance. Somewhere in history, it was changed to
	   cartesian coordinates. So we need to convert. I think
	   "manual" input should still be done this way -- it's a real
	   pain if the computer isn't working! Manual mode is still confusing
	   because it involves giving x and y motions, yet the coordinates
	   are always displayed y - x, where +y is downward! */

	
        int irowq=game.quadrant.x, icolq=game.quadrant.y, itemp=0, iprompt=0, key=0;
	double xi, xj, xk, xl;
	double deltax, deltay;
	int automatic = -1;
	coord incr;

	/* Get course direction and distance. If user types bad values, return
	   with DIREC = -1.0. */

	game.direc = -1.0;
	
	if (game.landed == 1 && !isprobe) {
		prout("Dummy! You can't leave standard orbit until you");
		proutn("are back aboard the ");
		crmshp();
		prout(".");
		chew();
		return;
	}
	while (automatic == -1) {
		if (game.damage[DCOMPTR]) {
			if (isprobe)
				prout("Computer damaged; manual navigation only");
			else
				prout("Computer damaged; manual movement only");
			chew();
			automatic = 0;
			key = IHEOL;
			break;
		}
		if (isprobe && akey != -1) {
			/* For probe launch, use pre-scaned value first time */
			key = akey;
			akey = -1;
		}
		else 
			key = scan();

		if (key == IHEOL) {
			proutn("Manual or automatic- ");
			iprompt = 1;
			chew();
		}
		else if (key == IHALPHA) {
			if (isit("manual")) {
				automatic =0;
				key = scan();
				break;
			}
			else if (isit("automatic")) {
				automatic = 1;
				key = scan();
				break;
			}
			else {
				huh();
				chew();
				return;
			}
		}
		else { /* numeric */
			if (isprobe)
				prout("(Manual navigation assumed.)");
			else
				prout("(Manual movement assumed.)");
			automatic = 0;
			break;
		}
	}

	if (automatic) {
		while (key == IHEOL) {
			if (isprobe)
				proutn("Target quadrant or quadrant&sector- ");
			else
				proutn("Destination sector or quadrant&sector- ");
			chew();
			iprompt = 1;
			key = scan();
		}

		if (key != IHREAL) {
			huh();
			return;
		}
		xi = aaitem;
		key = scan();
		if (key != IHREAL){
			huh();
			return;
		}
		xj = aaitem;
		key = scan();
		if (key == IHREAL) {
			/* both quadrant and sector specified */
			xk = aaitem;
			key = scan();
			if (key != IHREAL) {
				huh();
				return;
			}
			xl = aaitem;

			irowq = xi + 0.5;
			icolq = xj + 0.5;
			incr.y = xk + 0.5;
			incr.x = xl + 0.5;
		}
		else {
			if (isprobe) {
				/* only quadrant specified -- go to center of dest quad */
				irowq = xi + 0.5;
				icolq = xj + 0.5;
				incr.y = incr.x = 5;
			}
			else {
				incr.y = xi + 0.5;
				incr.x = xj + 0.5;
			}
			itemp = 1;
		}
		if (!VALID_QUADRANT(icolq,irowq)||!VALID_SECTOR(incr.x,incr.y)) {
		    huh();
		    return;
		}
		skip(1);
		if (!isprobe) {
			if (itemp) {
				if (iprompt) {
					prout("Helmsman Sulu- \"Course locked in for %s.\"",
						cramlc(sector, incr));
				}
			}
			else prout("Ensign Chekov- \"Course laid in, Captain.\"");
		}
		deltax = icolq - game.quadrant.y + 0.1*(incr.x-game.sector.y);
		deltay = game.quadrant.x - irowq + 0.1*(game.sector.x-incr.y);
	}
	else { /* manual */
		while (key == IHEOL) {
			proutn("X and Y displacements- ");
			chew();
			iprompt = 1;
			key = scan();
		}
		itemp = 2;
		if (key != IHREAL) {
			huh();
			return;
		}
		deltax = aaitem;
		key = scan();
		if (key != IHREAL) {
			huh();
			return;
		}
		deltay = aaitem;
	}
	/* Check for zero movement */
	if (deltax == 0 && deltay == 0) {
		chew();
		return;
	}
	if (itemp == 2 && !isprobe) {
		skip(1);
		prout("Helmsman Sulu- \"Aye, Sir.\"");
	}
	game.dist = sqrt(deltax*deltax + deltay*deltay);
	game.direc = atan2(deltax, deltay)*1.90985932;
	if (game.direc < 0.0) game.direc += 12.0;
	chew();
	return;

}
		


void impuls(void) 
{
    double power;

    game.ididit = 0;
    if (game.damage[DIMPULS]) {
	chew();
	skip(1);
	prout("Engineer Scott- \"The impulse engines are damaged, Sir.\"");
	return;
    }

    if (game.energy > 30.0) {
	getcd(FALSE, 0);
	if (game.direc == -1.0) return;
	power = 20.0 + 100.0*game.dist;
    }
    else
	power = 30.0;

    if (power >= game.energy) {
	/* Insufficient power for trip */
	skip(1);
	prout("First Officer Spock- \"Captain, the impulse engines");
	prout("require 20.0 units to engage, plus 100.0 units per");
	if (game.energy > 30) {
	    proutn("quadrant.  We can go, therefore, a maximum of %d", 
		   (int)(0.01 * (game.energy-20.0)-0.05));
	    prout(" quadrants.\"");
	}
	else {
	    prout("quadrant.  They are, therefore, useless.\"");
	}
	chew();
	return;
    }
    /* Make sure enough time is left for the trip */
    game.optime = game.dist/0.095;
    if (game.optime >= game.state.remtime) {
	prout("First Officer Spock- \"Captain, our speed under impulse");
	prout("power is only 0.95 sectors per stardate. Are you sure");
	proutn("we dare spend the time?\" ");
	if (ja() == 0) return;
    }
    /* Activate impulse engines and pay the cost */
    imove();
    game.ididit = 1;
    if (game.alldone) return;
    power = 20.0 + 100.0*game.dist;
    game.energy -= power;
    game.optime = game.dist/0.095;
    if (game.energy <= 0) finish(FNRG);
    return;
}


void warp(int i) 
{
    int blooey=0, twarp=0, iwarp;
    double power;

    if (i!=2) { /* Not WARPX entry */
	game.ididit = 0;
	if (game.damage[DWARPEN] > 10.0) {
	    chew();
	    skip(1);
	    prout("Engineer Scott- \"The impulse engines are damaged, Sir.\"");
	    return;
	}
	if (game.damage[DWARPEN] > 0.0 && game.warpfac > 4.0) {
	    chew();
	    skip(1);
	    prout("Engineer Scott- \"Sorry, Captain. Until this damage");
	    prout("  is repaired, I can only give you warp 4.\"");
	    return;
	}
			
	/* Read in course and distance */
	getcd(FALSE, 0);
	if (game.direc == -1.0) return;

	/* Make sure starship has enough energy for the trip */
	power = (game.dist+0.05)*game.warpfac*game.warpfac*game.warpfac*(game.shldup+1);


	if (power >= game.energy) {
	    /* Insufficient power for trip */
	    game.ididit = 0;
	    skip(1);
	    prout("Engineering to bridge--");
	    if (game.shldup==0 || 0.5*power > game.energy) {
		iwarp = pow((game.energy/(game.dist+0.05)), 0.333333333);
		if (iwarp <= 0) {
		    prout("We can't do it, Captain. We haven't the energy.");
		}
		else {
		    proutn("We haven't the energy, but we could do it at warp %d", iwarp);
		    if (game.shldup) {
			prout(",");
			prout("if you'll lower the shields.");
		    }
		    else
			prout(".");
		}
	    }
	    else
		prout("We haven't the energy to go that far with the shields up.");
	    return;
	}
						
	/* Make sure enough time is left for the trip */
	game.optime = 10.0*game.dist/game.wfacsq;
	if (game.optime >= 0.8*game.state.remtime) {
	    skip(1);
	    prout("First Officer Spock- \"Captain, I compute that such");
	    proutn("  a trip would require approximately %2.0f",
		   100.0*game.optime/game.state.remtime);
	    prout(" percent of our");
	    proutn("  remaining time.  Are you sure this is wise?\" ");
	    if (ja() == 0) { game.ididit = 0; game.optime=0; return;}
	}
    }
    /* Entry WARPX */
    if (game.warpfac > 6.0) {
	/* Decide if engine damage will occur */
	double prob = game.dist*(6.0-game.warpfac)*(6.0-game.warpfac)/66.666666666;
	if (prob > Rand()) {
	    blooey = 1;
	    game.dist = Rand()*game.dist;
	}
	/* Decide if time warp will occur */
	if (0.5*game.dist*pow(7.0,game.warpfac-10.0) > Rand()) twarp=1;
#ifdef DEBUG
	if (game.idebug &&game.warpfac==10 && twarp==0) {
	    blooey=0;
	    proutn("Force time warp? ");
	    if (ja()==1) twarp=1;
	}
#endif
	if (blooey || twarp) {
	    /* If time warp or engine damage, check path */
	    /* If it is obstructed, don't do warp or damage */
	    double angle = ((15.0-game.direc)*0.5235998);
	    double deltax = -sin(angle);
	    double deltay = cos(angle);
	    double bigger, x, y;
	    int n, l, ix, iy;
	    if (fabs(deltax) > fabs(deltay))
		bigger = fabs(deltax);
	    else
		bigger = fabs(deltay);
			
	    deltax /= bigger;
	    deltay /= bigger;
	    n = 10.0 * game.dist * bigger +0.5;
	    x = game.sector.x;
	    y = game.sector.y;
	    for (l = 1; l <= n; l++) {
		x += deltax;
		ix = x + 0.5;
		y += deltay;
		iy = y +0.5;
		if (!VALID_SECTOR(ix, iy)) break;
		if (game.quad[ix][iy] != IHDOT) {
		    blooey = 0;
		    twarp = 0;
		}
	    }
	}
    }
				

    /* Activate Warp Engines and pay the cost */
    imove();
    if (game.alldone) return;
    game.energy -= game.dist*game.warpfac*game.warpfac*game.warpfac*(game.shldup+1);
    if (game.energy <= 0) finish(FNRG);
    game.optime = 10.0*game.dist/game.wfacsq;
    if (twarp) timwrp();
    if (blooey) {
	game.damage[DWARPEN] = game.damfac*(3.0*Rand()+1.0);
	skip(1);
	prout("Engineering to bridge--");
	prout("  Scott here.  The warp engines are damaged.");
	prout("  We'll have to reduce speed to warp 4.");
    }
    game.ididit = 1;
    return;
}



void setwrp(void) 
{
    int key;
    double oldfac;
	
    while ((key=scan()) == IHEOL) {
	chew();
	proutn("Warp factor- ");
    }
    chew();
    if (key != IHREAL) {
	huh();
	return;
    }
    if (game.damage[DWARPEN] > 10.0) {
	prout("Warp engines inoperative.");
	return;
    }
    if (game.damage[DWARPEN] > 0.0 && aaitem > 4.0) {
	prout("Engineer Scott- \"I'm doing my best, Captain,");
	prout("  but right now we can only go warp 4.\"");
	return;
    }
    if (aaitem > 10.0) {
	prout("Helmsman Sulu- \"Our top speed is warp 10, Captain.\"");
	return;
    }
    if (aaitem < 1.0) {
	prout("Helmsman Sulu- \"We can't go below warp 1, Captain.\"");
	return;
    }
    oldfac = game.warpfac;
    game.warpfac = aaitem;
    game.wfacsq=game.warpfac*game.warpfac;
    if (game.warpfac <= oldfac || game.warpfac <= 6.0) {
	proutn("Helmsman Sulu- \"Warp factor %d, Captain.\"", 
	       (int)game.warpfac);
	return;
    }
    if (game.warpfac < 8.00) {
	prout("Engineer Scott- \"Aye, but our maximum safe speed is warp 6.\"");
	return;
    }
    if (game.warpfac == 10.0) {
	prout("Engineer Scott- \"Aye, Captain, we'll try it.\"");
	return;
    }
    prout("Engineer Scott- \"Aye, Captain, but our engines may not take it.\"");
    return;
}

void atover(int igrab) 
{
    double power, distreq;

    chew();
    /* is captain on planet? */
    if (game.landed==1) {
	if (game.damage[DTRANSP]) {
	    finish(FPNOVA);
	    return;
	}
	prout("Scotty rushes to the transporter controls.");
	if (game.shldup) {
	    prout("But with the shields up it's hopeless.");
	    finish(FPNOVA);
	}
	prouts("His desperate attempt to rescue you . . .");
	if (Rand() <= 0.5) {
	    prout("fails.");
	    finish(FPNOVA);
	    return;
	}
	prout("SUCCEEDS!");
	if (game.imine) {
	    game.imine = 0;
	    proutn("The crystals mined were ");
	    if (Rand() <= 0.25) {
		prout("lost.");
	    }
	    else {
		prout("saved.");
		game.icrystl = 1;
	    }
	}
    }
    if (igrab) return;

    /* Check to see if captain in shuttle craft */
    if (game.icraft) finish(FSTRACTOR);
    if (game.alldone) return;

    /* Inform captain of attempt to reach safety */
    skip(1);
    do {
	if (game.justin) {
	    prouts("***RED ALERT!  READ ALERT!");
	    skip(1);
	    proutn("The ");
	    crmshp();
	    prout(" has stopped in a quadrant containing");
	    prouts("   a supernova.");
	    skip(2);
	}
	proutn("***Emergency automatic override attempts to hurl ");
	crmshp();
	skip(1);
	prout("safely out of quadrant.");
	if (game.damage[DRADIO] == 0.0)
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].charted = TRUE;
	/* Try to use warp engines */
	if (game.damage[DWARPEN]) {
	    skip(1);
	    prout("Warp engines damaged.");
	    finish(FSNOVAED);
	    return;
	}
	game.warpfac = 6.0+2.0*Rand();
	game.wfacsq = game.warpfac * game.warpfac;
	prout("Warp factor set to %d", (int)game.warpfac);
	power = 0.75*game.energy;
	game.dist = power/(game.warpfac*game.warpfac*game.warpfac*(game.shldup+1));
	distreq = 1.4142+Rand();
	if (distreq < game.dist) game.dist = distreq;
	game.optime = 10.0*game.dist/game.wfacsq;
	game.direc = 12.0*Rand();	/* How dumb! */
	game.justin = 0;
	game.inorbit = 0;
	warp(2);
	if (game.justin == 0) {
	    /* This is bad news, we didn't leave quadrant. */
	    if (game.alldone) return;
	    skip(1);
	    prout("Insufficient energy to leave quadrant.");
	    finish(FSNOVAED);
	    return;
	}
    } while 
	/* Repeat if another snova */
	(game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova);
    if (KLINGREM==0) 
	finish(FWON); /* Snova killed remaining enemy. */
}

void timwrp() 
{
    int l, gotit;
    prout("***TIME WARP ENTERED.");
    if (game.state.snap && Rand() < 0.5) {
	/* Go back in time */
	prout("You are traveling backwards in time %d stardates.",
	      (int)(game.state.date-game.snapsht.date));
	game.state = game.snapsht;
	game.state.snap = 0;
	if (game.state.remcom) {
	    schedule(FTBEAM, expran(game.intime/game.state.remcom));
	    schedule(FBATTAK, expran(0.3*game.intime));
	}
	schedule(FSNOVA, expran(0.5*game.intime));
	/* next snapshot will be sooner */
	schedule(FSNAP, expran(0.25*game.state.remtime));
				
	if (game.state.nscrem) schedule(FSCMOVE, 0.2777);
	game.isatb = 0;
	unschedule(FCDBAS);
	unschedule(FSCDBAS);
	game.battle.x = game.battle.y = 0;

	/* Make sure Galileo is consistant -- Snapshot may have been taken
	   when on planet, which would give us two Galileos! */
	gotit = 0;
	for (l = 0; l < game.inplan; l++) {
	    if (game.state.plnets[l].known == shuttle_down) {
		gotit = 1;
		if (game.iscraft==1 && game.ship==IHE) {
		    prout("Checkov-  \"Security reports the Galileo has disappeared, Sir!");
		    game.iscraft = 0;
		}
	    }
	}
	/* Likewise, if in the original time the Galileo was abandoned, but
	   was on ship earlier, it would have vanished -- lets restore it */
	if (game.iscraft==0 && gotit==0 && game.damage[DSHUTTL] >= 0.0) {
	    prout("Checkov-  \"Security reports the Galileo has reappeared in the dock!\"");
	    game.iscraft = 1;
	}
	/* 
	 * There used to be code to do the actual reconstrction here,
	 * but the starchart is now part of the snapshotted galaxy state.
	 */
	prout("Spock has reconstructed a correct star chart from memory");
    }
    else {
	/* Go forward in time */
	game.optime = -0.5*game.intime*log(Rand());
	prout("You are traveling forward in time %d stardates.", (int)game.optime);
	/* cheat to make sure no tractor beams occur during time warp */
	postpone(FTBEAM, game.optime);
	game.damage[DRADIO] += game.optime;
    }
    newqad(0);
    events();	/* Stas Sergeev added this -- do pending events */
}

void probe(void) 
{
    double angle, bigger;
    int key;
    /* New code to launch a deep space probe */
    if (game.nprobes == 0) {
	chew();
	skip(1);
	if (game.ship == IHE) 
	    prout("Engineer Scott- \"We have no more deep space probes, Sir.\"");
	else
	    prout("Ye Faerie Queene has no deep space probes.");
	return;
    }
    if (game.damage[DDSP] != 0.0) {
	chew();
	skip(1);
	prout("Engineer Scott- \"The probe launcher is damaged, Sir.\"");
	return;
    }
    if (is_scheduled(FDSPROB)) {
	chew();
	skip(1);
	if (game.damage[DRADIO] != 0 && game.condit != IHDOCKED) {
	    prout("Spock-  \"Records show the previous probe has not yet");
	    prout("   reached its destination.\"");
	}
	else
	    prout("Uhura- \"The previous probe is still reporting data, Sir.\"");
	return;
    }
    key = scan();

    if (key == IHEOL) {
	/* slow mode, so let Kirk know how many probes there are left */
	prout(game.nprobes==1 ? "%d probe left." : "%d probes left.", game.nprobes);
	proutn("Are you sure you want to fire a probe? ");
	if (ja()==0) return;
    }

    game.isarmed = FALSE;
    if (key == IHALPHA && strcmp(citem,"armed") == 0) {
	game.isarmed = TRUE;
	key = scan();
    }
    else if (key == IHEOL) {
	proutn("Arm NOVAMAX warhead? ");
	game.isarmed = ja();
    }
    getcd(TRUE, key);
    if (game.direc == -1.0) return;
    game.nprobes--;
    angle = ((15.0 - game.direc) * 0.5235988);
    game.probeinx = -sin(angle);
    game.probeiny = cos(angle);
    if (fabs(game.probeinx) > fabs(game.probeiny))
	bigger = fabs(game.probeinx);
    else
	bigger = fabs(game.probeiny);
		
    game.probeiny /= bigger;
    game.probeinx /= bigger;
    game.proben = 10.0*game.dist*bigger +0.5;
    game.probex = game.quadrant.x*QUADSIZE + game.sector.x - 1;	// We will use better packing than original
    game.probey = game.quadrant.y*QUADSIZE + game.sector.y - 1;
    game.probec = game.quadrant;
    schedule(FDSPROB, 0.01); // Time to move one sector
    prout("Ensign Chekov-  \"The deep space probe is launched, Captain.\"");
    game.ididit = 1;
    return;
}

void mayday(void) 
{
    /* There's more than one way to move in this game! */
    double ddist, xdist, probf;
    int line = 0, l, ix, iy;

    chew();
    /* Test for game.conditions which prevent calling for help */
    if (game.condit == IHDOCKED) {
	prout("Lt. Uhura-  \"But Captain, we're already docked.\"");
	return;
    }
    if (game.damage[DRADIO] != 0) {
	prout("Subspace radio damaged.");
	return;
    }
    if (game.state.rembase==0) {
	prout("Lt. Uhura-  \"Captain, I'm not getting any response from Starbase.\"");
	return;
    }
    if (game.landed == 1) {
	proutn("You must be aboard the ");
	crmshp();
	prout(".");
	return;
    }
    /* OK -- call for help from nearest starbase */
    game.nhelp++;
    if (game.base.x!=0) {
	/* There's one in this quadrant */
	ddist = sqrt(square(game.base.x-game.sector.x)+square(game.base.y-game.sector.y));
    }
    else {
	ddist = FOREVER;
	for_starbases(l) {
	    xdist=10.0*sqrt(square(game.state.baseq[l].x-game.quadrant.x)+square(game.state.baseq[l].y-game.quadrant.y));
	    if (xdist < ddist) {
		ddist = xdist;
		line = l;
	    }
	}
	/* Since starbase not in quadrant, set up new quadrant */
	game.quadrant.x = game.state.baseq[line].x;
	game.quadrant.y = game.state.baseq[line].y;
	newqad(1);
    }
    /* dematerialize starship */
    game.quad[game.sector.x][game.sector.y]=IHDOT;
    proutn("Starbase in %s responds--", cramlc(quadrant, game.quadrant));
    proutn("");
    crmshp();
    prout(" dematerializes.");
    game.sector.x=0;
    for (l = 1; l <= 5; l++) {
	ix = game.base.x+3.0*Rand()-1;
	iy = game.base.y+3.0*Rand()-1;
	if (VALID_SECTOR(ix,iy) && game.quad[ix][iy]==IHDOT) {
	    /* found one -- finish up */
	    game.sector.x=ix;
	    game.sector.y=iy;
	    break;
	}
    }
    if (game.sector.x==0){
	prout("You have been lost in space...");
	finish(FMATERIALIZE);
	return;
    }
    /* Give starbase three chances to rematerialize starship */
    probf = pow((1.0 - pow(0.98,ddist)), 0.33333333);
    for (l = 1; l <= 3; l++) {
	switch (l) {
	case 1: proutn("1st"); break;
	case 2: proutn("2nd"); break;
	case 3: proutn("3rd"); break;
	}
	proutn(" attempt to re-materialize ");
	crmshp();
	switch (l){
	case 1: game.quad[ix][iy]=IHMATER0;
	    break;
	case 2: game.quad[ix][iy]=IHMATER1;
	    break;
	case 3: game.quad[ix][iy]=IHMATER2;
	    break;
	}
	textcolor(RED);
	warble();
	if (Rand() > probf) break;
	prout("fails.");
	delay(500);
	textcolor(DEFAULT);
    }
    if (l > 3) {
	game.quad[ix][iy]=IHQUEST;
	game.alive = 0;
	drawmaps(1);
	setwnd(message_window);
	finish(FMATERIALIZE);
	return;
    }
    game.quad[ix][iy]=game.ship;
    textcolor(GREEN);
    prout("succeeds.");
    textcolor(DEFAULT);
    dock(0);
    skip(1);
    prout("Lt. Uhura-  \"Captain, we made it!\"");
}
