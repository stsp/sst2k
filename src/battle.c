#include "sst.h"

void doshield(int i) 
{
    int key;
    enum {NONE, SHUP, SHDN, NRG} action = NONE;

    game.ididit = 0;

    if (i == 2) action = SHUP;
    else {
	key = scan();
	if (key == IHALPHA) {
	    if (isit("transfer"))
		action = NRG;
	    else {
		chew();
		if (game.damage[DSHIELD]) {
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
	    if (ja()) {
		proutn(_("Energy to transfer to shields- "));
		action = NRG;
	    }
	    else if (game.damage[DSHIELD]) {
		prout(_("Shields damaged and down."));
		return;
	    }
	    else if (game.shldup) {
		proutn(_("Shields are up. Do you want them down? "));
		if (ja()) action = SHDN;
		else {
		    chew();
		    return;
		}
	    }
	    else {
		proutn(_("Shields are down. Do you want them up? "));
		if (ja()) action = SHUP;
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
	game.shldup = 1;
	game.shldchg = 1;
	if (game.condit != IHDOCKED) game.energy -= 50.0;
	prout(_("Shields raised."));
	if (game.energy <= 0) {
	    skip(1);
	    prout(_("Shields raising uses up last of energy."));
	    finish(FNRG);
	    return;
	}
	game.ididit=1;
	return;
    case SHDN:
	if (game.shldup==0) {
	    prout(_("Shields already down."));
	    return;
	}
	game.shldup=0;
	game.shldchg=1;
	prout(_("Shields lowered."));
	game.ididit=1;
	return;
    case NRG:
	while (scan() != IHREAL) {
	    chew();
	    proutn(_("Energy to transfer to shields- "));
	}
	chew();
	if (aaitem==0) return;
	if (aaitem > game.energy) {
	    prout(_("Insufficient ship energy."));
	    return;
	}
	game.ididit = 1;
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
	    game.ididit = 0;
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

void ram(int ibumpd, int ienm, int ix, int iy)
{
    double type = 1.0, extradm;
    int icas, l;
	
    prouts(_("***RED ALERT!  RED ALERT!"));
    skip(1);
    prout(_("***COLLISION IMMINENT."));
    skip(2);
    proutn("***");
    crmshp();
    switch (ienm) {
    case IHR: type = 1.5; break;
    case IHC: type = 2.0; break;
    case IHS: type = 2.5; break;
    case IHT: type = 0.5; break;
    case IHQUEST: type = 4.0; break;
    }
    proutn(ibumpd ? _(" rammed by ") : _(" rams "));
    crmena(0, ienm, 2, ix, iy);
    if (ibumpd) proutn(_(" (original position)"));
    skip(1);
    deadkl(ix, iy, ienm, game.sectx, game.secty);
    proutn("***");
    crmshp();
    prout(_(" heavily damaged."));
    icas = 10.0+20.0*Rand();
    prout(_("***Sickbay reports %d casualties"), icas);
    game.casual += icas;
    for (l=0; l < NDEVICES; l++) {
	if (l == DDRAY) 
	    continue; // Don't damage deathray 
	if (game.damage[l] < 0) 
	    continue;
	extradm = (10.0*type*Rand()+1.0)*game.damfac;
	game.damage[l] += game.optime + extradm; /* Damage for at least time of travel! */
    }
    game.shldup = 0;
    if (KLINGREM) {
	pause_game(2);
	dreprt();
    }
    else finish(FWON);
    return;
}

void torpedo(double course, double r, int inx, int iny, double *hit, int i, int n)
{
    int l, iquad=0, ix=0, iy=0, jx=0, jy=0, shoved=0, ll;
	
    double ac=course + 0.25*r;
    double angle = (15.0-ac)*0.5235988;
    double bullseye = (15.0 - course)*0.5235988;
    double deltax=-sin(angle), deltay=cos(angle), x=inx, y=iny, bigger;
    double ang, temp, xx, yy, kp, h1;

    bigger = fabs(deltax);
    if (fabs(deltay) > bigger) bigger = fabs(deltay);
    deltax /= bigger;
    deltay /= bigger;
    if (game.damage[DSRSENS]==0 || game.condit==IHDOCKED) 
	setwnd(srscan_window);
    else 
	setwnd(message_window);
    /* Loop to move a single torpedo */
    for (l=1; l <= 15; l++) {
	x += deltax;
	ix = x + 0.5;
	y += deltay;
	iy = y + 0.5;
	if (!VALID_SECTOR(ix, iy)) break;
	iquad=game.quad[ix][iy];
	tracktorpedo(ix, iy, l, i, n, iquad);
	if (iquad==IHDOT) continue;
	/* hit something */
	setwnd(message_window);
	skip(1);	/* start new line after text track */
	switch(iquad) {
	case IHE: /* Hit our ship */
	case IHF:
	    skip(1);
	    proutn(_("Torpedo hits "));
	    crmshp();
	    prout(".");
	    *hit = 700.0 + 100.0*Rand() -
		1000.0*sqrt(square(ix-inx)+square(iy-iny))*
		fabs(sin(bullseye-angle));
	    *hit = fabs(*hit);
	    newcnd(); /* we're blown out of dock */
	    /* We may be displaced. */
	    if (game.landed==1 || game.condit==IHDOCKED) return; /* Cheat if on a planet */
	    ang = angle + 2.5*(Rand()-0.5);
	    temp = fabs(sin(ang));
	    if (fabs(cos(ang)) > temp) temp = fabs(cos(ang));
	    xx = -sin(ang)/temp;
	    yy = cos(ang)/temp;
	    jx=ix+xx+0.5;
	    jy=iy+yy+0.5;
	    if (!VALID_SECTOR(jx, jy)) return;
	    if (game.quad[jx][jy]==IHBLANK) {
		finish(FHOLE);
		return;
	    }
	    if (game.quad[jx][jy]!=IHDOT) {
		/* can't move into object */
		return;
	    }
	    game.sectx = jx;
	    game.secty = jy;
	    crmshp();
	    shoved = 1;
	    break;
					  
	case IHC: /* Hit a commander */
	case IHS:
	    if (Rand() <= 0.05) {
		crmena(1, iquad, 2, ix, iy);
		prout(_(" uses anti-photon device;"));
		prout(_("   torpedo neutralized."));
		return;
	    }
	case IHR: /* Hit a regular enemy */
	case IHK:
	    /* find the enemy */
	    for_local_enemies(ll)
		if (ix==game.kx[ll] && iy==game.ky[ll]) break;
	    kp = fabs(game.kpower[ll]);
	    h1 = 700.0 + 100.0*Rand() -
		1000.0*sqrt(square(ix-inx)+square(iy-iny))*
		fabs(sin(bullseye-angle));
	    h1 = fabs(h1);
	    if (kp < h1) h1 = kp;
	    game.kpower[ll] -= (game.kpower[ll]<0 ? -h1 : h1);
	    if (game.kpower[ll] == 0) {
		deadkl(ix, iy, iquad, ix, iy);
		return;
	    }
	    crmena(1, iquad, 2, ix, iy);
	    /* If enemy damaged but not destroyed, try to displace */
	    ang = angle + 2.5*(Rand()-0.5);
	    temp = fabs(sin(ang));
	    if (fabs(cos(ang)) > temp) temp = fabs(cos(ang));
	    xx = -sin(ang)/temp;
	    yy = cos(ang)/temp;
	    jx=ix+xx+0.5;
	    jy=iy+yy+0.5;
	    if (!VALID_SECTOR(jx, jy)) {
		prout(_(" damaged but not destroyed."));
		return;
	    }
	    if (game.quad[jx][jy]==IHBLANK) {
		prout(_(" buffeted into black hole."));
		deadkl(ix, iy, iquad, jx, jy);
		return;
	    }
	    if (game.quad[jx][jy]!=IHDOT) {
		/* can't move into object */
		prout(_(" damaged but not destroyed."));
		return;
	    }
	    proutn(_(" damaged--"));
	    game.kx[ll] = jx;
	    game.ky[ll] = jy;
	    shoved = 1;
	    break;
	case IHB: /* Hit a base */
	    skip(1);
	    prout(_("***STARBASE DESTROYED.."));
	    for_starbases(ll) {
		if (game.state.baseqx[ll]==game.quadx && game.state.baseqy[ll]==game.quady) {
		    game.state.baseqx[ll]=game.state.baseqx[game.state.rembase];
		    game.state.baseqy[ll]=game.state.baseqy[game.state.rembase];
		    break;
		}
	    }
	    game.quad[ix][iy]=IHDOT;
	    game.state.rembase--;
	    game.basex=game.basey=0;
	    game.state.galaxy[game.quadx][game.quady].starbase--;
	    game.state.chart[game.quadx][game.quady].starbase--;
	    game.state.basekl++;
	    newcnd();
	    return;
	case IHP: /* Hit a planet */
	    crmena(1, iquad, 2, ix, iy);
	    prout(_(" destroyed."));
	    game.state.nplankl++;
	    game.state.galaxy[game.quadx][game.quady].planet = NULL;
	    DESTROY(&game.state.plnets[game.iplnet]);
	    game.iplnet = 0;
	    game.plnetx = game.plnety = 0;
	    game.quad[ix][iy] = IHDOT;
	    if (game.landed==1) {
		/* captain perishes on planet */
		finish(FDPLANET);
	    }
	    return;
	case IHSTAR: /* Hit a star */
	    if (Rand() > 0.10) {
		nova(ix, iy);
		return;
	    }
	    crmena(1, IHSTAR, 2, ix, iy);
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
		deadkl(ix, iy, iquad, ix, iy);
	    } else {
		/*
		 * Stas Sergeev added the possibility that
		 * you can shove the Thingy and piss it off.
		 * It then becomes an enemy and may fire at you.
		 */
		iqengry=1;
		shoved=1;
	    }
	    return;
	case IHBLANK: /* Black hole */
	    skip(1);
	    crmena(1, IHBLANK, 2, ix, iy);
	    prout(_(" swallows torpedo."));
	    return;
	case IHWEB: /* hit the web */
	    skip(1);
	    prout(_("***Torpedo absorbed by Tholian web."));
	    return;
	case IHT:  /* Hit a Tholian */
	    h1 = 700.0 + 100.0*Rand() -
		1000.0*sqrt(square(ix-inx)+square(iy-iny))*
		fabs(sin(bullseye-angle));
	    h1 = fabs(h1);
	    if (h1 >= 600) {
		game.quad[ix][iy] = IHDOT;
		game.ithere = 0;
		game.ithx = game.ithy = 0;
		deadkl(ix, iy, iquad, ix, iy);
		return;
	    }
	    skip(1);
	    crmena(1, IHT, 2, ix, iy);
	    if (Rand() > 0.05) {
		prout(_(" survives photon blast."));
		return;
	    }
	    prout(_(" disappears."));
	    game.quad[ix][iy] = IHWEB;
	    game.ithere = game.ithx = game.ithy = 0;
	    game.nenhere--;
	    {
		int dum, my;
		dropin(IHBLANK, &dum, &my);
	    }
	    return;
					
	default: /* Problem! */
	    skip(1);
	    proutn("Don't know how to handle collision with ");
	    crmena(1, iquad, 2, ix, iy);
	    skip(1);
	    return;
	}
	break;
    }
    if(curwnd!=message_window) {
	setwnd(message_window);
    }
    if (shoved) {
	game.quad[jx][jy]=iquad;
	game.quad[ix][iy]=IHDOT;
	prout(_(" displaced by blast to %s "), cramlc(sector, jx, jy));
	for_local_enemies(ll)
	    game.kdist[ll] = game.kavgd[ll] = sqrt(square(game.sectx-game.kx[ll])+square(game.secty-game.ky[ll]));
	sortkl();
	return;
    }
    skip(1);
    prout(_("Torpedo missed."));
    return;
}

static void fry(double hit) 
{
    double ncrit, extradm;
    int ktr=1, l, ll, j, cdam[NDEVICES];

    /* a critical hit occured */
    if (hit < (275.0-25.0*game.skill)*(1.0+0.5*Rand())) return;

    ncrit = 1.0 + hit/(500.0+100.0*Rand());
    proutn(_("***CRITICAL HIT--"));
    /* Select devices and cause damage */
    for (l = 0; l < ncrit && 0 < NDEVICES; l++) {
	do {
	    j = NDEVICES*Rand();
	    /* Cheat to prevent shuttle damage unless on ship */
	} while 
	      (game.damage[j]<0.0 || (j==DSHUTTL && game.iscraft!=1) || j==DDRAY);
	cdam[l] = j;
	extradm = (hit*game.damfac)/(ncrit*(75.0+25.0*Rand()));
	game.damage[j] += extradm;
	if (l > 0) {
	    for (ll=2; ll<=l && j != cdam[ll-1]; ll++) ;
	    if (ll<=l) continue;
	    ktr += 1;
	    if (ktr==3) skip(1);
	    proutn(_(" and "));
	}
	proutn(device[j]);
    }
    prout(_(" damaged."));
    if (game.damage[DSHIELD] && game.shldup) {
	prout(_("***Shields knocked down."));
	game.shldup=0;
    }
}

void attack(int torps_ok) 
{
    /* torps_ok == 0 forces use of phasers in an attack */
    int percent, ihurt=0, l, i=0, jx, jy, iquad, itflag;
    int atackd = 0, attempt = 0;
    double hit;
    double pfac, dustfac, hitmax=0.0, hittot=0.0, chgfac=1.0, r;

    game.iattak = 1;
    if (game.alldone) return;
#ifdef DEBUG
    if (game.idebug) prout("ATTACK!");
#endif

    if (game.ithere) movetho();

    if (game.neutz) { /* The one chance not to be attacked */
	game.neutz = 0;
	return;
    }
    if ((((game.comhere || game.ishere) && (game.justin == 0)) || game.skill == SKILL_EMERITUS)&&(torps_ok!=0)) movcom();
    if (game.nenhere==0 || (game.nenhere==1 && iqhere && iqengry==0)) return;
    pfac = 1.0/game.inshld;
    if (game.shldchg == 1) chgfac = 0.25+0.5*Rand();
    skip(1);
    if (game.skill <= SKILL_FAIR) i = 2;
    for_local_enemies(l) {
	if (game.kpower[l] < 0) continue;	/* too weak to attack */
	/* compute hit strength and diminsh shield power */
	r = Rand();
	/* Increase chance of photon torpedos if docked or enemy energy low */
	if (game.condit == IHDOCKED) r *= 0.25;
	if (game.kpower[l] < 500) r *= 0.25; 
	jx = game.kx[l];
	jy = game.ky[l];
	iquad = game.quad[jx][jy];
	if (iquad==IHT || (iquad==IHQUEST && !iqengry)) continue;
	itflag = (iquad == IHK && r > 0.0005) || !torps_ok ||
	    (iquad==IHC && r > 0.015) ||
	    (iquad==IHR && r > 0.3) ||
	    (iquad==IHS && r > 0.07) ||
	    (iquad==IHQUEST && r > 0.05);
	if (itflag) {
	    /* Enemy uses phasers */
	    if (game.condit == IHDOCKED) continue; /* Don't waste the effort! */
	    attempt = 1; /* Attempt to attack */
	    dustfac = 0.8+0.05*Rand();
	    hit = game.kpower[l]*pow(dustfac,game.kavgd[l]);
	    game.kpower[l] *= 0.75;
	}
	else { /* Enemy used photon torpedo */
	    double course = 1.90985*atan2((double)game.secty-jy, (double)jx-game.sectx);
	    hit = 0;
	    proutn(_("***TORPEDO INCOMING"));
	    if (game.damage[DSRSENS] <= 0.0) {
		proutn(_(" From "));
		crmena(0, iquad, i, jx, jy);
	    }
	    attempt = 1;
	    prout("  ");
	    r = (Rand()+Rand())*0.5 -0.5;
	    r += 0.002*game.kpower[l]*r;
	    torpedo(course, r, jx, jy, &hit, 1, 1);
	    if (KLINGREM==0) 
		finish(FWON); /* Klingons did themselves in! */
	    if (game.state.galaxy[game.quadx][game.quady].supernova || game.alldone) 
		return; /* Supernova or finished */
	    if (hit == 0) continue;
	}
	if (game.shldup != 0 || game.shldchg != 0 || game.condit==IHDOCKED) {
	    /* shields will take hits */
	    double absorb, hitsh, propor = pfac*game.shield*(game.condit==IHDOCKED ? 2.1 : 1.0);
	    if(propor < 0.1) propor = 0.1;
	    hitsh = propor*chgfac*hit+1.0;
	    atackd=1;
	    absorb = 0.8*hitsh;
	    if (absorb > game.shield) absorb = game.shield;
	    game.shield -= absorb;
	    hit -= hitsh;
	    if (game.condit==IHDOCKED) dock(0);
	    if (propor > 0.1 && hit < 0.005*game.energy) continue;
	}
	/* It's a hit -- print out hit size */
	atackd = 1; /* We weren't going to check casualties, etc. if
		       shields were down for some strange reason. This
		       doesn't make any sense, so I've fixed it */
	ihurt = 1;
	proutn(_("%d unit hit"), (int)hit);
	if ((game.damage[DSRSENS] > 0 && itflag) || game.skill<=SKILL_FAIR) {
	    proutn(_(" on the "));
	    crmshp();
	}
	if (game.damage[DSRSENS] <= 0.0 && itflag) {
	    proutn(_(" from "));
	    crmena(0, iquad, i, jx, jy);
	}
	skip(1);
	/* Decide if hit is critical */
	if (hit > hitmax) hitmax = hit;
	hittot += hit;
	fry(hit);
	game.energy -= hit;
	if (game.condit==IHDOCKED) 
	    dock(0);
    }
    if (game.energy <= 0) {
	/* Returning home upon your shield, not with it... */
	finish(FBATTLE);
	return;
    }
    if (attempt == 0 && game.condit == IHDOCKED)
	prout(_("***Enemies decide against attacking your ship."));
    if (atackd == 0) return;
    percent = 100.0*pfac*game.shield+0.5;
    if (ihurt==0) {
	/* Shields fully protect ship */
	proutn(_("Enemy attack reduces shield strength to "));
    }
    else {
	/* Print message if starship suffered hit(s) */
	skip(1);
	proutn(_("Energy left %2d    shields "), (int)game.energy);
	if (game.shldup) proutn(_("up "));
	else if (game.damage[DSHIELD] == 0) proutn(_("down "));
	else proutn(_("damaged, "));
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
	}
    }
    /* After attack, reset average distance to enemies */
    for_local_enemies(l)
	game.kavgd[l] = game.kdist[l];
    sortkl();
    return;
}
		
void deadkl(int ix, int iy, int type, int ixx, int iyy) 
{
    /* Added ixx and iyy allow enemy to "move" before dying */

    int i,j;

    skip(1);
    crmena(1, type, 2, ixx, iyy);
    /* Decide what kind of enemy it is and update approriately */
    if (type == IHR) {
	/* chalk up a Romulan */
	game.state.galaxy[game.quadx][game.quady].romulans--;
	game.irhere--;
	game.state.nromrem--;
    }
    else if (type == IHT) {
	/* Killed a Tholian */
	game.ithere = 0;
    }
    else if (type == IHQUEST) {
	/* Killed a Thingy */
	iqhere=iqengry=thingx=thingy=0;
    }
    else {
	/* Some type of a Klingon */
	game.state.galaxy[game.quadx][game.quady].klingons--;
	game.klhere--;
	switch (type) {
	case IHC:
	    game.comhere = 0;
	    for_commanders (i)
		if (game.state.cx[i]==game.quadx && game.state.cy[i]==game.quady) break;
	    game.state.cx[i] = game.state.cx[game.state.remcom];
	    game.state.cy[i] = game.state.cy[game.state.remcom];
	    game.state.cx[game.state.remcom] = 0;
	    game.state.cy[game.state.remcom] = 0;
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
	    game.ishere = game.state.isx = game.state.isy = game.isatb = game.iscate = 0;
	    unschedule(FSCMOVE);
	    unschedule(FSCDBAS);
	    break;
	}
    }

    /* For each kind of enemy, finish message to player */
    prout(_(" destroyed."));
    game.quad[ix][iy] = IHDOT;
    if (KLINGREM==0) return;

    game.state.remtime = game.state.remres/(game.state.remkl + 4*game.state.remcom);

    /* Remove enemy ship from arrays describing local game.conditions */
    if (is_scheduled(FCDBAS) && game.batx==game.quadx && game.baty==game.quady && type==IHC)
	unschedule(FCDBAS);
    for_local_enemies(i)
	if (game.kx[i]==ix && game.ky[i]==iy) break;
    game.nenhere--;
    if (i <= game.nenhere)  {
	for (j=i; j<=game.nenhere; j++) {
	    game.kx[j] = game.kx[j+1];
	    game.ky[j] = game.ky[j+1];
	    game.kpower[j] = game.kpower[j+1];
	    game.kavgd[j] = game.kdist[j] = game.kdist[j+1];
	}
    }
    game.kx[game.nenhere+1] = 0;
    game.ky[game.nenhere+1] = 0;
    game.kdist[game.nenhere+1] = 0;
    game.kavgd[game.nenhere+1] = 0;
    game.kpower[game.nenhere+1] = 0;
    return;
}

static int targetcheck(double x, double y, double *course) 
{
    double deltx, delty;
    /* Return TRUE if target is invalid */
    if (!VALID_SECTOR(x, y)) {
	huh();
	return 1;
    }
    deltx = 0.1*(y - game.secty);
    delty = 0.1*(game.sectx - x);
    if (deltx==0 && delty== 0) {
	skip(1);
	prout(_("Spock-  \"Bridge to sickbay.  Dr. McCoy,"));
	prout(_("  I recommend an immediate review of"));
	prout(_("  the Captain's psychological profile.\""));
	chew();
	return 1;
    }
    *course = 1.90985932*atan2(deltx, delty);
    return 0;
}

void photon(void) 
{
    double targ[4][3], course[4];
    double r, dummy;
    int key, n, i, osuabor;

    game.ididit = 0;

    if (game.damage[DPHOTON]) {
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
	    if (n <= game.torps) break;
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
	if (targetcheck(targ[i][1], targ[i][2], &course[i])) return;
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
	    if (targetcheck(targ[i][1], targ[i][2], &course[i])) return;
	}
    }
    game.ididit = 1;
    /* Loop for moving <n> torpedoes */
    osuabor = 0;
    for (i = 1; i <= n && !osuabor; i++) {
	if (game.condit != IHDOCKED) game.torps--;
	r = (Rand()+Rand())*0.5 -0.5;
	if (fabs(r) >= 0.47) {
	    /* misfire! */
	    r = (Rand()+1.2) * r;
	    if (n>1) {
		prouts(_("***TORPEDO NUMBER %d MISFIRES"), i);
	    }
	    else prouts(_("***TORPEDO MISFIRES."));
	    skip(1);
	    if (i < n)
		prout(_("  Remainder of burst aborted."));
	    osuabor=1;
	    if (Rand() <= 0.2) {
		prout(_("***Photon tubes damaged by misfire."));
		game.damage[DPHOTON] = game.damfac*(1.0+2.0*Rand());
		break;
	    }
	}
	if (game.shldup || game.condit == IHDOCKED) 
	    r *= 1.0 + 0.0001*game.shield;
	torpedo(course[i], r, game.sectx, game.secty, &dummy, i, n);
	if (game.alldone || game.state.galaxy[game.quadx][game.quady].supernova)
	    return;
    }
    if (KLINGREM==0) finish(FWON);
}

	

static void overheat(double rpow) 
{
    if (rpow > 1500) {
	double chekbrn = (rpow-1500.)*0.00038;
	if (Rand() <= chekbrn) {
	    prout(_("Weapons officer Sulu-  \"Phasers overheated, sir.\""));
	    game.damage[DPHASER] = game.damfac*(1.0 + Rand()) * (1.0+chekbrn);
	}
    }
}

static int checkshctrl(double rpow) 
{
    double hit;
    int icas;
	
    skip(1);
    if (Rand() < .998) {
	prout(_("Shields lowered."));
	return 0;
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
	return 1;
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
	game.casual -= icas;
    }
    skip(1);
    prout(_("Phaser energy dispersed by shields."));
    prout(_("Enemy unaffected."));
    overheat(rpow);
    return 1;
}
	

void phasers(void) 
{
    double hits[21], rpow=0, extra, powrem, over, temp;
    int kz = 0, k=1, i, irec=0; /* Cheating inhibitor */
    int ifast=0, no=0, ipoop=1, msgflag = 1;
    enum {NOTSET, MANUAL, FORCEMAN, AUTOMATIC} automode = NOTSET;
    int key=0;

    skip(1);
    /* SR sensors and Computer */
    if (game.damage[DSRSENS]+game.damage[DCOMPTR] > 0) ipoop = 0;
    if (game.condit == IHDOCKED) {
	prout(_("Phasers can't be fired through base shields."));
	chew();
	return;
    }
    if (game.damage[DPHASER] != 0) {
	prout(_("Phaser control damaged."));
	chew();
	return;
    }
    if (game.shldup) {
	if (game.damage[DSHCTRL]) {
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
	ifast = 1;
		
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
		if ((!ipoop) && game.nenhere != 0) {
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
		no = 1;
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
	    else if (!ipoop)
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
	    else if (!ipoop)
		automode = FORCEMAN;
	    else 
		proutn(_("Manual or automatic? "));
	}
    }
				
    switch (automode) {
    case AUTOMATIC:
	if (key == IHALPHA && isit("no")) {
	    no = 1;
	    key = scan();
	}
	if (key != IHREAL && game.nenhere != 0) {
	    prout(_("Phasers locked on target. Energy available: %.2f"),
		  ifast?game.energy-200.0:game.energy,1,2);
	}
	irec=0;
	do {
	    chew();
	    if (!kz) for_local_enemies(i)
		irec+=fabs(game.kpower[i])/(PHASEFAC*pow(0.90,game.kdist[i]))*
		    (1.01+0.05*Rand()) + 1.0;
	    kz=1;
	    proutn(_("(%d) units required. "), irec);
	    chew();
	    proutn(_("Units to fire= "));
	    key = scan();
	    if (key!=IHREAL) return;
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
	    no = 1;
	}
	if (ifast) {
	    game.energy -= 200; /* Go and do it! */
	    if (checkshctrl(rpow)) return;
	}
	chew();
	game.energy -= rpow;
	extra = rpow;
	if (game.nenhere) {
	    extra = 0.0;
	    powrem = rpow;
	    for_local_enemies(i) {
		hits[i] = 0.0;
		if (powrem <= 0) continue;
		hits[i] = fabs(game.kpower[i])/(PHASEFAC*pow(0.90,game.kdist[i]));
		over = (0.01 + 0.05*Rand())*hits[i];
		temp = powrem;
		powrem -= hits[i] + over;
		if (powrem <= 0 && temp < hits[i]) hits[i] = temp;
		if (powrem <= 0) over = 0.0;
		extra += over;
	    }
	    if (powrem > 0.0) extra += powrem;
	    hittem(hits);
	    game.ididit=1;
	}
	if (extra > 0 && game.alldone == 0) {
	    if (game.ithere) {
		proutn(_("*** Tholian web absorbs "));
		if (game.nenhere>0) proutn(_("excess "));
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
	if (game.damage[DCOMPTR]!=0)
	    prout(_("Battle comuter damaged, manual file only."));
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
	    int ii = game.kx[k], jj = game.ky[k];
	    int ienm = game.quad[ii][jj];
	    if (msgflag) {
		proutn(_("Energy available= %.2f"),
		       game.energy-.006-(ifast?200:0));
		skip(1);
		msgflag = 0;
		rpow = 0.0;
	    }
	    if (game.damage[DSRSENS] && !(abs(game.sectx-ii) < 2 && abs(game.secty-jj) < 2) &&
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
		if (ipoop && k > kz)
		    irec=(fabs(game.kpower[k])/(PHASEFAC*pow(0.9,game.kdist[k])))*
			(1.01+0.05*Rand()) + 1.0;
		kz = k;
		proutn("(");
		if (game.damage[DCOMPTR]==0) proutn("%d", irec);
		else proutn("??");
		proutn(")  ");
		proutn(_("units to fire at "));
		crmena(0, ienm, 2, ii, jj);
		proutn("-  ");
		key = scan();
	    }
	    if (key == IHALPHA && isit("no")) {
		no = 1;
		key = scan();
		continue;
	    }
	    if (key == IHALPHA) {
		huh();
		return;
	    }
	    if (key == IHEOL) {
		if (k==1) { /* Let me say I'm baffled by this */
		    msgflag = 1;
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
	    no = 1;
	}
	game.energy -= rpow;
	chew();
	if (ifast) {
	    game.energy -= 200.0;
	    if (checkshctrl(rpow)) return;
	}
	hittem(hits);
	game.ididit=1;
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
		game.shldup = 0;
	    }
	    else
		prout(_("Shields raised."));
	}
	else
	    game.shldup = 0;
    }
    overheat(rpow);
}

void hittem(double *hits) 
{
    double kp, kpow, wham, hit, dustfac, kpini;
    int nenhr2=game.nenhere, k=1, kk=1, ii, jj, ienm;

    skip(1);

    for (; k <= nenhr2; k++, kk++) {
	if ((wham = hits[k])==0) continue;
	dustfac = 0.9 + 0.01*Rand();
	hit = wham*pow(dustfac,game.kdist[kk]);
	kpini = game.kpower[kk];
	kp = fabs(kpini);
	if (PHASEFAC*hit < kp) kp = PHASEFAC*hit;
	game.kpower[kk] -= (game.kpower[kk] < 0 ? -kp: kp);
	kpow = game.kpower[kk];
	ii = game.kx[kk];
	jj = game.ky[kk];
	if (hit > 0.005) {
	    if (game.damage[DSRSENS]==0)
		boom(ii, jj);
	    proutn(_("%d unit hit on "), (int)hit);
	}
	else
	    proutn(_("Very small hit on "));
	ienm = game.quad[ii][jj];
	if (ienm==IHQUEST) iqengry=1;
	crmena(0,ienm,2,ii,jj);
	skip(1);
	if (kpow == 0) {
	    deadkl(ii, jj, ienm, ii, jj);
	    if (KLINGREM==0) finish(FWON);
	    if (game.alldone) return;
	    kk--; /* don't do the increment */
	}
	else /* decide whether or not to emasculate klingon */
	    if (kpow > 0 && Rand() >= 0.9 &&
		kpow <= ((0.4 + 0.4*Rand())*kpini)) {
		prout(_("***Mr. Spock-  \"Captain, the vessel at "),
		      cramlc(sector,ii,jj));
		prout(_("   has just lost its firepower.\""));
		game.kpower[kk] = -kpow;
	    }
    }
    return;
}

