#include <unistd.h>
#include "sst.h"

void doshield(int i) 
{
    int key;
    enum {NONE, SHUP, SHDN, NRG} action = NONE;

    ididit = 0;

    if (i == 2) action = SHUP;
    else {
	key = scan();
	if (key == IHALPHA) {
	    if (isit("transfer"))
		action = NRG;
	    else {
		chew();
		if (game.damage[DSHIELD]) {
		    prout("Shields damaged and down.");
		    return;
		}
		if (isit("up"))
		    action = SHUP;
		else if (isit("down"))
		    action = SHDN;
	    }
	}
	if (action==NONE) {
	    proutn("Do you wish to change shield energy? ");
	    if (ja()) {
		proutn("Energy to transfer to shields- ");
		action = NRG;
	    }
	    else if (game.damage[DSHIELD]) {
		prout("Shields damaged and down.");
		return;
	    }
	    else if (shldup) {
		proutn("Shields are up. Do you want them down? ");
		if (ja()) action = SHDN;
		else {
		    chew();
		    return;
		}
	    }
	    else {
		proutn("Shields are down. Do you want them up? ");
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
	if (shldup) {
	    prout("Shields already up.");
	    return;
	}
	shldup = 1;
	shldchg = 1;
	if (condit != IHDOCKED) energy -= 50.0;
	prout("Shields raised.");
	if (energy <= 0) {
	    skip(1);
	    prout("Shields raising uses up last of energy.");
	    finish(FNRG);
	    return;
	}
	ididit=1;
	return;
    case SHDN:
	if (shldup==0) {
	    prout("Shields already down.");
	    return;
	}
	shldup=0;
	shldchg=1;
	prout("Shields lowered.");
	ididit=1;
	return;
    case NRG:
	while (scan() != IHREAL) {
	    chew();
	    proutn("Energy to transfer to shields- ");
	}
	chew();
	if (aaitem==0) return;
	if (aaitem > energy) {
	    prout("Insufficient ship energy.");
	    return;
	}
	ididit = 1;
	if (shield+aaitem >= inshld) {
	    prout("Shield energy maximized.");
	    if (shield+aaitem > inshld) {
		prout("Excess energy requested returned to ship energy");
	    }
	    energy -= inshld-shield;
	    shield = inshld;
	    return;
	}
	if (aaitem < 0.0 && energy-aaitem > inenrg) {
	    /* Prevent shield drain loophole */
	    skip(1);
	    prout("Engineering to bridge--");
	    prout("  Scott here. Power circuit problem, Captain.");
	    prout("  I can't drain the shields.");
	    ididit = 0;
	    return;
	}
	if (shield+aaitem < 0) {
	    prout("All shield energy transferred to ship.");
	    energy += shield;
	    shield = 0.0;
	    return;
	}
	proutn("Scotty- \"");
	if (aaitem > 0)
	    prout("Transferring energy to shields.\"");
	else
	    prout("Draining energy from shields.\"");
	shield += aaitem;
	energy -= aaitem;
	return;
    case NONE:;	/* avoid gcc warning */
    }
}

void ram(int ibumpd, int ienm, int ix, int iy)
{
    double type = 1.0, extradm;
    int icas, l;
	
    prouts("***RED ALERT!  RED ALERT!");
    skip(1);
    prout("***COLLISION IMMINENT.");
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
    proutn(ibumpd ? " rammed by " : " rams ");
    crmena(0, ienm, 2, ix, iy);
    if (ibumpd) proutn(" (original position)");
    skip(1);
    deadkl(ix, iy, ienm, sectx, secty);
    proutn("***");
    crmshp();
    prout(" heavily damaged.");
    icas = 10.0+20.0*Rand();
    prout("***Sickbay reports %d casualties", icas);
    casual += icas;
    for (l=1; l <= NDEVICES; l++) {
	if (l == DDRAY) continue; // Don't damage deathray 
	if (game.damage[l] < 0) continue;
	extradm = (10.0*type*Rand()+1.0)*damfac;
	game.damage[l] += Time + extradm; /* Damage for at least time of travel! */
    }
    shldup = 0;
    if (game.state.remkl) {
	pause_game(2);
	dreprt();
    }
    else finish(FWON);
    return;
}

void torpedo(double course, double r, int inx, int iny, double *hit, int wait, int i, int n) 
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
    if (game.damage[DSRSENS]==0 || condit==IHDOCKED) 
	setwnd(srscan_window);
    else 
	setwnd(message_window);
    /* Loop to move a single torpedo */
    for (l=1; l <= 15; l++) {
	x += deltax;
	ix = x + 0.5;
	if (ix < 1 || ix > QUADSIZE) break;
	y += deltay;
	iy = y + 0.5;
	if (iy < 1 || iy > QUADSIZE) break;
	iquad=game.quad[ix][iy];
	tracktorpedo(x, y, ix, iy, wait, l, i, n, iquad);
	wait = 1;
	if (iquad==IHDOT) continue;
	/* hit something */
	setwnd(message_window);
	skip(1);	/* start new line after text track */
	switch(iquad) {
	case IHE: /* Hit our ship */
	case IHF:
	    skip(1);
	    proutn("Torpedo hits ");
	    crmshp();
	    prout(".");
	    *hit = 700.0 + 100.0*Rand() -
		1000.0*sqrt(square(ix-inx)+square(iy-iny))*
		fabs(sin(bullseye-angle));
	    *hit = fabs(*hit);
	    newcnd(); /* we're blown out of dock */
	    /* We may be displaced. */
	    if (landed==1 || condit==IHDOCKED) return; /* Cheat if on a planet */
	    ang = angle + 2.5*(Rand()-0.5);
	    temp = fabs(sin(ang));
	    if (fabs(cos(ang)) > temp) temp = fabs(cos(ang));
	    xx = -sin(ang)/temp;
	    yy = cos(ang)/temp;
	    jx=ix+xx+0.5;
	    jy=iy+yy+0.5;
	    if (jx<1 || jx>QUADSIZE || jy<1 ||jy > QUADSIZE) return;
	    if (game.quad[jx][jy]==IHBLANK) {
		finish(FHOLE);
		return;
	    }
	    if (game.quad[jx][jy]!=IHDOT) {
		/* can't move into object */
		return;
	    }
	    sectx = jx;
	    secty = jy;
	    crmshp();
	    shoved = 1;
	    break;
					  
	case IHC: /* Hit a commander */
	case IHS:
	    if (Rand() <= 0.05) {
		crmena(1, iquad, 2, ix, iy);
		prout(" uses anti-photon device;");
		prout("   torpedo neutralized.");
		return;
	    }
	case IHR: /* Hit a regular enemy */
	case IHK:
	    /* find the enemy */
	    for (ll=1; ll <= nenhere; ll++)
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
	    if (jx<1 || jx>QUADSIZE || jy<1 ||jy > QUADSIZE) {
		prout(" damaged but not destroyed.");
		return;
	    }
	    if (game.quad[jx][jy]==IHBLANK) {
		prout(" buffeted into black hole.");
		deadkl(ix, iy, iquad, jx, jy);
		return;
	    }
	    if (game.quad[jx][jy]!=IHDOT) {
		/* can't move into object */
		prout(" damaged but not destroyed.");
		return;
	    }
	    proutn(" damaged--");
	    game.kx[ll] = jx;
	    game.ky[ll] = jy;
	    shoved = 1;
	    break;
	case IHB: /* Hit a base */
	    skip(1);
	    prout("***STARBASE DESTROYED..");
	    if (game.starch[quadx][quady] < 0) game.starch[quadx][quady] = 0;
	    for (ll=1; ll<=game.state.rembase; ll++) {
		if (game.state.baseqx[ll]==quadx && game.state.baseqy[ll]==quady) {
		    game.state.baseqx[ll]=game.state.baseqx[game.state.rembase];
		    game.state.baseqy[ll]=game.state.baseqy[game.state.rembase];
		    break;
		}
	    }
	    game.quad[ix][iy]=IHDOT;
	    game.state.rembase--;
	    basex=basey=0;
	    game.state.galaxy[quadx][quady] -= BASE_PLACE;
	    game.state.basekl++;
	    newcnd();
	    return;
	case IHP: /* Hit a planet */
	    crmena(1, iquad, 2, ix, iy);
	    prout(" destroyed.");
	    game.state.nplankl++;
	    game.state.newstuf[quadx][quady] -= 1;
	    DESTROY(&game.state.plnets[iplnet]);
	    iplnet = 0;
	    plnetx = plnety = 0;
	    game.quad[ix][iy] = IHDOT;
	    if (landed==1) {
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
	    prout(" unaffected by photon blast.");
	    return;
	case IHQUEST: /* Hit a thingy */
	    if (Rand()>0.7) {	// Used to be certain death 
		skip(1);
		prouts("AAAAIIIIEEEEEEEEAAAAAAAAUUUUUGGGGGHHHHHHHHHHHH!!!");
		skip(1);
		prouts("    HACK!     HACK!    HACK!        *CHOKE!*  ");
		skip(1);
		proutn("Mr. Spock-");
		prouts("  \"Fascinating!\"");
		skip(1);
		deadkl(ix, iy, iquad, ix, iy);
	    } else {
		/*
		 * Stas Sergeev added the possibility that
		 * you can shove the Thingy.
		 */
		iqengry=1;
		shoved=1;
	    }
	    return;
	case IHBLANK: /* Black hole */
	    skip(1);
	    crmena(1, IHBLANK, 2, ix, iy);
	    prout(" swallows torpedo.");
	    return;
	case IHWEB: /* hit the web */
	    skip(1);
	    prout("***Torpedo absorbed by Tholian web.");
	    return;
	case IHT:  /* Hit a Tholian */
	    h1 = 700.0 + 100.0*Rand() -
		1000.0*sqrt(square(ix-inx)+square(iy-iny))*
		fabs(sin(bullseye-angle));
	    h1 = fabs(h1);
	    if (h1 >= 600) {
		game.quad[ix][iy] = IHDOT;
		ithere = 0;
		ithx = ithy = 0;
		deadkl(ix, iy, iquad, ix, iy);
		return;
	    }
	    skip(1);
	    crmena(1, IHT, 2, ix, iy);
	    if (Rand() > 0.05) {
		prout(" survives photon blast.");
		return;
	    }
	    prout(" disappears.");
	    game.quad[ix][iy] = IHWEB;
	    ithere = ithx = ithy = 0;
	    nenhere--;
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
	prout(" displaced by blast to %s ", cramlc(sector, jx, jy));
	for (ll=1; ll<=nenhere; ll++)
	    game.kdist[ll] = game.kavgd[ll] = sqrt(square(sectx-game.kx[ll])+square(secty-game.ky[ll]));
	sortkl();
	return;
    }
    skip(1);
    prout("Torpedo missed.");
    return;
}

static void fry(double hit) 
{
    double ncrit, extradm;
    int ktr=1, l, ll, j, cdam[NDEVICES+1];

    /* a critical hit occured */
    if (hit < (275.0-25.0*skill)*(1.0+0.5*Rand())) return;

    ncrit = 1.0 + hit/(500.0+100.0*Rand());
    proutn("***CRITICAL HIT--");
    /* Select devices and cause damage */
    for (l = 1; l <= ncrit && l <= NDEVICES; l++) {
	do {
	    j = NDEVICES*Rand()+1.0;
	    /* Cheat to prevent shuttle damage unless on ship */
	} while (game.damage[j] < 0.0 || (j == DSHUTTL && iscraft != 1) ||
		 j == DDRAY);
	cdam[l] = j;
	extradm = (hit*damfac)/(ncrit*(75.0+25.0*Rand()));
	game.damage[j] += extradm;
	if (l > 1) {
	    for (ll=2; ll<=l && j != cdam[ll-1]; ll++) ;
	    if (ll<=l) continue;
	    ktr += 1;
	    if (ktr==3) skip(1);
	    proutn(" and ");
	}
	proutn(device[j]);
    }
    prout(" damaged.");
    if (game.damage[DSHIELD] && shldup) {
	prout("***Shields knocked down.");
	shldup=0;
    }
}

void attack(int torps_ok) 
{
    /* torps_ok == 0 forces use of phasers in an attack */
    int percent, ihurt=0, l, i=0, jx, jy, iquad, itflag;
    int atackd = 0, attempt = 0;
    double hit;
    double pfac, dustfac, hitmax=0.0, hittot=0.0, chgfac=1.0, r;

    iattak = 1;
    if (alldone) return;
#ifdef DEBUG
    if (idebug) prout("ATTACK!");
#endif

    if (ithere) movetho();

    if (neutz) { /* The one chance not to be attacked */
	neutz = 0;
	return;
    }
    if ((((comhere || ishere) && (justin == 0)) || skill == SKILL_EMERITUS)&&(torps_ok!=0)) movcom();
    if (nenhere==0 || (nenhere==1 && iqhere && iqengry==0)) return;
    pfac = 1.0/inshld;
    if (shldchg == 1) chgfac = 0.25+0.5*Rand();
    skip(1);
    if (skill <= SKILL_FAIR) i = 2;
    for (l=1; l <= nenhere; l++) {
	if (game.kpower[l] < 0) continue;	/* too weak to attack */
	/* compute hit strength and diminsh shield power */
	r = Rand();
	/* Increase chance of photon torpedos if docked or enemy energy low */
	if (condit == IHDOCKED) r *= 0.25;
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
	    if (condit == IHDOCKED) continue; /* Don't waste the effort! */
	    attempt = 1; /* Attempt to attack */
	    dustfac = 0.8+0.05*Rand();
	    hit = game.kpower[l]*pow(dustfac,game.kavgd[l]);
	    game.kpower[l] *= 0.75;
	}
	else { /* Enemy used photon torpedo */
	    double course = 1.90985*atan2((double)secty-jy, (double)jx-sectx);
	    hit = 0;
	    proutn("***TORPEDO INCOMING");
	    if (game.damage[DSRSENS] <= 0.0) {
		proutn(" From ");
		crmena(0, iquad, i, jx, jy);
	    }
	    attempt = 1;
	    prout("  ");
	    r = (Rand()+Rand())*0.5 -0.5;
	    r += 0.002*game.kpower[l]*r;
	    torpedo(course, r, jx, jy, &hit, 0, 1, 1);
	    if (game.state.remkl==0) finish(FWON); /* Klingons did themselves in! */
	    if (game.state.galaxy[quadx][quady] == SUPERNOVA_PLACE ||
		alldone) return; /* Supernova or finished */
	    if (hit == 0) continue;
	}
	if (shldup != 0 || shldchg != 0 || condit==IHDOCKED) {
	    /* shields will take hits */
	    double absorb, hitsh, propor = pfac*shield*(condit==IHDOCKED ? 2.1 : 1.0);
	    if(propor < 0.1) propor = 0.1;
	    hitsh = propor*chgfac*hit+1.0;
	    atackd=1;
	    absorb = 0.8*hitsh;
	    if (absorb > shield) absorb = shield;
	    shield -= absorb;
	    hit -= hitsh;
	    if (condit==IHDOCKED) dock(0);
	    if (propor > 0.1 && hit < 0.005*energy) continue;
	}
	/* It's a hit -- print out hit size */
	atackd = 1; /* We weren't going to check casualties, etc. if
		       shields were down for some strange reason. This
		       doesn't make any sense, so I've fixed it */
	ihurt = 1;
	proutn("%d unit hit", (int)hit);
	if ((game.damage[DSRSENS] > 0 && itflag) || skill<=SKILL_FAIR) {
	    proutn(" on the ");
	    crmshp();
	}
	if (game.damage[DSRSENS] <= 0.0 && itflag) {
	    proutn(" from ");
	    crmena(0, iquad, i, jx, jy);
	}
	skip(1);
	/* Decide if hit is critical */
	if (hit > hitmax) hitmax = hit;
	hittot += hit;
	fry(hit);
	prout("Hit %g energy %g", hit, energy);
	energy -= hit;
	if (condit==IHDOCKED) 
	    dock(0);
    }
    if (energy <= 0) {
	/* Returning home upon your shield, not with it... */
	finish(FBATTLE);
	return;
    }
    if (attempt == 0 && condit == IHDOCKED)
	prout("***Enemies decide against attacking your ship.");
    if (atackd == 0) return;
    percent = 100.0*pfac*shield+0.5;
    if (ihurt==0) {
	/* Shields fully protect ship */
	proutn("Enemy attack reduces shield strength to ");
    }
    else {
	/* Print message if starship suffered hit(s) */
	skip(1);
	proutn("Energy left %2d    shields ", (int)energy);
	if (shldup) proutn("up ");
	else if (game.damage[DSHIELD] == 0) proutn("down ");
	else proutn("damaged, ");
    }
    prout("%d%%,   torpedoes left %d", percent, torps);
    /* Check if anyone was hurt */
    if (hitmax >= 200 || hittot >= 500) {
	int icas= hittot*Rand()*0.015;
	if (icas >= 2) {
	    skip(1);
	    prout("Mc Coy-  \"Sickbay to bridge.  We suffered %d casualties", icas);
	    prout("   in that last attack.\"");
	    casual += icas;
	}
    }
    /* After attack, reset average distance to enemies */
    for (l = 1; l <= nenhere; l++)
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
	game.state.newstuf[quadx][quady] -= ROMULAN_PLACE;
	irhere--;
	game.state.nromkl++;
	game.state.nromrem--;
    }
    else if (type == IHT) {
	/* Killed a Tholian */
	ithere = 0;
    }
    else if (type == IHQUEST) {
	/* Killed a Thingy */
	iqhere=iqengry=thingx=thingy=0;
    }
    else {
	/* Some type of a Klingon */
	game.state.galaxy[quadx][quady] -= KLINGON_PLACE;
	klhere--;
	game.state.remkl--;
	switch (type) {
	case IHC:
	    comhere = 0;
	    for (i=1; i<=game.state.remcom; i++)
		if (game.state.cx[i]==quadx && game.state.cy[i]==quady) break;
	    game.state.cx[i] = game.state.cx[game.state.remcom];
	    game.state.cy[i] = game.state.cy[game.state.remcom];
	    game.state.cx[game.state.remcom] = 0;
	    game.state.cy[game.state.remcom] = 0;
	    game.state.remcom--;
	    game.future[FTBEAM] = 1e30;
	    if (game.state.remcom != 0)
		game.future[FTBEAM] = game.state.date + expran(1.0*incom/game.state.remcom);
	    game.state.killc++;
	    break;
	case IHK:
	    game.state.killk++;
	    break;
	case IHS:
	    game.state.nscrem = ishere = game.state.isx = game.state.isy = isatb = iscate = 0;
	    game.state.nsckill = 1;
	    game.future[FSCMOVE] = game.future[FSCDBAS] = 1e30;
	    break;
	}
    }

    /* For each kind of enemy, finish message to player */
    prout(" destroyed.");
    game.quad[ix][iy] = IHDOT;
    if (game.state.remkl==0) return;

    game.state.remtime = game.state.remres/(game.state.remkl + 4*game.state.remcom);

    /* Remove enemy ship from arrays describing local conditions */
    if (game.future[FCDBAS] < 1e30 && batx==quadx && baty==quady && type==IHC)
	game.future[FCDBAS] = 1e30;
    for (i=1; i<=nenhere; i++)
	if (game.kx[i]==ix && game.ky[i]==iy) break;
    nenhere--;
    if (i <= nenhere)  {
	for (j=i; j<=nenhere; j++) {
	    game.kx[j] = game.kx[j+1];
	    game.ky[j] = game.ky[j+1];
	    game.kpower[j] = game.kpower[j+1];
	    game.kavgd[j] = game.kdist[j] = game.kdist[j+1];
	}
    }
    game.kx[nenhere+1] = 0;
    game.ky[nenhere+1] = 0;
    game.kdist[nenhere+1] = 0;
    game.kavgd[nenhere+1] = 0;
    game.kpower[nenhere+1] = 0;
    return;
}

static int targetcheck(double x, double y, double *course) 
{
    double deltx, delty;
    /* Return TRUE if target is invalid */
    if (x < 1.0 || x > QUADSIZE || y < 1.0 || y > QUADSIZE) {
	huh();
	return 1;
    }
    deltx = 0.1*(y - secty);
    delty = 0.1*(sectx - x);
    if (deltx==0 && delty== 0) {
	skip(1);
	prout("Spock-  \"Bridge to sickbay.  Dr. McCoy,");
	prout("  I recommend an immediate review of");
	prout("  the Captain's psychological profile.\"");
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

    ididit = 0;

    if (game.damage[DPHOTON]) {
	prout("Photon tubes damaged.");
	chew();
	return;
    }
    if (torps == 0) {
	prout("No torpedoes left.");
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
	    prout("%d torpedoes left.", torps);
	    proutn("Number of torpedoes to fire- ");
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
		prout("Maximum of 3 torpedoes per burst.");
		key = IHEOL;
		return;
	    }
	    if (n <= torps) break;
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
	    proutn("Target sector for torpedo number %d- ", i);
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
    ididit = 1;
    /* Loop for moving <n> torpedoes */
    osuabor = 0;
    for (i = 1; i <= n && !osuabor; i++) {
	if (condit != IHDOCKED) torps--;
	r = (Rand()+Rand())*0.5 -0.5;
	if (fabs(r) >= 0.47) {
	    /* misfire! */
	    r = (Rand()+1.2) * r;
	    if (n>1) {
		prouts("***TORPEDO NUMBER %d MISFIRES", i);
	    }
	    else prouts("***TORPEDO MISFIRES.");
	    skip(1);
	    if (i < n)
		prout("  Remainder of burst aborted.");
	    osuabor=1;
	    if (Rand() <= 0.2) {
		prout("***Photon tubes damaged by misfire.");
		game.damage[DPHOTON] = damfac*(1.0+2.0*Rand());
		break;
	    }
	}
	if (shldup || condit == IHDOCKED) 
	    r *= 1.0 + 0.0001*shield;
	torpedo(course[i], r, sectx, secty, &dummy, 0, i, n);
	if (alldone||game.state.galaxy[quadx][quady]==SUPERNOVA_PLACE)
	    return;
    }
    if (game.state.remkl==0) finish(FWON);
}

	

static void overheat(double rpow) 
{
    if (rpow > 1500) {
	double chekbrn = (rpow-1500.)*0.00038;
	if (Rand() <= chekbrn) {
	    prout("Weapons officer Sulu-  \"Phasers overheated, sir.\"");
	    game.damage[DPHASER] = damfac*(1.0 + Rand()) * (1.0+chekbrn);
	}
    }
}

static int checkshctrl(double rpow) 
{
    double hit;
    int icas;
	
    skip(1);
    if (Rand() < .998) {
	prout("Shields lowered.");
	return 0;
    }
    /* Something bad has happened */
    prouts("***RED ALERT!  RED ALERT!");
    skip(2);
    hit = rpow*shield/inshld;
    energy -= rpow+hit*0.8;
    shield -= hit*0.2;
    if (energy <= 0.0) {
	prouts("Sulu-  \"Captain! Shield malf***********************\"");
	skip(1);
	stars();
	finish(FPHASER);
	return 1;
    }
    prouts("Sulu-  \"Captain! Shield malfunction! Phaser fire contained!\"");
    skip(2);
    prout("Lt. Uhura-  \"Sir, all decks reporting damage.\"");
    icas = hit*Rand()*0.012;
    skip(1);
    fry(0.8*hit);
    if (icas) {
	skip(1);
	prout("McCoy to bridge- \"Severe radiation burns, Jim.");
	prout("  %d casualties so far.\"", icas);
	casual -= icas;
    }
    skip(1);
    prout("Phaser energy dispersed by shields.");
    prout("Enemy unaffected.");
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
    if (condit == IHDOCKED) {
	prout("Phasers can't be fired through base shields.");
	chew();
	return;
    }
    if (game.damage[DPHASER] != 0) {
	prout("Phaser control damaged.");
	chew();
	return;
    }
    if (shldup) {
	if (game.damage[DSHCTRL]) {
	    prout("High speed shield control damaged.");
	    chew();
	    return;
	}
	if (energy <= 200.0) {
	    prout("Insufficient energy to activate high-speed shield control.");
	    chew();
	    return;
	}
	prout("Weapons Officer Sulu-  \"High-speed shield control enabled, sir.\"");
	ifast = 1;
		
    }
    /* Original code so convoluted, I re-did it all */
    while (automode==NOTSET) {
	key=scan();
	if (key == IHALPHA) {
	    if (isit("manual")) {
		if (nenhere==0) {
		    prout("There is no enemy present to select.");
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
		if ((!ipoop) && nenhere != 0) {
		    automode = FORCEMAN;
		}
		else {
		    if (nenhere==0)
			prout("Energy will be expended into space.");
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
	    if (nenhere==0) {
		prout("Energy will be expended into space.");
		automode = AUTOMATIC;
	    }
	    else if (!ipoop)
		automode = FORCEMAN;
	    else
		automode = AUTOMATIC;
	}
	else {
	    /* IHEOL */
	    if (nenhere==0) {
		prout("Energy will be expended into space.");
		automode = AUTOMATIC;
	    }
	    else if (!ipoop)
		automode = FORCEMAN;
	    else 
		proutn("Manual or automatic? ");
	}
    }
				
    switch (automode) {
    case AUTOMATIC:
	if (key == IHALPHA && isit("no")) {
	    no = 1;
	    key = scan();
	}
	if (key != IHREAL && nenhere != 0) {
	    prout("Phasers locked on target. Energy available: %.2f",
		  ifast?energy-200.0:energy,1,2);
	}
	irec=0;
	do {
	    chew();
	    if (!kz) for (i = 1; i <= nenhere; i++)
		irec+=fabs(game.kpower[i])/(PHASEFAC*pow(0.90,game.kdist[i]))*
		    (1.01+0.05*Rand()) + 1.0;
	    kz=1;
	    proutn("(%d) units required. ", irec);
	    chew();
	    proutn("Units to fire= ");
	    key = scan();
	    if (key!=IHREAL) return;
	    rpow = aaitem;
	    if (rpow > (ifast?energy-200:energy)) {
		proutn("Energy available= %.2f",
		       ifast?energy-200:energy);
		skip(1);
		key = IHEOL;
	    }
	} while (rpow > (ifast?energy-200:energy));
	if (rpow<=0) {
	    /* chicken out */
	    chew();
	    return;
	}
	if ((key=scan()) == IHALPHA && isit("no")) {
	    no = 1;
	}
	if (ifast) {
	    energy -= 200; /* Go and do it! */
	    if (checkshctrl(rpow)) return;
	}
	chew();
	energy -= rpow;
	extra = rpow;
	if (nenhere) {
	    extra = 0.0;
	    powrem = rpow;
	    for (i = 1; i <= nenhere; i++) {
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
	    ididit=1;
	}
	if (extra > 0 && alldone == 0) {
	    if (ithere) {
		proutn("*** Tholian web absorbs ");
		if (nenhere>0) proutn("excess ");
		prout("phaser energy.");
	    }
	    else {
		prout("%d expended on empty space.", (int)extra);
	    }
	}
	break;

    case FORCEMAN:
	chew();
	key = IHEOL;
	if (game.damage[DCOMPTR]!=0)
	    prout("Battle comuter damaged, manual file only.");
	else {
	    skip(1);
	    prouts("---WORKING---");
	    skip(1);
	    prout("Short-range-sensors-damaged");
	    prout("Insufficient-data-for-automatic-phaser-fire");
	    prout("Manual-fire-must-be-used");
	    skip(1);
	}
    case MANUAL:
	rpow = 0.0;
	for (k = 1; k <= nenhere;) {
	    int ii = game.kx[k], jj = game.ky[k];
	    int ienm = game.quad[ii][jj];
	    if (msgflag) {
		proutn("Energy available= %.2f",
		       energy-.006-(ifast?200:0));
		skip(1);
		msgflag = 0;
		rpow = 0.0;
	    }
	    if (game.damage[DSRSENS] && !(abs(sectx-ii) < 2 && abs(secty-jj) < 2) &&
		(ienm == IHC || ienm == IHS)) {
		cramen(ienm);
		prout(" can't be located without short range scan.");
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
		proutn("units to fire at ");
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
				
	    if (rpow > (ifast?energy-200:energy)) {
		prout("Available energy exceeded -- try again.");
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
	energy -= rpow;
	chew();
	if (ifast) {
	    energy -= 200.0;
	    if (checkshctrl(rpow)) return;
	}
	hittem(hits);
	ididit=1;
    case NOTSET:;	/* avoid gcc warning */
    }
    /* Say shield raised or malfunction, if necessary */
    if (alldone) 
	return;
    if (ifast) {
	skip(1);
	if (no == 0) {
	    if (Rand() >= 0.99) {
		prout("Sulu-  \"Sir, the high-speed shield control has malfunctioned . . .");
		prouts("         CLICK   CLICK   POP  . . .");
		prout(" No  response, sir!");
		shldup = 0;
	    }
	    else
		prout("Shields raised.");
	}
	else
	    shldup = 0;
    }
    overheat(rpow);
}

void hittem(double *hits) 
{
    double kp, kpow, wham, hit, dustfac, kpini;
    int nenhr2=nenhere, k=1, kk=1, ii, jj, ienm;

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
	    proutn("%d unit hit on ", (int)hit);
	}
	else
	    proutn("Very small hit on ");
	ienm = game.quad[ii][jj];
	if (ienm==IHQUEST) iqengry=1;
	crmena(0,ienm,2,ii,jj);
	skip(1);
	if (kpow == 0) {
	    deadkl(ii, jj, ienm, ii, jj);
	    if (game.state.remkl==0) finish(FWON);
	    if (alldone) return;
	    kk--; /* don't do the increment */
	}
	else /* decide whether or not to emasculate klingon */
	    if (kpow > 0 && Rand() >= 0.9 &&
		kpow <= ((0.4 + 0.4*Rand())*kpini)) {
		prout("***Mr. Spock-  \"Captain, the vessel at ",
		      cramlc(sector,ii,jj));
		prout("   has just lost its firepower.\"");
		game.kpower[kk] = -kpow;
	    }
    }
    return;
}

