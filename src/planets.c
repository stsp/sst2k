#include "sst.h"

static char *classes[] = {"M","N","O"};

static int consumeTime(void) 
{
/* I think most of this avoidance was caused by overlay scheme.
   Let's see what happens if all events can occur here */

//  double asave;
    game.ididit = 1;
#if 0
    /* Don't worry about this */
    if (scheduled(FTBEAM) <= game.state.date+game.optime && game.state.remcom != 0 && game.condit != IHDOCKED) {
	/* We are about to be tractor beamed -- operation fails */
	return 1;
    }
#endif
//	asave = scheduled(FSNOVA);
//	unschedule(FSNOVA); /* defer supernovas */
    events();	/* Used to avoid if FSCMOVE is scheduled within time */
//	schedule(FSNOVA, asave-game.state.time);
    /*fails if game over, quadrant super-novas or we've moved to new quadrant*/
    if (game.alldone || game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova || game.justin != 0) return 1;
    return 0;
}

void preport(void) 
{
    bool iknow = false;
    int i;
    skip(1);
    chew();
    prout("Spock-  \"Planet report follows, Captain.\"");
    skip(1);
    for (i = 0; i < game.inplan; i++) {
	if ((game.state.plnets[i].known != unknown
	    && game.state.plnets[i].crystals != 0)
	    || (idebug && game.state.plnets[i].w.x !=0)
	    ) {
	    iknow = true;
	    if (idebug && game.state.plnets[i].known==unknown)
		proutn("(Unknown) ");
	    proutn(cramlc(quadrant, game.state.plnets[i].w));
	    proutn("   class ");
	    proutn(classes[game.state.plnets[i].pclass]);
	    proutn("   ");
	    if (game.state.plnets[i].crystals <= 0) proutn("no ");
	    prout("dilithium crystals present.");
	    if (game.state.plnets[i].known==shuttle_down) 
		prout("    Shuttle Craft Galileo on surface.");
	}
    }
    if (iknow==0) prout("No information available.");
}

void orbit(void) 
{
    skip(1);
    chew();
    if (game.inorbit) {
	prout("Already in standard orbit.");
	return;
    }
    if (game.damage[DWARPEN] != 0 && game.damage[DIMPULS] != 0) {
	prout("Both warp and impulse engines damaged.");
	return;
    }
    if (game.plnet.x == 0 || abs(game.sector.x-game.plnet.x) > 1 || abs(game.sector.y-game.plnet.y) > 1) {
	crmshp();
	prout(" not adjacent to planet.");
	skip(1);
	return;
    }
    game.optime = 0.02+0.03*Rand();
    prout("Helmsman Sulu-  \"Entering standard orbit, Sir.\"");
    newcnd();
    if (consumeTime()) return;
    game.height = (1400.0+7200.0*Rand());
    prout("Sulu-  \"Entered orbit at altitude %.2f kilometers.\"", game.height);
    game.inorbit = true;
    game.ididit = true;
}

void sensor(void) 
{
    skip(1);
    chew();
    if (game.damage[DSRSENS] != 0.0) {
	prout("Short range sensors damaged.");
	return;
    }
    if (!game.plnet.x && (game.options & OPTION_TTY)) {
	prout("Spock- \"No planet in this quadrant, Captain.\"");
	return;
    }
    if ((game.plnet.x != 0)&& (game.state.plnets[game.iplnet].known == unknown)) {
	prout("Spock-  \"Sensor scan for %s-", cramlc(quadrant, game.quadrant));
	skip(1);
	prout("         Planet at %s is of class %s.",
	      cramlc(sector,game.plnet),
	      classes[game.state.plnets[game.iplnet].pclass]);
	if (game.state.plnets[game.iplnet].known==shuttle_down) 
	    prout("         Sensors show Galileo still on surface.");
	proutn("         Readings indicate");
	if (game.state.plnets[game.iplnet].crystals == 0) proutn(" no");
	prout(" dilithium crystals present.\"");
	if (game.state.plnets[game.iplnet].known == unknown) game.state.plnets[game.iplnet].known = known;
    }
}

void beam(void) 
{
    chew();
    skip(1);
    if (game.damage[DTRANSP] != 0) {
	prout("Transporter damaged.");
	if (game.damage[DSHUTTL]==0 && (game.state.plnets[game.iplnet].known==shuttle_down || game.iscraft == 1)) {
	    skip(1);
	    proutn("Spock-  \"May I suggest the shuttle craft, Sir?\" ");
	    if (ja() != 0) shuttle();
	}
	return;
    }
    if (!game.inorbit) {
	crmshp();
	prout(" not in standard orbit.");
	return;
    }
    if (game.shldup) {
	prout("Impossible to transport through shields.");
	return;
    }
    if (game.state.plnets[game.iplnet].known==unknown) {
	prout("Spock-  \"Captain, we have no information on this planet");
	prout("  and Starfleet Regulations clearly state that in this situation");
	prout("  you may not go down.\"");
	return;
    }
    if (game.landed==1) {
	/* Coming from planet */
	if (game.state.plnets[game.iplnet].known==shuttle_down) {
	    proutn("Spock-  \"Wouldn't you rather take the Galileo?\" ");
	    if (ja() != 0) {
		chew();
		return;
	    }
	    prout("Your crew hides the Galileo to prevent capture by aliens.");
	}
	prout("Landing party assembled, ready to beam up.");
	skip(1);
	prout("Kirk whips out communicator...");
	prouts("BEEP  BEEP  BEEP");
	skip(2);
	prout("\"Kirk to enterprise-  Lock on coordinates...energize.\"");
    }
    else {
	/* Going to planet */
	if (game.state.plnets[game.iplnet].crystals==0) {
	    prout("Spock-  \"Captain, I fail to see the logic in");
	    prout("  exploring a planet with no dilithium crystals.");
	    proutn("  Are you sure this is wise?\" ");
	    if (ja()==0) {
		chew();
		return;
	    }
	}
	prout("Scotty-  \"Transporter room ready, Sir.\"");
	skip(1);
	prout("Kirk, and landing party prepare to beam down to planet surface.");
	skip(1);
	prout("Kirk-  \"Energize.\"");
    }
    game.ididit=1;
    skip(1);
    prouts("WWHOOOIIIIIRRRRREEEE.E.E.  .  .  .  .   .    .");
    skip(2);
    if (Rand() > 0.98) {
	prouts("BOOOIIIOOOIIOOOOIIIOIING . . .");
	skip(2);
	prout("Scotty-  \"Oh my God!  I've lost them.\"");
	finish(FLOST);
	return;
    }
    prouts(".    .   .  .  .  .  .E.E.EEEERRRRRIIIIIOOOHWW");
    skip(2);
    prout("Transport complete.");
    game.landed = -game.landed;
    if (game.landed==1 && game.state.plnets[game.iplnet].known==shuttle_down) {
	prout("The shuttle craft Galileo is here!");
    }
    if (game.landed!=1 && game.imine==1) {
	game.icrystl = 1;
	game.cryprob = 0.05;
    }
    game.imine = 0;
    return;
}

void mine(void) 
{
    skip(1);
    chew();
    if (game.landed!= 1) {
	prout("Mining party not on planet.");
	return;
    }
    if (game.state.plnets[game.iplnet].crystals == MINED) {
	prout("This planet has already been strip-mined for dilithium.");
	return;
    }
    else if (game.state.plnets[game.iplnet].crystals == 0) {
	prout("No dilithium crystals on this planet.");
	return;
    }
    if (game.imine == 1) {
	prout("You've already mined enough crystals for this trip.");
	return;
    }
    if (game.icrystl == 1 && game.cryprob == 0.05) {
	proutn("With all those fresh crystals aboard the ");
	crmshp();
	skip(1);
	prout("there's no reason to mine more at this time.");
	return;
    }
    game.optime = (0.1+0.2*Rand())*game.state.plnets[game.iplnet].pclass;
    if (consumeTime()) return;
    prout("Mining operation complete.");
    game.state.plnets[game.iplnet].crystals = MINED;
    game.imine = 1;
    game.ididit=1;
}

void usecrystals(void) 
{
    game.ididit=0;
    skip(1);
    chew();
    if (game.icrystl!=1) {
	prout("No dilithium crystals available.");
	return;
    }
    if (game.energy >= 1000) {
	prout("Spock-  \"Captain, Starfleet Regulations prohibit such an operation");
	prout("  except when game.condition Yellow exists.");
	return;
    }
    prout("Spock- \"Captain, I must warn you that loading");
    prout("  raw dilithium crystals into the ship's power");
    prout("  system may risk a severe explosion.");
    proutn("  Are you sure this is wise?\" ");
    if (ja()==0) {
	chew();
	return;
    }
    skip(1);
    prout("Engineering Officer Scott-  \"(GULP) Aye Sir.");
    prout("  Mr. Spock and I will try it.\"");
    skip(1);
    prout("Spock-  \"Crystals in place, Sir.");
    prout("  Ready to activate circuit.\"");
    skip(1);
    prouts("Scotty-  \"Keep your fingers crossed, Sir!\"");
    skip(1);
    if (Rand() <= game.cryprob) {
	prouts("  \"Activating now! - - No good!  It's***");
	skip(2);
	prouts("***RED ALERT!  RED A*L********************************");
	skip(1);
	stars();
	prouts("******************   KA-BOOM!!!!   *******************");
	skip(1);
	kaboom();
	return;
    }
    game.energy += 5000.0*(1.0 + 0.9*Rand());
    prouts("  \"Activating now! - - ");
    prout("The instruments");
    prout("   are going crazy, but I think it's");
    prout("   going to work!!  Congratulations, Sir!\"");
    game.cryprob *= 2.0;
    game.ididit=1;
}

void shuttle(void) 
{
    chew();
    skip(1);
    if(game.damage[DSHUTTL] != 0.0) {
	if (game.damage[DSHUTTL] == -1.0) {
	    if (game.inorbit && game.state.plnets[game.iplnet].known == shuttle_down)
		prout("Ye Faerie Queene has no shuttle craft bay to dock it at.");
	    else
		prout("Ye Faerie Queene had no shuttle craft.");
	}
	else if (game.damage[DSHUTTL] > 0)
	    prout("The Galileo is damaged.");
	else prout("Shuttle craft is now serving Big Macs.");
	return;
    }
    if (!game.inorbit) {
	crmshp();
	prout(" not in standard orbit.");
	return;
    }
    if ((game.state.plnets[game.iplnet].known != shuttle_down) && game.iscraft != 1) {
	prout("Shuttle craft not currently available.");
	return;
    }
    if (game.landed==-1 && game.state.plnets[game.iplnet].known==shuttle_down) {
	prout("You will have to beam down to retrieve the shuttle craft.");
	return;
    }
    if (game.shldup || game.condit == IHDOCKED) {
	prout("Shuttle craft cannot pass through shields.");
	return;
    }
    if (game.state.plnets[game.iplnet].known==unknown) {
	prout("Spock-  \"Captain, we have no information on this planet");
	prout("  and Starfleet Regulations clearly state that in this situation");
	prout("  you may not fly down.\"");
	return;
    }
    game.optime = 3.0e-5*game.height;
    if (game.optime >= 0.8*game.state.remtime) {
	prout("First Officer Spock-  \"Captain, I compute that such");
	proutn("  a maneuver would require approximately 2d%% of our",
	       (int)(100*game.optime/game.state.remtime));
	prout("remaining time.");
	proutn("Are you sure this is wise?\" ");
	if (ja()==0) {
	    game.optime = 0.0;
	    return;
	}
    }
    if (game.landed == 1) {
	/* Kirk on planet */
	if (game.iscraft==1) {
	    /* Galileo on ship! */
	    if (game.damage[DTRANSP]==0) {
		proutn("Spock-  \"Would you rather use the transporter?\" ");
		if (ja() != 0) {
		    beam();
		    return;
		}
		proutn("Shuttle crew");
	    }
	    else
		proutn("Rescue party");
	    prout(" boards Galileo and swoops toward planet surface.");
	    game.iscraft = 0;
	    skip(1);
	    if (consumeTime()) return;
	    game.state.plnets[game.iplnet].known=shuttle_down;
	    prout("Trip complete.");
	    return;
	}
	else {
	    /* Ready to go back to ship */
	    prout("You and your mining party board the");
	    prout("shuttle craft for the trip back to the Enterprise.");
	    skip(1);
	    prout("The short hop begins . . .");
	    game.state.plnets[game.iplnet].known=known;
	    game.icraft = 1;
	    skip(1);
	    game.landed = -1;
	    if (consumeTime()) return;
	    game.iscraft = 1;
	    game.icraft = 0;
	    if (game.imine!=0) {
		game.icrystl = 1;
		game.cryprob = 0.05;
	    }
	    game.imine = 0;
	    prout("Trip complete.");
	    return;
	}
    }
    else {
	/* Kirk on ship */
	/* and so is Galileo */
	prout("Mining party assembles in the hangar deck,");
	prout("ready to board the shuttle craft \"Galileo\".");
	skip(1);
	prouts("The hangar doors open; the trip begins.");
	skip(1);
	game.icraft = 1;
	game.iscraft = 0;
	if (consumeTime()) return;
	game.state.plnets[game.iplnet].known = shuttle_down;
	game.landed = 1;
	game.icraft = 0;
	prout("Trip complete");
	return;
    }
}

void deathray(void) 
{
    double dprob, r = Rand();
	
    game.ididit = 0;
    skip(1);
    chew();
    if (game.ship != IHE) {
	prout("Ye Faerie Queene has no death ray.");
	return;
    }
    if (game.nenhere==0) {
	prout("Sulu-  \"But Sir, there are no enemies in this quadrant.\"");
	return;
    }
    if (game.damage[DDRAY] > 0.0) {
	prout("Death Ray is damaged.");
	return;
    }
    prout("Spock-  \"Captain, the 'Experimental Death Ray'");
    prout("  is highly unpredictible.  Considering the alternatives,");
    proutn("  are you sure this is wise?\" ");
    if (ja()==0) return;
    prout("Spock-  \"Acknowledged.\"");
    skip(1);
    game.ididit=1;
    prouts("WHOOEE ... WHOOEE ... WHOOEE ... WHOOEE");
    skip(1);
    prout("Crew scrambles in emergency preparation.");
    prout("Spock and Scotty ready the death ray and");
    prout("prepare to channel all ship's power to the device.");
    skip(1);
    prout("Spock-  \"Preparations complete, sir.\"");
    prout("Kirk-  \"Engage!\"");
    skip(1);
    prouts("WHIRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR");
    skip(1);
    dprob = .30;
    if (game.options & OPTION_PLAIN)
	dprob = .5;
    if (r > dprob) {
	prouts("Sulu- \"Captain!  It's working!\"");
	skip(2);
	while (game.nenhere > 0)
	    deadkl(game.ks[1], game.quad[game.ks[1].x][game.ks[1].y],game.ks[1].x,game.ks[1].y);
	prout("Ensign Chekov-  \"Congratulations, Captain!\"");
	if (KLINGREM == 0) finish(FWON);
	if ((game.options & OPTION_PLAIN) == 0) {
	    prout("Spock-  \"Captain, I believe the `Experimental Death Ray'");
	    if (Rand() <= 0.05) {
		prout("   is still operational.\"");
	    }
	    else {
		prout("   has been rendered nonfunctional.\"");
		game.damage[DDRAY] = 39.95;
	    }
	}
	return;
    }
    r = Rand();	// Pick failure method 
    if (r <= .30) {
	prouts("Sulu- \"Captain!  It's working!\"");
	skip(1);
	prouts("***RED ALERT!  RED ALERT!");
	skip(1);
	prout("***MATTER-ANTIMATTER IMPLOSION IMMINENT!");
	skip(1);
	prouts("***RED ALERT!  RED A*L********************************");
	skip(1);
	stars();
	prouts("******************   KA-BOOM!!!!   *******************");
	skip(1);
	kaboom();
	return;
    }
    if (r <= .55) {
	prouts("Sulu- \"Captain!  Yagabandaghangrapl, brachriigringlanbla!\"");
	skip(1);
	prout("Lt. Uhura-  \"Graaeek!  Graaeek!\"");
	skip(1);
	prout("Spock-  \"Fascinating!  . . . All humans aboard");
	prout("  have apparently been transformed into strange mutations.");
	prout("  Vulcans do not seem to be affected.");
	skip(1);
	prout("Kirk-  \"Raauch!  Raauch!\"");
	finish(FDRAY);
	return;
    }
    if (r <= 0.75) {
	int i,j;
	prouts("Sulu- \"Captain!  It's   --WHAT?!?!\"");
	skip(2);
	proutn("Spock-  \"I believe the word is");
	prouts(" *ASTONISHING*");
	prout(" Mr. Sulu.");
	for_sectors(i)
	    for_sectors(j)
		if (game.quad[i][j] == IHDOT) game.quad[i][j] = IHQUEST;
	prout("  Captain, our quadrant is now infested with");
	prouts(" - - - - - -  *THINGS*.");
	skip(1);
	prout("  I have no logical explanation.\"");
	return;
    }
    prouts("Sulu- \"Captain!  The Death Ray is creating tribbles!\"");
    skip(1);
    prout("Scotty-  \"There are so many tribbles down here");
    prout("  in Engineering, we can't move for 'em, Captain.\"");
    finish(FTRIBBLE);
    return;
}

char *systemname(int pindx)
{
    static char	*names[NINHAB] =
    {
	/*
	 * I used <http://www.memory-alpha.org> to find planets
	 * with references in ST:TOS.  Eath and the Alpha Centauri
	 * Colony have been omitted.
 	 *
	 * Some planets marked Class G and P here will be displayed as class M
	 * because of the way planets are generated. This is a known bug.
	 */
	"ERROR",
	// Federation Worlds
	"Andoria (Fesoan)",	/* several episodes */
	"Tellar Prime (Miracht)",	/* TOS: "Journey to Babel" */
	"Vulcan (T'Khasi)",	/* many episodes */
	"Medusa",		/* TOS: "Is There in Truth No Beauty?" */
	"Argelius II (Nelphia)",/* TOS: "Wolf in the Fold" ("IV" in BSD) */
	"Ardana",		/* TOS: "The Cloud Minders" */
	"Catulla (Cendo-Prae)",	/* TOS: "The Way to Eden" */
	"Gideon",		/* TOS: "The Mark of Gideon" */
	"Aldebaran III",	/* TOS: "The Deadly Years" */
	"Alpha Majoris I",	/* TOS: "Wolf in the Fold" */
	"Altair IV",		/* TOS: "Amok Time */
	"Ariannus",		/* TOS: "Let That Be Your Last Battlefield" */
	"Benecia"		/* TOS: "The Conscience of the King" */
	"Beta Niobe I (Sarpeidon)",	/* TOS: "All Our Yesterdays" */
	"Alpha Carinae II",	/* TOS: "The Ultimate Computer" */
	"Capella IV (Kohath)",	/* TOS: "Friday's Child" (Class G) */
	"Daran V",		/* TOS: "For the World is Hollow and I Have Touched the Sky" */
	"Deneb II",		/* TOS: "Wolf in the Fold" ("IV" in BSD) */
	"Eminiar VII",		/* TOS: "A Taste of Armageddon" */
	"Gamma Canaris IV",	/* TOS: "Metamorphosis" */
	"Gamma Tranguli VI (Vaalel)",	/* TOS: "The Apple" */
	"Ingraham B",		/* TOS: "Operation: Annihilate" */
	"Janus IV",		/* TOS: "The Devil in the Dark" */
	"Makus III",		/* TOS: "The Galileo Seven" */
	"Marcos XII",		/* TOS: "And the Children Shall Lead", */
	"Omega IV",		/* TOS: "The Omega Glory" */
	"Regulus V",		/* TOS: "Amok Time */
	"Deeva",		/* TOS: "Operation -- Annihilate!" */
	/* Worlds from BSD Trek */
	"Rigel II",		/* TOS: "Shore Leave" ("III" in BSD) */
	"Beta III",		/* TOS: "The Return of the Archons" */
	"Triacus",		/* TOS: "And the Children Shall Lead", */
	"Exo III",		/* TOS: "What Are Little Girls Made Of?" (Class P) */
#if 0
	// Others
	"Hansen's Planet",	/* TOS: "The Galileo Seven" */
	"Taurus IV",		/* TOS: "The Galileo Seven" (class G) */
	"Antos IV (Doraphane)",	/* TOS: "Whom Gods Destroy", "Who Mourns for Adonais?" */
	"Izar",			/* TOS: "Whom Gods Destroy" */
	"Tiburon",		/* TOS: "The Way to Eden" */
	"Merak II",		/* TOS: "The Cloud Minders" */ 
	"Coridan (Desotriana)",	/* TOS: "Journey to Babel" */
	"Iotia",		/* TOS: "A Piece of the Action" */
#endif
   };

    return names[pindx];
}
