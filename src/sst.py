"""
sst.py =-- Super Star Trek in Python
"""
import math

PHASEFAC	= 2.0
GALSIZE 	= 8
NINHAB  	= GALSIZE * GALSIZE / 2
MAXUNINHAB	= 10
PLNETMAB	= NINHAB + MAXUNINHAB
QUADSIZE	= 10
BASEMAX 	= 5
FULLCREW	= 428	 # BSD Trek was 387, that's wrong
MAXKLGAME	= 127
MAXKLQUAD	= 9
FOREVER 	= 1e30

# These types have not been dealt with yet 
IHQUEST = '?',
IHWEB = '#',
IHMATER0 = '-',
IHMATER1 = 'o',
IHMATER2 = '0',

class coord:
    def __init(self, x=None, y=None):
        self.x = x
        self.y = y
    def invalidate(self):
        self.x = self.y = None
    def is_valid(self):
        return self.x != None and self.y != None
    def __eq__(self, other):
        return self.x == other.y and self.x == other.y
    def __add__(self, other):
        return coord(self.x+self.x, self.y+self.y)
    def __sub__(self, other):
        return coord(self.x-self.x, self.y-self.y)
    def distance(self, other):
        return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2)
    def sgn(self):
        return coord(self.x / abs(x), self.y / abs(y));
    def __hash__(self):
        return hash((x, y))
    def __str__(self):
        return "%d - %d" % (self.x, self.y)

class feature:
    "A feature in the current quadrant (ship, star, black hole, etc)." 
    def __init__(self):
        self.type = None	# name of feature type
        self.location = None	# location
    def distance(self):
        return self.location.distance(game.sector)
    def __str__(self):
        return self.name[0]

empty = None	# Value of empty space in game.quad

class ship(feature):
    "An enemy ship in the current quadrant." 
    def __init__(self):
        feature.__init__(self)
        self.type = None	# klingon, romulan, commander,
        			# supercommander, tholian
        self.power = None	# power
        if self.type in ("Klingon", "Commander", "Super-Commander"):
            game.remkl += 1
        elif self.type == "Romulan":
            game.romrem += 1
    def __del__(self):
        if self.type in ("Klingon", "Commander", "Super-Commander"):
            game.remkl -= 1
        elif self.type == "Romulan":
            game.romrem -= 1
    def sectormove(self, dest):
        "Move this ship within the current quadrant." 
        if self.location:
            game.quad[self.location] = None
        game.quad[dest] = self
        self.location = dest

class planet(feature):
    "A planet.  May be inhabited or not, may hold dilithium crystals or not."
    def __init(self):
        feature.__init__(self)
        self.name = None
        self.crystals = None	# "absent", "present", or "mined"
        self.inhabited = False
        self.known = "unknown"	# Other values: "known" and "shuttle down"
    def __str__(self):
        if self.inhabited:
            return '@'
        else:
            return 'P'

class star(feature):
    "A star.  Has no state, just knows how to identify iself."
    def __init(self):
        feature.__init__(self)
    def __str__(self):
        return '*'

class web(feature):
    "A bit of Tholian web.  Has no state, just knows how to identify iself."
    def __init(self):
        feature.__init__(self)
    def __str__(self):
        return '*'

class blackhole(feature):
    "A black hole.  Has no hair, just knows how to identify iself."
    def __init(self):
        feature.__init__(self)
    def __str__(self):
        return '*'

class starbase(feature):
    "Starbases also have no features."
    def __init(self):
        feature.__init__(self)
    def __del__(self):
        game.state.bases.remove(self.location)
    def __str__(self):
        return 'B'

class quadrant:
    def __init__(self):
        self.stars = None
        self.planet = None
        self.starbase = None
        self.klingons = None
        self.romulans = None
        self.supernova = None
        self.charted = None
        self.status = "secure"	# Other valuues: "distressed", "enslaved"
    def enemies(self):
        "List enemies in this quadrant."
        lst = []
        for feature in self.quad.values:
            if not isinstance(feature, ship):
                continue
            if feature.name not in ("Enterprise", "Faerie Queene"):
                lst.append(feature)
        return lst

class page:
    "A chart page.  The starchart is a 2D array of these."
    def __init__(self):
	self.stars = None	# Will hold a number
	self.starbase = None	# Will hold a bool
	self.klingons = None	# Will hold a number

class snapshot:
    "State of the universe.  The galaxy is a 2D array of these."
    def __init__(self):
        self.crew = None	# crew complement
	self.remkl = None	# remaining klingons
	self.remcom = None	# remaining commanders
	self.nscrem = None	# remaining super commanders
	self.rembase = None	# remaining bases
	self.starkl = None	# destroyed stars
	self.basekl = None	# destroyed bases
	self.nromrem = None	# Romulans remaining
	self.nplankl = None	# destroyed uninhabited planets
	self.nworldkl = None	# destroyed inhabited planets
        self.plnets = [];	# List of planets known
        self.date = None	# stardate
	self.remres = None	# remaining resources
	self. remtime = None	# remaining time
        self.bases = [] 	# Base quadrant coordinates
        self.kcmdr = [] 	# Commander quadrant coordinates
        self.kscmdr = None	# Supercommander quadrant coordinates
        self.galaxy = {}	# Dictionary of quadrant objects
        self.chart = {}		# Dictionary of page objects

def damaged(dev):
    return game.damage[dev] != 0.0

class event:
    def __init__(self):
        self.date = None	# The only mandatory attribute

class game:
    def __init__(self):
        self.options = []		# List of option strings
        self.state = snapshot()		# State of the universe
        self.snapsht = snapshot()	# For backwards timetravel
        self.quad = {}			# contents of our quadrant
        self.kpower = {}		# enemy energy levels
        self.kdist = {}			# enemy distances
        self.kavgd = {}			# average distances
        self.damage = {}		# damage encountered
        self.future = []		# future events
        self.passwd = None		# Self Destruct password
        # Coordinate members start here
	self.enemies = {}			# enemy sector locations
	self.quadrant = None		# where we are
        self.sector = None
	self.tholian = None		# coordinates of Tholian
	self.base = None		# position of base in current quadrant
	self.battle = None		# base coordinates being attacked
	self.plnet = None		# location of planet in quadrant
	self.probec = None		# current probe quadrant
        # Flag members start here
	self.gamewon = None		# Finished!
	self.ididit = None		# action taken -- allows enemy to attack
	self.alive = None		# we are alive (not killed)
	self.justin = None		# just entered quadrant
	self.shldup = None		# shields are up
	self.shldchg = None		# shield changing (affects efficiency)
	self.comhere = None		# commander here
	self.ishere = None		# super-commander in quadrant
	self.iscate = None		# super commander is here
	self.ientesc = None		# attempted escape from supercommander
	self.ithere = None		# Tholian is here 
	self.resting = None		# rest time
	self.icraft = None		# Kirk in Galileo
	self.landed = None		# party on planet or on ship
	self.alldone = None		# game is now finished
	self.neutz = None		# Romulan Neutral Zone
	self.isarmed = None		# probe is armed
	self.inorbit = None		# orbiting a planet
	self.imine = None		# mining
	self.icrystl = None		# dilithium crystals aboard
	self.iseenit = None		# seen base attack report
	self.thawed = None		# thawed game
        # String members start here
        self.condition = None		# green, yellow, red, docked, dead,
        self.iscraft = None		# onship, offship, removed
        self.skill = None		# levels: none, novice, fair, good,
        				# expert, emeritus
        # Integer nembers sart here
	self.inkling = None		# initial number of klingons
	self.inbase = None		# initial number of bases
	self.incom = None		# initial number of commanders
	self.inscom = None		# initial number of commanders
	self.inrom = None		# initial number of commanders
	self.instar = None		# initial stars
	self.intorps = None		# initial/max torpedoes
	self.torps = None		# number of torpedoes
	self.ship = None		# ship type -- 'E' is Enterprise
	self.abandoned = None		# count of crew abandoned in space
	self.length = None		# length of game
	self.klhere = None		# klingons here
	self.casual = None		# causalties
	self.nhelp = None		# calls for help
	self.nkinks = None		# count of energy-barrier crossings
	self.iplnet = None		# planet # in quadrant
	self.inplan = None		# initial planets
	self.irhere = None		# Romulans in quadrant
	self.isatb = None		# =1 if super commander is attacking base
	self.tourn = None		# tournament number
	self.proben = None		# number of moves for probe
	self.nprobes = None		# number of probes available
        # Float members start here
	self.inresor = None		# initial resources
	self.intime = None		# initial time
	self.inenrg = None		# initial/max energy
	self.inshld = None		# initial/max shield
	self.inlsr = None		# initial life support resources
	self.indate = None		# initial date
	self.energy = None		# energy level
	self.shield = None		# shield level
	self.warpfac = None		# warp speed
	self.wfacsq = None		# squared warp factor
	self.lsupres = None		# life support reserves
	self.dist = None		# movement distance
	self.direc = None		# movement direction
	self.optime = None		# time taken by current operation
	self.docfac = None		# repair factor when docking (constant?)
	self.damfac = None		# damage factor
	self.lastchart = None		# time star chart was last updated
	self.cryprob = None		# probability that crystal will work
	self.probex = None		# location of probe
	self.probey = None		#
	self.probeinx = None		# probe x,y increment
	self.probeiny = None		#
	self.height = None		# height of orbit around planet

def communicating():
    "Are we in communication with Starfleet Command?"
    return (not damaged("DRADIO")) or game.condition == docked

# Code corresponding to ai.c begins here

def tryexit(look, ship, irun):
    # a bad guy attempts to bug out of the quadrant
    iq = coord()
    iq.x = game.quadrant.x+(look.x+(QUADSIZE-1))/QUADSIZE - 1
    iq.y = game.quadrant.y+(look.y+(QUADSIZE-1))/QUADSIZE - 1
    if not valid_quadrant(iq) or \
	game.state.galaxy[iq].supernova or \
        game.state.galaxy[iq].klingons > 8:
	return False;	# no can do -- neg energy, supernovae, or >8 Klingons
    if ship.type == "Romulan":
        return False	# Romulans cannot escape
    if not irun:
	# avoid intruding on another commander's territory
        if ship.type == "Commander":
            if iq in gamestate.kcmdr:
                return False
	    # refuse to leave if currently attacking starbase:
            if game.battle == game.quadrant:
                return False;
	# don't leave if over 1000 units of energy
        if ship.power > 1000.0:
            return false;
    # Print escape message and move out of quadrant.
    # We know this if either short or long range sensors are working
    if not damaged("DSRSENS") or not damaged("DLRSENS") or game.condition=="docked":
	crmena(True, "sector", ship)
	prout(" escapes to quadrant %s (and regains strength)." % iq)
    # handle local matters related to escape
    game.quad[ship.location] = None;
    if game.condition != "docked":
        newcnd()
    # Handle global matters related to escape
    game.state.galaxy[game.quadrant].klingons -= 1
    game.state.galaxy[iq].klingons += 1
    if ship.type == "Super-Commander":
	game.ishere = False
	game.iscate = False
	game.ientesc = False
	game.isatb = 0
	schedule("FSCMOVE", 0.2777)
	unschedule("FSCDBAS")
	game.state.kscmdr = iq
    else:
        for (n, cmdr) in enumerate(game.state.kcmdr):
            if cmdr == game.quadrant:
		game.state.kcmdr[n] = iq
		break
	game.comhere = False
    return True		# successful exit

def sgn(n): n / abs(n)

'''
Algorithm for moving bad guys:

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

 If docked, is reduced by roughly 1.75*game.skill, generally forcing a
 retreat, especially at high skill levels.

 *  Motion is limited to skill level, except for SC hi-tailing it out.
'''

def movebaddy(ship):
    # tactical movement for the bad guys
    bugout = False
    # This should probably be just game.comhere + game.ishere
    if game.skill >= SKILL_EXPERT:
        nbaddys = int((game.comhere*2 + game.ishere*2+game.klhere*1.23+game.irhere*1.5)/2.0)
    else:
	nbaddys = game.comhere + game.ishere
    dist1 = ship.distance()
    mdist = round(dist1 + 0.5)		# Nearest integer distance
    # If SC, check with spy to see if should high-tail it
    if ship.type == "Super-Commander" and \
           (ship.power <= 500.0 or (game.condition==docked and not damaged("DPHOTON"))):
	bugout = True;
	motion = -QUADSIZE;
    else:
	# decide whether to advance, retreat, or hold position
	forces = ship.power + 100.0*len(game.quad.enemies()) + 400*(nbaddys-1)
        if not game.shldup:
            forces += 1000.0 		# Good for enemy if shield is down!
        if not damaged("DPHASER") or not damaged("DPHOTON"):
            if damaged(DPHASER):
		forces += 300.0
	    else:
		forces -= 0.2*(game.energy - 2500.0);
	    if damaged("DPHOTON"):
		forces += 300.0
	    else:
		forces -= 50.0*game.torps
	else:
	    # phasers and photon tubes both out!
	    forces += 1000.0
	motion = 0;
        if forces <= 1000.0 and game.condition != "docked":	# Typical case
	    motion = ((forces+200.0*Rand())/150.0) - 5.0
	else:
            if forces > 1000.0:	# Very strong -- move in for kill
		motion = (1.0-square(Rand()))*dist1 + 1.0
	    if game.condition == "docked" and "base" in game.options:
                 # protected by base -- back off !
		motion -= game.skill * (2.0-Rand()**2)
	if idebug:
	    proutn("=== MOTION = %1.2f, FORCES = %1.2f, ", motion, forces)
	# don't move if no motion
        if motion == 0:
            return
	# Limit motion according to skill
        if abs(motion) > game.skill:
            if motion < 0:
                motion = -game.kill
            else:
                motion = game.skill
    # calculate preferred number of steps
    nsteps = abs(motion)
    if motion > 0 and nsteps > mdist: # don't overshoot
        nsteps = mdist
    if nsteps > QUADSIZE: # This shouldn't be necessary
        nsteps = QUADSIZE
    if nsteps < 1:  # This shouldn't be necessary
        nsteps = 1
    if idebug:
	proutn("NSTEPS = %d:", nsteps)
    # Compute preferred values of delta X and Y
    me = game.sector - com;
    if 2.0 * abs(me.x) < abs(me.y):
        me.x = 0;
    if 2.0 * abs(me.y) < abs(game.sector.x-com.x):
        me.y = 0;
    if me.x != 0: me.x = sgn(me.x*motion)
    if me.y != 0: me.y = sgn(me.y*motion)
    next = com;
    # main move loop
    for ll in range(nsteps):
        if idebug:
	    proutn(" %d", ll+1)
	# Check if preferred position available
	look = next + me
	krawl = me.sgn()
	success = False
	attempts = 0 # Settle meysterious hang problem
        while attempts < 20 and not success:
            attempts += 1
            if look.x < 1 or look.x > QUADSIZE:
                if motion < 0 and tryexit(look, ship, bugout):
		    return
                if krawl.x == me.x or me.y == 0:
                    break
		look.x = next.x + krawl.x
		krawl.x = -krawl.x
	    elif look.y < 1 or look.y > QUADSIZE:
                if motion < 0 and tryexit(look, ship, bugout):
                    return
                if krawl.y == me.y or me.x == 0:
                    break
                look.y = next.y + krawl.y
                krawl.y = -krawl.y
	    elif "ramming" in game.options and game.quad[look] != IHDOT:
		# See if we should ram ship
                if game.quad[look] == game.ship and ienm in (IHC, IHS):
		    ram(true, ienm, com)
		    return
                if krawl.x != me.x and me.y != 0:
		    look.x = next.x + krawlx
		    krawl.x = -krawl.x
		elif krawly != me.y and me.x != 0:
		    look.y = next.y + krawly
		    krawl.y = -krawl.y
		else:
                    break # we have failed
	    else:
                success = True
        if success:
	    next = look
            if idebug:
		proutn(str(next))
	else:
            break # done early
    if idebug:
	prout("")
    # Put ship in place within same quadrant
    if next != ship.location:
	# it moved
        if not damaged("DSRSENS") or game.condition == "docked":
	    proutn("*** %s from sector %s" % (ship, ship.location))
            if ship.distance() < dist1:
                prout(" advances to sector %s" % ship.location)
            else:
                prout(" retreats to sector %s" % ship.location)
        ship.sectormove(next)

def movcom(): 
    "Allow enemies to move."
    for enemy in self.quad.enemies():
        if enemy.type == "Commander":
            movebaddy(enemy)
            break
    for enemy in self.quad.enemies():
        if enemy.type == "Super-Commander":
            movebaddy(enemy)
            break
    # Ff skill level is high, move other Klingons and Romulans too!
    # Move these last so they can base their actions on what the
    # commander(s) do.
    if game.skill >= SKILL_EXPERT and "movebaddy" in game.options: 
        for enemy in self.quad.enemies():
            if enemy.type in ("Klingon", "Romulan"):
                movebaddy(enemy)
                break

def movescom(ship, avoid):
    # commander movement helper
    global ipage
    if game.state.kscmdr == game.quadrant or \
	game.state.galaxy[iq].supernova or \
        game.state.galaxy[iq].klingons > 8: 
	return True
    if avoid:
	# Avoid quadrants with bases if we want to avoid Enterprise
        for base in game.state.starbases:
            if base.location == ship.location:
		return True
    if game.justin and not game.iscate:
        return True
    # Super-Commander has scooted, Remove him from current quadrant.
    if game.state.kscmdr == game.quadrant:
	game.iscate = False
	game.isatb = 0
	game.ientesc = False
	unschedule("FSCDBAS")
        if game.condition != "docked":
            newcnd()
        ship.sectormove(None)
    # do the actual move
    game.state.galaxy[game.state.kscmdr].klingons -= 1
    game.state.kscmdr = iq
    game.state.galaxy[game.state.kscmdr].klingons += 1
    # check for a helpful planet in the destination quadrant
    for planet in game.state.plnets:
	if planet.location == game.state.kscmdr and planet.crystals=="present":
	    # destroy the planet
	    game.state.plnets.remove(planet)
            if communicating():
                if not ipage:
                    pause_game(True)
		ipage = true
		prout("Lt. Uhura-  \"Captain, Starfleet Intelligence reports")
		proutn(_("   a planet in "))
		proutn(cramlc(quadrant, game.state.kscmdr))
		prout(" has been destroyed")
		prout("   by the Super-commander.\"")
	    break
    return False # looks good!
			
def scom():
    # move the Super Commander
    if (idebug):
        prout("== SCOM")

    # Decide on being active or passive
    passive = ((NKILLC+NKILLK)/(game.state.date+0.01-game.indate) < 0.1*game.skill*(game.skill+1.0) \
               or (game.state.date-game.indate) < 3.0)
    if not game.iscate and passive:
	# compute move away from Enterprise
	idelta = game.state.kscmdr - game.quadrant
        if distance(game.state.kscmdr) > 2.0:
	    # circulate in space
	    idelta,x = game.state.kscmdr.y-game.quadrant.y
	    idelta,y = game.quadrant.x-game.state.kscmdr.x
    else:
        if len(game.state.bases):
            unschedule("FSCMOVE")
	    return
	sc = game.state.kscmdr
	# compute distances to starbases
        game.starbases.sort(lambda x, y: cmp(distance(x, game.quadrant), distance(y, game.quadrant)))
	# look for nearest base without a commander, no Enterprise, and
	# without too many Klingons, and not already under attack.
        nearest = filter(game.starbases,
                         lambda x: game.state.galaxy[x].supernova \
                         and game.state.galaxy[x].klingons <= 8)
        if game.quadrant in nearest:
            nearest.remove(game.quadrant)
        if game.battle in nearest:
            nearest.remove(game.battle)
        # if there is a commander, and no other base is appropriate,
        # we will take the one with the commander
        nocmd = filter(lambda x: x.location not in game.state.kcmdr, nearest)
        if len(nocmd):
            nearest = nocmd
        ibq = nearest[0]
        if len(nearest) == 0:
            return	# Nothing suitable -- wait until next time
	# decide how to move toward base
	idelta = ibq - game.state.kscmdr
    # maximum movement is 1 quadrant in either or both axis
    delta = delta.sgn()
    # try moving in both x and y directions
    iq = game.state.kscmdr + idelta
    if movescom(iq, passive):
	# failed -- try some other maneuvers
        if ideltax==0 or ideltay==0:
	    # attempt angle move
            if ideltax != 0:
		iq.y = game.state.kscmdr.y + 1
                if movescom(iq, passive):
		    iq.y = game.state.kscmdr.y - 1
		    movescom(iq, passive)
	    else:
		iq.x = game.state.kscmdr.x + 1
                if movescom(iq, passive):
		    iq.x = game.state.kscmdr.x - 1
		    movescom(iq, passive)
	else:
	    # try moving just in x or y
	    iq.y = game.state.kscmdr.y
            if movescom(iq, passive):
		iq.y = game.state.kscmdr.y + ideltay
		iq.x = game.state.kscmdr.x
		movescom(iq, passive)
    # check for a base
    if game.state.rembase == 0:
	unschedule("FSCMOVE")
    else:
        for ibq in game.bases:
            if ibq == game.state.kscmdr and game.state.kscmdr == game.battle:
                # attack the base
                if passive:
                    return # no, don't attack base!
                game.iseenit = false
                game.isatb = 1
                schedule("FSCDBAS", 1.0 +2.0*Rand())
                if is_scheduled("FCDBAS"):
                    postpone("FSCDBAS", scheduled("FCDBAS")-game.state.date)
                if not communicating():
                    return # no warning
                game.iseenit = True
                if not ipage:
                    pause_game(true)
                ipage = True
                proutn(_("Lt. Uhura-  \"Captain, the starbase in "))
                proutn(cramlc(quadrant, game.state.kscmdr))
                skip(1)
                prout("   reports that it is under attack from the Klingon Super-commander.")
                proutn("   It can survive until stardate %d.\"",
                       int(scheduled(FSCDBAS)))
                if not game.resting:
                    return
                prout("Mr. Spock-  \"Captain, shall we cancel the rest period?\"")
                if ja() == false:
                    return
                game.resting = False
                game.optime = 0.0 # actually finished
                return
    # Check for intelligence report
    if (Rand() > 0.2 or not communicating() or
        not game.state.galaxy[game.state.kscmdr].charted):
	return
    if ipage:
        pause_game(true)
        ipage = true
    prout(_("Lt. Uhura-  \"Captain, Starfleet Intelligence reports"))
    proutn(_("   the Super-commander is in "))
    proutn(cramlc(quadrant, game.state.kscmdr))
    prout(".\"")
    return

def movetho(void):
    "Move the Tholian (an instance of ship type pointed at by game.tholian)." 
    if not game.tholian or game.justin:
        return
    next = coord()
    if game.tholian.location.x == 1 and game.tholian.location.y == 1:
	next.x = 1
        next.y = QUADSIZE
    elif game.tholian.location.x == 1 and game.tholian.location.y == QUADSIZE:
	next.x = next.y = QUADSIZE
    elif game.tholian.location.x == QUADSIZE and game.tholian.location.y == QUADSIZE:
	next.x = QUADSIZE
        next.y = 1
    elif game.tholian.location.x == QUADSIZE and game.tholian.location.y == 1:
	next.x = next.y = 1
    else:
	# something is wrong!
	game.tholian = None
	return
    # Do nothing if we are blocked
    if game.quad[next] != empty and not isinstance(game.quad[next]. web):
        return
    # Now place some web
    im = (next - game.tholian.location).sgn()
    if game.tholian.x != next.x:
	# move in x axis
	while game.tholian.location.x != next.x:
	    game.tholian.location.x += im.x
            if game.quad[game.tholian.location] == empty:
                game.quad[game.tholian.location] = web()
    elif game.tholian.y != next.y:
	# move in y axis
	while game.tholian.y != next.y:
	    game.tholian.y += im.y
            if game.quad[game.tholian.location] == empty:
                game.quad[game.tholian.location] = web()
    # web is done, move ship
    game.tholian.movesector(next)
    # check to see if all holes plugged
    for i in range(1, QUADSIZE+1):
	if (not isinstance(game.quad[(1,i)],web)) and game.quad[(1,i)]!=game.tholian:
            return
        if (not isinstance(game.quad[(QUADSIZE,i)],web)) and game.quad[(QUADSIZE,i)]!=game.tholian:
            return
        if (not isinstance(game.quad[(i,1)],web)) and game.quad[(i,1)]!=game.tholian:
            return
        if (not isinstance(game.quad[(i.QUADSIZE)],web)) and game.quad[(i,QUADSIZE)]!=game.tholian:
            return
    # All plugged up -- Tholian splits
    game.quad[game.tholian] = web()
    ship.movesector(None)
    crmena(True, IHT, sector, game.tholian)
    prout(" completes web.")
    game.tholian = None
    return

