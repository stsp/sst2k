#include "sst.h"

static int tryexit(int lookx, int looky, int ienm, int loccom, int irun) {
	int iqx, iqy, l;

	iqx = quadx+(lookx+9)/10 - 1;
	iqy = quady+(looky+9)/10 - 1;
	if (iqx < 1 || iqx > 8 || iqy < 1 || iqy > 8 ||
		game.state.galaxy[iqx][iqy] > 899)
		return 0; /* no can do -- neg energy, supernovae, or >8 Klingons */
	if (ienm == IHR) return 0; /* Romulans cannot escape! */
	if (irun == 0) {
		/* avoid intruding on another commander's territory */
		if (ienm == IHC) {
			for (l = 1; l <= game.state.remcom; l++)
				if (game.state.cx[l]==iqx && game.state.cy[l]==iqy) return 0;
			/* refuse to leave if currently attacking starbase */
			if (batx==quadx && baty==quady) return 0;
		}
		/* don't leave if over 1000 units of energy */
		if (game.kpower[loccom] > 1000.) return 0;
	}
	/* print escape message and move out of quadrant.
	   We know this if either short or long range sensors are working */
	if (game.damage[DSRSENS] == 0.0 || game.damage[DLRSENS] == 0.0 ||
		condit == IHDOCKED) {
		proutn("***");
		cramen(ienm);
		proutn(" escapes to %s (and regains strength).",
		       cramlc(quadrant, iqx, iqy));
	}
	/* handle local matters related to escape */
	game.kx[loccom] = game.kx[nenhere];
	game.ky[loccom] = game.ky[nenhere];
	game.kavgd[loccom] = game.kavgd[nenhere];
	game.kpower[loccom] = game.kpower[nenhere];
	game.kdist[loccom] = game.kdist[nenhere];
	klhere--;
	nenhere--;
	if (condit != IHDOCKED) newcnd();
	/* Handle global matters related to escape */
	game.state.galaxy[quadx][quady] -= 100;
	game.state.galaxy[iqx][iqy] += 100;
	if (ienm==IHS) {
		ishere=0;
		iscate=0;
		ientesc=0;
		isatb=0;
		game.future[FSCMOVE]=0.2777+game.state.date;
		game.future[FSCDBAS]=1e30;
		game.state.isx=iqx;
		game.state.isy=iqy;
	}
	else {
		for (l=1; l<=game.state.remcom; l++) {
			if (game.state.cx[l]==quadx && game.state.cy[l]==quady) {
				game.state.cx[l]=iqx;
				game.state.cy[l]=iqy;
				break;
			}
		}
		comhere = 0;
	}
	return 1; /* success */
}


static void movebaddy(int comx, int comy, int loccom, int ienm) {
	int motion, mdist, nsteps, mx, my, nextx, nexty, lookx, looky, ll;
	int irun = 0;
	int krawlx, krawly;
	int success;
	int attempts;
	/* This should probably be just comhere + ishere */
	int nbaddys = skill > 3 ?
				  (int)((comhere*2 + ishere*2+klhere*1.23+irhere*1.5)/2.0):
				  (comhere + ishere);
	double dist1, forces;

	dist1 = game.kdist[loccom];
	mdist = dist1 + 0.5; /* Nearest integer distance */

	/* If SC, check with spy to see if should hi-tail it */
	if (ienm==IHS &&
		(game.kpower[loccom] <= 500.0 || (condit==IHDOCKED && game.damage[DPHOTON]==0))) {
		irun = 1;
		motion = -10;
	}
	else {
		/* decide whether to advance, retreat, or hold position */
/* Algorithm:
   * Enterprise has "force" based on condition of phaser and photon torpedoes.
     If both are operating full strength, force is 1000. If both are damaged,
	 force is -1000. Having shields down subtracts an additional 1000.

   * Enemy has forces equal to the energy of the attacker plus
     100*(K+R) + 500*(C+S) - 400 for novice through good levels OR
	 346*K + 400*R + 500*(C+S) - 400 for expert and emeritus.

	 Attacker Initial energy levels (nominal):
	          Klingon   Romulan   Commander   Super-Commander
	 Novice    400        700        1200        
	 Fair      425        750        1250
	 Good      450        800        1300        1750
	 Expert    475        850        1350        1875
	 Emeritus  500        900        1400        2000
     VARIANCE   75        200         200         200

	 Enemy vessels only move prior to their attack. In Novice - Good games
	 only commanders move. In Expert games, all enemy vessels move if there
	 is a commander present. In Emeritus games all enemy vessels move.

  *  If Enterprise is not docked, an agressive action is taken if enemy
     forces are 1000 greater than Enterprise.

	 Agressive action on average cuts the distance between the ship and
	 the enemy to 1/4 the original.

  *  At lower energy advantage, movement units are proportional to the
     advantage with a 650 advantage being to hold ground, 800 to move forward
	 1, 950 for two, 150 for back 4, etc. Variance of 100.

	 If docked, is reduced by roughly 1.75*skill, generally forcing a
	 retreat, especially at high skill levels.

  *  Motion is limited to skill level, except for SC hi-tailing it out.
  */

		forces = game.kpower[loccom]+100.0*nenhere+400*(nbaddys-1);
		if (shldup==0) forces += 1000; /* Good for enemy if shield is down! */
		if (game.damage[DPHASER] == 0.0 || game.damage[DPHOTON] == 0.0) {
			if (game.damage[DPHASER] != 0) /* phasers damaged */
				forces += 300.0;
			else
				forces -= 0.2*(energy - 2500.0);
			if (game.damage[DPHOTON] != 0) /* photon torpedoes damaged */
				forces += 300.0;
			else
				forces -= 50.0*torps;
		}
		else {
			/* phasers and photon tubes both out! */
			forces += 1000.0;
		}
		motion = 0;
		if (forces <= 1000.0 && condit != IHDOCKED) /* Typical situation */
			motion = ((forces+200.0*Rand())/150.0) - 5.0;
		else {
			if (forces > 1000.0) /* Very strong -- move in for kill */
				motion = (1.0-square(Rand()))*dist1 + 1.0;
			if (condit==IHDOCKED) /* protected by base -- back off ! */
				motion -= skill*(2.0-square(Rand()));
		}
#ifdef DEBUG
		if (idebug) {
			proutn("MOTION = %1.2f", motion);
			proutn("  FORCES = %1,2f", forces);
		}
#endif
		/* don't move if no motion */
		if (motion==0) return;
		/* Limit motion according to skill */
		if (abs(motion) > skill) motion = (motion < 0) ? -skill : skill;
	}
	/* calcuate preferred number of steps */
	nsteps = motion < 0 ? -motion : motion;
	if (motion > 0 && nsteps > mdist) nsteps = mdist; /* don't overshoot */
	if (nsteps > 10) nsteps = 10; /* This shouldn't be necessary */
	if (nsteps < 1) nsteps = 1; /* This shouldn't be necessary */
#ifdef DEBUG
	if (idebug) {
		prout("NSTEPS = %d", nsteps);
	}
#endif
	/* Compute preferred values of delta X and Y */
	mx = sectx - comx;
	my = secty - comy;
	if (2.0 * abs(mx) < abs(my)) mx = 0;
	if (2.0 * abs(my) < abs(sectx-comx)) my = 0;
	if (mx != 0) mx = mx*motion < 0 ? -1 : 1;
	if (my != 0) my = my*motion < 0 ? -1 : 1;
	nextx = comx;
	nexty = comy;
	game.quad[comx][comy] = IHDOT;
	/* main move loop */
	for (ll = 1; ll <= nsteps; ll++) {
#ifdef DEBUG
		if (idebug) {
			prout("%d", ll);
		}
#endif
		/* Check if preferred position available */
		lookx = nextx + mx;
		looky = nexty + my;
		krawlx = mx < 0 ? 1 : -1;
		krawly = my < 0 ? 1 : -1;
		success = 0;
		attempts = 0; /* Settle mysterious hang problem */
		while (attempts++ < 20 && !success) {
			if (lookx < 1 || lookx > 10) {
				if (motion < 0 && tryexit(lookx, looky, ienm, loccom, irun))
					return;
				if (krawlx == mx || my == 0) break;
				lookx = nextx + krawlx;
				krawlx = -krawlx;
			}
			else if (looky < 1 || looky > 10) {
				if (motion < 0 && tryexit(lookx, looky, ienm, loccom, irun))
					return;
				if (krawly == my || mx == 0) break;
				looky = nexty + krawly;
				krawly = -krawly;
			}
			else if (game.quad[lookx][looky] != IHDOT) {
				/* See if we should ram ship */
				if (game.quad[lookx][looky] == ship &&
					(ienm == IHC || ienm == IHS)) {
					ram(1, ienm, comx, comy);
					return;
				}
				if (krawlx != mx && my != 0) {
					lookx = nextx + krawlx;
					krawlx = -krawlx;
				}
				else if (krawly != my && mx != 0) {
					looky = nexty + krawly;
					krawly = -krawly;
				}
				else break; /* we have failed */
			}
			else success = 1;
		}
		if (success) {
			nextx = lookx;
			nexty = looky;
#ifdef DEBUG
			if (idebug) {
				prout(cramlc(neither, nextx, nexty));
			}
#endif
		}
		else break; /* done early */
	}
	/* Put commander in place within same quadrant */
	game.quad[nextx][nexty] = ienm;
	if (nextx != comx || nexty != comy) {
		/* it moved */
		game.kx[loccom] = nextx;
		game.ky[loccom] = nexty;
		game.kdist[loccom] = game.kavgd[loccom] =
					sqrt(square(sectx-nextx)+square(secty-nexty));
		if (game.damage[DSRSENS] == 0 || condit == IHDOCKED) {
			proutn("***");
			cramen(ienm);
			if (game.kdist[loccom] < dist1) proutn(" advances to");
			else proutn(" retreats to ");
			prout(cramlc(sector, nextx, nexty));
		}
	}
}

void movcom(void) {
	int ix, iy, i;

#ifdef DEBUG
	if (idebug) prout("MOVCOM");
#endif

	/* Figure out which Klingon is the commander (or Supercommander)
	   and do move */
	if (comhere) for (i = 1; i <= nenhere; i++) {
		ix = game.kx[i];
		iy = game.ky[i];
		if (game.quad[ix][iy] == IHC) {
			movebaddy(ix, iy, i, IHC);
			break;
		}
	}
	if (ishere) for (i = 1; i <= nenhere; i++) {
		ix = game.kx[i];
		iy = game.ky[i];
		if (game.quad[ix][iy] == IHS) {
			movebaddy(ix, iy, i, IHS);
			break;
		}
	}
	/* if skill level is high, move other Klingons and Romulans too!
	   Move these last so they can base their actions on what the
       commander(s) do. */
	if (skill > 3) for (i = 1; i <= nenhere; i++) {
		ix = game.kx[i];
		iy = game.ky[i];
		if (game.quad[ix][iy] == IHK || game.quad[ix][iy] == IHR)
			movebaddy(ix, iy, i, game.quad[ix][iy]);
	}

	sortkl();
}

static int checkdest(int iqx, int iqy, int flag, int *ipage) {
	int i, j;

	if ((iqx==quadx && iqy==quady) ||
		iqx < 1 || iqx > 8 || iqy < 1 || iqy > 8 ||
		game.state.galaxy[iqx][iqy] > 899) return 1;
	if (flag) {
		/* Avoid quadrants with bases if we want to avoid Enterprise */
		for (i = 1; i <= game.state.rembase; i++)
			if (game.state.baseqx[i]==iqx && game.state.baseqy[i]==iqy) return 1;
	}

	/* do the move */
	game.state.galaxy[game.state.isx][game.state.isy] -= 100;
	game.state.isx = iqx;
	game.state.isy = iqy;
	game.state.galaxy[game.state.isx][game.state.isy] += 100;
	if (iscate) {
		/* SC has scooted, Remove him from current quadrant */
		iscate=0;
		isatb=0;
		ishere=0;
		ientesc=0;
		game.future[FSCDBAS]=1e30;
		for (i = 1; i <= nenhere; i++) 
			if (game.quad[game.kx[i]][game.ky[i]] == IHS) break;
		game.quad[game.kx[i]][game.ky[i]] = IHDOT;
		game.kx[i] = game.kx[nenhere];
		game.ky[i] = game.ky[nenhere];
		game.kdist[i] = game.kdist[nenhere];
		game.kavgd[i] = game.kavgd[nenhere];
		game.kpower[i] = game.kpower[nenhere];
		klhere--;
		nenhere--;
		if (condit!=IHDOCKED) newcnd();
		sortkl();
	}
	/* check for a helpful planet */
	for (i = 0; i < inplan; i++) {
		if (game.state.plnets[i].x==game.state.isx && game.state.plnets[i].y==game.state.isy &&
			game.state.plnets[i].crystals == 1) {
			/* destroy the planet */
		        DESTROY(&game.state.plnets[i]);
			game.state.newstuf[game.state.isx][game.state.isy] -= 1;
			if (game.damage[DRADIO] == 0.0 || condit == IHDOCKED) {
				if (*ipage==0) pause(1);
				*ipage = 1;
				prout("Lt. Uhura-  \"Captain, Starfleet Intelligence reports");
				proutn("   a planet in ");
				proutn(cramlc(quadrant, game.state.isx, game.state.isy));
				prout(" has been destroyed");
				prout("   by the Super-commander.\"");
			}
			break;
		}
	}
	return 0; /* looks good! */
}
			
		
	


void scom(int *ipage) {
	int i, i2, j, ideltax, ideltay, ibqx, ibqy, sx, sy, ifindit, iwhichb;
	int iqx, iqy;
	int basetbl[6];
	double bdist[6];
	int flag;
#ifdef DEBUG
	if (idebug) prout("SCOM");
#endif

	/* Decide on being active or passive */
	flag = ((game.state.killc+game.state.killk)/(game.state.date+0.01-indate) < 0.1*skill*(skill+1.0) ||
			(game.state.date-indate) < 3.0);
	if (iscate==0 && flag) {
		/* compute move away from Enterprise */
		ideltax = game.state.isx-quadx;
		ideltay = game.state.isy-quady;
		if (sqrt(ideltax*(double)ideltax+ideltay*(double)ideltay) > 2.0) {
			/* circulate in space */
			ideltax = game.state.isy-quady;
			ideltay = quadx-game.state.isx;
		}
	}
	else {
		/* compute distances to starbases */
		if (game.state.rembase <= 0) {
			/* nothing left to do */
			game.future[FSCMOVE] = 1e30;
			return;
		}
		sx = game.state.isx;
		sy = game.state.isy;
		for (i = 1; i <= game.state.rembase; i++) {
			basetbl[i] = i;
			ibqx = game.state.baseqx[i];
			ibqy = game.state.baseqy[i];
			bdist[i] = sqrt(square(ibqx-sx) + square(ibqy-sy));
		}
		if (game.state.rembase > 1) {
			/* sort into nearest first order */
			int iswitch;
			do {
				iswitch = 0;
				for (i=1; i < game.state.rembase-1; i++) {
					if (bdist[i] > bdist[i+1]) {
						int ti = basetbl[i];
						double t = bdist[i];
						bdist[i] = bdist[i+1];
						bdist[i+1] = t;
						basetbl[i] = basetbl[i+1];
						basetbl[i+1] =ti;
						iswitch = 1;
					}
				}
			} while (iswitch);
		}
		/* look for nearest base without a commander, no Enterprise, and
		   without too many Klingons, and not already under attack. */
		ifindit = iwhichb = 0;

		for (i2 = 1; i2 <= game.state.rembase; i2++) {
			i = basetbl[i2];	/* bug in original had it not finding nearest*/
			ibqx = game.state.baseqx[i];
			ibqy = game.state.baseqy[i];
			if ((ibqx == quadx && ibqy == quady) ||
				(ibqx == batx && ibqy == baty) ||
				game.state.galaxy[ibqx][ibqy] > 899) continue;
			/* if there is a commander, an no other base is appropriate,
			   we will take the one with the commander */
			for (j = 1; j <= game.state.remcom; j++) {
				if (ibqx==game.state.cx[j] && ibqy==game.state.cy[j] && ifindit!= 2) {
						ifindit = 2;
						iwhichb = i;
						break;
				}
			}
			if (j > game.state.remcom) { /* no commander -- use this one */
				ifindit = 1;
				iwhichb = i;
				break;
			}
		}
		if (ifindit==0) return; /* Nothing suitable -- wait until next time*/
		ibqx = game.state.baseqx[iwhichb];
		ibqy = game.state.baseqy[iwhichb];
		/* decide how to move toward base */
		ideltax = ibqx - game.state.isx;
		ideltay = ibqy - game.state.isy;
	}
	/* Maximum movement is 1 quadrant in either or both axis */
	if (ideltax > 1) ideltax = 1;
	if (ideltax < -1) ideltax = -1;
	if (ideltay > 1) ideltay = 1;
	if (ideltay < -1) ideltay = -1;

	/* try moving in both x and y directions */
	iqx = game.state.isx + ideltax;
	iqy = game.state.isy + ideltax;
	if (checkdest(iqx, iqy, flag, ipage)) {
		/* failed -- try some other maneuvers */
		if (ideltax==0 || ideltay==0) {
			/* attempt angle move */
			if (ideltax != 0) {
				iqy = game.state.isy + 1;
				if (checkdest(iqx, iqy, flag, ipage)) {
					iqy = game.state.isy - 1;
					checkdest(iqx, iqy, flag, ipage);
				}
			}
			else {
				iqx = game.state.isx + 1;
				if (checkdest(iqx, iqy, flag, ipage)) {
					iqx = game.state.isx - 1;
					checkdest(iqx, iqy, flag, ipage);
				}
			}
		}
		else {
			/* try moving just in x or y */
			iqy = game.state.isy;
			if (checkdest(iqx, iqy, flag, ipage)) {
				iqy = game.state.isy + ideltay;
				iqx = game.state.isx;
				checkdest(iqx, iqy, flag, ipage);
			}
		}
	}
	/* check for a base */
	if (game.state.rembase == 0) {
		game.future[FSCMOVE] = 1e30;
	}
	else for (i=1; i<=game.state.rembase; i++) {
		ibqx = game.state.baseqx[i];
		ibqy = game.state.baseqy[i];
		if (ibqx==game.state.isx && ibqy == game.state.isy && game.state.isx != batx && game.state.isy != baty) {
			/* attack the base */
			if (flag) return; /* no, don't attack base! */
			iseenit = 0;
			isatb=1;
			game.future[FSCDBAS] = game.state.date + 1.0 +2.0*Rand();
			if (batx != 0) game.future[FSCDBAS] += game.future[FCDBAS]-game.state.date;
			if (game.damage[DRADIO] > 0 && condit != IHDOCKED)
				return; /* no warning */
			iseenit = 1;
			if (*ipage == 0)  pause(1);
			*ipage=1;
			proutn("Lt. Uhura-  \"Captain, the starbase in ");
			proutn(cramlc(quadrant, game.state.isx, game.state.isy));
			skip(1);
			prout("   reports that it is under attack from the Klingon Super-commander.");
			proutn("   It can survive until stardate %d.\"",
			       (int)game.future[FSCDBAS]);
			if (resting==0) return;
			prout("Mr. Spock-  \"Captain, shall we cancel the rest period?\"");
			if (ja()==0) return;
			resting = 0;
			Time = 0.0; /* actually finished */
			return;
		}
	}
	/* Check for intelligence report */
	if (
#ifdef DEBUG
		idebug==0 &&
#endif
		(Rand() > 0.2 ||
		 (game.damage[DRADIO] > 0.0 && condit != IHDOCKED) ||
		 game.starch[game.state.isx][game.state.isy] > 0))
		return;
	if (*ipage==0) pause(1);
	*ipage = 1;
	prout("Lt. Uhura-  \"Captain, Starfleet Intelligence reports");
	proutn("   the Super-commander is in ");
	proutn(cramlc(quadrant, game.state.isx, game.state. isy));
	prout(".\"");
	return;
}

void movetho(void) {
	int idx, idy, im, i, dum, my;
	/* Move the Tholian */
	if (ithere==0 || justin == 1) return;

	if (ithx == 1 && ithy == 1) {
		idx = 1; idy = 10;
	}
	else if (ithx == 1 && ithy == 10) {
		idx = 10; idy = 10;
	}
	else if (ithx == 10 && ithy == 10) {
		idx = 10; idy = 1;
	}
	else if (ithx == 10 && ithy == 1) {
		idx = 1; idy = 1;
	}
	else {
		/* something is wrong! */
		ithere = 0;
		return;
	}

	/* Do nothing if we are blocked */
	if (game.quad[idx][idy]!= IHDOT && game.quad[idx][idy]!= IHWEB) return;
	game.quad[ithx][ithy] = IHWEB;

	if (ithx != idx) {
		/* move in x axis */
		im = fabs((double)idx - ithx)/((double)idx - ithx);
		while (ithx != idx) {
			ithx += im;
			if (game.quad[ithx][ithy]==IHDOT) game.quad[ithx][ithy] = IHWEB;
		}
	}
	else if (ithy != idy) {
		/* move in y axis */
		im = fabs((double)idy - ithy)/((double)idy - ithy);
		while (ithy != idy) {
			ithy += im;
			if (game.quad[ithx][ithy]==IHDOT) game.quad[ithx][ithy] = IHWEB;
		}
	}
	game.quad[ithx][ithy] = IHT;

	/* check to see if all holes plugged */
	for (i = 1; i < 11; i++) {
		if (game.quad[1][i]!=IHWEB && game.quad[1][i]!=IHT) return;
		if (game.quad[10][i]!=IHWEB && game.quad[10][i]!=IHT) return;
		if (game.quad[i][1]!=IHWEB && game.quad[i][1]!=IHT) return;
		if (game.quad[i][10]!=IHWEB && game.quad[i][10]!=IHT) return;
	}
	/* All plugged up -- Tholian splits */
	game.quad[ithx][ithy]=IHWEB;
	dropin(IHBLANK, &dum, &my);
	crmena(1,IHT, 2, ithx, ithy);
	prout(" completes web.");
	ithere = ithx = ithy = 0;
	return;
}
