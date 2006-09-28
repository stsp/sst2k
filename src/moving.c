#include <unistd.h>
#include "sstlinux.h"
#include "sst.h"

static void getcd(bool, int);

void imove(bool novapush)
/* movement execution for warp, impulse, supernova, and tractor-beam events */
{
    double angle, deltax, deltay, bigger, x, y,
        finald, stopegy, probf;
    int n, m, kink, kinks;
    feature iquad;
    coord w, final;
    bool trbeam = false;

    w.x = w.y = 0;
    if (game.inorbit) {
	prout(_("Helmsman Sulu- \"Leaving standard orbit.\""));
	game.inorbit = false;
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
	trbeam = true;
	game.condition = red;
	game.dist = game.dist*(scheduled(FTBEAM)-game.state.date)/game.optime + 0.1;
	game.optime = scheduled(FTBEAM) - game.state.date + 1e-5;
    }
    /* Move within the quadrant */
    game.quad[game.sector.x][game.sector.y] = IHDOT;
    x = game.sector.x;
    y = game.sector.y;
    n = 10.0*game.dist*bigger+0.5;

    if (n > 0) {
	for (m = 1; m <= n; m++) {
	    w.x = (x += deltax) + 0.5;
	    w.y = (y += deltay) + 0.5;
	    if (!VALID_SECTOR(w.x, w.y)) {
		/* Leaving quadrant -- allow final enemy attack */
		/* Don't do it if being pushed by Nova */
		if (game.nenhere != 0 && !novapush) {
		    newcnd();
		    for_local_enemies(m) {
			finald = distance(w, game.ks[m]);
			game.kavgd[m] = 0.5 * (finald + game.kdist[m]);
		    }
		    /*
		     * Stas Sergeev added the condition
		     * that attacks only happen if Klingons
		     * are present and your skill is good.
		     */
		    if (game.skill > SKILL_GOOD && game.klhere > 0 && !game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova)
			attack(false);
		    if (game.alldone)
			return;
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
		    if (kink)
			kinks = 1;
		} while (kink);

		if (kinks) {
		    game.nkinks += 1;
		    if (game.nkinks == 3) {
			/* Three strikes -- you're out! */
			finish(FNEG3);
			return;
		    }
		    skip(1);
		    prout(_("YOU HAVE ATTEMPTED TO CROSS THE NEGATIVE ENERGY BARRIER"));
		    prout(_("AT THE EDGE OF THE GALAXY.  THE THIRD TIME YOU TRY THIS,"));
		    prout(_("YOU WILL BE DESTROYED."));
		}
		/* Compute final position in new quadrant */
		if (trbeam) /* Don't bother if we are to be beamed */
		    return;
		game.quadrant.x = (w.x+(QUADSIZE-1))/QUADSIZE;
		game.quadrant.y = (w.y+(QUADSIZE-1))/QUADSIZE;
		game.sector.x = w.x - QUADSIZE*(game.quadrant.x-1);
		game.sector.y = w.y - QUADSIZE*(game.quadrant.y-1);
		skip(1);
		prout(_("Entering %s."), cramlc(quadrant, game.quadrant));
		game.quad[game.sector.x][game.sector.y] = game.ship;
		newqad(false);
		if (game.skill>SKILL_NOVICE)
		    attack(false);  
		return;
	    }
	    iquad = game.quad[w.x][w.y];
	    if (iquad != IHDOT) {
		/* object encountered in flight path */
		stopegy = 50.0*game.dist/game.optime;
		game.dist = distance(game.sector, w) / (QUADSIZE * 1.0);
		switch (iquad) {
		case IHT: /* Ram a Tholian */
		case IHK: /* Ram enemy ship */
		case IHC:
		case IHS:
		case IHR:
		case IHQUEST:
		    game.sector = w;
		    ram(false, iquad, game.sector);
		    final = game.sector;
		    break;
		case IHBLANK:
		    skip(1);
		    prouts(_("***RED ALERT!  RED ALERT!"));
		    skip(1);
		    proutn("***");
		    crmshp();
		    proutn(_(" pulled into black hole at "));
		    prout(cramlc(sector, w));
		    /*
		     * Getting pulled into a black hole was certain
		     * death in Almy's original.  Stas Sergeev added a
		     * possibility that you'll get timewarped instead.
		     */
		    n=0;
		    for (m=0;m<NDEVICES;m++)
			if (game.damage[m]>0) 
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
			proutn(_(" encounters Tholian web at "));
		    else
			proutn(_(" blocked by object at "));
		    proutn(cramlc(sector, w));
		    prout(";");
		    proutn(_("Emergency stop required "));
		    prout(_("%2d units of energy."), (int)stopegy);
		    game.energy -= stopegy;
		    final.x = x-deltax+0.5;
		    final.y = y-deltay+0.5;
		    game.sector = final;
		    if (game.energy <= 0) {
			finish(FNRG);
			return;
		    }
		    break;
		}
		goto no_quad_change;	/* sorry! */
	    }
	}
	game.dist = distance(game.sector, w) / (QUADSIZE * 1.0);
	game.sector = w;
    }
    final = game.sector;
no_quad_change:
    /* No quadrant change -- compute new avg enemy distances */
    game.quad[game.sector.x][game.sector.y] = game.ship;
    if (game.nenhere) {
	for_local_enemies(m) {
	    finald = distance(w, game.ks[m]);
	    game.kavgd[m] = 0.5 * (finald+game.kdist[m]);
	    game.kdist[m] = finald;
	}
	sortklings();
	if (!game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova)
	    attack(false);
	for_local_enemies(m) game.kavgd[m] = game.kdist[m];
    }
    newcnd();
    drawmaps(0);
    setwnd(message_window);
    return;
}

void dock(bool verbose) 
/* dock our ship at a starbase */
{
    chew();
    if (game.condition == docked && verbose) {
	prout(_("Already docked."));
	return;
    }
    if (game.inorbit) {
	prout(_("You must first leave standard orbit."));
	return;
    }
    if (!is_valid(game.base) || abs(game.sector.x-game.base.x) > 1 || abs(game.sector.y-game.base.y) > 1) {
	crmshp();
	prout(_(" not adjacent to base."));
	return;
    }
    game.condition = docked;
    if (verbose)
	prout(_("Docked."));
    game.ididit = true;
    if (game.energy < game.inenrg)
	game.energy = game.inenrg;
    game.shield = game.inshld;
    game.torps = game.intorps;
    game.lsupres = game.inlsr;
    game.state.crew = FULLCREW;
    if (!damaged(DRADIO) &&
	(is_scheduled(FCDBAS) || game.isatb == 1) && !game.iseenit) {
	/* get attack report from base */
	prout(_("Lt. Uhura- \"Captain, an important message from the starbase:\""));
	attackreport(false);
	game.iseenit = true;
    }
}

/* 
 * This program originally required input in terms of a (clock)
 * direction and distance. Somewhere in history, it was changed to
 * cartesian coordinates. So we need to convert. I think
 * "manual" input should still be done this way -- it's a real
 * pain if the computer isn't working! Manual mode is still confusing
 * because it involves giving x and y motions, yet the coordinates
 * are always displayed y - x, where +y is downward!
 */

static void getcd(bool isprobe, int akey)
/* get course and distance */
{
    int irowq=game.quadrant.x, icolq=game.quadrant.y, key=0;
    double xi, xj, xk, xl;
    double deltax, deltay;
    enum {unspecified, manual, automatic} navmode = unspecified;
    enum {curt, normal, verbose} itemp = curt;
    coord incr;
    bool iprompt = false;

    /* Get course direction and distance. If user types bad values, return
       with DIREC = -1.0. */

    game.direc = -1.0;
	
    if (game.landed && !isprobe) {
	prout(_("Dummy! You can't leave standard orbit until you"));
	proutn(_("are back aboard the ship."));
	chew();
	return;
    }
    while (navmode == unspecified) {
	if (damaged(DNAVSYS)) {
	    if (isprobe)
		prout(_("Computer damaged; manual navigation only"));
	    else
		prout(_("Computer damaged; manual movement only"));
	    chew();
	    navmode = manual;
	    key = IHEOL;
	    break;
	}
	if (isprobe && akey != -1) {
	    /* For probe launch, use pre-scanned value first time */
	    key = akey;
	    akey = -1;
	}
	else 
	    key = scan();

	if (key == IHEOL) {
	    proutn(_("Manual or automatic- "));
	    iprompt = true;
	    chew();
	}
	else if (key == IHALPHA) {
	    if (isit("manual")) {
		navmode = manual;
		key = scan();
		break;
	    }
	    else if (isit("automatic")) {
		navmode = automatic;
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
		prout(_("(Manual navigation assumed.)"));
	    else
		prout(_("(Manual movement assumed.)"));
	    navmode = automatic;
	    break;
	}
    }

    if (navmode == automatic) {
	while (key == IHEOL) {
	    if (isprobe)
		proutn(_("Target quadrant or quadrant&sector- "));
	    else
		proutn(_("Destination sector or quadrant&sector- "));
	    chew();
	    iprompt = true;
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
	    itemp = normal;
	}
	if (!VALID_QUADRANT(icolq,irowq)||!VALID_SECTOR(incr.x,incr.y)) {
	    huh();
	    return;
	}
	skip(1);
	if (!isprobe) {
	    if (itemp > curt) {
		if (iprompt) {
		    prout(_("Helmsman Sulu- \"Course locked in for %s.\""),
			  cramlc(sector, incr));
		}
	    }
	    else
		prout(_("Ensign Chekov- \"Course laid in, Captain.\""));
	}
	deltax = icolq - game.quadrant.y + 0.1*(incr.x-game.sector.y);
	deltay = game.quadrant.x - irowq + 0.1*(game.sector.x-incr.y);
    }
    else { /* manual */
	while (key == IHEOL) {
	    proutn(_("X and Y displacements- "));
	    chew();
	    iprompt = true;
	    key = scan();
	}
	itemp = verbose;
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
    if (itemp == verbose && !isprobe) {
	skip(1);
	prout(_("Helmsman Sulu- \"Aye, Sir.\""));
    }
    game.dist = sqrt(deltax*deltax + deltay*deltay);
    game.direc = atan2(deltax, deltay)*1.90985932;
    if (game.direc < 0.0)
	game.direc += 12.0;
    chew();
    return;
}
		


void impulse(void) 
/* move under impulse power */
{
    double power;

    game.ididit = false;
    if (damaged(DIMPULS)) {
	chew();
	skip(1);
	prout(_("Engineer Scott- \"The impulse engines are damaged, Sir.\""));
	return;
    }

    if (game.energy > 30.0) {
	getcd(false, 0);
	if (game.direc == -1.0)
	    return;
	power = 20.0 + 100.0*game.dist;
    }
    else
	power = 30.0;

    if (power >= game.energy) {
	/* Insufficient power for trip */
	skip(1);
	prout(_("First Officer Spock- \"Captain, the impulse engines"));
	prout(_("require 20.0 units to engage, plus 100.0 units per"));
	if (game.energy > 30) {
	    proutn(_("quadrant.  We can go, therefore, a maximum of %d"),
		   (int)(0.01 * (game.energy-20.0)-0.05));
	    prout(_(" quadrants.\""));
	}
	else {
	    prout(_("quadrant.  They are, therefore, useless.\""));
	}
	chew();
	return;
    }
    /* Make sure enough time is left for the trip */
    game.optime = game.dist/0.095;
    if (game.optime >= game.state.remtime) {
	prout(_("First Officer Spock- \"Captain, our speed under impulse"));
	prout(_("power is only 0.95 sectors per stardate. Are you sure"));
	proutn(_("we dare spend the time?\" "));
	if (ja() == false)
	    return;
    }
    /* Activate impulse engines and pay the cost */
    imove(false);
    game.ididit = true;
    if (game.alldone)
	return;
    power = 20.0 + 100.0*game.dist;
    game.energy -= power;
    game.optime = game.dist/0.095;
    if (game.energy <= 0)
	finish(FNRG);
    return;
}


void warp(bool timewarp)
/* move under warp drive */
{
    int iwarp;
    bool blooey = false, twarp = false;
    double power;

    if (!timewarp) { /* Not WARPX entry */
	game.ididit = false;
	if (game.damage[DWARPEN] > 10.0) {
	    chew();
	    skip(1);
	    prout(_("Engineer Scott- \"The impulse engines are damaged, Sir.\""));
	    return;
	}
	if (damaged(DWARPEN) && game.warpfac > 4.0) {
	    chew();
	    skip(1);
	    prout(_("Engineer Scott- \"Sorry, Captain. Until this damage"));
	    prout(_("  is repaired, I can only give you warp 4.\""));
	    return;
	}
			
	/* Read in course and distance */
	getcd(false, 0);
	if (game.direc == -1.0)
	    return;

	/* Make sure starship has enough energy for the trip */
	power = (game.dist+0.05)*game.warpfac*game.warpfac*game.warpfac*(game.shldup+1);


	if (power >= game.energy) {
	    /* Insufficient power for trip */
	    game.ididit = false;
	    skip(1);
	    prout(_("Engineering to bridge--"));
	    if (!game.shldup || 0.5*power > game.energy) {
		iwarp = pow((game.energy/(game.dist+0.05)), 0.333333333);
		if (iwarp <= 0) {
		    prout(_("We can't do it, Captain. We don't have enough energy."));
		}
		else {
		    proutn(_("We don't have enough energy, but we could do it at warp %d"), iwarp);
		    if (game.shldup) {
			prout(",");
			prout(_("if you'll lower the shields."));
		    }
		    else
			prout(".");
		}
	    }
	    else
		prout(_("We haven't the energy to go that far with the shields up."));
	    return;
	}
						
	/* Make sure enough time is left for the trip */
	game.optime = 10.0*game.dist/game.wfacsq;
	if (game.optime >= 0.8*game.state.remtime) {
	    skip(1);
	    prout(_("First Officer Spock- \"Captain, I compute that such"));
	    proutn(_("  a trip would require approximately %2.0f"),
		   100.0*game.optime/game.state.remtime);
	    prout(_(" percent of our"));
	    proutn(_("  remaining time.  Are you sure this is wise?\" "));
	    if (ja() == false) {
		game.ididit = false;
		game.optime=0; 
		return;
	    }
	}
    }
    /* Entry WARPX */
    if (game.warpfac > 6.0) {
	/* Decide if engine damage will occur */
	double prob = game.dist*(6.0-game.warpfac)*(6.0-game.warpfac)/66.666666666;
	if (prob > Rand()) {
	    blooey = true;
	    game.dist = Rand()*game.dist;
	}
	/* Decide if time warp will occur */
	if (0.5*game.dist*pow(7.0,game.warpfac-10.0) > Rand())
	    twarp = true;
	if (idebug && game.warpfac==10 && !twarp) {
	    blooey = false;
	    proutn("=== Force time warp? ");
	    if (ja() == true)
		twarp = true;
	}
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
		if (!VALID_SECTOR(ix, iy))
		    break;
		if (game.quad[ix][iy] != IHDOT) {
		    blooey = false;
		    twarp = false;
		}
	    }
	}
    }
				

    /* Activate Warp Engines and pay the cost */
    imove(false);
    if (game.alldone)
	return;
    game.energy -= game.dist*game.warpfac*game.warpfac*game.warpfac*(game.shldup+1);
    if (game.energy <= 0)
	finish(FNRG);
    game.optime = 10.0*game.dist/game.wfacsq;
    if (twarp)
	timwrp();
    if (blooey) {
	game.damage[DWARPEN] = game.damfac*(3.0*Rand()+1.0);
	skip(1);
	prout(_("Engineering to bridge--"));
	prout(_("  Scott here.  The warp engines are damaged."));
	prout(_("  We'll have to reduce speed to warp 4."));
    }
    game.ididit = true;
    return;
}



void setwarp(void) 
/* change the warp factor */
{
    int key;
    double oldfac;
	
    while ((key=scan()) == IHEOL) {
	chew();
	proutn(_("Warp factor- "));
    }
    chew();
    if (key != IHREAL) {
	huh();
	return;
    }
    if (game.damage[DWARPEN] > 10.0) {
	prout(_("Warp engines inoperative."));
	return;
    }
    if (damaged(DWARPEN) && aaitem > 4.0) {
	prout(_("Engineer Scott- \"I'm doing my best, Captain,"));
	prout(_("  but right now we can only go warp 4.\""));
	return;
    }
    if (aaitem > 10.0) {
	prout(_("Helmsman Sulu- \"Our top speed is warp 10, Captain.\""));
	return;
    }
    if (aaitem < 1.0) {
	prout(_("Helmsman Sulu- \"We can't go below warp 1, Captain.\""));
	return;
    }
    oldfac = game.warpfac;
    game.warpfac = aaitem;
    game.wfacsq=game.warpfac*game.warpfac;
    if (game.warpfac <= oldfac || game.warpfac <= 6.0) {
	prout(_("Helmsman Sulu- \"Warp factor %d, Captain.\""),
	       (int)game.warpfac);
	return;
    }
    if (game.warpfac < 8.00) {
	prout(_("Engineer Scott- \"Aye, but our maximum safe speed is warp 6.\""));
	return;
    }
    if (game.warpfac == 10.0) {
	prout(_("Engineer Scott- \"Aye, Captain, we'll try it.\""));
	return;
    }
    prout(_("Engineer Scott- \"Aye, Captain, but our engines may not take it.\""));
    return;
}

void atover(bool igrab) 
/* cope with being tossed out of quadrant by supernova or yanked by beam */
{
    double power, distreq;

    chew();
    /* is captain on planet? */
    if (game.landed) {
	if (damaged(DTRANSP)) {
	    finish(FPNOVA);
	    return;
	}
	prout(_("Scotty rushes to the transporter controls."));
	if (game.shldup) {
	    prout(_("But with the shields up it's hopeless."));
	    finish(FPNOVA);
	}
	prouts(_("His desperate attempt to rescue you . . ."));
	if (Rand() <= 0.5) {
	    prout(_("fails."));
	    finish(FPNOVA);
	    return;
	}
	prout(_("SUCCEEDS!"));
	if (game.imine) {
	    game.imine = false;
	    proutn(_("The crystals mined were "));
	    if (Rand() <= 0.25) {
		prout(_("lost."));
	    }
	    else {
		prout(_("saved."));
		game.icrystl = true;
	    }
	}
    }
    if (igrab)
	return;

    /* Check to see if captain in shuttle craft */
    if (game.icraft)
	finish(FSTRACTOR);
    if (game.alldone)
	return;

    /* Inform captain of attempt to reach safety */
    skip(1);
    do {
	if (game.justin) {
	    prouts(_("***RED ALERT!  RED ALERT!"));
	    skip(1);
	    proutn(_("The "));
	    crmshp();
	    prout(_(" has stopped in a quadrant containing"));
	    prouts(_("   a supernova."));
	    skip(2);
	}
	proutn(_("***Emergency automatic override attempts to hurl "));
	crmshp();
	skip(1);
	prout(_("safely out of quadrant."));
	if (!damaged(DRADIO))
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].charted = true;
	/* Try to use warp engines */
	if (damaged(DWARPEN)) {
	    skip(1);
	    prout(_("Warp engines damaged."));
	    finish(FSNOVAED);
	    return;
	}
	game.warpfac = 6.0+2.0*Rand();
	game.wfacsq = game.warpfac * game.warpfac;
	prout(_("Warp factor set to %d"), (int)game.warpfac);
	power = 0.75*game.energy;
	game.dist = power/(game.warpfac*game.warpfac*game.warpfac*(game.shldup+1));
	distreq = 1.4142+Rand();
	if (distreq < game.dist)
	    game.dist = distreq;
	game.optime = 10.0*game.dist/game.wfacsq;
	game.direc = 12.0*Rand();	/* How dumb! */
	game.justin = false;
	game.inorbit = false;
	warp(true);
	if (!game.justin) {
	    /* This is bad news, we didn't leave quadrant. */
	    if (game.alldone)
		return;
	    skip(1);
	    prout(_("Insufficient energy to leave quadrant."));
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
/* let's do the time warp again */
{
    int l;
    bool gotit;
    prout(_("***TIME WARP ENTERED."));
    if (game.state.snap && Rand() < 0.5) {
	/* Go back in time */
	prout(_("You are traveling backwards in time %d stardates."),
	      (int)(game.state.date-game.snapsht.date));
	game.state = game.snapsht;
	game.state.snap = false;
	if (game.state.remcom) {
	    schedule(FTBEAM, expran(game.intime/game.state.remcom));
	    schedule(FBATTAK, expran(0.3*game.intime));
	}
	schedule(FSNOVA, expran(0.5*game.intime));
	/* next snapshot will be sooner */
	schedule(FSNAP, expran(0.25*game.state.remtime));
				
	if (game.state.nscrem)
	    schedule(FSCMOVE, 0.2777);	    
	game.isatb = 0;
	unschedule(FCDBAS);
	unschedule(FSCDBAS);
	invalidate(game.battle);

	/* Make sure Galileo is consistant -- Snapshot may have been taken
	   when on planet, which would give us two Galileos! */
	gotit = false;
	for (l = 0; l < game.inplan; l++) {
	    if (game.state.planets[l].known == shuttle_down) {
		gotit = true;
		if (game.iscraft == onship && game.ship==IHE) {
		    prout(_("Checkov-  \"Security reports the Galileo has disappeared, Sir!"));
		    game.iscraft = offship;
		}
	    }
	}
	/* Likewise, if in the original time the Galileo was abandoned, but
	   was on ship earlier, it would have vanished -- lets restore it */
	if (game.iscraft == offship && !gotit && game.damage[DSHUTTL] >= 0.0) {
	    prout(_("Checkov-  \"Security reports the Galileo has reappeared in the dock!\""));
	    game.iscraft = onship;
	}
	/* 
	 * There used to be code to do the actual reconstrction here,
	 * but the starchart is now part of the snapshotted galaxy state.
	 */
	prout(_("Spock has reconstructed a correct star chart from memory"));
    }
    else {
	/* Go forward in time */
	game.optime = -0.5*game.intime*log(Rand());
	prout(_("You are traveling forward in time %d stardates."), (int)game.optime);
	/* cheat to make sure no tractor beams occur during time warp */
	postpone(FTBEAM, game.optime);
	game.damage[DRADIO] += game.optime;
    }
    newqad(false);
    events();	/* Stas Sergeev added this -- do pending events */
}

void probe(void) 
/* launch deep-space probe */
{
    double angle, bigger;
    int key;
    /* New code to launch a deep space probe */
    if (game.nprobes == 0) {
	chew();
	skip(1);
	if (game.ship == IHE) 
	    prout(_("Engineer Scott- \"We have no more deep space probes, Sir.\""));
	else
	    prout(_("Ye Faerie Queene has no deep space probes."));
	return;
    }
    if (damaged(DDSP)) {
	chew();
	skip(1);
	prout(_("Engineer Scott- \"The probe launcher is damaged, Sir.\""));
	return;
    }
    if (is_scheduled(FDSPROB)) {
	chew();
	skip(1);
	if (damaged(DRADIO) && game.condition != docked) {
	    prout(_("Spock-  \"Records show the previous probe has not yet"));
	    prout(_("   reached its destination.\""));
	}
	else
	    prout(_("Uhura- \"The previous probe is still reporting data, Sir.\""));
	return;
    }
    key = scan();

    if (key == IHEOL) {
	/* slow mode, so let Kirk know how many probes there are left */
	prout(game.nprobes==1 ? _("%d probe left.") : _("%d probes left."), game.nprobes);
	proutn(_("Are you sure you want to fire a probe? "));
	if (ja() == false)
	    return;
    }

    game.isarmed = false;
    if (key == IHALPHA && strcmp(citem,"armed") == 0) {
	game.isarmed = true;
	key = scan();
    }
    else if (key == IHEOL) {
	proutn(_("Arm NOVAMAX warhead? "));
	game.isarmed = ja();
    }
    getcd(true, key);
    if (game.direc == -1.0)
	return;
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
    prout(_("Ensign Chekov-  \"The deep space probe is launched, Captain.\""));
    game.ididit = true;
    return;
}

/*
 *	Here's how the mayday code works:
 *
 *	First, the closest starbase is selected.  If there is a
 *	a starbase in your own quadrant, you are in good shape.
 *	This distance takes quadrant distances into account only.
 *
 *	A magic number is computed based on the distance which acts
 *	as the probability that you will be rematerialized.  You
 *	get three tries.
 *
 *	When it is determined that you should be able to be remater-
 *	ialized (i.e., when the probability thing mentioned above
 *	comes up positive), you are put into that quadrant (anywhere).
 *	Then, we try to see if there is a spot adjacent to the star-
 *	base.  If not, you can't be rematerialized!!!  Otherwise,
 *	it drops you there.  It only tries five times to find a spot
 *	to drop you.  After that, it's your problem.
 */

void mayday(void) 
/* yell for help from nearest starbase */
{
    /* There's more than one way to move in this game! */
    double ddist, xdist, probf;
    int line = 0, m, ix, iy;

    chew();
    /* Test for conditions which prevent calling for help */
    if (game.condition == docked) {
	prout(_("Lt. Uhura-  \"But Captain, we're already docked.\""));
	return;
    }
    if (damaged(DRADIO)) {
	prout(_("Subspace radio damaged."));
	return;
    }
    if (game.state.rembase==0) {
	prout(_("Lt. Uhura-  \"Captain, I'm not getting any response from Starbase.\""));
	return;
    }
    if (game.landed) {
	proutn(_("You must be aboard the "));
	crmshp();
	prout(".");
	return;
    }
    /* OK -- call for help from nearest starbase */
    game.nhelp++;
    if (game.base.x!=0) {
	/* There's one in this quadrant */
	ddist = distance(game.base, game.sector);
    }
    else {
	ddist = FOREVER;
	for_starbases(m) {
	    xdist = QUADSIZE * distance(game.state.baseq[m], game.quadrant);
	    if (xdist < ddist) {
		ddist = xdist;
		line = m;
	    }
	}
	/* Since starbase not in quadrant, set up new quadrant */
	game.quadrant = game.state.baseq[line];
	newqad(true);
    }
    /* dematerialize starship */
    game.quad[game.sector.x][game.sector.y]=IHDOT;
    proutn(_("Starbase in %s responds--"), cramlc(quadrant, game.quadrant));
    crmshp();
    prout(_(" dematerializes."));
    game.sector.x=0;
    for (m = 1; m <= 5; m++) {
	ix = game.base.x+3.0*Rand()-1;
	iy = game.base.y+3.0*Rand()-1;
	if (VALID_SECTOR(ix,iy) && game.quad[ix][iy]==IHDOT) {
	    /* found one -- finish up */
	    game.sector.x=ix;
	    game.sector.y=iy;
	    break;
	}
    }
    if (!is_valid(game.sector)){
	prout(_("You have been lost in space..."));
	finish(FMATERIALIZE);
	return;
    }
    /* Give starbase three chances to rematerialize starship */
    probf = pow((1.0 - pow(0.98,ddist)), 0.33333333);
    for (m = 1; m <= 3; m++) {
	switch (m) {
	case 1: proutn(_("1st")); break;
	case 2: proutn(_("2nd")); break;
	case 3: proutn(_("3rd")); break;
	}
	proutn(_(" attempt to re-materialize "));
	crmshp();
	switch (m){
	case 1: game.quad[ix][iy]=IHMATER0;
	    break;
	case 2: game.quad[ix][iy]=IHMATER1;
	    break;
	case 3: game.quad[ix][iy]=IHMATER2;
	    break;
	}
	textcolor(RED);
	warble();
	if (Rand() > probf)
	    break;
	prout(_("fails."));
	delay(500);
	textcolor(DEFAULT);
    }
    if (m > 3) {
	game.quad[ix][iy]=IHQUEST;
	game.alive = false;
	drawmaps(1);
	setwnd(message_window);
	finish(FMATERIALIZE);
	return;
    }
    game.quad[ix][iy]=game.ship;
    textcolor(GREEN);
    prout(_("succeeds."));
    textcolor(DEFAULT);
    dock(false);
    skip(1);
    prout(_("Lt. Uhura-  \"Captain, we made it!\""));
}

/*
**  Abandon Ship
**
**	The ship is abandoned.  If your current ship is the Faire
**	Queene, or if your shuttlecraft is dead, you're out of
**	luck.  You need the shuttlecraft in order for the captain
**	(that's you!!) to escape.
**
**	Your crew can beam to an inhabited starsystem in the
**	quadrant, if there is one and if the transporter is working.
**	If there is no inhabited starsystem, or if the transporter
**	is out, they are left to die in outer space.
**
**	If there are no starbases left, you are captured by the
**	Klingons, who torture you mercilessly.  However, if there
**	is at least one starbase, you are returned to the
**	Federation in a prisoner of war exchange.  Of course, this
**	can't happen unless you have taken some prisoners.
**
*/

void abandon(void) 
/* abandon ship */
{
    int nb, l;
    struct quadrant *q;

    chew();
    if (game.condition==docked) {
	if (game.ship!=IHE) {
	    prout(_("You cannot abandon Ye Faerie Queene."));
	    return;
	}
    }
    else {
	/* Must take shuttle craft to exit */
	if (game.damage[DSHUTTL]==-1) {
	    prout(_("Ye Faerie Queene has no shuttle craft."));
	    return;
	}
	if (game.damage[DSHUTTL]<0) {
	    prout(_("Shuttle craft now serving Big Macs."));
	    return;
	}
	if (game.damage[DSHUTTL]>0) {
	    prout(_("Shuttle craft damaged."));
	    return;
	}
	if (game.landed) {
	    prout(_("You must be aboard the ship."));
	    return;
	}
	if (game.iscraft != onship) {
	    prout(_("Shuttle craft not currently available."));
	    return;
	}
	/* Print abandon ship messages */
	skip(1);
	prouts(_("***ABANDON SHIP!  ABANDON SHIP!"));
	skip(1);
	prouts(_("***ALL HANDS ABANDON SHIP!"));
	skip(2);
	prout(_("Captain and crew escape in shuttle craft."));
	if (game.state.rembase==0) {
	    /* Oops! no place to go... */
	    finish(FABANDN);
	    return;
	}
	q = &game.state.galaxy[game.quadrant.x][game.quadrant.y];
	/* Dispose of crew */
	if (!(game.options & OPTION_WORLDS) && !damaged(DTRANSP)) {
	    prout(_("Remainder of ship's complement beam down"));
	    prout(_("to nearest habitable planet."));
	} else if (q->planet != NOPLANET && !damaged(DTRANSP)) {
	    prout(_("Remainder of ship's complement beam down"));
	    prout(_("to %s."), systnames[q->planet]);
	} else {
	    prout(_("Entire crew of %d left to die in outer space."),
		    game.state.crew);
	    game.casual += game.state.crew;
	    game.abandoned += game.state.crew;
	}

	/* If at least one base left, give 'em the Faerie Queene */
	skip(1);
	game.icrystl = false; /* crystals are lost */
	game.nprobes = 0; /* No probes */
	prout(_("You are captured by Klingons and released to"));
	prout(_("the Federation in a prisoner-of-war exchange."));
	nb = Rand()*game.state.rembase+1;
	/* Set up quadrant and position FQ adjacient to base */
	if (!same(game.quadrant, game.state.baseq[nb])) {
	    game.quadrant = game.state.baseq[nb];
	    game.sector.x = game.sector.y = 5;
	    newqad(true);
	}
	for (;;) {
	    /* position next to base by trial and error */
	    game.quad[game.sector.x][game.sector.y] = IHDOT;
	    for_sectors(l) {
		game.sector.x = 3.0*Rand() - 1.0 + game.base.x;
		game.sector.y = 3.0*Rand() - 1.0 + game.base.y;
		if (VALID_SECTOR(game.sector.x, game.sector.y) &&
		    game.quad[game.sector.x][game.sector.y] == IHDOT) break;
	    }
	    if (l < QUADSIZE+1)
		break; /* found a spot */
	    game.sector.x=QUADSIZE/2;
	    game.sector.y=QUADSIZE/2;
	    newqad(true);
	}
    }
    /* Get new commission */
    game.quad[game.sector.x][game.sector.y] = game.ship = IHF;
    game.state.crew = FULLCREW;
    prout(_("Starfleet puts you in command of another ship,"));
    prout(_("the Faerie Queene, which is antiquated but,"));
    prout(_("still useable."));
    if (game.icrystl)
	prout(_("The dilithium crystals have been moved."));
    game.imine = false;
    game.iscraft = offship; /* Galileo disappears */
    /* Resupply ship */
    game.condition=docked;
    for (l = 0; l < NDEVICES; l++) 
	game.damage[l] = 0.0;
    game.damage[DSHUTTL] = -1;
    game.energy = game.inenrg = 3000.0;
    game.shield = game.inshld = 1250.0;
    game.torps = game.intorps = 6;
    game.lsupres=game.inlsr=3.0;
    game.shldup=false;
    game.warpfac=5.0;
    game.wfacsq=25.0;
    return;
}
