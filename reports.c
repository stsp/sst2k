#include "sst.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef SERGEEV
#include <conio.h>
#include "sstlinux.h"
#else
#define c_printf proutn
#endif /* SERGEEV */

void attakreport(int l) {
     if (!l) {
	if (game.future[FCDBAS] < 1e30) {
		prout("Starbase in %s is currently under Commander attack.",
		      cramlc(quadrant, batx, baty));
		prout("It can hold out until Stardate %d.", 
		      (int)game.future[FCDBAS]);
	}
	if (isatb == 1) {
		prout("Starbase in %s is under Super-commander attack.",
		      cramlc(quadrant, game.state.isx, game.state.isy));
		prout("It can hold out until Stardate %d.", 
		      (int)game.future[FSCDBAS]);
	}
     } else {
        if (game.future[FCDBAS] < 1e30)
           proutn("Base in %i - %i attacked by C. Alive until %.1f", batx, baty, game.future[FCDBAS]);
        if (isatb == 1)
           proutn("Base in %i - %i attacked by S. Alive until %.1f", game.state.isx, game.state.isy, game.future[FSCDBAS]);
     }
#ifdef SERGEEV
     clreol();
#endif /* SERGEEV */
}
	

void report(void) {
	char *s1,*s2,*s3;

	chew();
	s1 = (thawed?"thawed ":"");
	switch (length) {
		case 1: s2="short"; break;
		case 2: s2="medium"; break;
		case 4: s2="long"; break;
		default: s2="unknown length"; break;
	}
	switch (skill) {
		case 1: s3="novice"; break;
		case 2: s3="fair"; break;
		case 3: s3="good"; break;
		case 4: s3="expert"; break;
		case 5: s3="emeritus"; break;
		default: s3="skilled"; break;
	}
	prout("");
	prout("You %s playing a %s%s %s game.",
		   alldone? "were": "are now", s1, s2, s3);
	if (skill>3 && thawed && !alldone) prout("No plaque is allowed.");
	if (tourn) prout("This is tournament game %d.", tourn);
	prout("Your secret password is \"%s\"",game.passwd);
	proutn("%d of %d Klingons have been killed",
		   game.state.killk+game.state.killc+game.state.nsckill, inkling);
	if (game.state.killc) prout(", including %d Commander%s.", game.state.killc, game.state.killc==1?"":"s");
	else if (game.state.killk+game.state.nsckill > 0) prout(", but no Commanders.");
	else prout(".");
	if (skill > 2) prout("The Super Commander has %sbeen destroyed.",
						  game.state.nscrem?"not ":"");
	if (game.state.rembase != inbase) {
		proutn("There ");
		if (inbase-game.state.rembase==1) proutn("has been 1 base");
		else {
		    proutn("have been %d bases", inbase-game.state.rembase);
		}
		prout(" destroyed, %d remaining.", game.state.rembase);
	}
	else prout("There are %d bases.", inbase);
	if (game.damage[DRADIO] == 0.0 || condit == IHDOCKED || iseenit) {
		/* Don't report this if not seen and
			either the radio is dead or not at base! */
		attakreport(0);
		iseenit = 1;
	}
	if (casual) prout("%d casualt%s suffered so far.",
					   casual, casual==1? "y" : "ies");
	if (nhelp) prout("There were %d call%s for help.",
					  nhelp, nhelp==1 ? "" : "s");
	if (ship == IHE) {
		proutn("You have ");
		if (nprobes) proutn("%d", nprobes);
		else proutn("no");
		proutn(" deep space probe");
		if (nprobes!=1) proutn("s");
		prout(".");
	}
	if ((game.damage[DRADIO] == 0.0 || condit == IHDOCKED)&&
		game.future[FDSPROB] != 1e30) {
		if (isarmed) 
			proutn("An armed deep space probe is in");
		else
			proutn("A deep space probe is in");
		proutn(cramlc(quadrant, probecx, probecy));
		prout(".");
	}
	if (icrystl) {
		if (cryprob <= .05)
			prout("Dilithium crystals aboard ship... not yet used.");
		else {
			int i=0;
			double ai = 0.05;
			while (cryprob > ai) {
				ai *= 2.0;
				i++;
			}
			prout("Dilithium crystals have been used %d time%s.",
				   i, i==1? "" : "s");
		}
	}
	skip(1);
}
	
void lrscan(void) {
	int x, y;
	chew();
	if (game.damage[DLRSENS] != 0.0) {
		/* Now allow base's sensors if docked */
		if (condit != IHDOCKED) {
			prout("LONG-RANGE SENSORS DAMAGED.");
			return;
		}
		proutn("Starbase's long-range scan");
	}
	else {
		prout("Long-range scan");
	}
	for (x = quadx-1; x <= quadx+1; x++) {
		proutn(" ");
		for (y = quady-1; y <= quady+1; y++) {
			if (x == 0 || x > 8 || y == 0 || y > 8)
                                proutn("  -1");
			else {
                                if (game.state.galaxy[x][y]<1000) proutn(" %3d", game.state.galaxy[x][y]);
                                else proutn("***");
				game.starch[x][y] = game.damage[DRADIO] > 0 ? game.state.galaxy[x][y]+1000 : 1;
			}
		}
                prout(" ");
	}
}

void dreprt(void) {
	int jdam = FALSE, i;
	chew();

	for (i = 1; i <= NDEVICES; i++) {
		if (game.damage[i] > 0.0) {
			if (!jdam) {
				prout("DEVICE            -REPAIR TIMES-");
				prout("                IN FLIGHT   DOCKED");
				jdam = TRUE;
			}
			prout("  %16s %8.2f  %8.2f", 
				device[i],
			        game.damage[i]+0.05,
				docfac*game.damage[i]+0.005);
		}
	}
	if (!jdam) prout("All devices functional.");
}

void chart(int nn) {
	int i,j;
	char *cp;
	chew();
	if (stdamtim != 1e30 && stdamtim != game.state.date && condit == IHDOCKED) {
                c_printf("Spock-  \"I revised the Star Chart from the starbase's records.\"\n\r");
	}
        if (nn == 0) c_printf("       STAR CHART FOR THE KNOWN GALAXY\n\r");
	if (stdamtim != 1e30) {
		if (condit == IHDOCKED) {
			/* We are docked, so restore chart from base information */
			stdamtim = game.state.date;
			for (i=1; i <= 8 ; i++)
				for (j=1; j <= 8; j++)
					if (game.starch[i][j] == 1) game.starch[i][j] = game.state.galaxy[i][j]+1000;
		}
		else {
		    proutn("(Last surveillance update %d stardates ago.",
			   (int)(game.state.date-stdamtim));
		}
	}

	prout("      1    2    3    4    5    6    7    8");
	for (i = 1; i <= 8; i++) {
                c_printf("%d |", i);
		for (j = 1; j <= 8; j++) {
		    char buf[4];
                        c_printf("  ");
			if (game.starch[i][j] < 0)
                                strcpy(buf, ".1.");
			else if (game.starch[i][j] == 0)
                                strcpy(buf, "...");
			else if (game.starch[i][j] > 999)
                                if ((i==quadx)&&(j==quady)){
#ifdef SERGEEV
                                   gotoxy(wherex()-1,wherey());
#endif /* SERGEEV */
                                   if (game.starch[i][i]<2000)
				       sprintf(buf, "%03d", game.starch[i][j]-1000);
                                   else 
				       strcpy(buf, "***");
                                }
                                else
                                    if (game.starch[i][j]<2000) 
					sprintf(buf, "%03d", game.starch[i][j]-1000);
                                    else 
					strcpy(buf, "***");
                        else if ((i==quadx)&&(j==quady)){
#ifdef SERGEEV
                                gotoxy(wherex()-1,wherey());
#endif /* SERGEEV */
                                sprintf(buf, "%03d", game.state.galaxy[i][j]);
                        }
                        else if (game.state.galaxy[i][j]>=1000)
                                strcpy(buf, "***");
			else
				sprintf(buf, "%03d", game.state.galaxy[i][j]);
			for (cp = buf; cp < buf + sizeof(buf); cp++)
			    if (*cp == '0')
				*cp = '.';
			c_printf(buf);
		}
                c_printf("  |");
                if (i<8) c_printf("\n\r");
	}
#ifdef SERGEEV
	proutn("");	/* flush output */
#else
	skip(2);
#endif
}
		
		
int srscan(int l) {
        char *cp = NULL;
        int leftside=TRUE, rightside=TRUE, i, j, jj, k=0, nn=FALSE, t, dam=0;
	int goodScan=TRUE;
	switch (l) {
		case 1: // SRSCAN
			if (game.damage[DSRSENS] != 0) {
				/* Allow base's sensors if docked */
				if (condit != IHDOCKED) {
                                        prout("   S.R. SENSORS DAMAGED!");
					goodScan=FALSE;
				}
				else
                                        prout("  [Using Base's sensors]");
			}
                        else c_printf("     Short-range scan\n\r");
                        if (goodScan) game.starch[quadx][quady] = game.damage[DRADIO]>0.0 ? game.state.galaxy[quadx][quady]+1000:1;
			scan();
			if (isit("chart")) nn = TRUE;
			rightside = FALSE;
			chew();
                        c_printf("    1 2 3 4 5 6 7 8 9 10\n\r");
			break;
		case 2: // REQUEST
                        leftside=FALSE;
                        break;
		case 3: // STATUS
			chew();
			leftside = FALSE;
			skip(1);
	}
	if (condit != IHDOCKED) newcnd();
	for (i = 1; i <= 10; i++) {
		jj = (k!=0 ? k : i);
		if (leftside) {
			proutn("%2d  ", i);
			for (j = 1; j <= 10; j++) {
                                if (goodScan || (abs(i-sectx)<= 1 && abs(j-secty) <= 1)){
                                   if ((game.quad[i][j]==IHMATER0)||(game.quad[i][j]==IHMATER1)||(game.quad[i][j]==IHMATER2)||(game.quad[i][j]==IHE)||(game.quad[i][j]==IHF)){
#ifdef SERGEEV
                                        switch (condit) {
                                                case IHRED: textcolor(RED); break;
                                                case IHGREEN: textcolor(GREEN); break;
                                                case IHYELLOW: textcolor(YELLOW); break;
                                                case IHDOCKED: textcolor(LIGHTGRAY); break;
                                                case IHDEAD: textcolor(WHITE);
                                        }
                                        if (game.quad[i][j]!=ship) highvideo();
#endif /* SERGEEV */
                                   }
#ifdef SERGEEV
                                   if (game.quad[i][j] & 128) highvideo();
#endif /* SERGEEV */
                                   c_printf("%c ",game.quad[i][j] & 127);
#ifdef SERGEEV
                                   textcolor(LIGHTGRAY);
#endif /* SERGEEV */
                                }
				else
					proutn("- ");
			}
		}
		if (rightside) {
			switch (jj) {
				case 1:
                                        proutn("Stardate      %.1f, Time Left %.2f", game.state.date, game.state.remtime);
					break;
				case 2:
					if (condit != IHDOCKED) newcnd();
					switch (condit) {
						case IHRED: cp = "RED"; break;
						case IHGREEN: cp = "GREEN"; break;
						case IHYELLOW: cp = "YELLOW"; break;
						case IHDOCKED: cp = "DOCKED"; break;
                                                case IHDEAD: cp="DEAD"; break;
					}
                                        for (t=0;t<=NDEVICES;t++)
                                            if (game.damage[t]>0) dam++;
                                        proutn("Condition     %s, %i DAMAGES", cp, dam);
					break;
				case 3:
                                        proutn("Position      %d - %d , %d - %d",
					    quadx, quady, sectx, secty);
					break;
				case 4:
                                        proutn("Life Support  ");
					if (game.damage[DLIFSUP] != 0.0) {
						if (condit == IHDOCKED)
                                                        proutn("DAMAGED, Base provides");
						else
                                                        proutn("DAMAGED, reserves=%4.2f", lsupres);
					}
					else
						proutn("ACTIVE");
					break;
				case 5:
					proutn("Warp Factor   %.1f", warpfac);
					break;
				case 6:
					proutn("Energy        %.2f", energy);
					break;
				case 7:
					proutn("Torpedoes     %d", torps);
					break;
				case 8:
					proutn("Shields       ");
					if (game.damage[DSHIELD] != 0)
						proutn("DAMAGED,");
					else if (shldup)
						proutn("UP,");
					else
						proutn("DOWN,");
					proutn(" %d%% %.1f units",
						   (int)((100.0*shield)/inshld + 0.5), shield);
					break;
				case 9:
					proutn("Klingons Left %d", game.state.remkl);
					break;
				case 10:
                                        attakreport(1);
					break;
			}
		}
    		if (i<10) c_printf("\n\r");
	        if (k!=0) return(goodScan);
	}
	if (nn) chart(1);
#ifdef SERGEEV
	proutn("");
#else
	skip(2);
#endif /* SERGEEV */
        return(goodScan);
}
			
			
void eta(void) {
        int ix1, ix2, iy1, iy2, prompt=FALSE;
	int wfl;
	double ttime, twarp, tpower;
	if (game.damage[DCOMPTR] != 0.0) {
		prout("COMPUTER DAMAGED, USE A POCKET CALCULATOR.");
		skip(1);
		return;
	}
	if (scan() != IHREAL) {
		prompt = TRUE;
		chew();
		proutn("Destination quadrant and/or sector? ");
		if (scan()!=IHREAL) {
			huh();
			return;
		}
	}
	iy1 = aaitem +0.5;
	if (scan() != IHREAL) {
		huh();
		return;
	}
	ix1 = aaitem + 0.5;
	if (scan() == IHREAL) {
		iy2 = aaitem + 0.5;
		if (scan() != IHREAL) {
			huh();
			return;
		}
		ix2 = aaitem + 0.5;
	}
        else {
                if (quady>ix1) ix2 = 1;
                else ix2=10;
                if (quadx>iy1) iy2 = 1;
                else iy2=10;
	}

	if (ix1 > 8 || ix1 < 1 || iy1 > 8 || iy1 < 1 ||
		ix2 > 10 || ix2 < 1 || iy2 > 10 || iy2 < 1) {
		huh();
		return;
	}
	dist = sqrt(square(iy1-quadx+0.1*(iy2-sectx))+
				square(ix1-quady+0.1*(ix2-secty)));
	wfl = FALSE;

	if (prompt) prout("Answer \"no\" if you don't know the value:");
	while (TRUE) {
		chew();
		proutn("Time or arrival date? ");
		if (scan()==IHREAL) {
			ttime = aaitem;
			if (ttime > game.state.date) ttime -= game.state.date; // Actually a star date
			if (ttime <= 1e-10 ||
				(twarp=(floor(sqrt((10.0*dist)/ttime)*10.0)+1.0)/10.0) > 10) {
				prout("We'll never make it, sir.");
				chew();
				return;
			}
			if (twarp < 1.0) twarp = 1.0;
			break;
		}
		chew();
		proutn("Warp factor? ");
		if (scan()== IHREAL) {
			wfl = TRUE;
			twarp = aaitem;
			if (twarp<1.0 || twarp > 10.0) {
				huh();
				return;
			}
			break;
		}
		prout("Captain, certainly you can give me one of these.");
	}
	while (TRUE) {
		chew();
		ttime = (10.0*dist)/square(twarp);
		tpower = dist*twarp*twarp*twarp*(shldup+1);
		if (tpower >= energy) {
			prout("Insufficient energy, sir.");
			if (shldup==0 || tpower > energy*2.0) {
				if (!wfl) return;
				proutn("New warp factor to try? ");
				if (scan() == IHREAL) {
					wfl = TRUE;
					twarp = aaitem;
					if (twarp<1.0 || twarp > 10.0) {
						huh();
						return;
					}
					continue;
				}
				else {
					chew();
					skip(1);
					return;
				}
			}
			prout("But if you lower your shields,");
			proutn("remaining");
			tpower /= 2;
		}
		else
			proutn("Remaining");
		prout(" energy will be %.2f.", energy-tpower);
		if (wfl) {
			prout("And we will arrive at stardate %.2f.",
				game.state.date+ttime);
		}
		else if (twarp==1.0)
			prout("Any warp speed is adequate.");
		else {
			prout("Minimum warp needed is %.2f,", twarp);
			prout("and we will arrive at stardate %.2f.",
				game.state.date+ttime);
		}
		if (game.state.remtime < ttime)
			prout("Unfortunately, the Federation will be destroyed by then.");
		if (twarp > 6.0)
			prout("You'll be taking risks at that speed, Captain");
		if ((isatb==1 && game.state.isy == ix1 && game.state.isx == iy1 &&
			 game.future[FSCDBAS]< ttime+game.state.date)||
			(game.future[FCDBAS]<ttime+game.state.date && baty==ix1 && batx == iy1))
			prout("The starbase there will be destroyed by then.");
		proutn("New warp factor to try? ");
		if (scan() == IHREAL) {
			wfl = TRUE;
			twarp = aaitem;
			if (twarp<1.0 || twarp > 10.0) {
				huh();
				return;
			}
		}
		else {
			chew();
			skip(1);
			return;
		}
	}
			
}
