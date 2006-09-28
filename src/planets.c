#include "sst.h"

static char *classes[] = {"M","N","O"};

static bool consumeTime(void)
/* abort a lengthy operation if an event interrupts it */
{
    game.ididit = true;
    events();
    if (game.alldone || game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova || game.justin) 
	return true;
    return false;
}

void preport(void) 
/* report on (uninhabited) planets in the galaxy */
{
    bool iknow = false;
    int i;
    skip(1);
    chew();
    prout(_("Spock-  \"Planet report follows, Captain.\""));
    skip(1);
    for (i = 0; i < game.inplan; i++) {
	if ((game.state.plnets[i].known != unknown
	    && game.state.plnets[i].inhabited == UNINHABITED)
	    || (idebug && game.state.plnets[i].w.x !=0)
	    ) {
	    iknow = true;
	    if (idebug && game.state.plnets[i].known==unknown)
		proutn("(Unknown) ");
	    proutn(cramlc(quadrant, game.state.plnets[i].w));
	    proutn(_("   class "));
	    proutn(classes[game.state.plnets[i].pclass]);
	    proutn("   ");
	    if (game.state.plnets[i].crystals != present)
		proutn(_("no "));
	    prout(_("dilithium crystals present."));
	    if (game.state.plnets[i].known==shuttle_down) 
		prout(_("    Shuttle Craft Galileo on surface."));
	}
    }
    if (!iknow)
	prout(_("No information available."));
}

void orbit(void)
/* enter standard orbit */
{
    skip(1);
    chew();
    if (game.inorbit) {
	prout(_("Already in standard orbit."));
	return;
    }
    if (damaged(DWARPEN) && damaged(DIMPULS)) {
	prout(_("Both warp and impulse engines damaged."));
	return;
    }
    if (!is_valid(game.plnet) || abs(game.sector.x-game.plnet.x) > 1 || abs(game.sector.y-game.plnet.y) > 1) {
	crmshp();
	prout(_(" not adjacent to planet."));
	skip(1);
	return;
    }
    game.optime = 0.02+0.03*Rand();
    prout(_("Helmsman Sulu-  \"Entering standard orbit, Sir.\""));
    newcnd();
    if (consumeTime())
	return;
    game.height = (1400.0+7200.0*Rand());
    prout(_("Sulu-  \"Entered orbit at altitude %.2f kilometers.\""), game.height);
    game.inorbit = true;
    game.ididit = true;
}

void sensor(void)
/* examine planets in this quadrant */
{
    skip(1);
    chew();
    if (damaged(DSRSENS)) {
	prout(_("Short range sensors damaged."));
	return;
    }
    if (!game.plnet.x && (game.options & OPTION_TTY)) {
	prout(_("Spock- \"No planet in this quadrant, Captain.\""));
	return;
    }
    if ((game.plnet.x != 0)&& (game.state.plnets[game.iplnet].known == unknown)) {
	prout(_("Spock-  \"Sensor scan for %s-"), cramlc(quadrant, game.quadrant));
	skip(1);
	prout(_("         Planet at %s is of class %s."),
	      cramlc(sector,game.plnet),
	      classes[game.state.plnets[game.iplnet].pclass]);
	if (game.state.plnets[game.iplnet].known==shuttle_down) 
	    prout(_("         Sensors show Galileo still on surface."));
	proutn(_("         Readings indicate"));
	if (game.state.plnets[game.iplnet].crystals != present)
	    proutn(_(" no"));
	prout(_(" dilithium crystals present.\""));
	if (game.state.plnets[game.iplnet].known == unknown)
	    game.state.plnets[game.iplnet].known = known;
    }
}

void beam(void) 
/* use the transporter */
{
    chew();
    skip(1);
    if (damaged(DTRANSP)) {
	prout(_("Transporter damaged."));
	if (!damaged(DSHUTTL) && (game.state.plnets[game.iplnet].known==shuttle_down || game.iscraft == onship)) {
	    skip(1);
	    proutn(_("Spock-  \"May I suggest the shuttle craft, Sir?\" "));
	    if (ja() == true)
		shuttle();
	}
	return;
    }
    if (!game.inorbit) {
	crmshp();
	prout(_(" not in standard orbit."));
	return;
    }
    if (game.shldup) {
	prout(_("Impossible to transport through shields."));
	return;
    }
    if (game.state.plnets[game.iplnet].known==unknown) {
	prout(_("Spock-  \"Captain, we have no information on this planet"));
	prout(_("  and Starfleet Regulations clearly state that in this situation"));
	prout(_("  you may not go down.\""));
	return;
    }
    if (game.landed) {
	/* Coming from planet */
	if (game.state.plnets[game.iplnet].known==shuttle_down) {
	    proutn(_("Spock-  \"Wouldn't you rather take the Galileo?\" "));
	    if (ja() == true) {
		chew();
		return;
	    }
	    prout(_("Your crew hides the Galileo to prevent capture by aliens."));
	}
	prout(_("Landing party assembled, ready to beam up."));
	skip(1);
	prout(_("Kirk whips out communicator..."));
	prouts(_("BEEP  BEEP  BEEP"));
	skip(2);
	prout(_("\"Kirk to enterprise-  Lock on coordinates...energize.\""));
    }
    else {
	/* Going to planet */
	if (game.state.plnets[game.iplnet].crystals==absent) {
	    prout(_("Spock-  \"Captain, I fail to see the logic in"));
	    prout(_("  exploring a planet with no dilithium crystals."));
	    proutn(_("  Are you sure this is wise?\" "));
	    if (ja() == false) {
		chew();
		return;
	    }
	}
	prout(_("Scotty-  \"Transporter room ready, Sir.\""));
	skip(1);
	prout(_("Kirk and landing party prepare to beam down to planet surface."));
	skip(1);
	prout(_("Kirk-  \"Energize.\""));
    }
    game.ididit = true;
    skip(1);
    prouts("WWHOOOIIIIIRRRRREEEE.E.E.  .  .  .  .   .    .");
    skip(2);
    if (Rand() > 0.98) {
	prouts("BOOOIIIOOOIIOOOOIIIOIING . . .");
	skip(2);
	prout(_("Scotty-  \"Oh my God!  I've lost them.\""));
	finish(FLOST);
	return;
    }
    prouts(".    .   .  .  .  .  .E.E.EEEERRRRRIIIIIOOOHWW");
    skip(2);
    prout(_("Transport complete."));
    game.landed = !game.landed;
    if (game.landed && game.state.plnets[game.iplnet].known==shuttle_down) {
	prout(_("The shuttle craft Galileo is here!"));
    }
    if (!game.landed && game.imine) {
	game.icrystl = true;
	game.cryprob = 0.05;
    }
    game.imine = false;
    return;
}

void mine(void) 
/* strip-mine a world for dilithium */
{
    skip(1);
    chew();
    if (!game.landed) {
	prout(_("Mining party not on planet."));
	return;
    }
    if (game.state.plnets[game.iplnet].crystals == mined) {
	prout(_("This planet has already been strip-mined for dilithium."));
	return;
    }
    else if (game.state.plnets[game.iplnet].crystals == absent) {
	prout(_("No dilithium crystals on this planet."));
	return;
    }
    if (game.imine) {
	prout(_("You've already mined enough crystals for this trip."));
	return;
    }
    if (game.icrystl && game.cryprob == 0.05) {
	proutn(_("With all those fresh crystals aboard the "));
	crmshp();
	skip(1);
	prout(_("there's no reason to mine more at this time."));
	return;
    }
    game.optime = (0.1+0.2*Rand())*game.state.plnets[game.iplnet].pclass;
    if (consumeTime())
	return;
    prout(_("Mining operation complete."));
    game.state.plnets[game.iplnet].crystals = mined;
    game.imine = game.ididit = true;
}

void usecrystals(void)
/* use dilithium crystals */
{
    game.ididit = false;
    skip(1);
    chew();
    if (!game.icrystl) {
	prout(_("No dilithium crystals available."));
	return;
    }
    if (game.energy >= 1000) {
	prout(_("Spock-  \"Captain, Starfleet Regulations prohibit such an operation"));
	prout(_("  except when Condition Yellow exists."));
	return;
    }
    prout(_("Spock- \"Captain, I must warn you that loading"));
    prout(_("  raw dilithium crystals into the ship's power"));
    prout(_("  system may risk a severe explosion."));
    proutn(_("  Are you sure this is wise?\" "));
    if (ja() == false) {
	chew();
	return;
    }
    skip(1);
    prout(_("Engineering Officer Scott-  \"(GULP) Aye Sir."));
    prout(_("  Mr. Spock and I will try it.\""));
    skip(1);
    prout(_("Spock-  \"Crystals in place, Sir."));
    prout(_("  Ready to activate circuit.\""));
    skip(1);
    prouts(_("Scotty-  \"Keep your fingers crossed, Sir!\""));
    skip(1);
    if (Rand() <= game.cryprob) {
	prouts(_("  \"Activating now! - - No good!  It's***"));
	skip(2);
	prouts(_("***RED ALERT!  RED A*L********************************"));
	skip(1);
	stars();
	prouts(_("******************   KA-BOOM!!!!   *******************"));
	skip(1);
	kaboom();
	return;
    }
    game.energy += 5000.0*(1.0 + 0.9*Rand());
    prouts(_("  \"Activating now! - - "));
    prout(_("The instruments"));
    prout(_("   are going crazy, but I think it's"));
    prout(_("   going to work!!  Congratulations, Sir!\""));
    game.cryprob *= 2.0;
    game.ididit = true;
}

void shuttle(void) 
/* use shuttlecraft for planetary jaunt */
{
    chew();
    skip(1);
    if(damaged(DSHUTTL)) {
	if (game.damage[DSHUTTL] == -1.0) {
	    if (game.inorbit && game.state.plnets[game.iplnet].known == shuttle_down)
		prout(_("Ye Faerie Queene has no shuttle craft bay to dock it at."));
	    else
		prout(_("Ye Faerie Queene had no shuttle craft."));
	}
	else if (game.damage[DSHUTTL] > 0)
	    prout(_("The Galileo is damaged."));
	else /* game.damage[DSHUTTL] < 0 */ 
	    prout(_("Shuttle craft is now serving Big Macs."));
	return;
    }
    if (!game.inorbit) {
	crmshp();
	prout(_(" not in standard orbit."));
	return;
    }
    if ((game.state.plnets[game.iplnet].known != shuttle_down) && game.iscraft != onship) {
	prout(_("Shuttle craft not currently available."));
	return;
    }
    if (!game.landed && game.state.plnets[game.iplnet].known==shuttle_down) {
	prout(_("You will have to beam down to retrieve the shuttle craft."));
	return;
    }
    if (game.shldup || game.condition == docked) {
	prout(_("Shuttle craft cannot pass through shields."));
	return;
    }
    if (game.state.plnets[game.iplnet].known==unknown) {
	prout(_("Spock-  \"Captain, we have no information on this planet"));
	prout(_("  and Starfleet Regulations clearly state that in this situation"));
	prout(_("  you may not fly down.\""));
	return;
    }
    game.optime = 3.0e-5*game.height;
    if (game.optime >= 0.8*game.state.remtime) {
	prout(_("First Officer Spock-  \"Captain, I compute that such"));
	proutn(_("  a maneuver would require approximately %2d%% of our"),
	       (int)(100*game.optime/game.state.remtime));
	prout(_("remaining time."));
	proutn(_("Are you sure this is wise?\" "));
	if (ja() == false) {
	    game.optime = 0.0;
	    return;
	}
    }
    if (game.landed) {
	/* Kirk on planet */
	if (game.iscraft == onship) {
	    /* Galileo on ship! */
	    if (!damaged(DTRANSP)) {
		proutn(_("Spock-  \"Would you rather use the transporter?\" "));
		if (ja() == true) {
		    beam();
		    return;
		}
		proutn(_("Shuttle crew"));
	    }
	    else
		proutn(_("Rescue party"));
	    prout(_(" boards Galileo and swoops toward planet surface."));
	    game.iscraft = offship;
	    skip(1);
	    if (consumeTime())
		return;
	    game.state.plnets[game.iplnet].known=shuttle_down;
	    prout(_("Trip complete."));
	    return;
	}
	else {
	    /* Ready to go back to ship */
	    prout(_("You and your mining party board the"));
	    prout(_("shuttle craft for the trip back to the Enterprise."));
	    skip(1);
	    prout(_("The short hop begins . . ."));
	    game.state.plnets[game.iplnet].known=known;
	    game.icraft = true;
	    skip(1);
	    game.landed = false;
	    if (consumeTime())
		return;
	    game.iscraft = onship;
	    game.icraft = false;
	    if (game.imine) {
		game.icrystl = true;
		game.cryprob = 0.05;
	    }
	    game.imine = false;
	    prout(_("Trip complete."));
	    return;
	}
    }
    else {
	/* Kirk on ship */
	/* and so is Galileo */
	prout(_("Mining party assembles in the hangar deck,"));
	prout(_("ready to board the shuttle craft \"Galileo\"."));
	skip(1);
	prouts(_("The hangar doors open; the trip begins."));
	skip(1);
	game.icraft = true;
	game.iscraft = offship;
	if (consumeTime())
	    return;
	game.state.plnets[game.iplnet].known = shuttle_down;
	game.landed = true;
	game.icraft = false;
	prout(_("Trip complete."));
	return;
    }
}

void deathray(void)
/* use the big zapper */
{
    double dprob, r = Rand();
	
    game.ididit = false;
    skip(1);
    chew();
    if (game.ship != IHE) {
	prout(_("Ye Faerie Queene has no death ray."));
	return;
    }
    if (game.nenhere==0) {
	prout(_("Sulu-  \"But Sir, there are no enemies in this quadrant.\""));
	return;
    }
    if (damaged(DDRAY)) {
	prout(_("Death Ray is damaged."));
	return;
    }
    prout(_("Spock-  \"Captain, the 'Experimental Death Ray'"));
    prout(_("  is highly unpredictible.  Considering the alternatives,"));
    proutn(_("  are you sure this is wise?\" "));
    if (ja() == false)
	return;
    prout(_("Spock-  \"Acknowledged.\""));
    skip(1);
    game.ididit = true;
    prouts(_("WHOOEE ... WHOOEE ... WHOOEE ... WHOOEE"));
    skip(1);
    prout(_("Crew scrambles in emergency preparation."));
    prout(_("Spock and Scotty ready the death ray and"));
    prout(_("prepare to channel all ship's power to the device."));
    skip(1);
    prout(_("Spock-  \"Preparations complete, sir.\""));
    prout(_("Kirk-  \"Engage!\""));
    skip(1);
    prouts(_("WHIRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"));
    skip(1);
    dprob = .30;
    if (game.options & OPTION_PLAIN)
	dprob = .5;
    if (r > dprob) {
	prouts(_("Sulu- \"Captain!  It's working!\""));
	skip(2);
	while (game.nenhere > 0)
	    deadkl(game.ks[1], game.quad[game.ks[1].x][game.ks[1].y],game.ks[1]);
	prout(_("Ensign Chekov-  \"Congratulations, Captain!\""));
	if (KLINGREM == 0)
	    finish(FWON);    
	if ((game.options & OPTION_PLAIN) == 0) {
	    prout(_("Spock-  \"Captain, I believe the `Experimental Death Ray'"));
	    if (Rand() <= 0.05) {
		prout(_("   is still operational.\""));
	    }
	    else {
		prout(_("   has been rendered nonfunctional.\""));
		game.damage[DDRAY] = 39.95;
	    }
	}
	return;
    }
    r = Rand();	// Pick failure method 
    if (r <= .30) {
	prouts(_("Sulu- \"Captain!  It's working!\""));
	skip(1);
	prouts(_("***RED ALERT!  RED ALERT!"));
	skip(1);
	prout(_("***MATTER-ANTIMATTER IMPLOSION IMMINENT!"));
	skip(1);
	prouts(_("***RED ALERT!  RED A*L********************************"));
	skip(1);
	stars();
	prouts(_("******************   KA-BOOM!!!!   *******************"));
	skip(1);
	kaboom();
	return;
    }
    if (r <= .55) {
	prouts(_("Sulu- \"Captain!  Yagabandaghangrapl, brachriigringlanbla!\""));
	skip(1);
	prout(_("Lt. Uhura-  \"Graaeek!  Graaeek!\""));
	skip(1);
	prout(_("Spock-  \"Fascinating!  . . . All humans aboard"));
	prout(_("  have apparently been transformed into strange mutations."));
	prout(_("  Vulcans do not seem to be affected."));
	skip(1);
	prout(_("Kirk-  \"Raauch!  Raauch!\""));
	finish(FDRAY);
	return;
    }
    if (r <= 0.75) {
	int i,j;
	prouts(_("Sulu- \"Captain!  It's   --WHAT?!?!\""));
	skip(2);
	proutn(_("Spock-  \"I believe the word is"));
	prouts(_(" *ASTONISHING*"));
	prout(_(" Mr. Sulu."));
	for_sectors(i)
	    for_sectors(j)
		if (game.quad[i][j] == IHDOT)
		    game.quad[i][j] = IHQUEST;
	prout(_("  Captain, our quadrant is now infested with"));
	prouts(_(" - - - - - -  *THINGS*."));
	skip(1);
	prout(_("  I have no logical explanation.\""));
	return;
    }
    prouts(_("Sulu- \"Captain!  The Death Ray is creating tribbles!\""));
    skip(1);
    prout(_("Scotty-  \"There are so many tribbles down here"));
    prout(_("  in Engineering, we can't move for 'em, Captain.\""));
    finish(FTRIBBLE);
    return;
}
