#include <unistd.h>
#include "sstlinux.h"
#include "sst.h"

static void getcd(int, int);

void imove(void) 
{
    double angle, deltax, deltay, bigger, x, y,
        finald, finalx, finaly, stopegy, probf;
    int trbeam = 0, n, l, ix=0, iy=0, kink, kinks, iquad;

    if (inorbit) {
	prout("Helmsman Sulu- \"Leaving standard orbit.\"");
	inorbit = FALSE;
    }

    angle = ((15.0 - direc) * 0.5235988);
    deltax = -sin(angle);
    deltay = cos(angle);
    if (fabs(deltax) > fabs(deltay))
	bigger = fabs(deltax);
    else
	bigger = fabs(deltay);
		
    deltay /= bigger;
    deltax /= bigger;

    /* If tractor beam is to occur, don't move full distance */
    if (game.state.date+Time >= game.future[FTBEAM]) {
	trbeam = 1;
	condit = IHRED;
	dist = dist*(game.future[FTBEAM]-game.state.date)/Time + 0.1;
	Time = game.future[FTBEAM] - game.state.date + 1e-5;
    }
    /* Move within the quadrant */
    game.quad[sectx][secty] = IHDOT;
    x = sectx;
    y = secty;
    n = 10.0*dist*bigger+0.5;

    if (n > 0) {
	for (l = 1; l <= n; l++) {
	    ix = (x += deltax) + 0.5;
	    iy = (y += deltay) + 0.5;
	    if (ix < 1 || ix > QUADSIZE || iy < 1 || iy > QUADSIZE) {
		/* Leaving quadrant -- allow final enemy attack */
		/* Don't do it if being pushed by Nova */
		if (nenhere != 0 && iattak != 2) {
		    newcnd();
		    for (l = 1; l <= nenhere; l++) {
			finald = sqrt((ix-game.kx[l])*(double)(ix-game.kx[l]) +
				      (iy-game.ky[l])*(double)(iy-game.ky[l]));
			game.kavgd[l] = 0.5 * (finald+game.kdist[l]);
		    }
		    /*
		     * Stas Sergeev added the condition
		     * that attacks only happen if Klingons
		     * are present and your skill is good.
		     */
		    if (skill > SKILL_GOOD && klhere > 0 && game.state.galaxy[quadx][quady] != SUPERNOVA_PLACE)
			attack(0);
		    if (alldone) return;
		}
		/* compute final position -- new quadrant and sector */
		x = QUADSIZE*(quadx-1)+sectx;
		y = QUADSIZE*(quady-1)+secty;
		ix = x+10.0*dist*bigger*deltax+0.5;
		iy = y+10.0*dist*bigger*deltay+0.5;
		/* check for edge of galaxy */
		kinks = 0;
		do {
		    kink = 0;
		    if (ix <= 0) {
			ix = -ix + 1;
			kink = 1;
		    }
		    if (iy <= 0) {
			iy = -iy + 1;
			kink = 1;
		    }
		    if (ix > GALSIZE*QUADSIZE) {
			ix = (GALSIZE*QUADSIZE*2)+1 - ix;
			kink = 1;
		    }
		    if (iy > GALSIZE*QUADSIZE) {
			iy = (GALSIZE*QUADSIZE*2)+1 - iy;
			kink = 1;
		    }
		    if (kink) kinks = 1;
		} while (kink);

		if (kinks) {
		    nkinks += 1;
		    if (nkinks == 3) {
			/* Three strikes -- you're out! */
			finish(FNEG3);
			return;
		    }
		    prout("\n\rYOU HAVE ATTEMPTED TO CROSS THE NEGATIVE ENERGY BARRIER\n\r"
			  "AT THE EDGE OF THE GALAXY.  THE THIRD TIME YOU TRY THIS,\n\r"
			  "YOU WILL BE DESTROYED.\n\r");
		}
		/* Compute final position in new quadrant */
		if (trbeam) return; /* Don't bother if we are to be beamed */
		quadx = (ix+(QUADSIZE-1))/QUADSIZE;
		quady = (iy+(QUADSIZE-1))/QUADSIZE;
		sectx = ix - QUADSIZE*(quadx-1);
		secty = iy - QUADSIZE*(quady-1);
		prout("\n\rEntering %s.",
		      cramlc(quadrant, quadx, quady));
		game.quad[sectx][secty] = ship;
		newqad(0);
		if (skill>SKILL_NOVICE) attack(0);
		return;
	    }
	    iquad = game.quad[ix][iy];
	    if (iquad != IHDOT) {
		/* object encountered in flight path */
		stopegy = 50.0*dist/Time;
		dist=0.1*sqrt((sectx-ix)*(double)(sectx-ix) +
			      (secty-iy)*(double)(secty-iy));
		switch (iquad) {
		case IHT: /* Ram a Tholian */
		case IHK: /* Ram enemy ship */
		case IHC:
		case IHS:
		case IHR:
		case IHQUEST:
		    sectx = ix;
		    secty = iy;
		    ram(0, iquad, sectx, secty);
		    finalx = sectx;
		    finaly = secty;
		    break;
		case IHBLANK:
		    skip(1);
		    prouts("***RED ALERT!  RED ALERT!");
		    skip(1);
		    proutn("***");
		    crmshp();
		    proutn(" pulled into black hole at ");
		    prout(cramlc(sector, ix, iy));
		    /*
		     * Getting pulled into a black hole was certain
		     * death in Almy's original.  Stas Sergeev added a
		     * possibility that you'll get timewarped instead.
		     */
		    n=0;
		    for (l=1;l<=NDEVICES+1;l++)
			if (game.damage[l]>0) n++;
		    probf=pow(1.4,(energy+shield)/5000.0-1.0)*
			pow(1.3,1.0/(n+1)-1.0);
		    if (Rand()>probf) 
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
		    proutn(cramlc(sector, ix,iy));
		    prout(";");
		    proutn("Emergency stop required ");
		    prout("%2d units of energy.", (int)stopegy);
		    energy -= stopegy;
		    finalx = x-deltax+0.5;
		    sectx = finalx;
		    finaly = y-deltay+0.5;
		    secty = finaly;
		    if (energy <= 0) {
			finish(FNRG);
			return;
		    }
		    break;
		}
		goto no_quad_change;	/* sorry! */
	    }
	}
	dist = 0.1*sqrt((sectx-ix)*(double)(sectx-ix) +
			(secty-iy)*(double)(secty-iy));
	sectx = ix;
	secty = iy;
    }
    finalx = sectx;
    finaly = secty;
no_quad_change:
    /* No quadrant change -- compute new avg enemy distances */
    game.quad[sectx][secty] = ship;
    if (nenhere) {
	for (l = 1; l <= nenhere; l++) {
	    finald = sqrt((ix-game.kx[l])*(double)(ix-game.kx[l]) +
			  (iy-game.ky[l])*(double)(iy-game.ky[l]));
	    game.kavgd[l] = 0.5 * (finald+game.kdist[l]);
	    game.kdist[l] = finald;
	}
	sortkl();
	if (game.state.galaxy[quadx][quady] != SUPERNOVA_PLACE && iattak == 0)
	    attack(0);
	for (l = 1 ; l <= nenhere; l++) game.kavgd[l] = game.kdist[l];
    }
    newcnd();
    iattak = 0;
    drawmaps(0);
    return;
}

void dock(int l) 
{
    chew();
    if (condit == IHDOCKED && l) {
	prout("Already docked.");
	return;
    }
    if (inorbit) {
	prout("You must first leave standard orbit.");
	return;
    }
    if (basex==0 || abs(sectx-basex) > 1 || abs(secty-basey) > 1) {
	crmshp();
	prout(" not adjacent to base.");
	return;
    }
    condit = IHDOCKED;
    if (l) prout("Docked.");
    ididit=1;
    if (energy < inenrg) energy = inenrg;
    shield = inshld;
    torps = intorps;
    lsupres = inlsr;
    if (stdamtim != 1e30 &&
	(game.future[FCDBAS] < 1e30 || isatb == 1) && iseenit == 0) {
	/* get attack report from base */
	prout("Lt. Uhura- \"Captain, an important message from the starbase:\"");
	attakreport(0);
	iseenit = 1;
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

	
        int irowq=quadx, icolq=quady, irows, icols, itemp=0, iprompt=0, key=0;
	double xi, xj, xk, xl;
	double deltax, deltay;
	int automatic = -1;

	/* Get course direction and distance. If user types bad values, return
	   with DIREC = -1.0. */

	direc = -1.0;
	
	if (landed == 1 && !isprobe) {
		prout("Dummy! You can't leave standard orbit until you");
		proutn("are back abourt the ");
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
			irows = xk + 0.5;
			icols = xl + 0.5;
		}
		else {
			if (isprobe) {
				/* only quadrant specified -- go to center of dest quad */
				irowq = xi + 0.5;
				icolq = xj + 0.5;
				irows = icols = 5;
			}
			else {
				irows = xi + 0.5;
				icols = xj + 0.5;
			}
			itemp = 1;
		}
		if (irowq<1 || irowq > GALSIZE || icolq<1 || icolq > GALSIZE ||
			irows<1 || irows > QUADSIZE || icols<1 || icols > QUADSIZE) {
				huh();
				return;
			}
		skip(1);
		if (!isprobe) {
			if (itemp) {
				if (iprompt) {
					prout("Helmsman Sulu- \"Course locked in for %s.\"",
						cramlc(sector, irows, icols));
				}
			}
			else prout("Ensign Chekov- \"Course laid in, Captain.\"");
		}
		deltax = icolq - quady + 0.1*(icols-secty);
		deltay = quadx - irowq + 0.1*(sectx-irows);
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
	dist = sqrt(deltax*deltax + deltay*deltay);
	direc = atan2(deltax, deltay)*1.90985932;
	if (direc < 0.0) direc += 12.0;
	chew();
	return;

}
		


void impuls(void) 
{
    double power;

    ididit = 0;
    if (game.damage[DIMPULS]) {
	chew();
	skip(1);
	prout("Engineer Scott- \"The impulse engines are damaged, Sir.\"");
	return;
    }

    if (energy > 30.0) {
	getcd(FALSE, 0);
	if (direc == -1.0) return;
	power = 20.0 + 100.0*dist;
    }
    else
	power = 30.0;

    if (power >= energy) {
	/* Insufficient power for trip */
	skip(1);
	prout("First Officer Spock- \"Captain, the impulse engines");
	prout("require 20.0 units to engage, plus 100.0 units per");
	if (energy > 30) {
	    proutn("quadrant.  We can go, therefore, a maximum of %d", 
		   (int)(0.01 * (energy-20.0)-0.05));
	    prout(" quadrants.\"");
	}
	else {
	    prout("quadrant.  They are, therefore, useless.\"");
	}
	chew();
	return;
    }
    /* Make sure enough time is left for the trip */
    Time = dist/0.095;
    if (Time >= game.state.remtime) {
	prout("First Officer Spock- \"Captain, our speed under impulse");
	prout("power is only 0.95 sectors per stardate. Are you sure");
	proutn("we dare spend the time?\" ");
	if (ja() == 0) return;
    }
    /* Activate impulse engines and pay the cost */
    imove();
    ididit = 1;
    if (alldone) return;
    power = 20.0 + 100.0*dist;
    energy -= power;
    Time = dist/0.095;
    if (energy <= 0) finish(FNRG);
    return;
}


void warp(int i) 
{
    int blooey=0, twarp=0, iwarp;
    double power;

    if (i!=2) { /* Not WARPX entry */
	ididit = 0;
	if (game.damage[DWARPEN] > 10.0) {
	    chew();
	    skip(1);
	    prout("Engineer Scott- \"The impulse engines are damaged, Sir.\"");
	    return;
	}
	if (game.damage[DWARPEN] > 0.0 && warpfac > 4.0) {
	    chew();
	    skip(1);
	    prout("Engineer Scott- \"Sorry, Captain. Until this damage");
	    prout("  is repaired, I can only give you warp 4.\"");
	    return;
	}
			
	/* Read in course and distance */
	getcd(FALSE, 0);
	if (direc == -1.0) return;

	/* Make sure starship has enough energy for the trip */
	power = (dist+0.05)*warpfac*warpfac*warpfac*(shldup+1);


	if (power >= energy) {
	    /* Insufficient power for trip */
	    ididit = 0;
	    skip(1);
	    prout("Engineering to bridge--");
	    if (shldup==0 || 0.5*power > energy) {
		iwarp = pow((energy/(dist+0.05)), 0.333333333);
		if (iwarp <= 0) {
		    prout("We can't do it, Captain. We haven't the energy.");
		}
		else {
		    proutn("We haven't the energy, but we could do it at warp %d", iwarp);
		    if (shldup) {
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
	Time = 10.0*dist/wfacsq;
	if (Time >= 0.8*game.state.remtime) {
	    skip(1);
	    prout("First Officer Spock- \"Captain, I compute that such");
	    proutn("  a trip would require approximately %2.0f",
		   100.0*Time/game.state.remtime);
	    prout(" percent of our");
	    proutn("  remaining time.  Are you sure this is wise?\" ");
	    if (ja() == 0) { ididit = 0; Time=0; return;}
	}
    }
    /* Entry WARPX */
    if (warpfac > 6.0) {
	/* Decide if engine damage will occur */
	double prob = dist*(6.0-warpfac)*(6.0-warpfac)/66.666666666;
	if (prob > Rand()) {
	    blooey = 1;
	    dist = Rand()*dist;
	}
	/* Decide if time warp will occur */
	if (0.5*dist*pow(7.0,warpfac-10.0) > Rand()) twarp=1;
#ifdef DEBUG
	if (idebug &&warpfac==10 && twarp==0) {
	    blooey=0;
	    proutn("Force time warp? ");
	    if (ja()==1) twarp=1;
	}
#endif
	if (blooey || twarp) {
	    /* If time warp or engine damage, check path */
	    /* If it is obstructed, don't do warp or damage */
	    double angle = ((15.0-direc)*0.5235998);
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
	    n = 10.0 * dist * bigger +0.5;
	    x = sectx;
	    y = secty;
	    for (l = 1; l <= n; l++) {
		x += deltax;
		ix = x + 0.5;
		if (ix < 1 || ix > QUADSIZE) break;
		y += deltay;
		iy = y +0.5;
		if (iy < 1 || iy > QUADSIZE) break;
		if (game.quad[ix][iy] != IHDOT) {
		    blooey = 0;
		    twarp = 0;
		}
	    }
	}
    }
				

    /* Activate Warp Engines and pay the cost */
    imove();
    if (alldone) return;
    energy -= dist*warpfac*warpfac*warpfac*(shldup+1);
    if (energy <= 0) finish(FNRG);
    Time = 10.0*dist/wfacsq;
    if (twarp) timwrp();
    if (blooey) {
	game.damage[DWARPEN] = damfac*(3.0*Rand()+1.0);
	skip(1);
	prout("Engineering to bridge--");
	prout("  Scott here.  The warp engines are damaged.");
	prout("  We'll have to reduce speed to warp 4.");
    }
    ididit = 1;
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
	prout("Engineer Scott- \"I'm doing my best, Captain,\n"
	      "  but right now we can only go warp 4.\"");
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
    oldfac = warpfac;
    warpfac = aaitem;
    wfacsq=warpfac*warpfac;
    if (warpfac <= oldfac || warpfac <= 6.0) {
	proutn("Helmsman Sulu- \"Warp factor %d, Captain.\"", 
	       (int)warpfac);
	return;
    }
    if (warpfac < 8.00) {
	prout("Engineer Scott- \"Aye, but our maximum safe speed is warp 6.\"");
	return;
    }
    if (warpfac == 10.0) {
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
    if (landed==1) {
	if (game.damage[DTRANSP]) {
	    finish(FPNOVA);
	    return;
	}
	prout("Scotty rushes to the transporter controls.");
	if (shldup) {
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
	if (imine) {
	    imine = 0;
	    proutn("The crystals mined were ");
	    if (Rand() <= 0.25) {
		prout("lost.");
	    }
	    else {
		prout("saved.");
		icrystl = 1;
	    }
	}
    }
    if (igrab) return;

    /* Check to see if captain in shuttle craft */
    if (icraft) finish(FSTRACTOR);
    if (alldone) return;

    /* Inform captain of attempt to reach safety */
    skip(1);
    do {
	if (justin) {
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
	game.starch[quadx][quady] = game.damage[DRADIO] > 0.0 ? game.state.galaxy[quadx][quady]+SUPERNOVA_PLACE:1;

	/* Try to use warp engines */
	if (game.damage[DWARPEN]) {
	    skip(1);
	    prout("Warp engines damaged.");
	    finish(FSNOVAED);
	    return;
	}
	warpfac = 6.0+2.0*Rand();
	wfacsq = warpfac * warpfac;
	prout("Warp factor set to %d", (int)warpfac);
	power = 0.75*energy;
	dist = power/(warpfac*warpfac*warpfac*(shldup+1));
	distreq = 1.4142+Rand();
	if (distreq < dist) dist = distreq;
	Time = 10.0*dist/wfacsq;
	direc = 12.0*Rand();	/* How dumb! */
	justin = 0;
	inorbit = 0;
	warp(2);
	if (justin == 0) {
	    /* This is bad news, we didn't leave quadrant. */
	    if (alldone) return;
	    skip(1);
	    prout("Insufficient energy to leave quadrant.");
	    finish(FSNOVAED);
	    return;
	}
	/* Repeat if another snova */
    } while (game.state.galaxy[quadx][quady] == SUPERNOVA_PLACE);
    if (game.state.remkl==0) finish(FWON); /* Snova killed remaining enemy. */
}

void timwrp() 
{
    int l, ll, gotit;
    prout("***TIME WARP ENTERED.");
    if (game.state.snap && Rand() < 0.5) {
	/* Go back in time */
	prout("You are traveling backwards in time %d stardates.",
	      (int)(game.state.date-game.snapsht.date));
	game.state = game.snapsht;
	game.state.snap = 0;
	if (game.state.remcom) {
	    game.future[FTBEAM] = game.state.date + expran(intime/game.state.remcom);
	    game.future[FBATTAK] = game.state.date + expran(0.3*intime);
	}
	game.future[FSNOVA] = game.state.date + expran(0.5*intime);
	game.future[FSNAP] = game.state.date +expran(0.25*game.state.remtime); /* next snapshot will
										  be sooner */
	if (game.state.nscrem) game.future[FSCMOVE] = 0.2777;
	isatb = 0;
	game.future[FCDBAS] = game.future[FSCDBAS] = 1e30;
	batx = baty = 0;

	/* Make sure Galileo is consistant -- Snapshot may have been taken
	   when on planet, which would give us two Galileos! */
	gotit = 0;
	for (l = 0; l < inplan; l++) {
	    if (game.state.plnets[l].known == shuttle_down) {
		gotit = 1;
		if (iscraft==1 && ship==IHE) {
		    prout("Checkov-  \"Security reports the Galileo has disappeared, Sir!");
		    iscraft = 0;
		}
	    }
	}
	/* Likewise, if in the original time the Galileo was abandoned, but
	   was on ship earlier, it would have vanished -- lets restore it */
	if (iscraft==0 && gotit==0 && game.damage[DSHUTTL] >= 0.0) {
	    prout("Checkov-  \"Security reports the Galileo has reappeared in the dock!\"");
	    iscraft = 1;
	}

	/* Revert star chart to earlier era, if it was known then*/
	if (game.damage[DRADIO]==0.0 || stdamtim > game.state.date) {
	    for (l = 1; l <= GALSIZE; l++)
		for (ll = 1; ll <= GALSIZE; ll++)
		    if (game.starch[l][ll] > 1)
			game.starch[l][ll]=game.damage[DRADIO]>0.0 ? game.state.galaxy[l][ll]+SUPERNOVA_PLACE :1;
	    prout("Spock has reconstructed a correct star chart from memory");
	    if (game.damage[DRADIO] > 0.0) stdamtim = game.state.date;
	}
    }
    else {
	/* Go forward in time */
	Time = -0.5*intime*log(Rand());
	prout("You are traveling forward in time %d stardates.", (int)Time);
	/* cheat to make sure no tractor beams occur during time warp */
	game.future[FTBEAM] += Time;
	game.damage[DRADIO] += Time;
    }
    newqad(0);
    events();	/* Stas Sergeev added this -- do pending events */
}

void probe(void) 
{
    double angle, bigger;
    int key;
    /* New code to launch a deep space probe */
    if (nprobes == 0) {
	chew();
	skip(1);
	if (ship == IHE) 
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
    if (game.future[FDSPROB] != 1e30) {
	chew();
	skip(1);
	if (game.damage[DRADIO] != 0 && condit != IHDOCKED) {
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
	prout(nprobes==1 ? "%d probe left." : "%d probes left.", nprobes);
	proutn("Are you sure you want to fire a probe? ");
	if (ja()==0) return;
    }

    isarmed = FALSE;
    if (key == IHALPHA && strcmp(citem,"armed") == 0) {
	isarmed = TRUE;
	key = scan();
    }
    else if (key == IHEOL) {
	proutn("Arm NOVAMAX warhead? ");
	isarmed = ja();
    }
    getcd(TRUE, key);
    if (direc == -1.0) return;
    nprobes--;
    angle = ((15.0 - direc) * 0.5235988);
    probeinx = -sin(angle);
    probeiny = cos(angle);
    if (fabs(probeinx) > fabs(probeiny))
	bigger = fabs(probeinx);
    else
	bigger = fabs(probeiny);
		
    probeiny /= bigger;
    probeinx /= bigger;
    proben = 10.0*dist*bigger +0.5;
    probex = quadx*QUADSIZE + sectx - 1;	// We will use better packing than original
    probey = quady*QUADSIZE + secty - 1;
    probecx = quadx;
    probecy = quady;
    game.future[FDSPROB] = game.state.date + 0.01; // Time to move one sector
    prout("Ensign Chekov-  \"The deep space probe is launched, Captain.\"");
    ididit = 1;
    return;
}

void help(void) 
{
    /* There's more than one way to move in this game! */
    double ddist, xdist, probf;
    int line = 0, l, ix, iy;

    chew();
    /* Test for conditions which prevent calling for help */
    if (condit == IHDOCKED) {
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
    if (landed == 1) {
	proutn("You must be aboard the ");
	crmshp();
	prout(".");
	return;
    }
    /* OK -- call for help from nearest starbase */
    nhelp++;
    if (basex!=0) {
	/* There's one in this quadrant */
	ddist = sqrt(square(basex-sectx)+square(basey-secty));
    }
    else {
	ddist = 1e30;
	for (l = 1; l <= game.state.rembase; l++) {
	    xdist=10.0*sqrt(square(game.state.baseqx[l]-quadx)+square(game.state.baseqy[l]-quady));
	    if (xdist < ddist) {
		ddist = xdist;
		line = l;
	    }
	}
	/* Since starbase not in quadrant, set up new quadrant */
	quadx = game.state.baseqx[line];
	quady = game.state.baseqy[line];
	newqad(1);
    }
    /* dematerialize starship */
    game.quad[sectx][secty]=IHDOT;
    proutn("Starbase in %s responds--", cramlc(quadrant, quadx, quady));
    proutn("");
    crmshp();
    prout(" dematerializes.");
    sectx=0;
    for (l = 1; l <= 5; l++) {
	ix = basex+3.0*Rand()-1;
	iy = basey+3.0*Rand()-1;
	if (ix>=1 && ix<=QUADSIZE && iy>=1 && iy<=QUADSIZE && game.quad[ix][iy]==IHDOT) {
	    /* found one -- finish up */
	    sectx=ix;
	    secty=iy;
	    game.quad[ix][iy]=IHMATER0;
	    break;
	}
    }
    if (sectx==0){
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
	warble();
	if (Rand() > probf) break;
	switch (l){
	case 1: game.quad[ix][iy]=IHMATER1;
	    break;
	case 2: game.quad[ix][iy]=IHMATER2;
	    break;
	case 3: game.quad[ix][iy]=IHQUEST;
	    break;
	}
	textcolor(RED);
	prout("fails.");
	delay(500);
	textcolor(DEFAULT);
    }
    if (l > 3) {
	finish(FMATERIALIZE);
	return;
    }
    game.quad[ix][iy]=ship;
    textcolor(GREEN);
    prout("succeeds.");
    textcolor(DEFAULT);
    dock(0);
    skip(1);
    prout("Lt. Uhura-  \"Captain, we made it!\"");
}
