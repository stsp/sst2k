#include "sst.h"

void doshield(bool raise) 
/* change shield status */
{
    int key;
    enum {NONE, SHUP, SHDN, NRG} action = NONE;

    game.ididit = false;

    if (raise) 
	action = SHUP;
    else {
	key = scan();
	if (key == IHALPHA) {
	    if (isit("transfer"))
		action = NRG;
	    else {
		chew();
		if (damaged(DSHIELD)) {
		    prout(_("Shields damaged and down."));
		    return;
		}
		if (isit("up"))
		    action = SHUP;
		else if (isit("down"))
		    action = SHDN;
	    }
	}
	if (action==NONE) {
	    proutn(_("Do you wish to change shield energy? "));
	    if (ja() == true) {
		proutn(_("Energy to transfer to shields- "));
		action = NRG;
	    }
	    else if (damaged(DSHIELD)) {
		prout(_("Shields damaged and down."));
		return;
	    }
	    else if (game.shldup) {
		proutn(_("Shields are up. Do you want them down? "));
		if (ja() == true)
		    action = SHDN;
		else {
		    chew();
		    return;
		}
	    }
	    else {
		proutn(_("Shields are down. Do you want them up? "));
		if (ja() == true)
		    action = SHUP;
		else {
		    chew();
		    return;
		}
	    }
	}
    }
    switch (action) {
    case SHUP: /* raise shields */
	if (game.shldup) {
	    prout(_("Shields already up."));
	    return;
	}
	game.shldup = true;
	game.shldchg = true;
	if (game.condition != docked)
	    game.energy -= 50.0;
	prout(_("Shields raised."));
	if (game.energy <= 0) {
	    skip(1);
	    prout(_("Shields raising uses up last of energy."));
	    finish(FNRG);
	    return;
	}
	game.ididit=true;
	return;
    case SHDN:
	if (!game.shldup) {
	    prout(_("Shields already down."));
	    return;
	}
	game.shldup=false;
	game.shldchg=true;
	prout(_("Shields lowered."));
	game.ididit = true;
	return;
    case NRG:
	while (scan() != IHREAL) {
	    chew();
	    proutn(_("Energy to transfer to shields- "));
	}
	chew();
	if (aaitem==0)
	    return;
	if (aaitem > game.energy) {
	    prout(_("Insufficient ship energy."));
	    return;
	}
	game.ididit = true;
	if (game.shield+aaitem >= game.inshld) {
	    prout(_("Shield energy maximized."));
	    if (game.shield+aaitem > game.inshld) {
		prout(_("Excess energy requested returned to ship energy"));
	    }
	    game.energy -= game.inshld-game.shield;
	    game.shield = game.inshld;
	    return;
	}
	if (aaitem < 0.0 && game.energy-aaitem > game.inenrg) {
	    /* Prevent shield drain loophole */
	    skip(1);
	    prout(_("Engineering to bridge--"));
	    prout(_("  Scott here. Power circuit problem, Captain."));
	    prout(_("  I can't drain the shields."));
	    game.ididit = false;
	    return;
	}
	if (game.shield+aaitem < 0) {
	    prout(_("All shield energy transferred to ship."));
	    game.energy += game.shield;
	    game.shield = 0.0;
	    return;
	}
	proutn(_("Scotty- \""));
	if (aaitem > 0)
	    prout(_("Transferring energy to shields.\""));
	else
	    prout(_("Draining energy from shields.\""));
	game.shield += aaitem;
	game.energy -= aaitem;
	return;
    case NONE:;	/* avoid gcc warning */
    }
}

static int randdevice(void)
/* choose a device to damage, at random. */
{
    /*
     * Quoth Eric Allman in the code of BSD-Trek:
     * "Under certain conditions you can get a critical hit.  This
     * sort of hit damages devices.  The probability that a given
     * device is damaged depends on the device.  Well protected
     * devices (such as the computer, which is in the core of the
     * ship and has considerable redundancy) almost never get
     * damaged, whereas devices which are exposed (such as the
     * warp engines) or which are particularly delicate (such as
     * the transporter) have a much higher probability of being
     * damaged."
     *
     * This is one place where OPTION_PLAIN does not restore the
     * original behavior, which was equiprobable damage across
     * all devices.  If we wanted that, we'd return NDEVICES*Rand()
     * and have done with it.  Also, in the original game, DNAVYS
     * and DCOMPTR were the same device. 
     *
     * Instead, we use a table of weights similar to the one from BSD Trek.
     * BSD doesn't have the shuttle, shield controller, death ray, or probes. 
     * We don't have a cloaking device.  The shuttle got the allocation
     * for the cloaking device, then we shaved a half-percent off
     * everything to have some weight to give DSHCTRL/DDRAY/DDSP.
     */
    static int weights[NDEVICES] = {
	105,	/* DSRSENS: short range scanners	10.5% */
	105,	/* DLRSENS: long range scanners		10.5% */
	120,	/* DPHASER: phasers			12.0% */
	120,	/* DPHOTON: photon torpedoes		12.0% */
	25,	/* DLIFSUP: life support		 2.5% */
	65,	/* DWARPEN: warp drive			 6.5% */
	70,	/* DIMPULS: impulse engines		 6.5% */
	145,	/* DSHIELD: deflector shields		14.5% */
	30,	/* DRADIO:  subspace radio		 3.0% */
	45,	/* DSHUTTL: shuttle			 4.5% */
	15,	/* DCOMPTR: computer			 1.5% */
	20,	/* NAVCOMP: navigation system		 2.0% */
	75,	/* DTRANSP: transporter			 7.5% */
	20,	/* DSHCTRL: high-speed shield controller 2.0% */
	10,	/* DDRAY: death ray			 1.0% */
	30,	/* DDSP: deep-space probes		 3.0% */
    };
    int sum, i, idx = Rand() * 1000.0;	/* weights must sum to 1000 */

    for (i = sum = 0; i < NDEVICES; i++) {
	sum += weights[i];
	if (idx < sum)
	    return i;
    }
    return -1;	/* we should never get here, but this quiets GCC */
}

void ram(bool ibumpd, feature ienm, coord w)
/* make our ship ram something */
{
    double hardness, extradm;
    int icas, m, ncrits;
	
    prouts(_("***RED ALERT!  RED ALERT!"));
    skip(1);
    prout(_("***COLLISION IMMINENT."));
    skip(2);
    proutn("***");
    crmshp();
    switch (ienm) {
    case IHR: hardness = 1.5; break;
    case IHC: hardness = 2.0; break;
    case IHS: hardness = 2.5; break;
    case IHT: hardness = 0.5; break;
    case IHQUEST: hardness = 4.0; break;
    default: hardness = 1.0; break;
    }
    proutn(ibumpd ? _(" rammed by ") : _(" rams "));
    crmena(false, ienm, sector, w);
    if (ibumpd)
	proutn(_(" (original position)"));
    skip(1);
    deadkl(w, ienm, game.sector);
    proutn("***");
    crmshp();
    prout(_(" heavily damaged."));
    icas = 10.0+20.0*Rand();
    prout(_("***Sickbay reports %d casualties"), icas);
    game.casual += icas;
    game.state.crew -= icas;
    /*
     * In the pre-SST2K version, all devices got equiprobably damaged,
     * which was silly.  Instead, pick up to half the devices at
     * random according to our weighting table,
     */
    ncrits = Rand() * (NDEVICES/2);
    for (m=0; m < ncrits; m++) {
	int dev = randdevice();
	if (game.damage[dev] < 0) 
	    continue;
	extradm = (10.0*hardness*Rand()+1.0)*game.damfac;
	/* Damage for at least time of travel! */
	game.damage[dev] += game.optime + extradm;
    }
    game.shldup = false;
    prout(_("***Shields are down."));
    if (game.state.remkl + game.state.remcom + game.state.nscrem) {
	announce();
	damagereport();
    }
    else
	finish(FWON);
    return;
}

void torpedo(double course, double r, coord in, double *hit, int i, int n)
/* let a photon torpedo fly */
{
    int l, iquad=0, ll;
    bool shoved = false;
    double ac=course + 0.25*r;
    double angle = (15.0-ac)*0.5235988;
    double bullseye = (15.0 - course)*0.5235988;
    double deltax=-sin(angle), deltay=cos(angle), x=in.x, y=in.y, bigger;
    double ang, temp, xx, yy, kp, h1;
    struct quadrant *q = &game.state.galaxy[game.quadrant.x][game.quadrant.y];
    coord w, jw;

    w.x = w.y = jw.x = jw.y = 0;
    bigger = fabs(deltax);
    if (fabs(deltay) > bigger)
	bigger = fabs(deltay);
    deltax /= bigger;
    deltay /= bigger;
    if (!damaged(DSRSENS) || game.condition==docked) 
	setwnd(srscan_window);
    else 
	setwnd(message_window);
    /* Loop to move a single torpedo */
    for (l=1; l <= 15; l++) {
	x += deltax;
	w.x = x + 0.5;
	y += deltay;
	w.y = y + 0.5;
	if (!VALID_SECTOR(w.x, w.y))
	    break;
	iquad=game.quad[w.x][w.y];
	tracktorpedo(w, l, i, n, iquad);
	if (iquad==IHDOT)
	    continue;
	/* hit something */
	setwnd(message_window);
	if (damaged(DSRSENS) && !game.condition==docked)
	    skip(1);	/* start new line after text track */
	switch(iquad) {
	case IHE: /* Hit our ship */
	case IHF:
	    skip(1);
	    proutn(_("Torpedo hits "));
	    crmshp();
	    prout(".");
	    *hit = 700.0 + 100.0*Rand() -
		1000.0 * distance(w, in) * fabs(sin(bullseye-angle));
	    *hit = fabs(*hit);
	    newcnd(); /* we're blown out of dock */
	    /* We may be displaced. */
	    if (game.landed || game.condition==docked) 
		return; /* Cheat if on a planet */
	    ang = angle + 2.5*(Rand()-0.5);
	    temp = fabs(sin(ang));
	    if (fabs(cos(ang)) > temp)
		temp = fabs(cos(ang));
	    xx = -sin(ang)/temp;
	    yy = cos(ang)/temp;
	    jw.x=w.x+xx+0.5;
	    jw.y=w.y+yy+0.5;
	    if (!VALID_SECTOR(jw.x, jw.y))
		return;
	    if (game.quad[jw.x][jw.y]==IHBLANK) {
		finish(FHOLE);
		return;
	    }
	    if (game.quad[jw.x][jw.y]!=IHDOT) {
		/* can't move into object */
		return;
	    }
	    game.sector = jw;
	    crmshp();
	    shoved = true;
	    break;
					  
	case IHC: /* Hit a commander */
	case IHS:
	    if (Rand() <= 0.05) {
		crmena(true, iquad, sector, w);
		prout(_(" uses anti-photon device;"));
		prout(_("   torpedo neutralized."));
		return;
	    }
	case IHR: /* Hit a regular enemy */
	case IHK:
	    /* find the enemy */
	    for (ll = 1; ll <= game.nenhere; ll++)
		if (same(w, game.ks[ll]))
		    break;
	    kp = fabs(game.kpower[ll]);
	    h1 = 700.0 + 100.0*Rand() -
		1000.0 * distance(w, in) * fabs(sin(bullseye-angle));
	    h1 = fabs(h1);
	    if (kp < h1)
		h1 = kp;
	    game.kpower[ll] -= (game.kpower[ll]<0 ? -h1 : h1);
	    if (game.kpower[ll] == 0) {
		deadkl(w, iquad, w);
		return;
	    }
	    crmena(true, iquad, sector, w);
	    /* If enemy damaged but not destroyed, try to displace */
	    ang = angle + 2.5*(Rand()-0.5);
	    temp = fabs(sin(ang));
	    if (fabs(cos(ang)) > temp)
		temp = fabs(cos(ang));
	    xx = -sin(ang)/temp;
	    yy = cos(ang)/temp;
	    jw.x=w.x+xx+0.5;
	    jw.y=w.y+yy+0.5;
	    if (!VALID_SECTOR(jw.x, jw.y)) {
		prout(_(" damaged but not destroyed."));
		return;
	    }
	    if (game.quad[jw.x][jw.y]==IHBLANK) {
		prout(_(" buffeted into black hole."));
		deadkl(w, iquad, jw);
		return;
	    }
	    if (game.quad[jw.x][jw.y]!=IHDOT) {
		/* can't move into object */
		prout(_(" damaged but not destroyed."));
		return;
	    }
	    proutn(_(" damaged--"));
	    game.ks[ll] = jw;
	    shoved = true;
	    break;
	case IHB: /* Hit a base */
	    skip(1);
	    prout(_("***STARBASE DESTROYED.."));
	    for (ll = 1; ll <= game.state.rembase; ll++) {
		if (same(game.state.baseq[ll], game.quadrant)) {
		    game.state.baseq[ll]=game.state.baseq[game.state.rembase];
		    break;
		}
	    }
	    game.quad[w.x][w.y]=IHDOT;
	    game.state.rembase--;
	    game.base.x=game.base.y=0;
	    q->starbase--;
	    game.state.chart[game.quadrant.x][game.quadrant.y].starbase--;
	    game.state.basekl++;
	    newcnd();
	    return;
	case IHP: /* Hit a planet */
	    crmena(true, iquad, sector, w);
	    prout(_(" destroyed."));
	    game.state.nplankl++;
	    q->planet = NOPLANET;
	    game.state.planets[game.iplnet].pclass = destroyed;
	    game.iplnet = 0;
	    invalidate(game.plnet);
	    game.quad[w.x][w.y] = IHDOT;
	    if (game.landed) {
		/* captain perishes on planet */
		finish(FDPLANET);
	    }
	    return;
	case IHW: /* Hit an inhabited world -- very bad! */
	    crmena(true, iquad, sector, w);
	    prout(_(" destroyed."));
	    game.state.nworldkl++;
	    q->planet = NOPLANET;
	    game.state.planets[game.iplnet].pclass = destroyed;
	    game.iplnet = 0;
	    invalidate(game.plnet);
	    game.quad[w.x][w.y] = IHDOT;
	    if (game.landed) {
		/* captain perishes on planet */
		finish(FDPLANET);
	    }
	    prout(_("You have just destroyed an inhabited planet."));
	    prout(_("Celebratory rallies are being held on the Klingon homeworld."));
	    return;
	case IHSTAR: /* Hit a star */
	    if (Rand() > 0.10) {
		nova(w);
		return;
	    }
	    crmena(true, IHSTAR, sector, w);
	    prout(_(" unaffected by photon blast."));
	    return;
	case IHQUEST: /* Hit a thingy */
	    if (!(game.options & OPTION_THINGY) || Rand()>0.7) {
		skip(1);
		prouts(_("AAAAIIIIEEEEEEEEAAAAAAAAUUUUUGGGGGHHHHHHHHHHHH!!!"));
		skip(1);
		prouts(_("    HACK!     HACK!    HACK!        *CHOKE!*  "));
		skip(1);
		proutn(_("Mr. Spock-"));
		prouts(_("  \"Fascinating!\""));
		skip(1);
		deadkl(w, iquad, w);
	    } else {
		/*
		 * Stas Sergeev added the possibility that
		 * you can shove the Thingy and piss it off.
		 * It then becomes an enemy and may fire at you.
		 */
		iqengry = true;
		shoved = true;
	    }
	    return;
	case IHBLANK: /* Black hole */
	    skip(1);
	    crmena(true, IHBLANK, sector, w);
	    prout(_(" swallows torpedo."));
	    return;
	case IHWEB: /* hit the web */
	    skip(1);
	    prout(_("***Torpedo absorbed by Tholian web."));
	    return;
	case IHT:  /* Hit a Tholian */
	    h1 = 700.0 + 100.0*Rand() -
		1000.0 * distance(w, in) * fabs(sin(bullseye-angle));
	    h1 = fabs(h1);
	    if (h1 >= 600) {
		game.quad[w.x][w.y] = IHDOT;
		game.ithere = false;
		deadkl(w, iquad, w);
		return;
	    }
	    skip(1);
	    crmena(true, IHT, sector, w);
	    if (Rand() > 0.05) {
		prout(_(" survives photon blast."));
		return;
	    }
	    prout(_(" disappears."));
	    game.quad[w.x][w.y] = IHWEB;
	    game.ithere = false;
	    game.nenhere--;
	    dropin(IHBLANK);
	    return;
					
	default: /* Problem! */
	    skip(1);
	    proutn("Don't know how to handle collision with ");
	    crmena(true, iquad, sector, w);
	    skip(1);
	    return;
	}
	break;
    }
    if(curwnd!=message_window) {
	setwnd(message_window);
    }
    if (shoved) {
	game.quad[w.x][w.y]=IHDOT;
	game.quad[jw.x][jw.y]=iquad;
	prout(_(" displaced by blast to %s "), cramlc(sector, jw));
	for (ll = 1; ll <= game.nenhere; ll++)
	    game.kdist[ll] = game.kavgd[ll] = distance(game.sector,game.ks[ll]);
	sortklings();
	return;
    }
    skip(1);
    prout(_("Torpedo missed."));
    return;
}

static void fry(double hit)
/* critical-hit resolution */
{
    double ncrit, extradm;
    int ktr=1, loop1, loop2, j, cdam[NDEVICES];

    /* a critical hit occured */
    if (hit < (275.0-25.0*game.skill)*(1.0+0.5*Rand()))
	return;

    ncrit = 1.0 + hit/(500.0+100.0*Rand());
    proutn(_("***CRITICAL HIT--"));
    /* Select devices and cause damage */
    for (loop1 = 0; loop1 < ncrit; loop1++) {
	do {
	    j = randdevice();
	    /* Cheat to prevent shuttle damage unless on ship */
	} while 
	      (game.damage[j]<0.0 || (j==DSHUTTL && game.iscraft != onship));
	cdam[loop1] = j;
	extradm = (hit*game.damfac)/(ncrit*(75.0+25.0*Rand()));
	game.damage[j] += extradm;
	if (loop1 > 0) {
	    for (loop2 = 0; loop2 < loop1 && j != cdam[loop2]; loop2++) ;
	    if (loop2 < loop1)
		continue;
	    ktr += 1;
	    if (ktr==3)
		skip(1);
	    proutn(_(" and "));
	}
	proutn(device[j]);
    }
    prout(_(" damaged."));
    if (damaged(DSHIELD) && game.shldup) {
	prout(_("***Shields knocked down."));
	game.shldup=false;
    }
}

void attack(bool torps_ok) 
/* bad guy attacks us */
{
    /* torps_ok == false forces use of phasers in an attack */
    int percent, loop, iquad;
    bool usephasers, atackd = false, attempt = false, ihurt = false;
    double hit, pfac, dustfac, hitmax=0.0, hittot=0.0, chgfac=1.0, r;
    coord jay;
    enum loctype where = neither;

    /* game could be over at this point, check */
    if (game.alldone) 
	return;

    if (idebug) 
	prout("=== ATTACK!");

    /* Tholian gewts to move before attacking */
    if (game.ithere) 
	movetholian();

    /* if you have just entered the RNZ, you'll get a warning */
    if (game.neutz) { /* The one chance not to be attacked */
	game.neutz = false;
	return;
    }

    /* commanders get a chance to tac-move towards you */
    if ((((game.comhere || game.ishere) && !game.justin) || game.skill == SKILL_EMERITUS) && torps_ok) 
	moveklings();

    /* if no enemies remain after movement, we're done */
    if (game.nenhere==0 || (game.nenhere==1 && iqhere && !iqengry)) 
	return;

    /* set up partial hits if attack happens during shield status change */
    pfac = 1.0/game.inshld;
    if (game.shldchg)
	chgfac = 0.25+0.5*Rand();

    skip(1);

    /* message verbosity control */
    if (game.skill <= SKILL_FAIR) 
	where = sector;

    for (loop = 1; loop <= game.nenhere; loop++) {
	if (game.kpower[loop] < 0)
	    continue;	/* too weak to attack */
	/* compute hit strength and diminish shield power */
	r = Rand();
	/* Increase chance of photon torpedos if docked or enemy energy low */
	if (game.condition == docked)
	    r *= 0.25;
	if (game.kpower[loop] < 500)
	    r *= 0.25; 
	jay = game.ks[loop];
	iquad = game.quad[jay.x][jay.y];
	if (iquad==IHT || (iquad==IHQUEST && !iqengry))
	    continue;
	/* different enemies have different probabilities of throwing a torp */
	usephasers = !torps_ok || \
	    (iquad == IHK && r > 0.0005) || 
	    (iquad==IHC && r > 0.015) ||
	    (iquad==IHR && r > 0.3) ||
	    (iquad==IHS && r > 0.07) ||
	    (iquad==IHQUEST && r > 0.05);
	if (usephasers) {	    /* Enemy uses phasers */
	    if (game.condition == docked)
		continue; /* Don't waste the effort! */
	    attempt = true; /* Attempt to attack */
	    dustfac = 0.8+0.05*Rand();
	    hit = game.kpower[loop]*pow(dustfac,game.kavgd[loop]);
	    game.kpower[loop] *= 0.75;
	}
	else { /* Enemy uses photon torpedo */
	    double course = 1.90985*atan2((double)game.sector.y-jay.y, (double)jay.x-game.sector.x);
	    hit = 0;
	    proutn(_("***TORPEDO INCOMING"));
	    if (!damaged(DSRSENS)) {
		proutn(_(" From "));
		crmena(false, iquad, where, jay);
	    }
	    attempt = true;
	    prout("  ");
	    r = (Rand()+Rand())*0.5 -0.5;
	    r += 0.002*game.kpower[loop]*r;
	    torpedo(course, r, jay, &hit, 1, 1);
	    if ((game.state.remkl + game.state.remcom + game.state.nscrem)==0) 
		finish(FWON); /* Klingons did themselves in! */
	    if (game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova || game.alldone) 
		return; /* Supernova or finished */
	    if (hit == 0)
		continue;
	}
	/* incoming phaser or torpedo, shields may dissipate it */
	if (game.shldup || game.shldchg || game.condition==docked) {
	    /* shields will take hits */
	    double absorb, hitsh, propor = pfac*game.shield*(game.condition==docked ? 2.1 : 1.0);
	    if (propor < 0.1)
		propor = 0.1;
	    hitsh = propor*chgfac*hit+1.0;
	    absorb = 0.8*hitsh;
	    if (absorb > game.shield)
		absorb = game.shield;
	    game.shield -= absorb;
	    hit -= hitsh;
	    /* taking a hit blasts us out of a starbase dock */
	    if (game.condition == docked)
		dock(false);
	    /* but the shields may take care of it */
	    if (propor > 0.1 && hit < 0.005*game.energy) 
		continue;
	}
	/* hit from this opponent got through shields, so take damage */
	ihurt = true;
	proutn(_("%d unit hit"), (int)hit);
	if ((damaged(DSRSENS) && usephasers) || game.skill<=SKILL_FAIR) {
	    proutn(_(" on the "));
	    crmshp();
	}
	if (!damaged(DSRSENS) && usephasers) {
	    proutn(_(" from "));
	    crmena(false, iquad, where, jay);
	}
	skip(1);
	/* Decide if hit is critical */
	if (hit > hitmax)
	    hitmax = hit;
	hittot += hit;
	fry(hit);
	game.energy -= hit;
    }
    if (game.energy <= 0) {
	/* Returning home upon your shield, not with it... */
	finish(FBATTLE);
	return;
    }
    if (!attempt && game.condition == docked)
	prout(_("***Enemies decide against attacking your ship."));
    if (!atackd)
	return;
    percent = 100.0*pfac*game.shield+0.5;
    if (!ihurt) {
	/* Shields fully protect ship */
	proutn(_("Enemy attack reduces shield strength to "));
    }
    else {
	/* Print message if starship suffered hit(s) */
	skip(1);
	proutn(_("Energy left %2d    shields "), (int)game.energy);
	if (game.shldup)
	    proutn(_("up "));
	else if (!damaged(DSHIELD))
	    proutn(_("down "));
	else
	    proutn(_("damaged, "));
    }
    prout(_("%d%%,   torpedoes left %d"), percent, game.torps);
    /* Check if anyone was hurt */
    if (hitmax >= 200 || hittot >= 500) {
	int icas= hittot*Rand()*0.015;
	if (icas >= 2) {
	    skip(1);
	    prout(_("Mc Coy-  \"Sickbay to bridge.  We suffered %d casualties"), icas);
	    prout(_("   in that last attack.\""));
	    game.casual += icas;
	    game.state.crew -= icas;
	}
    }
    /* After attack, reset average distance to enemies */
    for (loop = 1; loop <= game.nenhere; loop++)
	game.kavgd[loop] = game.kdist[loop];
    sortklings();
    return;
}
		
void deadkl(coord w, feature type, coord mv)
/* kill a Klingon, Tholian, Romulan, or Thingy */
{
    /* Added mv to allow enemy to "move" before dying */
    int i,j;

    crmena(true, type, sector, mv);
    /* Decide what kind of enemy it is and update appropriately */
    if (type == IHR) {
	/* chalk up a Romulan */
	game.state.galaxy[game.quadrant.x][game.quadrant.y].romulans--;
	game.irhere--;
	game.state.nromrem--;
    }
    else if (type == IHT) {
	/* Killed a Tholian */
	game.ithere = false;
    }
    else if (type == IHQUEST) {
	/* Killed a Thingy */
	iqhere = iqengry = false;
	invalidate(thing);
    }
    else {
	/* Some type of a Klingon */
	game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons--;
	game.klhere--;
	switch (type) {
	case IHC:
	    game.comhere = false;
	    for (i = 1; i <= game.state.remcom; i++)
		if (same(game.state.kcmdr[i], game.quadrant)) 
		    break;
	    game.state.kcmdr[i] = game.state.kcmdr[game.state.remcom];
	    game.state.kcmdr[game.state.remcom].x = 0;
	    game.state.kcmdr[game.state.remcom].y = 0;
	    game.state.remcom--;
	    unschedule(FTBEAM);
	    if (game.state.remcom != 0)
		schedule(FTBEAM, expran(1.0*game.incom/game.state.remcom));
	    break;
	case IHK:
	    game.state.remkl--;
	    break;
	case IHS:
	    game.state.nscrem--;
	    game.ishere = false;
	    game.state.kscmdr.x = game.state.kscmdr.y = game.isatb = 0;
	    game.iscate = false;
	    unschedule(FSCMOVE);
	    unschedule(FSCDBAS);
	    break;
	default:	/* avoids a gcc warning */
	    prout("*** Internal error, deadkl() called on %c\n", type);
	    break;
	}
    }

    /* For each kind of enemy, finish message to player */
    prout(_(" destroyed."));
    game.quad[w.x][w.y] = IHDOT;
    if ((game.state.remkl + game.state.remcom + game.state.nscrem)==0)
	return;

    game.state.remtime = game.state.remkl + game.state.remcom > 0 ?
	    game.state.remres/(game.state.remkl + 4*game.state.remcom) : 99;

    /* Remove enemy ship from arrays describing local conditions */
    if (is_scheduled(FCDBAS) && same(game.battle, game.quadrant) && type==IHC)
	unschedule(FCDBAS);
    for (i = 1; i <= game.nenhere; i++)
	if (same(game.ks[i], w))
	    break;
    game.nenhere--;
    if (i <= game.nenhere)  {
	for (j=i; j<=game.nenhere; j++) {
	    game.ks[j] = game.ks[j+1];
	    game.kpower[j] = game.kpower[j+1];
	    game.kavgd[j] = game.kdist[j] = game.kdist[j+1];
	}
    }
    game.ks[game.nenhere+1].x = 0;
    game.ks[game.nenhere+1].x = 0;
    game.kdist[game.nenhere+1] = 0;
    game.kavgd[game.nenhere+1] = 0;
    game.kpower[game.nenhere+1] = 0;
    return;
}

static bool targetcheck(double x, double y, double *course) 
{
    double deltx, delty;
    /* Return true if target is invalid */
    if (!VALID_SECTOR(x, y)) {
	huh();
	return true;
    }
    deltx = 0.1*(y - game.sector.y);
    delty = 0.1*(game.sector.x - x);
    if (deltx==0 && delty== 0) {
	skip(1);
	prout(_("Spock-  \"Bridge to sickbay.  Dr. McCoy,"));
	prout(_("  I recommend an immediate review of"));
	prout(_("  the Captain's psychological profile.\""));
	chew();
	return true;
    }
    *course = 1.90985932*atan2(deltx, delty);
    return false;
}

void torps(void) 
/* launch photon torpedo salvo */
{
    double targ[4][3], course[4];
    double r, dummy;
    int key, n, i;

    game.ididit = false;

    if (damaged(DPHOTON)) {
	prout(_("Photon tubes damaged."));
	chew();
	return;
    }
    if (game.torps == 0) {
	prout(_("No torpedoes left."));
	chew();
	return;
    }
    key = scan();
    for (;;) {
	if (key == IHALPHA) {
	    huh();
	    return;
	}
	else if (key == IHEOL) {
	    prout(_("%d torpedoes left."), game.torps);
	    proutn(_("Number of torpedoes to fire- "));
	    key = scan();
	}
	else /* key == IHREAL */ {
	    n = aaitem + 0.5;
	    if (n <= 0) { /* abort command */
		chew();
		return;
	    }
	    if (n > 3) {
		chew();
		prout(_("Maximum of 3 torpedoes per burst."));
		key = IHEOL;
		return;
	    }
	    if (n <= game.torps)
		break;
	    chew();
	    key = IHEOL;
	}
    }
    for (i = 1; i <= n; i++) {
	key = scan();
	if (i==1 && key == IHEOL) {
	    break;	/* we will try prompting */
	}
	if (i==2 && key == IHEOL) {
	    /* direct all torpedoes at one target */
	    while (i <= n) {
		targ[i][1] = targ[1][1];
		targ[i][2] = targ[1][2];
		course[i] = course[1];
		i++;
	    }
	    break;
	}
	if (key != IHREAL) {
	    huh();
	    return;
	}
	targ[i][1] = aaitem;
	key = scan();
	if (key != IHREAL) {
	    huh();
	    return;
	}
	targ[i][2] = aaitem;
	if (targetcheck(targ[i][1], targ[i][2], &course[i]))
	    return;
    }
    chew();
    if (i == 1 && key == IHEOL) {
	/* prompt for each one */
	for (i = 1; i <= n; i++) {
	    proutn(_("Target sector for torpedo number %d- "), i);
	    key = scan();
	    if (key != IHREAL) {
		huh();
		return;
	    }
	    targ[i][1] = aaitem;
	    key = scan();
	    if (key != IHREAL) {
		huh();
		return;
	    }
	    targ[i][2] = aaitem;
	    chew();
	    if (targetcheck(targ[i][1], targ[i][2], &course[i]))
		return;
	}
    }
    game.ididit = true;
    /* Loop for moving <n> torpedoes */
    for (i = 1; i <= n; i++) {
	if (game.condition != docked)
	    game.torps--;
	r = (Rand()+Rand())*0.5 -0.5;
	if (fabs(r) >= 0.47) {
	    /* misfire! */
	    r = (Rand()+1.2) * r;
	    if (n>1) {
		prouts(_("***TORPEDO NUMBER %d MISFIRES"), i);
	    }
	    else
		prouts(_("***TORPEDO MISFIRES."));
	    skip(1);
	    if (i < n)
		prout(_("  Remainder of burst aborted."));
	    if (Rand() <= 0.2) {
		prout(_("***Photon tubes damaged by misfire."));
		game.damage[DPHOTON] = game.damfac*(1.0+2.0*Rand());
	    }
	    break;
	}
	if (game.shldup || game.condition == docked) 
	    r *= 1.0 + 0.0001*game.shield;
	torpedo(course[i], r, game.sector, &dummy, i, n);
	if (game.alldone || game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova)
	    return;
    }
    if ((game.state.remkl + game.state.remcom + game.state.nscrem)==0)
	finish(FWON);
}

	

static void overheat(double rpow)
/* check for phasers overheating */
{
    if (rpow > 1500) {
	double chekbrn = (rpow-1500.)*0.00038;
	if (Rand() <= chekbrn) {
	    prout(_("Weapons officer Sulu-  \"Phasers overheated, sir.\""));
	    game.damage[DPHASER] = game.damfac*(1.0 + Rand()) * (1.0+chekbrn);
	}
    }
}

static bool checkshctrl(double rpow) 
/* check shield control */
{
    double hit;
    int icas;
	
    skip(1);
    if (Rand() < 0.998) {
	prout(_("Shields lowered."));
	return false;
    }
    /* Something bad has happened */
    prouts(_("***RED ALERT!  RED ALERT!"));
    skip(2);
    hit = rpow*game.shield/game.inshld;
    game.energy -= rpow+hit*0.8;
    game.shield -= hit*0.2;
    if (game.energy <= 0.0) {
	prouts(_("Sulu-  \"Captain! Shield malf***********************\""));
	skip(1);
	stars();
	finish(FPHASER);
	return true;
    }
    prouts(_("Sulu-  \"Captain! Shield malfunction! Phaser fire contained!\""));
    skip(2);
    prout(_("Lt. Uhura-  \"Sir, all decks reporting damage.\""));
    icas = hit*Rand()*0.012;
    skip(1);
    fry(0.8*hit);
    if (icas) {
	skip(1);
	prout(_("McCoy to bridge- \"Severe radiation burns, Jim."));
	prout(_("  %d casualties so far.\""), icas);
	game.casual += icas;
	game.state.crew -= icas;
    }
    skip(1);
    prout(_("Phaser energy dispersed by shields."));
    prout(_("Enemy unaffected."));
    overheat(rpow);
    return true;
}
	

void phasers(void) 
/* fire phasers */
{
    double hits[21], rpow=0, extra, powrem, over, temp;
    int kz = 0, k=1, i, irec=0; /* Cheating inhibitor */
    bool ifast = false, no = false, itarg = true, msgflag = true;
    enum {NOTSET, MANUAL, FORCEMAN, AUTOMATIC} automode = NOTSET;
    int key=0;

    skip(1);
    /* SR sensors and Computer are needed fopr automode */
    if (damaged(DSRSENS) || damaged(DCOMPTR)) 
	itarg = false;
    if (game.condition == docked) {
	prout(_("Phasers can't be fired through base shields."));
	chew();
	return;
    }
    if (damaged(DPHASER)) {
	prout(_("Phaser control damaged."));
	chew();
	return;
    }
    if (game.shldup) {
	if (damaged(DSHCTRL)) {
	    prout(_("High speed shield control damaged."));
	    chew();
	    return;
	}
	if (game.energy <= 200.0) {
	    prout(_("Insufficient energy to activate high-speed shield control."));
	    chew();
	    return;
	}
	prout(_("Weapons Officer Sulu-  \"High-speed shield control enabled, sir.\""));
	ifast = true;
		
    }
    /* Original code so convoluted, I re-did it all */
    while (automode==NOTSET) {
	key=scan();
	if (key == IHALPHA) {
	    if (isit("manual")) {
		if (game.nenhere==0) {
		    prout(_("There is no enemy present to select."));
		    chew();
		    key = IHEOL;
		    automode=AUTOMATIC;
		}
		else {
		    automode = MANUAL;
		    key = scan();
		}
	    }
	    else if (isit("automatic")) {
		if ((!itarg) && game.nenhere != 0) {
		    automode = FORCEMAN;
		}
		else {
		    if (game.nenhere==0)
			prout(_("Energy will be expended into space."));
		    automode = AUTOMATIC;
		    key = scan();
		}
	    }
	    else if (isit("no")) {
		no = true;
	    }
	    else {
		huh();
		return;
	    }
	}
	else if (key == IHREAL) {
	    if (game.nenhere==0) {
		prout(_("Energy will be expended into space."));
		automode = AUTOMATIC;
	    }
	    else if (!itarg)
		automode = FORCEMAN;
	    else
		automode = AUTOMATIC;
	}
	else {
	    /* IHEOL */
	    if (game.nenhere==0) {
		prout(_("Energy will be expended into space."));
		automode = AUTOMATIC;
	    }
	    else if (!itarg)
		automode = FORCEMAN;
	    else 
		proutn(_("Manual or automatic? "));
	}
    }
				
    switch (automode) {
    case AUTOMATIC:
	if (key == IHALPHA && isit("no")) {
	    no = true;
	    key = scan();
	}
	if (key != IHREAL && game.nenhere != 0) {
	    prout(_("Phasers locked on target. Energy available: %.2f"),
		  ifast?game.energy-200.0:game.energy);
	}
	irec=0;
	do {
	    chew();
	    if (!kz)
		for (i = 1; i <= game.nenhere; i++)
		    irec += fabs(game.kpower[i])/(PHASEFAC*pow(0.90,game.kdist[i]))*
			(1.01+0.05*Rand()) + 1.0;
	    kz=1;
	    proutn(_("%d units required. "), irec);
	    chew();
	    proutn(_("Units to fire= "));
	    key = scan();
	    if (key!=IHREAL)
		return;
	    rpow = aaitem;
	    if (rpow > (ifast?game.energy-200:game.energy)) {
		proutn(_("Energy available= %.2f"),
		       ifast?game.energy-200:game.energy);
		skip(1);
		key = IHEOL;
	    }
	} while (rpow > (ifast?game.energy-200:game.energy));
	if (rpow<=0) {
	    /* chicken out */
	    chew();
	    return;
	}
	if ((key=scan()) == IHALPHA && isit("no")) {
	    no = true;
	}
	if (ifast) {
	    game.energy -= 200; /* Go and do it! */
	    if (checkshctrl(rpow))
		return;
	}
	chew();
	game.energy -= rpow;
	extra = rpow;
	if (game.nenhere) {
	    extra = 0.0;
	    powrem = rpow;
	    for (i = 1; i <= game.nenhere; i++) {
		hits[i] = 0.0;
		if (powrem <= 0)
		    continue;
		hits[i] = fabs(game.kpower[i])/(PHASEFAC*pow(0.90,game.kdist[i]));
		over = (0.01 + 0.05*Rand())*hits[i];
		temp = powrem;
		powrem -= hits[i] + over;
		if (powrem <= 0 && temp < hits[i])
		    hits[i] = temp;
		if (powrem <= 0)
		    over = 0.0;
		extra += over;
	    }
	    if (powrem > 0.0)
		extra += powrem;
	    hittem(hits);
	    game.ididit = true;
	}
	if (extra > 0 && !game.alldone) {
	    if (game.ithere) {
		proutn(_("*** Tholian web absorbs "));
		if (game.nenhere>0)
		    proutn(_("excess "));
		prout(_("phaser energy."));
	    }
	    else {
		prout(_("%d expended on empty space."), (int)extra);
	    }
	}
	break;

    case FORCEMAN:
	chew();
	key = IHEOL;
	if (damaged(DCOMPTR))
	    prout(_("Battle computer damaged, manual fire only."));
	else {
	    skip(1);
	    prouts(_("---WORKING---"));
	    skip(1);
	    prout(_("Short-range-sensors-damaged"));
	    prout(_("Insufficient-data-for-automatic-phaser-fire"));
	    prout(_("Manual-fire-must-be-used"));
	    skip(1);
	}
    case MANUAL:
	rpow = 0.0;
	for (k = 1; k <= game.nenhere;) {
	    coord aim = game.ks[k];
	    int ienm = game.quad[aim.x][aim.y];
	    if (msgflag) {
		proutn(_("Energy available= %.2f"),
		       game.energy-.006-(ifast?200:0));
		skip(1);
		msgflag = false;
		rpow = 0.0;
	    }
	    if (damaged(DSRSENS) && !(abs(game.sector.x-aim.x) < 2 && abs(game.sector.y-aim.y) < 2) &&
		(ienm == IHC || ienm == IHS)) {
		cramen(ienm);
		prout(_(" can't be located without short range scan."));
		chew();
		key = IHEOL;
		hits[k] = 0; /* prevent overflow -- thanks to Alexei Voitenko */
		k++;
		continue;
	    }
	    if (key == IHEOL) {
		chew();
		if (itarg && k > kz)
		    irec=(fabs(game.kpower[k])/(PHASEFAC*pow(0.9,game.kdist[k])))*
			(1.01+0.05*Rand()) + 1.0;
		kz = k;
		proutn("(");
		if (!damaged(DCOMPTR))
		    proutn("%d", irec);
		else
		    proutn("??");
		proutn(")  ");
		proutn(_("units to fire at "));
		crmena(false, ienm, sector, aim);
		proutn("-  ");
		key = scan();
	    }
	    if (key == IHALPHA && isit("no")) {
		no = true;
		key = scan();
		continue;
	    }
	    if (key == IHALPHA) {
		huh();
		return;
	    }
	    if (key == IHEOL) {
		if (k==1) { /* Let me say I'm baffled by this */
		    msgflag = true;
		}
		continue;
	    }
	    if (aaitem < 0) {
		/* abort out */
		chew();
		return;
	    }
	    hits[k] = aaitem;
	    rpow += aaitem;
	    /* If total requested is too much, inform and start over */
				
	    if (rpow > (ifast?game.energy-200:game.energy)) {
		prout(_("Available energy exceeded -- try again."));
		chew();
		return;
	    }
	    key = scan(); /* scan for next value */
	    k++;
	}
	if (rpow == 0.0) {
	    /* zero energy -- abort */
	    chew();
	    return;
	}
	if (key == IHALPHA && isit("no")) {
	    no = true;
	}
	game.energy -= rpow;
	chew();
	if (ifast) {
	    game.energy -= 200.0;
	    if (checkshctrl(rpow))
		return;
	}
	hittem(hits);
	game.ididit = true;
    case NOTSET:;	/* avoid gcc warning */
    }
    /* Say shield raised or malfunction, if necessary */
    if (game.alldone) 
	return;
    if (ifast) {
	skip(1);
	if (no == 0) {
	    if (Rand() >= 0.99) {
		prout(_("Sulu-  \"Sir, the high-speed shield control has malfunctioned . . ."));
		prouts(_("         CLICK   CLICK   POP  . . ."));
		prout(_(" No response, sir!"));
		game.shldup = false;
	    }
	    else
		prout(_("Shields raised."));
	}
	else
	    game.shldup = false;
    }
    overheat(rpow);
}

void hittem(double *hits) 
/* register a phaser hit on Klingons and Romulans */
{
    double kp, kpow, wham, hit, dustfac, kpini;
    int nenhr2=game.nenhere, k=1, kk=1, ienm;
    coord w;

    skip(1);

    for (; k <= nenhr2; k++, kk++) {
	if ((wham = hits[k])==0)
	    continue;
	dustfac = 0.9 + 0.01*Rand();
	hit = wham*pow(dustfac,game.kdist[kk]);
	kpini = game.kpower[kk];
	kp = fabs(kpini);
	if (PHASEFAC*hit < kp)
	    kp = PHASEFAC*hit;
	game.kpower[kk] -= (game.kpower[kk] < 0 ? -kp: kp);
	kpow = game.kpower[kk];
	w = game.ks[kk];
	if (hit > 0.005) {
	    if (!damaged(DSRSENS))
		boom(w);
	    proutn(_("%d unit hit on "), (int)hit);
	}
	else
	    proutn(_("Very small hit on "));
	ienm = game.quad[w.x][w.y];
	if (ienm==IHQUEST)
	    iqengry = true;
	crmena(false,ienm,sector,w);
	skip(1);
	if (kpow == 0) {
	    deadkl(w, ienm, w);
	    if ((game.state.remkl + game.state.remcom + game.state.nscrem)==0)
		finish(FWON);		
	    if (game.alldone)
		return;
	    kk--; /* don't do the increment */
	}
	else /* decide whether or not to emasculate klingon */
	    if (kpow > 0 && Rand() >= 0.9 &&
		kpow <= ((0.4 + 0.4*Rand())*kpini)) {
		prout(_("***Mr. Spock-  \"Captain, the vessel at %s"),
		      cramlc(sector, w));
		prout(_("   has just lost its firepower.\""));
		game.kpower[kk] = -kpow;
	    }
    }
    return;
}

