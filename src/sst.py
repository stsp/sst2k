#!/usr/bin/env python
"""
sst.py -- Super Star Trek 2K

SST2K is a Python translation of a C translation of a FORTRAN
original dating back to 1973.  Beautiful Python it is not, but it
works.  Translation by Eric S. Raymond; original game by David Matuszek
and Paul Reynolds, with modifications by Don Smith, Tom Almy,
Stas Sergeev, and Eric S. Raymond.

See the doc/HACKING file in the distribution for designers notes and advice
ion how to modify (and how not to modify!) this code.
"""
import os, sys, math, curses, time, readline, cPickle, random, copy, gettext, getpass

SSTDOC  	= "/usr/share/doc/sst/sst.doc"
DOC_NAME	= "sst.doc"

def _(str): return gettext.gettext(str)

GALSIZE 	= 8		# Galaxy size in quadrants
NINHAB  	= (GALSIZE * GALSIZE / 2)	# Number of inhabited worlds
MAXUNINHAB	= 10		# Maximum uninhabited worlds
QUADSIZE	= 10		# Quadrant size in sectors
BASEMIN		= 2				# Minimum starbases
BASEMAX 	= (GALSIZE * GALSIZE / 12)	# Maximum starbases
MAXKLGAME	= 127		# Maximum Klingons per game
MAXKLQUAD	= 9		# Maximum Klingons per quadrant
FULLCREW	= 428		# Crew size. BSD Trek was 387, that's wrong 
FOREVER 	= 1e30		# Time for the indefinite future
MAXBURST	= 3		# Max # of torps you can launch in one turn
MINCMDR 	= 10		# Minimum number of Klingon commanders
DOCKFAC		= 0.25		# Repair faster when docked
PHASEFAC	= 2.0		# Unclear what this is, it was in the C version

class TrekError:
    pass

class coord:
    def __init__(self, x=None, y=None):
        self.i = x
        self.j = y
    def valid_quadrant(self):
        return self.i>=0 and self.i<GALSIZE and self.j>=0 and self.j<GALSIZE
    def valid_sector(self):
	return self.i>=0 and self.i<QUADSIZE and self.j>=0 and self.j<QUADSIZE
    def invalidate(self):
        self.i = self.j = None
    def is_valid(self):
        return self.i != None and self.j != None
    def __eq__(self, other):
        return other != None and self.i == other.i and self.j == other.j
    def __ne__(self, other):
        return other == None or self.i != other.i or self.j != other.j
    def __add__(self, other):
        return coord(self.i+other.i, self.j+other.j)
    def __sub__(self, other):
        return coord(self.i-other.i, self.j-other.j)
    def __mul__(self, other):
        return coord(self.i*other, self.j*other)
    def __rmul__(self, other):
        return coord(self.i*other, self.j*other)
    def __div__(self, other):
        return coord(self.i/other, self.j/other)
    def __mod__(self, other):
        return coord(self.i % other, self.j % other)
    def __rdiv__(self, other):
        return coord(self.i/other, self.j/other)
    def roundtogrid(self):
        return coord(int(round(self.i)), int(round(self.j)))
    def distance(self, other=None):
        if not other: other = coord(0, 0)
        return math.sqrt((self.i - other.i)**2 + (self.j - other.j)**2)
    def bearing(self):
        return 1.90985*math.atan2(self.j, self.i)
    def sgn(self):
        s = coord()
        if self.i == 0:
            s.i = 0
        else:
            s.i = self.i / abs(self.i)
        if self.j == 0:
            s.j = 0
        else:
            s.j = self.j / abs(self.j)
        return s
    def quadrant(self):
        #print "Location %s -> %s" % (self, (self / QUADSIZE).roundtogrid())
        return self.roundtogrid() / QUADSIZE
    def sector(self):
        return self.roundtogrid() % QUADSIZE
    def scatter(self):
        s = coord()
        s.i = self.i + randrange(-1, 2)
        s.j = self.j + randrange(-1, 2)
        return s
    def __str__(self):
        if self.i == None or self.j == None:
            return "Nowhere"
        return "%s - %s" % (self.i+1, self.j+1)
    __repr__ = __str__

class planet:
    def __init__(self):
        self.name = None	# string-valued if inhabited
        self.quadrant = coord()	# quadrant located
        self.pclass = None	# could be ""M", "N", "O", or "destroyed"
        self.crystals = "absent"# could be "mined", "present", "absent"
        self.known = "unknown"	# could be "unknown", "known", "shuttle_down"
        self.inhabited = False	# is it inhabites?
    def __str__(self):
        return self.name

class quadrant:
    def __init__(self):
        self.stars = 0
        self.planet = None
	self.starbase = False
	self.klingons = 0
	self.romulans = 0
	self.supernova = False
	self.charted = False
        self.status = "secure"	# Could be "secure", "distressed", "enslaved"

class page:
    def __init__(self):
	self.stars = None
	self.starbase = None
	self.klingons = None

def fill2d(size, fillfun):
    "Fill an empty list in 2D."
    lst = []
    for i in range(size):
        lst.append([]) 
        for j in range(size):
            lst[i].append(fillfun(i, j))
    return lst

class snapshot:
    def __init__(self):
        self.snap = False	# snapshot taken
        self.crew = 0   	# crew complement
	self.remkl = 0  	# remaining klingons
	self.nscrem = 0		# remaining super commanders
	self.starkl = 0 	# destroyed stars
	self.basekl = 0 	# destroyed bases
	self.nromrem = 0	# Romulans remaining
	self.nplankl = 0	# destroyed uninhabited planets
	self.nworldkl = 0	# destroyed inhabited planets
        self.planets = []	# Planet information
        self.date = 0.0   	# stardate
	self.remres = 0 	# remaining resources
	self.remtime = 0	# remaining time
        self.baseq = [] 	# Base quadrant coordinates
        self.kcmdr = [] 	# Commander quadrant coordinates
	self.kscmdr = coord()	# Supercommander quadrant coordinates
        # the galaxy
        self.galaxy = fill2d(GALSIZE, lambda i, j: quadrant())
        # the starchart
    	self.chart = fill2d(GALSIZE, lambda i, j: page())

class event:
    def __init__(self):
        self.date = None	# A real number
        self.quadrant = None	# A coord structure

# game options 
OPTION_ALL	= 0xffffffff
OPTION_TTY	= 0x00000001	# old interface 
OPTION_CURSES	= 0x00000002	# new interface 
OPTION_IOMODES	= 0x00000003	# cover both interfaces 
OPTION_PLANETS	= 0x00000004	# planets and mining 
OPTION_THOLIAN	= 0x00000008	# Tholians and their webs (UT 1979 version)
OPTION_THINGY	= 0x00000010	# Space Thingy can shoot back (Stas, 2005)
OPTION_PROBE	= 0x00000020	# deep-space probes (DECUS version, 1980)
OPTION_SHOWME	= 0x00000040	# bracket Enterprise in chart 
OPTION_RAMMING	= 0x00000080	# enemies may ram Enterprise (Almy)
OPTION_MVBADDY	= 0x00000100	# more enemies can move (Almy)
OPTION_BLKHOLE	= 0x00000200	# black hole may timewarp you (Stas, 2005) 
OPTION_BASE	= 0x00000400	# bases have good shields (Stas, 2005)
OPTION_WORLDS	= 0x00000800	# logic for inhabited worlds (ESR, 2006)
OPTION_AUTOSCAN	= 0x00001000	# automatic LRSCAN before CHART (ESR, 2006)
OPTION_PLAIN	= 0x01000000	# user chose plain game 
OPTION_ALMY	= 0x02000000	# user chose Almy variant 

# Define devices 
DSRSENS	= 0
DLRSENS	= 1
DPHASER	= 2
DPHOTON	= 3
DLIFSUP	= 4
DWARPEN	= 5
DIMPULS	= 6
DSHIELD	= 7
DRADIO	= 0
DSHUTTL = 9
DCOMPTR = 10
DNAVSYS	= 11
DTRANSP = 12
DSHCTRL	= 13
DDRAY	= 14
DDSP	= 15
NDEVICES= 16	# Number of devices

SKILL_NONE	= 0
SKILL_NOVICE	= 1
SKILL_FAIR	= 2
SKILL_GOOD	= 3
SKILL_EXPERT	= 4
SKILL_EMERITUS	= 5

def damaged(dev):	return (game.damage[dev] != 0.0)
def communicating():	return not damaged(DRADIO) or game.condition=="docked"

# Define future events 
FSPY	= 0	# Spy event happens always (no future[] entry)
		# can cause SC to tractor beam Enterprise
FSNOVA  = 1	# Supernova
FTBEAM  = 2	# Commander tractor beams Enterprise
FSNAP   = 3	# Snapshot for time warp
FBATTAK = 4	# Commander attacks base
FCDBAS  = 5	# Commander destroys base
FSCMOVE = 6	# Supercommander moves (might attack base)
FSCDBAS = 7	# Supercommander destroys base
FDSPROB = 8	# Move deep space probe
FDISTR	= 9	# Emit distress call from an inhabited world 
FENSLV	= 10	# Inhabited word is enslaved */
FREPRO	= 11	# Klingons build a ship in an enslaved system
NEVENTS	= 12

# Abstract out the event handling -- underlying data structures will change
# when we implement stateful events 
def findevent(evtype):	return game.future[evtype]

class enemy:
    def __init__(self, type=None, loc=None, power=None):
        self.type = type
        self.location = coord()
        if loc:
            self.move(loc)
        self.power = power	# enemy energy level
        game.enemies.append(self)
    def move(self, loc):
        motion = (loc != self.location)
        if self.location.i is not None and self.location.j is not None:
            if motion:
                if self.type == 'T':
                    game.quad[self.location.i][self.location.j] = '#'
                else:
                    game.quad[self.location.i][self.location.j] = '.'
        if loc:
            self.location = copy.copy(loc)
            game.quad[self.location.i][self.location.j] = self.type
            self.kdist = self.kavgd = (game.sector - loc).distance()
        else:
            self.location = coord()
            self.kdist = self.kavgd = None
            game.enemies.remove(self)
        return motion
    def __repr__(self):
        return "<%s,%s.%f>" % (self.type, self.location, self.power)	# For debugging

class gamestate:
    def __init__(self):
        self.options = None	# Game options
        self.state = snapshot()	# A snapshot structure
        self.snapsht = snapshot()	# Last snapshot taken for time-travel purposes
        self.quad = None	# contents of our quadrant
        self.damage = [0.0] * NDEVICES	# damage encountered
        self.future = []		# future events
        for i in range(NEVENTS):
            self.future.append(event())
        self.passwd  = None;		# Self Destruct password
        self.enemies = []
        self.quadrant = None	# where we are in the large
        self.sector = None	# where we are in the small
        self.tholian = None	# Tholian enemy object
        self.base = None	# position of base in current quadrant
        self.battle = None	# base coordinates being attacked
        self.plnet = None	# location of planet in quadrant
        self.gamewon = False	# Finished!
        self.ididit = False	# action taken -- allows enemy to attack
        self.alive = False	# we are alive (not killed)
        self.justin = False	# just entered quadrant
        self.shldup = False	# shields are up
        self.shldchg = False	# shield is changing (affects efficiency)
        self.iscate = False	# super commander is here
        self.ientesc = False	# attempted escape from supercommander
        self.resting = False	# rest time
        self.icraft = False	# Kirk in Galileo
        self.landed = False	# party on planet (true), on ship (false)
        self.alldone = False	# game is now finished
        self.neutz = False	# Romulan Neutral Zone
        self.isarmed = False	# probe is armed
        self.inorbit = False	# orbiting a planet
        self.imine = False	# mining
        self.icrystl = False	# dilithium crystals aboard
        self.iseenit = False	# seen base attack report
        self.thawed = False	# thawed game
        self.condition = None	# "green", "yellow", "red", "docked", "dead"
        self.iscraft = None	# "onship", "offship", "removed"
        self.skill = None	# Player skill level
        self.inkling = 0	# initial number of klingons
        self.inbase = 0		# initial number of bases
        self.incom = 0		# initial number of commanders
        self.inscom = 0		# initial number of commanders
        self.inrom = 0		# initial number of commanders
        self.instar = 0		# initial stars
        self.intorps = 0	# initial/max torpedoes
        self.torps = 0		# number of torpedoes
        self.ship = 0		# ship type -- 'E' is Enterprise
        self.abandoned = 0	# count of crew abandoned in space
        self.length = 0		# length of game
        self.klhere = 0		# klingons here
        self.casual = 0		# causalties
        self.nhelp = 0		# calls for help
        self.nkinks = 0		# count of energy-barrier crossings
        self.iplnet = None	# planet # in quadrant
        self.inplan = 0		# initial planets
        self.irhere = 0		# Romulans in quadrant
        self.isatb = 0		# =1 if super commander is attacking base
        self.tourn = None	# tournament number
        self.nprobes = 0	# number of probes available
        self.inresor = 0.0	# initial resources
        self.intime = 0.0	# initial time
        self.inenrg = 0.0	# initial/max energy
        self.inshld = 0.0	# initial/max shield
        self.inlsr = 0.0	# initial life support resources
        self.indate = 0.0	# initial date
        self.energy = 0.0	# energy level
        self.shield = 0.0	# shield level
        self.warpfac = 0.0	# warp speed
        self.lsupres = 0.0	# life support reserves
        self.optime = 0.0	# time taken by current operation
        self.damfac = 0.0	# damage factor
        self.lastchart = 0.0	# time star chart was last updated
        self.cryprob = 0.0	# probability that crystal will work
        self.probe = None	# object holding probe course info
        self.height = 0.0	# height of orbit around planet
    def recompute(self):
        # Stas thinks this should be (C expression): 
        # game.state.remkl + len(game.state.kcmdr) > 0 ?
	#	game.state.remres/(game.state.remkl + 4*len(game.state.kcmdr)) : 99
        # He says the existing expression is prone to divide-by-zero errors
        # after killing the last klingon when score is shown -- perhaps also
        # if the only remaining klingon is SCOM.
        game.state.remtime = game.state.remres/(game.state.remkl + 4*len(game.state.kcmdr))

FWON = 0
FDEPLETE = 1
FLIFESUP = 2
FNRG = 3
FBATTLE = 4
FNEG3 = 5
FNOVA = 6
FSNOVAED = 7
FABANDN = 8
FDILITHIUM = 9
FMATERIALIZE = 10
FPHASER = 11
FLOST = 12
FMINING = 13
FDPLANET = 14
FPNOVA = 15
FSSC = 16
FSTRACTOR = 17
FDRAY = 18
FTRIBBLE = 19
FHOLE = 20
FCREW = 21

def withprob(p):
    return random.random() < p

def randrange(*args):
    return random.randrange(*args)

def randreal(*args):
    v = random.random()
    if len(args) == 1:
        v *= args[0] 		# from [0, args[0])
    elif len(args) == 2:
        v = args[0] + v*(args[1]-args[0])	# from [args[0], args[1])
    return v

# Code from ai.c begins here

def welcoming(iq):
    "Would this quadrant welcome another Klingon?"
    return iq.valid_quadrant() and \
	not game.state.galaxy[iq.i][iq.j].supernova and \
	game.state.galaxy[iq.i][iq.j].klingons < MAXKLQUAD

def tryexit(enemy, look, irun):
    "A bad guy attempts to bug out."
    iq = coord()
    iq.i = game.quadrant.i+(look.i+(QUADSIZE-1))/QUADSIZE - 1
    iq.j = game.quadrant.j+(look.j+(QUADSIZE-1))/QUADSIZE - 1
    if not welcoming(iq):
	return False;
    if enemy.type == 'R':
	return False; # Romulans cannot escape! 
    if not irun:
	# avoid intruding on another commander's territory 
	if enemy.type == 'C':
            if iq in game.state.kcmdr:
                return False
	    # refuse to leave if currently attacking starbase 
	    if game.battle == game.quadrant:
		return False
	# don't leave if over 1000 units of energy 
	if enemy.power > 1000.0:
	    return False
    # emit escape message and move out of quadrant.
    # we know this if either short or long range sensors are working
    if not damaged(DSRSENS) or not damaged(DLRSENS) or \
	game.condition == "docked":
	prout(crmena(True, enemy.type, "sector", enemy.location) + \
              (_(" escapes to Quadrant %s (and regains strength).") % q))
    # handle local matters related to escape
    enemy.move(None)
    game.klhere -= 1
    if game.condition != "docked":
	newcnd()
    # Handle global matters related to escape 
    game.state.galaxy[game.quadrant.i][game.quadrant.j].klingons -= 1
    game.state.galaxy[iq.i][iq.j].klingons += 1
    if enemy.type=='S':
	game.iscate = False
	game.ientesc = False
	game.isatb = 0
	schedule(FSCMOVE, 0.2777)
	unschedule(FSCDBAS)
	game.state.kscmdr=iq
    else:
	for cmdr in game.state.kcmdr:
	    if cmdr == game.quadrant:
		game.state.kcmdr[n] = iq
		break
    return True; # success 

# The bad-guy movement algorithm:
# 
# 1. Enterprise has "force" based on condition of phaser and photon torpedoes.
# If both are operating full strength, force is 1000. If both are damaged,
# force is -1000. Having shields down subtracts an additional 1000.
# 
# 2. Enemy has forces equal to the energy of the attacker plus
# 100*(K+R) + 500*(C+S) - 400 for novice through good levels OR
# 346*K + 400*R + 500*(C+S) - 400 for expert and emeritus.
# 
# Attacker Initial energy levels (nominal):
# Klingon   Romulan   Commander   Super-Commander
# Novice    400        700        1200        
# Fair      425        750        1250
# Good      450        800        1300        1750
# Expert    475        850        1350        1875
# Emeritus  500        900        1400        2000
# VARIANCE   75        200         200         200
# 
# Enemy vessels only move prior to their attack. In Novice - Good games
# only commanders move. In Expert games, all enemy vessels move if there
# is a commander present. In Emeritus games all enemy vessels move.
# 
# 3. If Enterprise is not docked, an aggressive action is taken if enemy
# forces are 1000 greater than Enterprise.
# 
# Agressive action on average cuts the distance between the ship and
# the enemy to 1/4 the original.
# 
# 4.  At lower energy advantage, movement units are proportional to the
# advantage with a 650 advantage being to hold ground, 800 to move forward
# 1, 950 for two, 150 for back 4, etc. Variance of 100.
# 
# If docked, is reduced by roughly 1.75*game.skill, generally forcing a
# retreat, especially at high skill levels.
# 
# 5.  Motion is limited to skill level, except for SC hi-tailing it out.

def movebaddy(enemy):
    "Tactical movement for the bad guys."
    next = coord(); look = coord()
    irun = False
    # This should probably be just (game.quadrant in game.state.kcmdr) + (game.state.kscmdr==game.quadrant) 
    if game.skill >= SKILL_EXPERT:
	nbaddys = (((game.quadrant in game.state.kcmdr)*2 + (game.state.kscmdr==game.quadrant)*2+game.klhere*1.23+game.irhere*1.5)/2.0)
    else:
	nbaddys = (game.quadrant in game.state.kcmdr) + (game.state.kscmdr==game.quadrant)
    dist1 = enemy.kdist
    mdist = int(dist1 + 0.5); # Nearest integer distance 
    # If SC, check with spy to see if should hi-tail it 
    if enemy.type=='S' and \
	(enemy.power <= 500.0 or (game.condition=="docked" and not damaged(DPHOTON))):
	irun = True
	motion = -QUADSIZE
    else:
	# decide whether to advance, retreat, or hold position 
	forces = enemy.power+100.0*len(game.enemies)+400*(nbaddys-1)
	if not game.shldup:
	    forces += 1000; # Good for enemy if shield is down! 
	if not damaged(DPHASER) or not damaged(DPHOTON):
            if damaged(DPHASER): # phasers damaged 
		forces += 300.0
	    else:
		forces -= 0.2*(game.energy - 2500.0)
	    if damaged(DPHOTON): # photon torpedoes damaged 
		forces += 300.0
	    else:
		forces -= 50.0*game.torps
	else:
	    # phasers and photon tubes both out! 
	    forces += 1000.0
	motion = 0
        if forces <= 1000.0 and game.condition != "docked": # Typical situation 
	    motion = ((forces + randreal(200))/150.0) - 5.0
	else:
            if forces > 1000.0: # Very strong -- move in for kill 
		motion = (1.0 - randreal())**2 * dist1 + 1.0
	    if game.condition=="docked" and (game.options & OPTION_BASE): # protected by base -- back off ! 
		motion -= game.skill*(2.0-randreal()**2)
	if idebug:
	    proutn("=== MOTION = %d, FORCES = %1.2f, " % (motion, forces))
	# don't move if no motion 
	if motion==0:
	    return
	# Limit motion according to skill 
	if abs(motion) > game.skill:
            if motion < 0:
                motion = -game.skill
            else:
                motion = game.skill
    # calculate preferred number of steps 
    nsteps = abs(int(motion))
    if motion > 0 and nsteps > mdist:
	nsteps = mdist; # don't overshoot 
    if nsteps > QUADSIZE:
	nsteps = QUADSIZE; # This shouldn't be necessary 
    if nsteps < 1:
	nsteps = 1; # This shouldn't be necessary 
    if idebug:
	proutn("NSTEPS = %d:" % nsteps)
    # Compute preferred values of delta X and Y 
    m = game.sector - enemy.location
    if 2.0 * abs(m.i) < abs(m.j):
	m.i = 0
    if 2.0 * abs(m.j) < abs(game.sector.i-enemy.location.i):
	m.j = 0
    m = (motion * m).sgn()
    next = enemy.location
    # main move loop 
    for ll in range(nsteps):
	if idebug:
	    proutn(" %d" % (ll+1))
	# Check if preferred position available 
	look = next + m
        if m.i < 0:
            krawli = 1
        else:
            krawli = -1
        if m.j < 0:
            krawlj = 1
        else:
            krawlj = -1
	success = False
	attempts = 0; # Settle mysterious hang problem 
	while attempts < 20 and not success:
            attempts += 1
	    if look.i < 0 or look.i >= QUADSIZE:
		if motion < 0 and tryexit(enemy, look, irun):
		    return
		if krawli == m.i or m.j == 0:
		    break
		look.i = next.i + krawli
		krawli = -krawli
	    elif look.j < 0 or look.j >= QUADSIZE:
		if motion < 0 and tryexit(enemy, look, irun):
		    return
		if krawlj == m.j or m.i == 0:
		    break
		look.j = next.j + krawlj
		krawlj = -krawlj
	    elif (game.options & OPTION_RAMMING) and game.quad[look.i][look.j] != '.':
		# See if enemy should ram ship 
		if game.quad[look.i][look.j] == game.ship and \
		    (enemy.type == 'C' or enemy.type == 'S'):
		    collision(rammed=True, enemy=enemy)
		    return
		if krawli != m.i and m.j != 0:
		    look.i = next.i + krawli
		    krawli = -krawli
		elif krawlj != m.j and m.i != 0:
		    look.j = next.j + krawlj
		    krawlj = -krawlj
		else:
		    break; # we have failed 
	    else:
		success = True
	if success:
	    next = look
	    if idebug:
		proutn(`next`)
	else:
	    break; # done early 
    if idebug:
	skip(1)
    if enemy.move(next):
	if not damaged(DSRSENS) or game.condition == "docked":
	    proutn(_("*** %s from Sector %s") % (cramen(enemy.type), enemy.location))
	    if enemy.kdist < dist1:
		proutn(_(" advances to "))
	    else:
		proutn(_(" retreats to "))
	    prout("Sector %s." % next)

def moveklings():
    "Sequence Klingon tactical movement."
    if idebug:
	prout("== MOVCOM")
    # Figure out which Klingon is the commander (or Supercommander)
    # and do move
    if game.quadrant in game.state.kcmdr:
        for enemy in game.enemies:
	    if enemy.type == 'C':
		movebaddy(enemy)
    if game.state.kscmdr==game.quadrant:
        for enemy in game.enemies:
	    if enemy.type == 'S':
		movebaddy(enemy)
		break
    # If skill level is high, move other Klingons and Romulans too!
    # Move these last so they can base their actions on what the
    # commander(s) do.
    if game.skill >= SKILL_EXPERT and (game.options & OPTION_MVBADDY):
        for enemy in game.enemies:
            if enemy.type in ('K', 'R'):
		movebaddy(enemy)
    game.enemies.sort(lambda x, y: cmp(x.kdist, y.kdist))

def movescom(iq, avoid):
    "Commander movement helper." 
    # Avoid quadrants with bases if we want to avoid Enterprise 
    if not welcoming(iq) or (avoid and iq in game.state.baseq):
	return False
    if game.justin and not game.iscate:
	return False
    # do the move 
    game.state.galaxy[game.state.kscmdr.i][game.state.kscmdr.j].klingons -= 1
    game.state.kscmdr = iq
    game.state.galaxy[game.state.kscmdr.i][game.state.kscmdr.j].klingons += 1
    if game.state.kscmdr==game.quadrant:
	# SC has scooted, remove him from current quadrant 
	game.iscate=False
	game.isatb=0
	game.ientesc = False
	unschedule(FSCDBAS)
	for enemy in game.enemies:
	    if enemy.type == 'S':
		break
	enemy.move(None)
	game.klhere -= 1
	if game.condition != "docked":
	    newcnd()
        game.enemies.sort(lambda x, y: cmp(x.kdist, y.kdist))
    # check for a helpful planet 
    for i in range(game.inplan):
	if game.state.planets[i].quadrant == game.state.kscmdr and \
	    game.state.planets[i].crystals == "present":
	    # destroy the planet 
	    game.state.planets[i].pclass = "destroyed"
	    game.state.galaxy[game.state.kscmdr.i][game.state.kscmdr.j].planet = None
	    if communicating():
		announce()
		prout(_("Lt. Uhura-  \"Captain, Starfleet Intelligence reports"))
		proutn(_("   a planet in Quadrant %s has been destroyed") % game.state.kscmdr)
		prout(_("   by the Super-commander.\""))
	    break
    return True; # looks good! 
			
def supercommander():
    "Move the Super Commander." 
    iq = coord(); sc = coord(); ibq = coord(); idelta = coord()
    basetbl = []
    if idebug:
	prout("== SUPERCOMMANDER")
    # Decide on being active or passive 
    avoid = ((game.incom - len(game.state.kcmdr) + game.inkling - game.state.remkl)/(game.state.date+0.01-game.indate) < 0.1*game.skill*(game.skill+1.0) or \
	    (game.state.date-game.indate) < 3.0)
    if not game.iscate and avoid:
	# compute move away from Enterprise 
	idelta = game.state.kscmdr-game.quadrant
	if idelta.distance() > 2.0:
	    # circulate in space 
	    idelta.i = game.state.kscmdr.j-game.quadrant.j
	    idelta.j = game.quadrant.i-game.state.kscmdr.i
    else:
	# compute distances to starbases 
	if not game.state.baseq:
	    # nothing left to do 
	    unschedule(FSCMOVE)
	    return
	sc = game.state.kscmdr
        for base in game.state.baseq:
	    basetbl.append((i, (base - sc).distance()))
	if game.state.baseq > 1:
            basetbl.sort(lambda x, y: cmp(x[1]. y[1]))
	# look for nearest base without a commander, no Enterprise, and
        # without too many Klingons, and not already under attack. 
	ifindit = iwhichb = 0
	for (i2, base) in enumerate(game.state.baseq):
	    i = basetbl[i2][0];	# bug in original had it not finding nearest
	    if base==game.quadrant or base==game.battle or not welcoming(base):
		continue
	    # if there is a commander, and no other base is appropriate,
	    # we will take the one with the commander
            for cmdr in game.state.kcmdr:
		if base == cmdr and ifindit != 2:
		    ifindit = 2
		    iwhichb = i
		    break
	    else:	# no commander -- use this one 
		ifindit = 1
		iwhichb = i
		break
	if ifindit==0:
	    return # Nothing suitable -- wait until next time
	ibq = game.state.baseq[iwhichb]
	# decide how to move toward base 
	idelta = ibq - game.state.kscmdr
    # Maximum movement is 1 quadrant in either or both axes 
    idelta = idelta.sgn()
    # try moving in both x and y directions
    # there was what looked like a bug in the Almy C code here,
    # but it might be this translation is just wrong.
    iq = game.state.kscmdr + idelta
    if not movescom(iq, avoid):
	# failed -- try some other maneuvers 
	if idelta.i==0 or idelta.j==0:
	    # attempt angle move 
	    if idelta.i != 0:
		iq.j = game.state.kscmdr.j + 1
		if not movescom(iq, avoid):
		    iq.j = game.state.kscmdr.j - 1
		    movescom(iq, avoid)
	    elif idelta.j != 0:
		iq.i = game.state.kscmdr.i + 1
		if not movescom(iq, avoid):
		    iq.i = game.state.kscmdr.i - 1
		    movescom(iq, avoid)
	else:
	    # try moving just in x or y 
	    iq.j = game.state.kscmdr.j
	    if not movescom(iq, avoid):
		iq.j = game.state.kscmdr.j + idelta.j
		iq.i = game.state.kscmdr.i
		movescom(iq, avoid)
    # check for a base 
    if len(game.state.baseq) == 0:
	unschedule(FSCMOVE)
    else:
        for ibq in game.state.baseq:
	    if ibq == game.state.kscmdr and game.state.kscmdr == game.battle:
		# attack the base 
		if avoid:
		    return # no, don't attack base! 
		game.iseenit = False
		game.isatb = 1
		schedule(FSCDBAS, randreal(1.0, 3.0))
		if is_scheduled(FCDBAS):
		    postpone(FSCDBAS, scheduled(FCDBAS)-game.state.date)
		if not communicating():
		    return # no warning 
		game.iseenit = True
		announce()
		prout(_("Lt. Uhura-  \"Captain, the starbase in Quadrant %s") \
                      % game.state.kscmdr)
		prout(_("   reports that it is under attack from the Klingon Super-commander."))
		proutn(_("   It can survive until stardate %d.\"") \
                       % int(scheduled(FSCDBAS)))
		if not game.resting:
		    return
		prout(_("Mr. Spock-  \"Captain, shall we cancel the rest period?\""))
		if ja() == False:
		    return
		game.resting = False
		game.optime = 0.0; # actually finished 
		return
    # Check for intelligence report 
    if not idebug and \
	(withprob(0.8) or \
	 (not communicating()) or \
	 not game.state.galaxy[game.state.kscmdr.i][game.state.kscmdr.j].charted):
	return
    announce()
    prout(_("Lt. Uhura-  \"Captain, Starfleet Intelligence reports"))
    proutn(_("   the Super-commander is in Quadrant %s,") % game.state.kscmdr)
    return

def movetholian():
    "Move the Tholian."
    if not game.tholian or game.justin:
	return
    id = coord()
    if game.tholian.location.i == 0 and game.tholian.location.j == 0:
	id.i = 0; id.j = QUADSIZE-1
    elif game.tholian.location.i == 0 and game.tholian.location.j == QUADSIZE-1:
	id.i = QUADSIZE-1; id.j = QUADSIZE-1
    elif game.tholian.location.i == QUADSIZE-1 and game.tholian.location.j == QUADSIZE-1:
	id.i = QUADSIZE-1; id.j = 0
    elif game.tholian.location.i == QUADSIZE-1 and game.tholian.location.j == 0:
	id.i = 0; id.j = 0
    else:
	# something is wrong! 
	game.tholian.move(None)
        prout("***Internal error: Tholian in a bad spot.")
	return
    # do nothing if we are blocked 
    if game.quad[id.i][id.j] not in ('.', '#'):
	return
    here = copy.copy(game.tholian.location)
    delta = (id - game.tholian.location).sgn()
    # move in x axis 
    while here.i != id.i:
        here.i += delta.i
        if game.quad[here.i][here.j]=='.':
            game.tholian.move(here)
    # move in y axis 
    while here.j != id.j:
        here.j += delta.j
        if game.quad[here.i][here.j]=='.':
            game.tholian.move(here)
    # check to see if all holes plugged 
    for i in range(QUADSIZE):
	if game.quad[0][i]!='#' and game.quad[0][i]!='T':
	    return
	if game.quad[QUADSIZE-1][i]!='#' and game.quad[QUADSIZE-1][i]!='T':
	    return
	if game.quad[i][0]!='#' and game.quad[i][0]!='T':
	    return
	if game.quad[i][QUADSIZE-1]!='#' and game.quad[i][QUADSIZE-1]!='T':
	    return
    # All plugged up -- Tholian splits 
    game.quad[game.tholian.location.i][game.tholian.location.j]='#'
    dropin(' ')
    prout(crmena(True, 'T', "sector", game.tholian) + _(" completes web."))
    game.tholian.move(None)
    return

# Code from battle.c begins here

def doshield(shraise):
    "Change shield status."
    action = "NONE"
    game.ididit = False
    if shraise:
	action = "SHUP"
    else:
	key = scanner.next()
	if key == "IHALPHA":
	    if scanner.sees("transfer"):
		action = "NRG"
	    else:
		if damaged(DSHIELD):
		    prout(_("Shields damaged and down."))
		    return
		if scanner.sees("up"):
		    action = "SHUP"
		elif scanner.sees("down"):
		    action = "SHDN"
	if action=="NONE":
	    proutn(_("Do you wish to change shield energy? "))
	    if ja() == True:
		action = "NRG"
	    elif damaged(DSHIELD):
		prout(_("Shields damaged and down."))
		return
	    elif game.shldup:
		proutn(_("Shields are up. Do you want them down? "))
		if ja() == True:
		    action = "SHDN"
		else:
		    scanner.chew()
		    return
	    else:
		proutn(_("Shields are down. Do you want them up? "))
		if ja() == True:
		    action = "SHUP"
		else:
		    scanner.chew()
		    return    
    if action == "SHUP": # raise shields 
	if game.shldup:
	    prout(_("Shields already up."))
	    return
	game.shldup = True
	game.shldchg = True
	if game.condition != "docked":
	    game.energy -= 50.0
	prout(_("Shields raised."))
	if game.energy <= 0:
	    skip(1)
	    prout(_("Shields raising uses up last of energy."))
	    finish(FNRG)
	    return
	game.ididit=True
	return
    elif action == "SHDN":
	if not game.shldup:
	    prout(_("Shields already down."))
	    return
	game.shldup=False
	game.shldchg=True
	prout(_("Shields lowered."))
	game.ididit = True
	return
    elif action == "NRG":
	while scanner.next() != "IHREAL":
	    scanner.chew()
	    proutn(_("Energy to transfer to shields- "))
        nrg = scanner.real
	scanner.chew()
	if nrg == 0:
	    return
	if nrg > game.energy:
	    prout(_("Insufficient ship energy."))
	    return
	game.ididit = True
	if game.shield+nrg >= game.inshld:
	    prout(_("Shield energy maximized."))
	    if game.shield+nrg > game.inshld:
		prout(_("Excess energy requested returned to ship energy"))
	    game.energy -= game.inshld-game.shield
	    game.shield = game.inshld
	    return
	if nrg < 0.0 and game.energy-nrg > game.inenrg:
	    # Prevent shield drain loophole 
	    skip(1)
	    prout(_("Engineering to bridge--"))
	    prout(_("  Scott here. Power circuit problem, Captain."))
	    prout(_("  I can't drain the shields."))
	    game.ididit = False
	    return
	if game.shield+nrg < 0:
	    prout(_("All shield energy transferred to ship."))
	    game.energy += game.shield
	    game.shield = 0.0
	    return
	proutn(_("Scotty- \""))
	if nrg > 0:
	    prout(_("Transferring energy to shields.\""))
	else:
	    prout(_("Draining energy from shields.\""))
	game.shield += nrg
	game.energy -= nrg
	return

def randdevice():
    "Choose a device to damage, at random."
    weights = (
	105,	# DSRSENS: short range scanners	10.5% 
	105,	# DLRSENS: long range scanners		10.5% 
	120,	# DPHASER: phasers			12.0% 
	120,	# DPHOTON: photon torpedoes		12.0% 
	25,	# DLIFSUP: life support			 2.5% 
	65,	# DWARPEN: warp drive			 6.5% 
	70,	# DIMPULS: impulse engines		 6.5% 
	145,	# DSHIELD: deflector shields		14.5% 
	30,	# DRADIO:  subspace radio		 3.0% 
	45,	# DSHUTTL: shuttle			 4.5% 
	15,	# DCOMPTR: computer			 1.5% 
	20,	# NAVCOMP: navigation system		 2.0% 
	75,	# DTRANSP: transporter			 7.5% 
	20,	# DSHCTRL: high-speed shield controller  2.0% 
	10,	# DDRAY: death ray			 1.0% 
	30,	# DDSP: deep-space probes		 3.0% 
    )
    assert(sum(weights) == 1000)
    idx = randrange(1000)
    sum = 0
    for (i, w) in enumerate(weights):
	sum += w
	if idx < sum:
	    return i
    return None;	# we should never get here

def collision(rammed, enemy):
    "Collision handling fot rammong events."
    prouts(_("***RED ALERT!  RED ALERT!"))
    skip(1)
    prout(_("***COLLISION IMMINENT."))
    skip(2)
    proutn("***")
    proutn(crmshp())
    hardness = {'R':1.5, 'C':2.0, 'S':2.5, 'T':0.5, '?':4.0}.get(enemy.type, 1.0)
    if rammed:
        proutn(_(" rammed by "))
    else:
        proutn(_(" rams "))
    proutn(crmena(False, enemy.type, "sector", enemy.location))
    if rammed:
	proutn(_(" (original position)"))
    skip(1)
    deadkl(enemy.location, enemy.type, game.sector)
    proutn("***" + crmship() + " heavily damaged.")
    icas = randrange(10, 30)
    prout(_("***Sickbay reports %d casualties"), icas)
    game.casual += icas
    game.state.crew -= icas
    # In the pre-SST2K version, all devices got equiprobably damaged,
    # which was silly.  Instead, pick up to half the devices at
    # random according to our weighting table,
    ncrits = randrange(NDEVICES/2)
    for m in range(ncrits):
	dev = randdevice()
	if game.damage[dev] < 0:
	    continue
	extradm = (10.0*hardness*randreal()+1.0)*game.damfac
	# Damage for at least time of travel! 
	game.damage[dev] += game.optime + extradm
    game.shldup = False
    prout(_("***Shields are down."))
    if game.state.remkl + len(game.state.kcmdr) + game.state.nscrem:
	announce()
	damagereport()
    else:
	finish(FWON)
    return

def torpedo(origin, bearing, dispersion, number, nburst):
    "Let a photon torpedo fly" 
    if not damaged(DSRSENS) or game.condition=="docked":
	setwnd(srscan_window)
    else: 
	setwnd(message_window)
    ac = bearing + 0.25*dispersion	# dispersion is a random variable
    bullseye = (15.0 - bearing)*0.5235988
    track = course(bearing=ac, distance=QUADSIZE, origin=cartesian(origin)) 
    bumpto = coord(0, 0)
    # Loop to move a single torpedo 
    setwnd(message_window)
    for step in range(1, QUADSIZE*2):
        if not track.next(): break
        w = track.sector()
	if not w.valid_sector():
	    break
	iquad=game.quad[w.i][w.j]
	tracktorpedo(origin, w, step, number, nburst, iquad)
	if iquad=='.':
	    continue
	# hit something 
	if not damaged(DSRSENS) or game.condition == "docked":
	    skip(1);	# start new line after text track 
	if iquad in ('E', 'F'): # Hit our ship 
	    skip(1)
	    prout(_("Torpedo hits %s.") % crmshp())
	    hit = 700.0 + randreal(100) - \
		1000.0 * (w-origin).distance() * math.fabs(math.sin(bullseye-track.angle))
	    newcnd(); # we're blown out of dock 
	    if game.landed or game.condition=="docked":
		return hit # Cheat if on a planet 
            # In the C/FORTRAN version, dispersion was 2.5 radians, which
            # is 143 degrees, which is almost exactly 4.8 clockface units
            displacement = course(track.bearing+randreal(-2.4,2.4), distance=2**0.5)
            displacement.next()
            bumpto = displacement.sector()
	    if not bumpto.valid_sector():
		return hit
	    if game.quad[bumpto.i][bumpto.j]==' ':
		finish(FHOLE)
		return hit
	    if game.quad[bumpto.i][bumpto.j]!='.':
		# can't move into object 
		return hit
	    game.sector = bumpto
	    proutn(crmshp())
            game.quad[w.i][w.j]='.'
            game.quad[bumpto.i][bumpto.j]=iquad
            prout(_(" displaced by blast to Sector %s ") % bumpto)
            for enemy in game.enemies:
                enemy.kdist = enemy.kavgd = (game.sector-enemy.location).distance()
            game.enemies.sort(lambda x, y: cmp(x.kdist, y.kdist))
            return None
	elif iquad in ('C', 'S', 'R', 'K'): # Hit a regular enemy 
	    # find the enemy 
	    if iquad in ('C', 'S') and withprob(0.05):
		prout(crmena(True, iquad, "sector", w) + _(" uses anti-photon device;"))
		prout(_("   torpedo neutralized."))
		return None
            for enemy in game.enemies:
		if w == enemy.location:
		    break
	    kp = math.fabs(enemy.power)
	    h1 = 700.0 + randrange(100) - \
		1000.0 * (w-origin).distance() * math.fabs(math.sin(bullseye-track.angle))
	    h1 = math.fabs(h1)
	    if kp < h1:
		h1 = kp
            if enemy.power < 0:
                enemy.power -= -h1
            else:
                enemy.power -= h1
	    if enemy.power == 0:
		deadkl(w, iquad, w)
		return None
	    proutn(crmena(True, iquad, "sector", w))
            displacement = course(track.bearing+randreal(-2.4,2.4), distance=2**0.5)
            displacement.next()
            bumpto = displacement.sector()
            if not bumpto.valid_sector():
		prout(_(" damaged but not destroyed."))
		return
	    if game.quad[bumpto.i][bumpto.j] == ' ':
		prout(_(" buffeted into black hole."))
		deadkl(w, iquad, bumpto)
	    if game.quad[bumpto.i][bumpto.j] != '.':
		prout(_(" damaged but not destroyed."))
            else:
                prout(_(" damaged-- displaced by blast to Sector %s ")%bumpto)
                enemy.location = bumpto
                game.quad[w.i][w.j]='.'
                game.quad[bumpto.i][bumpto.j]=iquad
                for enemy in game.enemies:
                    enemy.kdist = enemy.kavgd = (game.sector-enemy.location).distance()
                game.enemies.sort(lambda x, y: cmp(x.kdist, y.kdist))
            return None
	elif iquad == 'B': # Hit a base 
	    skip(1)
	    prout(_("***STARBASE DESTROYED.."))
            game.state.baseq = filter(lambda x: x != game.quadrant, game.state.baseq)
	    game.quad[w.i][w.j]='.'
	    game.base.invalidate()
	    game.state.galaxy[game.quadrant.i][game.quadrant.j].starbase -= 1
	    game.state.chart[game.quadrant.i][game.quadrant.j].starbase -= 1
	    game.state.basekl += 1
	    newcnd()
	    return None
	elif iquad == 'P': # Hit a planet 
	    prout(crmena(True, iquad, "sector", w) + _(" destroyed."))
	    game.state.nplankl += 1
	    game.state.galaxy[game.quadrant.i][game.quadrant.j].planet = None
	    game.iplnet.pclass = "destroyed"
	    game.iplnet = None
	    game.plnet.invalidate()
	    game.quad[w.i][w.j] = '.'
	    if game.landed:
		# captain perishes on planet 
		finish(FDPLANET)
	    return None
	elif iquad == '@': # Hit an inhabited world -- very bad! 
	    prout(crmena(True, iquad, "sector", w) + _(" destroyed."))
	    game.state.nworldkl += 1
	    game.state.galaxy[game.quadrant.i][game.quadrant.j].planet = None
	    game.iplnet.pclass = "destroyed"
	    game.iplnet = None
	    game.plnet.invalidate()
	    game.quad[w.i][w.j] = '.'
	    if game.landed:
		# captain perishes on planet 
		finish(FDPLANET)
	    prout(_("The torpedo destroyed an inhabited planet."))
	    return None
	elif iquad == '*': # Hit a star 
	    if withprob(0.9):
		nova(w)
            else:
                prout(crmena(True, '*', "sector", w) + _(" unaffected by photon blast."))
	    return None
	elif iquad == '?': # Hit a thingy 
	    if not (game.options & OPTION_THINGY) or withprob(0.3):
		skip(1)
		prouts(_("AAAAIIIIEEEEEEEEAAAAAAAAUUUUUGGGGGHHHHHHHHHHHH!!!"))
		skip(1)
		prouts(_("    HACK!     HACK!    HACK!        *CHOKE!*  "))
		skip(1)
		proutn(_("Mr. Spock-"))
		prouts(_("  \"Fascinating!\""))
		skip(1)
		deadkl(w, iquad, w)
	    else:
		# Stas Sergeev added the possibility that
		# you can shove the Thingy and piss it off.
		# It then becomes an enemy and may fire at you.
		thing.angry = True
		shoved = True
	    return None
	elif iquad == ' ': # Black hole 
	    skip(1)
	    prout(crmena(True, ' ', "sector", w) + _(" swallows torpedo."))
	    return None
	elif iquad == '#': # hit the web 
	    skip(1)
	    prout(_("***Torpedo absorbed by Tholian web."))
	    return None
	elif iquad == 'T':  # Hit a Tholian 
	    h1 = 700.0 + randrange(100) - \
		1000.0 * (w-origin).distance() * math.fabs(math.sin(bullseye-angle))
	    h1 = math.fabs(h1)
	    if h1 >= 600:
		game.quad[w.i][w.j] = '.'
		deadkl(w, iquad, w)
		game.tholian = None
		return None
	    skip(1)
	    proutn(crmena(True, 'T', "sector", w))
	    if withprob(0.05):
		prout(_(" survives photon blast."))
		return None
	    prout(_(" disappears."))
	    game.tholian.move(None)
	    game.quad[w.i][w.j] = '#'
	    dropin(' ')
	    return None
        else: # Problem!
	    skip(1)
	    proutn("Don't know how to handle torpedo collision with ")
	    proutn(crmena(True, iquad, "sector", w))
	    skip(1)
	    return None
	break
    skip(1)
    prout(_("Torpedo missed."))
    return None;

def fry(hit):
    "Critical-hit resolution." 
    if hit < (275.0-25.0*game.skill)*randreal(1.0, 1.5):
	return
    ncrit = int(1.0 + hit/(500.0+randreal(100)))
    proutn(_("***CRITICAL HIT--"))
    # Select devices and cause damage
    cdam = []
    for loop1 in range(ncrit):
        while True:
	    j = randdevice()
	    # Cheat to prevent shuttle damage unless on ship 
            if not (game.damage[j]<0.0 or (j==DSHUTTL and game.iscraft != "onship")):
                break
	cdam.append(j)
	extradm = (hit*game.damfac)/(ncrit*randreal(75, 100))
	game.damage[j] += extradm
    skipcount = 0
    for (i, j) in enumerate(cdam):
	proutn(device[j])
        if skipcount % 3 == 2 and i < len(cdam)-1:
            skip(1)
        skipcount += 1
        if i < len(cdam)-1:
            proutn(_(" and "))
    prout(_(" damaged."))
    if damaged(DSHIELD) and game.shldup:
	prout(_("***Shields knocked down."))
	game.shldup=False

def attack(torps_ok):
    # bad guy attacks us 
    # torps_ok == False forces use of phasers in an attack 
    # game could be over at this point, check
    if game.alldone:
	return
    attempt = False; ihurt = False;
    hitmax=0.0; hittot=0.0; chgfac=1.0
    where = "neither"
    if idebug:
	prout("=== ATTACK!")
    # Tholian gets to move before attacking 
    if game.tholian:
	movetholian()
    # if you have just entered the RNZ, you'll get a warning 
    if game.neutz: # The one chance not to be attacked 
	game.neutz = False
	return
    # commanders get a chance to tac-move towards you 
    if (((game.quadrant in game.state.kcmdr or game.state.kscmdr==game.quadrant) and not game.justin) or game.skill == SKILL_EMERITUS) and torps_ok:
	moveklings()
    # if no enemies remain after movement, we're done 
    if len(game.enemies)==0 or (len(game.enemies)==1 and thing == game.quadrant and not thing.angry):
	return
    # set up partial hits if attack happens during shield status change 
    pfac = 1.0/game.inshld
    if game.shldchg:
	chgfac = 0.25 + randreal(0.5)
    skip(1)
    # message verbosity control 
    if game.skill <= SKILL_FAIR:
	where = "sector"
    for enemy in game.enemies:
	if enemy.power < 0:
	    continue;	# too weak to attack 
	# compute hit strength and diminish shield power 
	r = randreal()
	# Increase chance of photon torpedos if docked or enemy energy is low 
	if game.condition == "docked":
	    r *= 0.25
	if enemy.power < 500:
	    r *= 0.25; 
	if enemy.type=='T' or (enemy.type=='?' and not thing.angry):
	    continue
	# different enemies have different probabilities of throwing a torp 
	usephasers = not torps_ok or \
	    (enemy.type == 'K' and r > 0.0005) or \
	    (enemy.type=='C' and r > 0.015) or \
	    (enemy.type=='R' and r > 0.3) or \
	    (enemy.type=='S' and r > 0.07) or \
	    (enemy.type=='?' and r > 0.05)
	if usephasers:	    # Enemy uses phasers 
	    if game.condition == "docked":
		continue; # Don't waste the effort! 
	    attempt = True; # Attempt to attack 
	    dustfac = randreal(0.8, 0.85)
	    hit = enemy.power*math.pow(dustfac,enemy.kavgd)
	    enemy.power *= 0.75
	else: # Enemy uses photon torpedo 
	    # We should be able to make the bearing() method work here
	    course = 1.90985*math.atan2(game.sector.j-enemy.location.j, enemy.location.i-game.sector.i)
	    hit = 0
	    proutn(_("***TORPEDO INCOMING"))
	    if not damaged(DSRSENS):
		proutn(_(" From ") + crmena(False, enemy.type, where, enemy.location))
	    attempt = True
	    prout("  ")
	    dispersion = (randreal()+randreal())*0.5 - 0.5
	    dispersion += 0.002*enemy.power*dispersion
	    hit = torpedo(enemy.location, course, dispersion, number=1, nburst=1)
	    if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)==0:
		finish(FWON); # Klingons did themselves in! 
	    if game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova or game.alldone:
		return # Supernova or finished 
	    if hit == None:
		continue
	# incoming phaser or torpedo, shields may dissipate it 
	if game.shldup or game.shldchg or game.condition=="docked":
	    # shields will take hits 
	    propor = pfac * game.shield
            if game.condition =="docked":
                propr *= 2.1
	    if propor < 0.1:
		propor = 0.1
	    hitsh = propor*chgfac*hit+1.0
	    absorb = 0.8*hitsh
	    if absorb > game.shield:
		absorb = game.shield
	    game.shield -= absorb
	    hit -= hitsh
	    # taking a hit blasts us out of a starbase dock 
	    if game.condition == "docked":
		dock(False)
	    # but the shields may take care of it 
	    if propor > 0.1 and hit < 0.005*game.energy:
		continue
	# hit from this opponent got through shields, so take damage 
	ihurt = True
	proutn(_("%d unit hit") % int(hit))
	if (damaged(DSRSENS) and usephasers) or game.skill<=SKILL_FAIR:
	    proutn(_(" on the ") + crmshp())
	if not damaged(DSRSENS) and usephasers:
	    prout(_(" from ") + crmena(False, enemy.type, where, enemy.location))
	skip(1)
	# Decide if hit is critical 
	if hit > hitmax:
	    hitmax = hit
	hittot += hit
	fry(hit)
	game.energy -= hit
    if game.energy <= 0:
	# Returning home upon your shield, not with it... 
	finish(FBATTLE)
	return
    if not attempt and game.condition == "docked":
	prout(_("***Enemies decide against attacking your ship."))
    percent = 100.0*pfac*game.shield+0.5
    if not ihurt:
	# Shields fully protect ship 
	proutn(_("Enemy attack reduces shield strength to "))
    else:
	# Emit message if starship suffered hit(s) 
	skip(1)
	proutn(_("Energy left %2d    shields ") % int(game.energy))
	if game.shldup:
	    proutn(_("up "))
	elif not damaged(DSHIELD):
	    proutn(_("down "))
	else:
	    proutn(_("damaged, "))
    prout(_("%d%%,   torpedoes left %d") % (percent, game.torps))
    # Check if anyone was hurt 
    if hitmax >= 200 or hittot >= 500:
	icas = randrange(int(hittot * 0.015))
	if icas >= 2:
	    skip(1)
	    prout(_("Mc Coy-  \"Sickbay to bridge.  We suffered %d casualties") % icas)
	    prout(_("   in that last attack.\""))
	    game.casual += icas
	    game.state.crew -= icas
    # After attack, reset average distance to enemies 
    for enemy in game.enemies:
	enemy.kavgd = enemy.kdist
    game.enemies.sort(lambda x, y: cmp(x.kdist, y.kdist))
    return
		
def deadkl(w, type, mv):
    "Kill a Klingon, Tholian, Romulan, or Thingy." 
    # Added mv to allow enemy to "move" before dying 
    proutn(crmena(True, type, "sector", mv))
    # Decide what kind of enemy it is and update appropriately 
    if type == 'R':
        # Chalk up a Romulan 
        game.state.galaxy[game.quadrant.i][game.quadrant.j].romulans -= 1
        game.irhere -= 1
        game.state.nromrem -= 1
    elif type == 'T':
        # Killed a Tholian 
        game.tholian = None
    elif type == '?':
        # Killed a Thingy
        global thing
        thing = None
    else:
        # Killed some type of Klingon 
        game.state.galaxy[game.quadrant.i][game.quadrant.j].klingons -= 1
        game.klhere -= 1
        if type == 'C':
            game.state.kcmdr.remove(game.quadrant)
            unschedule(FTBEAM)
            if game.state.kcmdr:
                schedule(FTBEAM, expran(1.0*game.incom/len(game.state.kcmdr)))
            if is_scheduled(FCDBAS) and game.battle == game.quadrant:
                unschedule(FCDBAS)    
        elif type ==  'K':
            game.state.remkl -= 1
        elif type ==  'S':
            game.state.nscrem -= 1
            game.state.kscmdr.invalidate()
            game.isatb = 0
            game.iscate = False
            unschedule(FSCMOVE)
            unschedule(FSCDBAS)
    # For each kind of enemy, finish message to player 
    prout(_(" destroyed."))
    if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)==0:
	return
    game.recompute()
    # Remove enemy ship from arrays describing local conditions
    for e in game.enemies:
	if e.location == w:
            e.move(None)
	    break
    return

def targetcheck(w):
    "Return None if target is invalid, otherwise return a course angle."
    if not w.valid_sector():
	huh()
	return None
    delta = coord()
    # FIXME: C code this was translated from is wacky -- why the sign reversal?
    delta.j = (w.j - game.sector.j);
    delta.i = (game.sector.i - w.i);
    if delta == coord(0, 0):
	skip(1)
	prout(_("Spock-  \"Bridge to sickbay.  Dr. McCoy,"))
	prout(_("  I recommend an immediate review of"))
	prout(_("  the Captain's psychological profile.\""))
	scanner.chew()
	return None
    return delta.bearing()

def photon():
    "Launch photon torpedo."
    course = []
    game.ididit = False
    if damaged(DPHOTON):
	prout(_("Photon tubes damaged."))
	scanner.chew()
	return
    if game.torps == 0:
	prout(_("No torpedoes left."))
	scanner.chew()
	return
    # First, get torpedo count
    while True:
        scanner.next()
	if scanner.token == "IHALPHA":
	    huh()
	    return
	elif scanner.token == "IHEOL" or not scanner.waiting():
	    prout(_("%d torpedoes left.") % game.torps)
            scanner.chew()
	    proutn(_("Number of torpedoes to fire- "))
            continue	# Go back around to get a number
	else: # key == "IHREAL"
	    n = scanner.int()
	    if n <= 0: # abort command 
		scanner.chew()
		return
	    if n > MAXBURST:
		scanner.chew()
		prout(_("Maximum of %d torpedoes per burst.") % MAXBURST)
		return
            if n > game.torps:
                scanner.chew()	# User requested more torps than available
                continue	# Go back around
            break	# All is good, go to next stage
    # Next, get targets
    target = []
    for i in range(n):
	key = scanner.next()
	if i==0 and key == "IHEOL":
	    break;	# no coordinate waiting, we will try prompting 
	if i==1 and key == "IHEOL":
	    # direct all torpedoes at one target 
	    while i < n:
		target.append(target[0])
		course.append(course[0])
		i += 1
	    break
        scanner.push(scanner.token)
        target.append(scanner.getcoord())
        if target[-1] == None:
            return
        course.append(targetcheck(target[-1]))
        if course[-1] == None:
	    return
    scanner.chew()
    if len(target) == 0:
	# prompt for each one 
	for i in range(n):
	    proutn(_("Target sector for torpedo number %d- ") % (i+1))
	    scanner.chew()
            target.append(scanner.getcoord())
            if target[-1] == None:
                return
            course.append(targetcheck(target[-1]))
            if course[-1] == None:
                return
    game.ididit = True
    # Loop for moving <n> torpedoes 
    for i in range(n):
	if game.condition != "docked":
	    game.torps -= 1
	dispersion = (randreal()+randreal())*0.5 -0.5
	if math.fabs(dispersion) >= 0.47:
	    # misfire! 
	    dispersion *= randreal(1.2, 2.2)
	    if n > 0:
		prouts(_("***TORPEDO NUMBER %d MISFIRES") % (i+1))
	    else:
		prouts(_("***TORPEDO MISFIRES."))
	    skip(1)
	    if i < n:
		prout(_("  Remainder of burst aborted."))
	    if withprob(0.2):
		prout(_("***Photon tubes damaged by misfire."))
		game.damage[DPHOTON] = game.damfac * randreal(1.0, 3.0)
	    break
	if game.shldup or game.condition == "docked":
	    dispersion *= 1.0 + 0.0001*game.shield
	torpedo(game.sector, course[i], dispersion, number=i, nburst=n)
	if game.alldone or game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova:
	    return
    if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)==0:
	finish(FWON);

def overheat(rpow):
    "Check for phasers overheating."
    if rpow > 1500:
        checkburn = (rpow-1500.0)*0.00038
        if withprob(checkburn):
	    prout(_("Weapons officer Sulu-  \"Phasers overheated, sir.\""))
	    game.damage[DPHASER] = game.damfac* randreal(1.0, 2.0) * (1.0+checkburn)

def checkshctrl(rpow):
    "Check shield control."
    skip(1)
    if withprob(0.998):
	prout(_("Shields lowered."))
	return False
    # Something bad has happened 
    prouts(_("***RED ALERT!  RED ALERT!"))
    skip(2)
    hit = rpow*game.shield/game.inshld
    game.energy -= rpow+hit*0.8
    game.shield -= hit*0.2
    if game.energy <= 0.0:
	prouts(_("Sulu-  \"Captain! Shield malf***********************\""))
	skip(1)
	stars()
	finish(FPHASER)
	return True
    prouts(_("Sulu-  \"Captain! Shield malfunction! Phaser fire contained!\""))
    skip(2)
    prout(_("Lt. Uhura-  \"Sir, all decks reporting damage.\""))
    icas = randrange(int(hit*0.012))
    skip(1)
    fry(0.8*hit)
    if icas:
	skip(1)
	prout(_("McCoy to bridge- \"Severe radiation burns, Jim."))
	prout(_("  %d casualties so far.\"") % icas)
	game.casual += icas
	game.state.crew -= icas
    skip(1)
    prout(_("Phaser energy dispersed by shields."))
    prout(_("Enemy unaffected."))
    overheat(rpow)
    return True;

def hittem(hits):
    "Register a phaser hit on Klingons and Romulans."
    nenhr2 = len(game.enemies); kk=0
    w = coord()
    skip(1)
    for (k, wham) in enumerate(hits):
	if wham==0:
	    continue
	dustfac = randreal(0.9, 1.0)
	hit = wham*math.pow(dustfac,game.enemies[kk].kdist)
	kpini = game.enemies[kk].power
	kp = math.fabs(kpini)
	if PHASEFAC*hit < kp:
	    kp = PHASEFAC*hit
        if game.enemies[kk].power < 0:
            game.enemies[kk].power -= -kp
        else:
            game.enemies[kk].power -= kp
	kpow = game.enemies[kk].power
	w = game.enemies[kk].location
	if hit > 0.005:
	    if not damaged(DSRSENS):
		boom(w)
	    proutn(_("%d unit hit on ") % int(hit))
	else:
	    proutn(_("Very small hit on "))
	ienm = game.quad[w.i][w.j]
	if ienm=='?':
	    thing.angry = True
	proutn(crmena(False, ienm, "sector", w))
	skip(1)
	if kpow == 0:
	    deadkl(w, ienm, w)
	    if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)==0:
		finish(FWON);		
	    if game.alldone:
		return
	    kk -= 1	# don't do the increment
            continue
	else: # decide whether or not to emasculate klingon 
	    if kpow>0 and withprob(0.9) and kpow <= randreal(0.4, 0.8)*kpini:
		prout(_("***Mr. Spock-  \"Captain, the vessel at Sector %s")%w)
		prout(_("   has just lost its firepower.\""))
		game.enemies[kk].power = -kpow
        kk += 1
    return

def phasers():
    "Fire phasers at bad guys."
    hits = []
    kz = 0; k = 1; irec=0 # Cheating inhibitor 
    ifast = False; no = False; itarg = True; msgflag = True; rpow=0
    automode = "NOTSET"
    key=0
    skip(1)
    # SR sensors and Computer are needed for automode 
    if damaged(DSRSENS) or damaged(DCOMPTR):
	itarg = False
    if game.condition == "docked":
	prout(_("Phasers can't be fired through base shields."))
	scanner.chew()
	return
    if damaged(DPHASER):
	prout(_("Phaser control damaged."))
	scanner.chew()
	return
    if game.shldup:
	if damaged(DSHCTRL):
	    prout(_("High speed shield control damaged."))
	    scanner.chew()
	    return
	if game.energy <= 200.0:
	    prout(_("Insufficient energy to activate high-speed shield control."))
	    scanner.chew()
	    return
	prout(_("Weapons Officer Sulu-  \"High-speed shield control enabled, sir.\""))
	ifast = True
    # Original code so convoluted, I re-did it all
    # (That was Tom Almy talking about the C code, I think -- ESR)
    while automode=="NOTSET":
	key=scanner.next()
	if key == "IHALPHA":
	    if scanner.sees("manual"):
		if len(game.enemies)==0:
		    prout(_("There is no enemy present to select."))
		    scanner.chew()
		    key = "IHEOL"
		    automode="AUTOMATIC"
		else:
		    automode = "MANUAL"
		    key = scanner.next()
	    elif scanner.sees("automatic"):
		if (not itarg) and len(game.enemies) != 0:
		    automode = "FORCEMAN"
		else:
		    if len(game.enemies)==0:
			prout(_("Energy will be expended into space."))
		    automode = "AUTOMATIC"
		    key = scanner.next()
	    elif scanner.sees("no"):
		no = True
	    else:
		huh()
		return
	elif key == "IHREAL":
	    if len(game.enemies)==0:
		prout(_("Energy will be expended into space."))
		automode = "AUTOMATIC"
	    elif not itarg:
		automode = "FORCEMAN"
	    else:
		automode = "AUTOMATIC"
	else:
	    # "IHEOL" 
	    if len(game.enemies)==0:
		prout(_("Energy will be expended into space."))
		automode = "AUTOMATIC"
	    elif not itarg:
		automode = "FORCEMAN"
	    else: 
		proutn(_("Manual or automatic? "))
                scanner.chew()
    avail = game.energy
    if ifast:
        avail -= 200.0
    if automode == "AUTOMATIC":
	if key == "IHALPHA" and scanner.sees("no"):
	    no = True
	    key = scanner.next()
	if key != "IHREAL" and len(game.enemies) != 0:
	    prout(_("Phasers locked on target. Energy available: %.2f")%avail)
	irec=0
        while True:
	    scanner.chew()
	    if not kz:
		for i in range(len(game.enemies)):
		    irec += math.fabs(game.enemies[i].power)/(PHASEFAC*math.pow(0.90,game.enemies[i].kdist))*randreal(1.01, 1.06) + 1.0
	    kz=1
	    proutn(_("%d units required. ") % irec)
	    scanner.chew()
	    proutn(_("Units to fire= "))
	    key = scanner.next()
	    if key!="IHREAL":
		return
	    rpow = scanner.real
	    if rpow > avail:
		proutn(_("Energy available= %.2f") % avail)
		skip(1)
		key = "IHEOL"
            if not rpow > avail:
                break
	if rpow<=0:
	    # chicken out 
	    scanner.chew()
	    return
        key=scanner.next()
	if key == "IHALPHA" and scanner.sees("no"):
	    no = True
	if ifast:
	    game.energy -= 200; # Go and do it! 
	    if checkshctrl(rpow):
		return
	scanner.chew()
	game.energy -= rpow
	extra = rpow
	if len(game.enemies):
	    extra = 0.0
	    powrem = rpow
	    for i in range(len(game.enemies)):
		hits.append(0.0)
		if powrem <= 0:
		    continue
		hits[i] = math.fabs(game.enemies[i].power)/(PHASEFAC*math.pow(0.90,game.enemies[i].kdist))
		over = randreal(1.01, 1.06) * hits[i]
		temp = powrem
		powrem -= hits[i] + over
		if powrem <= 0 and temp < hits[i]:
		    hits[i] = temp
		if powrem <= 0:
		    over = 0.0
		extra += over
	    if powrem > 0.0:
		extra += powrem
	    hittem(hits)
	    game.ididit = True
	if extra > 0 and not game.alldone:
	    if game.tholian:
		proutn(_("*** Tholian web absorbs "))
		if len(game.enemies)>0:
		    proutn(_("excess "))
		prout(_("phaser energy."))
	    else:
		prout(_("%d expended on empty space.") % int(extra))
    elif automode == "FORCEMAN":
	scanner.chew()
	key = "IHEOL"
	if damaged(DCOMPTR):
	    prout(_("Battle computer damaged, manual fire only."))
	else:
	    skip(1)
	    prouts(_("---WORKING---"))
	    skip(1)
	    prout(_("Short-range-sensors-damaged"))
	    prout(_("Insufficient-data-for-automatic-phaser-fire"))
	    prout(_("Manual-fire-must-be-used"))
	    skip(1)
    elif automode == "MANUAL":
	rpow = 0.0
        for k in range(len(game.enemies)):
	    aim = game.enemies[k].location
	    ienm = game.quad[aim.i][aim.j]
	    if msgflag:
		proutn(_("Energy available= %.2f") % (avail-0.006))
		skip(1)
		msgflag = False
		rpow = 0.0
	    if damaged(DSRSENS) and \
               not game.sector.distance(aim)<2**0.5 and ienm in ('C', 'S'):
		prout(cramen(ienm) + _(" can't be located without short range scan."))
		scanner.chew()
		key = "IHEOL"
		hits[k] = 0; # prevent overflow -- thanks to Alexei Voitenko 
		k += 1
		continue
	    if key == "IHEOL":
		scanner.chew()
		if itarg and k > kz:
		    irec=(abs(game.enemies[k].power)/(PHASEFAC*math.pow(0.9,game.enemies[k].kdist))) *	randreal(1.01, 1.06) + 1.0
		kz = k
		proutn("(")
		if not damaged(DCOMPTR):
		    proutn("%d" % irec)
		else:
		    proutn("??")
		proutn(")  ")
		proutn(_("units to fire at %s-  ") % crmena(False, ienm, "sector", aim))		
		key = scanner.next()
	    if key == "IHALPHA" and scanner.sees("no"):
		no = True
		key = scanner.next()
		continue
	    if key == "IHALPHA":
		huh()
		return
	    if key == "IHEOL":
		if k==1: # Let me say I'm baffled by this 
		    msgflag = True
		continue
	    if scanner.real < 0:
		# abort out 
		scanner.chew()
		return
	    hits[k] = scanner.real
	    rpow += scanner.real
	    # If total requested is too much, inform and start over 
            if rpow > avail:
		prout(_("Available energy exceeded -- try again."))
		scanner.chew()
		return
	    key = scanner.next(); # scan for next value 
	    k += 1
	if rpow == 0.0:
	    # zero energy -- abort 
	    scanner.chew()
	    return
	if key == "IHALPHA" and scanner.sees("no"):
	    no = True
	game.energy -= rpow
	scanner.chew()
	if ifast:
	    game.energy -= 200.0
	    if checkshctrl(rpow):
		return
	hittem(hits)
	game.ididit = True
     # Say shield raised or malfunction, if necessary 
    if game.alldone:
	return
    if ifast:
	skip(1)
	if no == 0:
	    if withprob(0.01):
		prout(_("Sulu-  \"Sir, the high-speed shield control has malfunctioned . . ."))
		prouts(_("         CLICK   CLICK   POP  . . ."))
		prout(_(" No response, sir!"))
		game.shldup = False
	    else:
		prout(_("Shields raised."))
	else:
	    game.shldup = False
    overheat(rpow);

# Code from events,c begins here.

# This isn't a real event queue a la BSD Trek yet -- you can only have one 
# event of each type active at any given time.  Mostly these means we can 
# only have one FDISTR/FENSLV/FREPRO sequence going at any given time
# BSD Trek, from which we swiped the idea, can have up to 5.

def unschedule(evtype):
    "Remove an event from the schedule."
    game.future[evtype].date = FOREVER
    return game.future[evtype]

def is_scheduled(evtype):
    "Is an event of specified type scheduled."
    return game.future[evtype].date != FOREVER

def scheduled(evtype):
    "When will this event happen?"
    return game.future[evtype].date

def schedule(evtype, offset):
    "Schedule an event of specified type."
    game.future[evtype].date = game.state.date + offset
    return game.future[evtype]

def postpone(evtype, offset):
    "Postpone a scheduled event."
    game.future[evtype].date += offset

def cancelrest():
    "Rest period is interrupted by event."
    if game.resting:
	skip(1)
	proutn(_("Mr. Spock-  \"Captain, shall we cancel the rest period?\""))
	if ja() == True:
	    game.resting = False
	    game.optime = 0.0
	    return True
    return False

def events():
    "Run through the event queue looking for things to do."
    i=0
    fintim = game.state.date + game.optime; yank=0
    ictbeam = False; istract = False
    w = coord(); hold = coord()
    ev = event(); ev2 = event()

    def tractorbeam(yank):
        "Tractor-beaming cases merge here." 
        announce()
        game.optime = (10.0/(7.5*7.5))*yank # 7.5 is yank rate (warp 7.5) 
        skip(1)
        prout("***" + crmshp() + _(" caught in long range tractor beam--"))
        # If Kirk & Co. screwing around on planet, handle 
        atover(True) # atover(true) is Grab 
        if game.alldone:
            return
        if game.icraft: # Caught in Galileo? 
            finish(FSTRACTOR)
            return
        # Check to see if shuttle is aboard 
        if game.iscraft == "offship":
            skip(1)
            if withprob(0.5):
                prout(_("Galileo, left on the planet surface, is captured"))
                prout(_("by aliens and made into a flying McDonald's."))
                game.damage[DSHUTTL] = -10
                game.iscraft = "removed"
            else:
                prout(_("Galileo, left on the planet surface, is well hidden."))
        if evcode == FSPY:
            game.quadrant = game.state.kscmdr
        else:
            game.quadrant = game.state.kcmdr[i]
        game.sector = randplace(QUADSIZE)
        prout(crmshp() + _(" is pulled to Quadrant %s, Sector %s") \
               % (game.quadrant, game.sector))
        if game.resting:
            prout(_("(Remainder of rest/repair period cancelled.)"))
            game.resting = False
        if not game.shldup:
            if not damaged(DSHIELD) and game.shield > 0:
                doshield(shraise=True) # raise shields 
                game.shldchg = False
            else:
                prout(_("(Shields not currently useable.)"))
        newqad()
        # Adjust finish time to time of tractor beaming 
        fintim = game.state.date+game.optime
        attack(torps_ok=False)
        if not game.state.kcmdr:
            unschedule(FTBEAM)
        else: 
            schedule(FTBEAM, game.optime+expran(1.5*game.intime/len(game.state.kcmdr)))

    def destroybase():
        "Code merges here for any commander destroying a starbase." 
        # Not perfect, but will have to do 
        # Handle case where base is in same quadrant as starship 
        if game.battle == game.quadrant:
            game.state.chart[game.battle.i][game.battle.j].starbase = False
            game.quad[game.base.i][game.base.j] = '.'
            game.base.invalidate()
            newcnd()
            skip(1)
            prout(_("Spock-  \"Captain, I believe the starbase has been destroyed.\""))
        elif game.state.baseq and communicating():
            # Get word via subspace radio 
            announce()
            skip(1)
            prout(_("Lt. Uhura-  \"Captain, Starfleet Command reports that"))
            proutn(_("   the starbase in Quadrant %s has been destroyed by") % game.battle)
            if game.isatb == 2: 
                prout(_("the Klingon Super-Commander"))
            else:
                prout(_("a Klingon Commander"))
            game.state.chart[game.battle.i][game.battle.j].starbase = False
        # Remove Starbase from galaxy 
        game.state.galaxy[game.battle.i][game.battle.j].starbase = False
        game.state.baseq = filter(lambda x: x != game.battle, game.state.baseq)
        if game.isatb == 2:
            # reinstate a commander's base attack 
            game.battle = hold
            game.isatb = 0
        else:
            game.battle.invalidate()
    if idebug:
	prout("=== EVENTS from %.2f to %.2f:" % (game.state.date, fintim))
	for i in range(1, NEVENTS):
	    if   i == FSNOVA:  proutn("=== Supernova       ")
	    elif i == FTBEAM:  proutn("=== T Beam          ")
	    elif i == FSNAP:   proutn("=== Snapshot        ")
	    elif i == FBATTAK: proutn("=== Base Attack     ")
	    elif i == FCDBAS:  proutn("=== Base Destroy    ")
	    elif i == FSCMOVE: proutn("=== SC Move         ")
	    elif i == FSCDBAS: proutn("=== SC Base Destroy ")
	    elif i == FDSPROB: proutn("=== Probe Move      ")
	    elif i == FDISTR:  proutn("=== Distress Call   ")
	    elif i == FENSLV:  proutn("=== Enslavement     ")
	    elif i == FREPRO:  proutn("=== Klingon Build   ")
	    if is_scheduled(i):
		prout("%.2f" % (scheduled(i)))
	    else:
		prout("never")
    radio_was_broken = damaged(DRADIO)
    hold.i = hold.j = 0
    while True:
	# Select earliest extraneous event, evcode==0 if no events 
	evcode = FSPY
	if game.alldone:
	    return
	datemin = fintim
	for l in range(1, NEVENTS):
	    if game.future[l].date < datemin:
		evcode = l
		if idebug:
		    prout("== Event %d fires" % evcode)
		datemin = game.future[l].date
	xtime = datemin-game.state.date
	game.state.date = datemin
	# Decrement Federation resources and recompute remaining time 
	game.state.remres -= (game.state.remkl+4*len(game.state.kcmdr))*xtime
        game.recompute()
	if game.state.remtime <=0:
	    finish(FDEPLETE)
	    return
	# Any crew left alive? 
	if game.state.crew <=0:
	    finish(FCREW)
	    return
	# Is life support adequate? 
	if damaged(DLIFSUP) and game.condition != "docked":
	    if game.lsupres < xtime and game.damage[DLIFSUP] > game.lsupres:
		finish(FLIFESUP)
		return
	    game.lsupres -= xtime
	    if game.damage[DLIFSUP] <= xtime:
		game.lsupres = game.inlsr
	# Fix devices 
	repair = xtime
	if game.condition == "docked":
	    repair /= DOCKFAC
	# Don't fix Deathray here 
	for l in range(NDEVICES):
	    if game.damage[l] > 0.0 and l != DDRAY:
                if game.damage[l]-repair > 0.0:
                    game.damage[l] -= repair
                else:
                    game.damage[l] = 0.0
	# If radio repaired, update star chart and attack reports 
	if radio_was_broken and not damaged(DRADIO):
	    prout(_("Lt. Uhura- \"Captain, the sub-space radio is working and"))
	    prout(_("   surveillance reports are coming in."))
	    skip(1)
	    if not game.iseenit:
		attackreport(False)
		game.iseenit = True
	    rechart()
	    prout(_("   The star chart is now up to date.\""))
	    skip(1)
	# Cause extraneous event EVCODE to occur 
	game.optime -= xtime
	if evcode == FSNOVA: # Supernova 
	    announce()
	    supernova(None)
	    schedule(FSNOVA, expran(0.5*game.intime))
	    if game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova:
		return
	elif evcode == FSPY: # Check with spy to see if SC should tractor beam 
	    if game.state.nscrem == 0 or \
		ictbeam or istract or \
                game.condition=="docked" or game.isatb==1 or game.iscate:
		return
	    if game.ientesc or \
		(game.energy<2000 and game.torps<4 and game.shield < 1250) or \
		(damaged(DPHASER) and (damaged(DPHOTON) or game.torps<4)) or \
		(damaged(DSHIELD) and \
		 (game.energy < 2500 or damaged(DPHASER)) and \
                 (game.torps < 5 or damaged(DPHOTON))):
		# Tractor-beam her! 
		istract = ictbeam = True
                tractorbeam((game.state.kscmdr-game.quadrant).distance())
	    else:
		return
	elif evcode == FTBEAM: # Tractor beam 
            if not game.state.kcmdr:
                unschedule(FTBEAM)
                continue
            i = randrange(len(game.state.kcmdr))
            yank = (game.state.kcmdr[i]-game.quadrant).distance()
            if istract or game.condition == "docked" or yank == 0:
                # Drats! Have to reschedule 
                schedule(FTBEAM, 
                         game.optime + expran(1.5*game.intime/len(game.state.kcmdr)))
                continue
            ictbeam = True
            tractorbeam(yank)
	elif evcode == FSNAP: # Snapshot of the universe (for time warp) 
	    game.snapsht = copy.deepcopy(game.state)
	    game.state.snap = True
	    schedule(FSNAP, expran(0.5 * game.intime))
	elif evcode == FBATTAK: # Commander attacks starbase 
	    if not game.state.kcmdr or not game.state.baseq:
		# no can do 
		unschedule(FBATTAK)
		unschedule(FCDBAS)
                continue
            try:
                for ibq in game.state.baseq:
                   for cmdr in game.state.kcmdr: 
                       if ibq == cmdr and ibq != game.quadrant and ibq != game.state.kscmdr:
                           raise ibq
                else:
                    # no match found -- try later 
                    schedule(FBATTAK, expran(0.3*game.intime))
                    unschedule(FCDBAS)
                    continue
            except coord:
                pass
	    # commander + starbase combination found -- launch attack 
	    game.battle = ibq
	    schedule(FCDBAS, randreal(1.0, 4.0))
	    if game.isatb: # extra time if SC already attacking 
		postpone(FCDBAS, scheduled(FSCDBAS)-game.state.date)
	    game.future[FBATTAK].date = game.future[FCDBAS].date + expran(0.3*game.intime)
	    game.iseenit = False
            if not communicating():
		continue # No warning :-( 
	    game.iseenit = True
	    announce()
	    skip(1)
	    prout(_("Lt. Uhura-  \"Captain, the starbase in Quadrant %s") % game.battle)
	    prout(_("   reports that it is under attack and that it can"))
	    prout(_("   hold out only until stardate %d.\"") % (int(scheduled(FCDBAS))))
	    if cancelrest():
                return
	elif evcode == FSCDBAS: # Supercommander destroys base 
	    unschedule(FSCDBAS)
	    game.isatb = 2
	    if not game.state.galaxy[game.state.kscmdr.i][game.state.kscmdr.j].starbase: 
		continue # WAS RETURN! 
	    hold = game.battle
	    game.battle = game.state.kscmdr
	    destroybase()
	elif evcode == FCDBAS: # Commander succeeds in destroying base 
	    if evcode==FCDBAS:
		unschedule(FCDBAS)
                if not game.state.baseq() \
                       or not game.state.galaxy[game.battle.i][game.battle.j].starbase:
		    game.battle.invalidate()
                    continue
		# find the lucky pair 
		for cmdr in game.state.kcmdr:
		    if cmdr == game.battle: 
			break
                else:
		    # No action to take after all 
		    continue
            destroybase()
	elif evcode == FSCMOVE: # Supercommander moves 
	    schedule(FSCMOVE, 0.2777)
	    if not game.ientesc and not istract and game.isatb != 1 and \
                   (not game.iscate or not game.justin): 
		supercommander()
	elif evcode == FDSPROB: # Move deep space probe 
	    schedule(FDSPROB, 0.01)
            if not game.probe.next():
		if not game.probe.quadrant().valid_quadrant() or \
		    game.state.galaxy[game.probe.quadrant().i][game.probe.quadrant().j].supernova:
		    # Left galaxy or ran into supernova
                    if communicating():
			announce()
			skip(1)
			proutn(_("Lt. Uhura-  \"The deep space probe "))
			if not game.probe.quadrant().valid_quadrant():
			    prout(_("has left the galaxy.\""))
			else:
			    prout(_("is no longer transmitting.\""))
		    unschedule(FDSPROB)
		    continue
                if communicating():
		    #announce()
		    skip(1)
		    prout(_("Lt. Uhura-  \"The deep space probe is now in Quadrant %s.\"") % game.probe.quadrant())
	    pdest = game.state.galaxy[game.probe.quadrant().i][game.probe.quadrant().j]
	    if communicating():
		chp = game.state.chart[game.probe.quadrant().i][game.probe.quadrant().j]
		chp.klingons = pdest.klingons
		chp.starbase = pdest.starbase
		chp.stars = pdest.stars
		pdest.charted = True
	    game.probe.moves -= 1 # One less to travel
	    if game.probe.arrived() and game.isarmed and pdest.stars:
		supernova(game.probe)		# fire in the hole!
		unschedule(FDSPROB)
		if game.state.galaxy[game.quadrant().i][game.quadrant().j].supernova: 
		    return
	elif evcode == FDISTR: # inhabited system issues distress call 
	    unschedule(FDISTR)
	    # try a whole bunch of times to find something suitable 
            for i in range(100):
		# need a quadrant which is not the current one,
		# which has some stars which are inhabited and
		# not already under attack, which is not
		# supernova'ed, and which has some Klingons in it
		w = randplace(GALSIZE)
		q = game.state.galaxy[w.i][w.j]
                if not (game.quadrant == w or q.planet == None or \
		      not q.planet.inhabited or \
		      q.supernova or q.status!="secure" or q.klingons<=0):
                    break
            else:
		# can't seem to find one; ignore this call 
		if idebug:
		    prout("=== Couldn't find location for distress event.")
		continue
	    # got one!!  Schedule its enslavement 
	    ev = schedule(FENSLV, expran(game.intime))
	    ev.quadrant = w
	    q.status = "distressed"
	    # tell the captain about it if we can 
	    if communicating():
		prout(_("Uhura- Captain, %s in Quadrant %s reports it is under attack") \
                        % (q.planet, `w`))
		prout(_("by a Klingon invasion fleet."))
		if cancelrest():
		    return
	elif evcode == FENSLV:		# starsystem is enslaved 
	    ev = unschedule(FENSLV)
	    # see if current distress call still active 
	    q = game.state.galaxy[ev.quadrant.i][ev.quadrant.j]
	    if q.klingons <= 0:
		q.status = "secure"
		continue
	    q.status = "enslaved"

	    # play stork and schedule the first baby 
	    ev2 = schedule(FREPRO, expran(2.0 * game.intime))
	    ev2.quadrant = ev.quadrant

	    # report the disaster if we can 
	    if communicating():
		prout(_("Uhura- We've lost contact with starsystem %s") % \
                        q.planet)
		prout(_("in Quadrant %s.\n") % ev.quadrant)
	elif evcode == FREPRO:		# Klingon reproduces 
	    # If we ever switch to a real event queue, we'll need to
	    # explicitly retrieve and restore the x and y.
	    ev = schedule(FREPRO, expran(1.0 * game.intime))
	    # see if current distress call still active 
	    q = game.state.galaxy[ev.quadrant.i][ev.quadrant.j]
	    if q.klingons <= 0:
		q.status = "secure"
		continue
	    if game.state.remkl >=MAXKLGAME:
		continue		# full right now 
	    # reproduce one Klingon 
	    w = ev.quadrant
            m = coord()
	    if game.klhere >= MAXKLQUAD:
                try:
                    # this quadrant not ok, pick an adjacent one 
                    for m.i in range(w.i - 1, w.i + 2):
                        for m.j in range(w.j - 1, w.j + 2):
                            if not m.valid_quadrant():
                                continue
                            q = game.state.galaxy[m.i][m.j]
                            # check for this quad ok (not full & no snova) 
                            if q.klingons >= MAXKLQUAD or q.supernova:
                                continue
                            raise "FOUNDIT"
                    else:
                        continue	# search for eligible quadrant failed
                except "FOUNDIT":
                    w = m
	    # deliver the child 
	    game.state.remkl += 1
	    q.klingons += 1
	    if game.quadrant == w:
                game.klhere += 1
		game.enemies.append(newkling())
	    # recompute time left
            game.recompute()
	    if communicating():
		if game.quadrant == w:
		    prout(_("Spock- sensors indicate the Klingons have"))
		    prout(_("launched a warship from %s.") % q.planet)
		else:
		    prout(_("Uhura- Starfleet reports increased Klingon activity"))
		    if q.planet != None:
			proutn(_("near %s ") % q.planet)
		    prout(_("in Quadrant %s.") % w)
				
def wait():
    "Wait on events."
    game.ididit = False
    while True:
	key = scanner.next()
	if key  != "IHEOL":
	    break
	proutn(_("How long? "))
    scanner.chew()
    if key != "IHREAL":
	huh()
	return
    origTime = delay = scanner.real
    if delay <= 0.0:
	return
    if delay >= game.state.remtime or len(game.enemies) != 0:
	proutn(_("Are you sure? "))
	if ja() == False:
	    return
    # Alternate resting periods (events) with attacks 
    game.resting = True
    while True:
	if delay <= 0:
	    game.resting = False
	if not game.resting:
	    prout(_("%d stardates left.") % int(game.state.remtime))
	    return
	temp = game.optime = delay
	if len(game.enemies):
	    rtime = randreal(1.0, 2.0)
	    if rtime < temp:
		temp = rtime
	    game.optime = temp
	if game.optime < delay:
	    attack(torps_ok=False)
	if game.alldone:
	    return
	events()
	game.ididit = True
	if game.alldone:
	    return
	delay -= temp
	# Repair Deathray if long rest at starbase 
	if origTime-delay >= 9.99 and game.condition == "docked":
	    game.damage[DDRAY] = 0.0
	# leave if quadrant supernovas
        if game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova:
            break
    game.resting = False
    game.optime = 0

def nova(nov):
    "Star goes nova." 
    course = (0.0, 10.5, 12.0, 1.5, 9.0, 0.0, 3.0, 7.5, 6.0, 4.5)
    newc = coord(); neighbor = coord(); bump = coord(0, 0)
    if withprob(0.05):
	# Wow! We've supernova'ed 
	supernova(game.quadrant)
	return
    # handle initial nova 
    game.quad[nov.i][nov.j] = '.'
    prout(crmena(False, '*', "sector", nov) + _(" novas."))
    game.state.galaxy[game.quadrant.i][game.quadrant.j].stars -= 1
    game.state.starkl += 1
    # Set up queue to recursively trigger adjacent stars 
    hits = [nov]
    kount = 0
    while hits:
        offset = coord()
        start = hits.pop()
        for offset.i in range(-1, 1+1):
            for offset.j in range(-1, 1+1):
                if offset.j==0 and offset.i==0:
                    continue
                neighbor = start + offset
                if not neighbor.valid_sector():
                    continue
                iquad = game.quad[neighbor.i][neighbor.j]
                # Empty space ends reaction
                if iquad in ('.', '?', ' ', 'T', '#'):
                    pass
                elif iquad == '*': # Affect another star 
                    if withprob(0.05):
                        # This star supernovas 
                        supernova(game.quadrant)
                        return
                    else:
                        hits.append(neighbor)
			game.state.galaxy[game.quadrant.i][game.quadrant.j].stars -= 1
			game.state.starkl += 1
			proutn(crmena(True, '*', "sector", neighbor))
			prout(_(" novas."))
                        game.quad[neighbor.i][neighbor.j] = '.'
                        kount += 1
                elif iquad in ('P', '@'): # Destroy planet 
                    game.state.galaxy[game.quadrant.i][game.quadrant.j].planet = None
                    if iquad == 'P':
                        game.state.nplankl += 1
                    else:
                        game.state.worldkl += 1
                    prout(crmena(True, 'B', "sector", neighbor) + _(" destroyed."))
                    game.iplnet.pclass = "destroyed"
                    game.iplnet = None
                    game.plnet.invalidate()
                    if game.landed:
                        finish(FPNOVA)
                        return
                    game.quad[neighbor.i][neighbor.j] = '.'
                elif iquad == 'B': # Destroy base 
                    game.state.galaxy[game.quadrant.i][game.quadrant.j].starbase = False
                    game.state.baseq = filter(lambda x: x!= game.quadrant, game.state.baseq)
                    game.base.invalidate()
                    game.state.basekl += 1
                    newcnd()
                    prout(crmena(True, 'B', "sector", neighbor) + _(" destroyed."))
                    game.quad[neighbor.i][neighbor.j] = '.'
                elif iquad in ('E', 'F'): # Buffet ship 
                    prout(_("***Starship buffeted by nova."))
                    if game.shldup:
                        if game.shield >= 2000.0:
                            game.shield -= 2000.0
                        else:
                            diff = 2000.0 - game.shield
                            game.energy -= diff
                            game.shield = 0.0
                            game.shldup = False
                            prout(_("***Shields knocked out."))
                            game.damage[DSHIELD] += 0.005*game.damfac*randreal()*diff
                    else:
                        game.energy -= 2000.0
                    if game.energy <= 0:
                        finish(FNOVA)
                        return
                    # add in course nova contributes to kicking starship
                    bump += (game.sector-hits[mm]).sgn()
                elif iquad == 'K': # kill klingon 
                    deadkl(neighbor, iquad, neighbor)
                elif iquad in ('C','S','R'): # Damage/destroy big enemies 
                    for ll in range(len(game.enemies)):
                        if game.enemies[ll].location == neighbor:
                            break
                    game.enemies[ll].power -= 800.0 # If firepower is lost, die 
                    if game.enemies[ll].power <= 0.0:
                        deadkl(neighbor, iquad, neighbor)
                        break
                    newc = neighbor + neighbor - hits[mm]
                    proutn(crmena(True, iquad, "sector", neighbor) + _(" damaged"))
                    if not newc.valid_sector():
                        # can't leave quadrant 
                        skip(1)
                        break
                    iquad1 = game.quad[newc.i][newc.j]
                    if iquad1 == ' ':
                        proutn(_(", blasted into ") + crmena(False, ' ', "sector", newc))
                        skip(1)
                        deadkl(neighbor, iquad, newc)
                        break
                    if iquad1 != '.':
                        # can't move into something else 
                        skip(1)
                        break
                    proutn(_(", buffeted to Sector %s") % newc)
                    game.quad[neighbor.i][neighbor.j] = '.'
                    game.quad[newc.i][newc.j] = iquad
                    game.enemies[ll].move(newc)
    # Starship affected by nova -- kick it away. 
    dist = kount*0.1
    direc = course[3*(bump.i+1)+bump.j+2]
    if direc == 0.0:
	dist = 0.0
    if dist == 0.0:
	return
    course = course(bearing=direc, distance=dist)
    game.optime = course.time(warp=4)
    skip(1)
    prout(_("Force of nova displaces starship."))
    imove(course, noattack=True)
    game.optime = course.time(warp=4)
    return
	
def supernova(w):
    "Star goes supernova."
    num = 0; npdead = 0
    if w != None: 
	nq = copy.copy(w)
    else:
	# Scheduled supernova -- select star at random. 
	stars = 0
        nq = coord()
	for nq.i in range(GALSIZE):
	    for nq.j in range(GALSIZE):
		stars += game.state.galaxy[nq.i][nq.j].stars
	if stars == 0:
	    return # nothing to supernova exists 
	num = randrange(stars) + 1
	for nq.i in range(GALSIZE):
	    for nq.j in range(GALSIZE):
		num -= game.state.galaxy[nq.i][nq.j].stars
		if num <= 0:
		    break
	    if num <=0:
		break
	if idebug:
	    proutn("=== Super nova here?")
	    if ja() == True:
		nq = game.quadrant
    if not nq == game.quadrant or game.justin:
	# it isn't here, or we just entered (treat as enroute) 
	if communicating():
	    skip(1)
	    prout(_("Message from Starfleet Command       Stardate %.2f") % game.state.date)
	    prout(_("     Supernova in Quadrant %s; caution advised.") % nq)
    else:
	ns = coord()
	# we are in the quadrant! 
	num = randrange(game.state.galaxy[nq.i][nq.j].stars) + 1
	for ns.i in range(QUADSIZE):
	    for ns.j in range(QUADSIZE):
		if game.quad[ns.i][ns.j]=='*':
		    num -= 1
		    if num==0:
			break
	    if num==0:
		break
	skip(1)
	prouts(_("***RED ALERT!  RED ALERT!"))
	skip(1)
	prout(_("***Incipient supernova detected at Sector %s") % ns)
	if (ns.i-game.sector.i)**2 + (ns.j-game.sector.j)**2 <= 2.1:
	    proutn(_("Emergency override attempts t"))
	    prouts("***************")
	    skip(1)
	    stars()
	    game.alldone = True
    # destroy any Klingons in supernovaed quadrant
    kldead = game.state.galaxy[nq.i][nq.j].klingons
    game.state.galaxy[nq.i][nq.j].klingons = 0
    if nq == game.state.kscmdr:
	# did in the Supercommander! 
	game.state.nscrem = game.state.kscmdr.i = game.state.kscmdr.j = game.isatb =  0
	game.iscate = False
	unschedule(FSCMOVE)
	unschedule(FSCDBAS)
    survivors = filter(lambda w: w != nq, game.state.kcmdr)
    comkills = len(game.state.kcmdr) - len(survivors)
    game.state.kcmdr = survivors
    kldead -= comkills
    if not game.state.kcmdr:
        unschedule(FTBEAM)
    game.state.remkl -= kldead
    # destroy Romulans and planets in supernovaed quadrant 
    nrmdead = game.state.galaxy[nq.i][nq.j].romulans
    game.state.galaxy[nq.i][nq.j].romulans = 0
    game.state.nromrem -= nrmdead
    # Destroy planets 
    for loop in range(game.inplan):
	if game.state.planets[loop].quadrant == nq:
	    game.state.planets[loop].pclass = "destroyed"
	    npdead += 1
    # Destroy any base in supernovaed quadrant
    game.state.baseq = filter(lambda x: x != nq, game.state.baseq)
    # If starship caused supernova, tally up destruction 
    if w != None:
	game.state.starkl += game.state.galaxy[nq.i][nq.j].stars
	game.state.basekl += game.state.galaxy[nq.i][nq.j].starbase
	game.state.nplankl += npdead
    # mark supernova in galaxy and in star chart 
    if game.quadrant == nq or communicating():
	game.state.galaxy[nq.i][nq.j].supernova = True
    # If supernova destroys last Klingons give special message 
    if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)==0 and not nq == game.quadrant:
	skip(2)
	if w == None:
	    prout(_("Lucky you!"))
	proutn(_("A supernova in %s has just destroyed the last Klingons.") % nq)
	finish(FWON)
	return
    # if some Klingons remain, continue or die in supernova 
    if game.alldone:
	finish(FSNOVAED)
    return

# Code from finish.c ends here.

def selfdestruct():
    "Self-destruct maneuver. Finish with a BANG!" 
    scanner.chew()
    if damaged(DCOMPTR):
	prout(_("Computer damaged; cannot execute destruct sequence."))
	return
    prouts(_("---WORKING---")); skip(1)
    prouts(_("SELF-DESTRUCT-SEQUENCE-ACTIVATED")); skip(1)
    prouts("   10"); skip(1)
    prouts("       9"); skip(1)
    prouts("          8"); skip(1)
    prouts("             7"); skip(1)
    prouts("                6"); skip(1)
    skip(1)
    prout(_("ENTER-CORRECT-PASSWORD-TO-CONTINUE-"))
    skip(1)
    prout(_("SELF-DESTRUCT-SEQUENCE-OTHERWISE-"))
    skip(1)
    prout(_("SELF-DESTRUCT-SEQUENCE-WILL-BE-ABORTED"))
    skip(1)
    scanner.next()
    scanner.chew()
    if game.passwd != scanner.token:
	prouts(_("PASSWORD-REJECTED;"))
	skip(1)
	prouts(_("CONTINUITY-EFFECTED"))
	skip(2)
	return
    prouts(_("PASSWORD-ACCEPTED")); skip(1)
    prouts("                   5"); skip(1)
    prouts("                      4"); skip(1)
    prouts("                         3"); skip(1)
    prouts("                            2"); skip(1)
    prouts("                              1"); skip(1)
    if withprob(0.15):
	prouts(_("GOODBYE-CRUEL-WORLD"))
	skip(1)
    kaboom()

def kaboom():
    stars()
    if game.ship=='E':
	prouts("***")
    prouts(_("********* Entropy of %s maximized *********") % crmshp())
    skip(1)
    stars()
    skip(1)
    if len(game.enemies) != 0:
	whammo = 25.0 * game.energy
	l=1
	while l <= len(game.enemies):
	    if game.enemies[l].power*game.enemies[l].kdist <= whammo: 
		deadkl(game.enemies[l].location, game.quad[game.enemies[l].location.i][game.enemies[l].location.j], game.enemies[l].location)
	    l += 1
    finish(FDILITHIUM)
				
def killrate():
    "Compute our rate of kils over time."
    elapsed = game.state.date - game.indate
    if elapsed == 0:	# Avoid divide-by-zero error if calculated on turn 0
        return 0
    else:
        starting = (game.inkling + game.incom + game.inscom)
        remaining = (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)
        return (starting - remaining)/elapsed

def badpoints():
    "Compute demerits."
    badpt = 5.0*game.state.starkl + \
            game.casual + \
            10.0*game.state.nplankl + \
            300*game.state.nworldkl + \
            45.0*game.nhelp +\
            100.0*game.state.basekl +\
            3.0*game.abandoned
    if game.ship == 'F':
        badpt += 100.0
    elif game.ship == None:
        badpt += 200.0
    return badpt

def finish(ifin):
    # end the game, with appropriate notfications 
    igotit = False
    game.alldone = True
    skip(3)
    prout(_("It is stardate %.1f.") % game.state.date)
    skip(1)
    if ifin == FWON: # Game has been won
	if game.state.nromrem != 0:
	    prout(_("The remaining %d Romulans surrender to Starfleet Command.") %
		  game.state.nromrem)

	prout(_("You have smashed the Klingon invasion fleet and saved"))
	prout(_("the Federation."))
	game.gamewon = True
	if game.alive:
            badpt = badpoints()
            if badpt < 100.0:
                badpt = 0.0	# Close enough!
            # killsPerDate >= RateMax
	    if game.state.date-game.indate < 5.0 or \
                killrate() >= 0.1*game.skill*(game.skill+1.0) + 0.1 + 0.008*badpt:
		skip(1)
		prout(_("In fact, you have done so well that Starfleet Command"))
		if game.skill == SKILL_NOVICE:
		    prout(_("promotes you one step in rank from \"Novice\" to \"Fair\"."))
		elif game.skill == SKILL_FAIR:
		    prout(_("promotes you one step in rank from \"Fair\" to \"Good\"."))
		elif game.skill == SKILL_GOOD:
		    prout(_("promotes you one step in rank from \"Good\" to \"Expert\"."))
		elif game.skill == SKILL_EXPERT:
		    prout(_("promotes you to Commodore Emeritus."))
		    skip(1)
		    prout(_("Now that you think you're really good, try playing"))
		    prout(_("the \"Emeritus\" game. It will splatter your ego."))
		elif game.skill == SKILL_EMERITUS:
		    skip(1)
		    proutn(_("Computer-  "))
		    prouts(_("ERROR-ERROR-ERROR-ERROR"))
		    skip(2)
		    prouts(_("  YOUR-SKILL-HAS-EXCEEDED-THE-CAPACITY-OF-THIS-PROGRAM"))
		    skip(1)
		    prouts(_("  THIS-PROGRAM-MUST-SURVIVE"))
		    skip(1)
		    prouts(_("  THIS-PROGRAM-MUST-SURVIVE"))
		    skip(1)
		    prouts(_("  THIS-PROGRAM-MUST-SURVIVE"))
		    skip(1)
		    prouts(_("  THIS-PROGRAM-MUST?- MUST ? - SUR? ? -?  VI"))
		    skip(2)
		    prout(_("Now you can retire and write your own Star Trek game!"))
		    skip(1)
		elif game.skill >= SKILL_EXPERT:
		    if game.thawed and not idebug:
			prout(_("You cannot get a citation, so..."))
		    else:
			proutn(_("Do you want your Commodore Emeritus Citation printed? "))
			scanner.chew()
			if ja() == True:
			    igotit = True
	    # Only grant long life if alive (original didn't!)
	    skip(1)
	    prout(_("LIVE LONG AND PROSPER."))
	score()
	if igotit:
	    plaque()	    
	return
    elif ifin == FDEPLETE: # Federation Resources Depleted
	prout(_("Your time has run out and the Federation has been"))
	prout(_("conquered.  Your starship is now Klingon property,"))
	prout(_("and you are put on trial as a war criminal.  On the"))
	proutn(_("basis of your record, you are "))
	if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)*3.0 > (game.inkling + game.incom + game.inscom):
	    prout(_("acquitted."))
	    skip(1)
	    prout(_("LIVE LONG AND PROSPER."))
	else:
	    prout(_("found guilty and"))
	    prout(_("sentenced to death by slow torture."))
	    game.alive = False
	score()
	return
    elif ifin == FLIFESUP:
	prout(_("Your life support reserves have run out, and"))
	prout(_("you die of thirst, starvation, and asphyxiation."))
	prout(_("Your starship is a derelict in space."))
    elif ifin == FNRG:
	prout(_("Your energy supply is exhausted."))
	skip(1)
	prout(_("Your starship is a derelict in space."))
    elif ifin == FBATTLE:
	prout(_("The %s has been destroyed in battle.") % crmshp())
	skip(1)
	prout(_("Dulce et decorum est pro patria mori."))
    elif ifin == FNEG3:
	prout(_("You have made three attempts to cross the negative energy"))
	prout(_("barrier which surrounds the galaxy."))
	skip(1)
	prout(_("Your navigation is abominable."))
	score()
    elif ifin == FNOVA:
	prout(_("Your starship has been destroyed by a nova."))
	prout(_("That was a great shot."))
	skip(1)
    elif ifin == FSNOVAED:
	prout(_("The %s has been fried by a supernova.") % crmshp())
	prout(_("...Not even cinders remain..."))
    elif ifin == FABANDN:
	prout(_("You have been captured by the Klingons. If you still"))
	prout(_("had a starbase to be returned to, you would have been"))
	prout(_("repatriated and given another chance. Since you have"))
	prout(_("no starbases, you will be mercilessly tortured to death."))
    elif ifin == FDILITHIUM:
	prout(_("Your starship is now an expanding cloud of subatomic particles"))
    elif ifin == FMATERIALIZE:
	prout(_("Starbase was unable to re-materialize your starship."))
	prout(_("Sic transit gloria mundi"))
    elif ifin == FPHASER:
	prout(_("The %s has been cremated by its own phasers.") % crmshp())
    elif ifin == FLOST:
	prout(_("You and your landing party have been"))
	prout(_("converted to energy, disipating through space."))
    elif ifin == FMINING:
	prout(_("You are left with your landing party on"))
	prout(_("a wild jungle planet inhabited by primitive cannibals."))
	skip(1)
	prout(_("They are very fond of \"Captain Kirk\" soup."))
	skip(1)
	prout(_("Without your leadership, the %s is destroyed.") % crmshp())
    elif ifin == FDPLANET:
	prout(_("You and your mining party perish."))
	skip(1)
	prout(_("That was a great shot."))
	skip(1)
    elif ifin == FSSC:
	prout(_("The Galileo is instantly annihilated by the supernova."))
	prout(_("You and your mining party are atomized."))
	skip(1)
	prout(_("Mr. Spock takes command of the %s and") % crmshp())
	prout(_("joins the Romulans, wreaking terror on the Federation."))
    elif ifin == FPNOVA:
	prout(_("You and your mining party are atomized."))
	skip(1)
	prout(_("Mr. Spock takes command of the %s and") % crmshp())
	prout(_("joins the Romulans, wreaking terror on the Federation."))
    elif ifin == FSTRACTOR:
	prout(_("The shuttle craft Galileo is also caught,"))
	prout(_("and breaks up under the strain."))
	skip(1)
	prout(_("Your debris is scattered for millions of miles."))
	prout(_("Without your leadership, the %s is destroyed.") % crmshp())
    elif ifin == FDRAY:
	prout(_("The mutants attack and kill Spock."))
	prout(_("Your ship is captured by Klingons, and"))
	prout(_("your crew is put on display in a Klingon zoo."))
    elif ifin == FTRIBBLE:
	prout(_("Tribbles consume all remaining water,"))
	prout(_("food, and oxygen on your ship."))
	skip(1)
	prout(_("You die of thirst, starvation, and asphyxiation."))
	prout(_("Your starship is a derelict in space."))
    elif ifin == FHOLE:
	prout(_("Your ship is drawn to the center of the black hole."))
	prout(_("You are crushed into extremely dense matter."))
    elif ifin == FCREW:
	prout(_("Your last crew member has died."))
    if game.ship == 'F':
	game.ship = None
    elif game.ship == 'E':
	game.ship = 'F'
    game.alive = False
    if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem) != 0:
	goodies = game.state.remres/game.inresor
	baddies = (game.state.remkl + 2.0*len(game.state.kcmdr))/(game.inkling+2.0*game.incom)
	if goodies/baddies >= randreal(1.0, 1.5):
	    prout(_("As a result of your actions, a treaty with the Klingon"))
	    prout(_("Empire has been signed. The terms of the treaty are"))
	    if goodies/baddies >= randreal(3.0):
		prout(_("favorable to the Federation."))
		skip(1)
		prout(_("Congratulations!"))
	    else:
		prout(_("highly unfavorable to the Federation."))
	else:
	    prout(_("The Federation will be destroyed."))
    else:
	prout(_("Since you took the last Klingon with you, you are a"))
	prout(_("martyr and a hero. Someday maybe they'll erect a"))
	prout(_("statue in your memory. Rest in peace, and try not"))
	prout(_("to think about pigeons."))
	game.gamewon = True
    score()

def score():
    "Compute player's score."
    timused = game.state.date - game.indate
    iskill = game.skill
    if (timused == 0 or (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem) != 0) and timused < 5.0:
	timused = 5.0
    perdate = killrate()
    ithperd = 500*perdate + 0.5
    iwon = 0
    if game.gamewon:
	iwon = 100*game.skill
    if game.ship == 'E': 
	klship = 0
    elif game.ship == 'F': 
	klship = 1
    else:
	klship = 2
    iscore = 10*(game.inkling - game.state.remkl) \
             + 50*(game.incom - len(game.state.kcmdr)) \
             + ithperd + iwon \
             + 20*(game.inrom - game.state.nromrem) \
             + 200*(game.inscom - game.state.nscrem) \
    	     - game.state.nromrem \
             - badpoints()
    if not game.alive:
	iscore -= 200
    skip(2)
    prout(_("Your score --"))
    if game.inrom - game.state.nromrem:
	prout(_("%6d Romulans destroyed                 %5d") %
	      (game.inrom - game.state.nromrem, 20*(game.inrom - game.state.nromrem)))
    if game.state.nromrem and game.gamewon:
	prout(_("%6d Romulans captured                  %5d") %
	      (game.state.nromrem, game.state.nromrem))
    if game.inkling - game.state.remkl:
	prout(_("%6d ordinary Klingons destroyed        %5d") %
	      (game.inkling - game.state.remkl, 10*(game.inkling - game.state.remkl)))
    if game.incom - len(game.state.kcmdr):
	prout(_("%6d Klingon commanders destroyed       %5d") %
	      (game.incom - len(game.state.kcmdr), 50*(game.incom - len(game.state.kcmdr))))
    if game.inscom - game.state.nscrem:
	prout(_("%6d Super-Commander destroyed          %5d") %
	      (game.inscom - game.state.nscrem, 200*(game.inscom - game.state.nscrem)))
    if ithperd:
	prout(_("%6.2f Klingons per stardate              %5d") %
	      (perdate, ithperd))
    if game.state.starkl:
	prout(_("%6d stars destroyed by your action     %5d") %
	      (game.state.starkl, -5*game.state.starkl))
    if game.state.nplankl:
	prout(_("%6d planets destroyed by your action   %5d") %
	      (game.state.nplankl, -10*game.state.nplankl))
    if (game.options & OPTION_WORLDS) and game.state.nworldkl:
	prout(_("%6d inhabited planets destroyed by your action   %5d") %
	      (game.state.nworldkl, -300*game.state.nworldkl))
    if game.state.basekl:
	prout(_("%6d bases destroyed by your action     %5d") %
	      (game.state.basekl, -100*game.state.basekl))
    if game.nhelp:
	prout(_("%6d calls for help from starbase       %5d") %
	      (game.nhelp, -45*game.nhelp))
    if game.casual:
	prout(_("%6d casualties incurred                %5d") %
	      (game.casual, -game.casual))
    if game.abandoned:
	prout(_("%6d crew abandoned in space            %5d") %
	      (game.abandoned, -3*game.abandoned))
    if klship:
	prout(_("%6d ship(s) lost or destroyed          %5d") %
	      (klship, -100*klship))
    if not game.alive:
	prout(_("Penalty for getting yourself killed        -200"))
    if game.gamewon:
	proutn(_("Bonus for winning "))
	if game.skill   == SKILL_NOVICE:	proutn(_("Novice game  "))
	elif game.skill == SKILL_FAIR:  	proutn(_("Fair game    "))
	elif game.skill ==  SKILL_GOOD: 	proutn(_("Good game    "))
	elif game.skill ==  SKILL_EXPERT:	proutn(_("Expert game  "))
	elif game.skill ==  SKILL_EMERITUS:	proutn(_("Emeritus game"))
	prout("           %5d" % iwon)
    skip(1)
    prout(_("TOTAL SCORE                               %5d") % iscore)

def plaque():
    "Emit winner's commemmorative plaque." 
    skip(2)
    while True:
        proutn(_("File or device name for your plaque: "))
        winner = cgetline()
        try:
            fp = open(winner, "w")
            break
        except IOError:
            prout(_("Invalid name."))

    proutn(_("Enter name to go on plaque (up to 30 characters): "))
    winner = cgetline()
    # The 38 below must be 64 for 132-column paper 
    nskip = 38 - len(winner)/2
    fp.write("\n\n\n\n")
    # --------DRAW ENTERPRISE PICTURE. 
    fp.write("                                       EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" )
    fp.write("                                      EEE                      E  : :                                         :  E\n" )
    fp.write("                                    EE   EEE                   E  : :                   NCC-1701              :  E\n")
    fp.write("EEEEEEEEEEEEEEEE        EEEEEEEEEEEEEEE  : :                              : E\n")
    fp.write(" E                                     EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n")
    fp.write("                      EEEEEEEEE               EEEEEEEEEEEEE                 E  E\n")
    fp.write("                               EEEEEEE   EEEEE    E          E              E  E\n")
    fp.write("                                      EEE           E          E            E  E\n")
    fp.write("                                                       E         E          E  E\n")
    fp.write("                                                         EEEEEEEEEEEEE      E  E\n")
    fp.write("                                                      EEE :           EEEEEEE  EEEEEEEE\n")
    fp.write("                                                    :E    :                 EEEE       E\n")
    fp.write("                                                   .-E   -:-----                       E\n")
    fp.write("                                                    :E    :                            E\n")
    fp.write("                                                      EE  :                    EEEEEEEE\n")
    fp.write("                                                       EEEEEEEEEEEEEEEEEEEEEEE\n")
    fp.write("\n\n\n")
    fp.write(_("                                                       U. S. S. ENTERPRISE\n"))
    fp.write("\n\n\n\n")
    fp.write(_("                                  For demonstrating outstanding ability as a starship captain\n"))
    fp.write("\n")
    fp.write(_("                                                Starfleet Command bestows to you\n"))
    fp.write("\n")
    fp.write("%*s%s\n\n" % (nskip, "", winner))
    fp.write(_("                                                           the rank of\n\n"))
    fp.write(_("                                                       \"Commodore Emeritus\"\n\n"))
    fp.write("                                                          ")
    if game.skill ==  SKILL_EXPERT:
        fp.write(_(" Expert level\n\n"))
    elif game.skill == SKILL_EMERITUS:
        fp.write(_("Emeritus level\n\n"))
    else:
        fp.write(_(" Cheat level\n\n"))
    timestring = time.ctime()
    fp.write(_("                                                 This day of %.6s %.4s, %.8s\n\n") %
                    (timestring+4, timestring+20, timestring+11))
    fp.write(_("                                                        Your score:  %d\n\n") % iscore)
    fp.write(_("                                                    Klingons per stardate:  %.2f\n") % perdate)
    fp.close()

# Code from io.c begins here

rows = linecount = 0	# for paging 
stdscr = None
replayfp = None
fullscreen_window = None
srscan_window     = None
report_window     = None
status_window     = None
lrscan_window     = None
message_window    = None
prompt_window     = None
curwnd = None

def iostart():
    global stdscr, rows
    gettext.bindtextdomain("sst", "/usr/local/share/locale")
    gettext.textdomain("sst")
    if not (game.options & OPTION_CURSES):
	ln_env = os.getenv("LINES")
        if ln_env:
            rows = ln_env
        else:
            rows = 25
    else:
	stdscr = curses.initscr()
	stdscr.keypad(True)
	curses.nonl()
	curses.cbreak()
        global fullscreen_window, srscan_window, report_window, status_window
        global lrscan_window, message_window, prompt_window
        (rows, columns)   = stdscr.getmaxyx()
	fullscreen_window = stdscr
	srscan_window     = curses.newwin(12, 25, 0,       0)
	report_window     = curses.newwin(11, 0,  1,       25)
	status_window     = curses.newwin(10, 0,  1,       39)
	lrscan_window     = curses.newwin(5,  0,  0,       64) 
	message_window    = curses.newwin(0,  0,  12,      0)
	prompt_window     = curses.newwin(1,  0,  rows-2,  0) 
	message_window.scrollok(True)
	setwnd(fullscreen_window)

def ioend():
    "Wrap up I/O."
    if game.options & OPTION_CURSES:
        stdscr.keypad(False)
        curses.echo()
        curses.nocbreak()
        curses.endwin()

def waitfor():
    "Wait for user action -- OK to do nothing if on a TTY"
    if game.options & OPTION_CURSES:
	stdscr.getch()

def announce():
    skip(1)
    prouts(_("[ANNOUNCEMENT ARRIVING...]"))
    skip(1)

def pause_game():
    if game.skill > SKILL_FAIR:
        prompt = _("[CONTINUE?]")
    else:
        prompt = _("[PRESS ENTER TO CONTINUE]")

    if game.options & OPTION_CURSES:
        drawmaps(0)
        setwnd(prompt_window)
        prompt_window.clear()
        prompt_window.addstr(prompt)
        prompt_window.getstr()
        prompt_window.clear()
        prompt_window.refresh()
        setwnd(message_window)
    else:
        global linecount
        sys.stdout.write('\n')
        proutn(prompt)
        raw_input()
        for j in range(rows):
            sys.stdout.write('\n')
        linecount = 0

def skip(i):
    "Skip i lines.  Pause game if this would cause a scrolling event."
    for dummy in range(i):
	if game.options & OPTION_CURSES:
            (y, x) = curwnd.getyx()
            (my, mx) = curwnd.getmaxyx()
	    if curwnd == message_window and y >= my - 3:
		pause_game()
		clrscr()
	    else:
                try:
                    curwnd.move(y+1, 0)
                except curses.error:
                    pass
	else:
            global linecount
	    linecount += 1
	    if rows and linecount >= rows:
		pause_game()
	    else:
		sys.stdout.write('\n')

def proutn(line):
    "Utter a line with no following line feed."
    if game.options & OPTION_CURSES:
	curwnd.addstr(line)
	curwnd.refresh()
    else:
	sys.stdout.write(line)
        sys.stdout.flush()

def prout(line):
    proutn(line)
    skip(1)

def prouts(line):
    "Emit slowly!" 
    for c in line:
        if not replayfp or replayfp.closed:	# Don't slow down replays
            time.sleep(0.03)
	proutn(c)
	if game.options & OPTION_CURSES:
	    curwnd.refresh()
	else:
	    sys.stdout.flush()
    if not replayfp or replayfp.closed:
        time.sleep(0.03)

def cgetline():
    "Get a line of input."
    if game.options & OPTION_CURSES:
	line = curwnd.getstr() + "\n"
	curwnd.refresh()
    else:
	if replayfp and not replayfp.closed:
            while True:
                line = replayfp.readline()
                proutn(line)
                if line == '':
                    prout("*** Replay finished")
                    replayfp.close()
                    break
                elif line[0] != "#":
                    break
	else:
	    line = raw_input() + "\n"
    if logfp:
	logfp.write(line)
    return line

def setwnd(wnd):
    "Change windows -- OK for this to be a no-op in tty mode."
    global curwnd
    if game.options & OPTION_CURSES:
        curwnd = wnd
        curses.curs_set(wnd == fullscreen_window or wnd == message_window or wnd == prompt_window)

def clreol():
    "Clear to end of line -- can be a no-op in tty mode" 
    if game.options & OPTION_CURSES:
        curwnd.clrtoeol()
        curwnd.refresh()

def clrscr():
    "Clear screen -- can be a no-op in tty mode."
    global linecount
    if game.options & OPTION_CURSES:
       curwnd.clear()
       curwnd.move(0, 0)
       curwnd.refresh()
    linecount = 0
    
#
# Things past this point have policy implications.
# 

def drawmaps(mode):
    "Hook to be called after moving to redraw maps."
    if game.options & OPTION_CURSES:
	if mode == 1:
	    sensor()
        setwnd(srscan_window)
        curwnd.move(0, 0)
        srscan()
	if mode != 2:
	    setwnd(status_window)
	    status_window.clear()
	    status_window.move(0, 0)
	    setwnd(report_window)
	    report_window.clear()
	    report_window.move(0, 0)
	    status()
	    setwnd(lrscan_window)
	    lrscan_window.clear()
	    lrscan_window.move(0, 0)
	    lrscan(silent=False)

def put_srscan_sym(w, sym):
    "Emit symbol for short-range scan."
    srscan_window.move(w.i+1, w.j*2+2)
    srscan_window.addch(sym)
    srscan_window.refresh()

def boom(w):
    "Enemy fall down, go boom."  
    if game.options & OPTION_CURSES:
	drawmaps(2)
	setwnd(srscan_window)
	srscan_window.attron(curses.A_REVERSE)
	put_srscan_sym(w, game.quad[w.i][w.j])
	#sound(500)
	#time.sleep(1.0)
	#nosound()
	srscan_window.attroff(curses.A_REVERSE)
	put_srscan_sym(w, game.quad[w.i][w.j])
	curses.delay_output(500)
	setwnd(message_window) 

def warble():
    "Sound and visual effects for teleportation."
    if game.options & OPTION_CURSES:
	drawmaps(2)
	setwnd(message_window)
	#sound(50)
    prouts("     . . . . .     ")
    if game.options & OPTION_CURSES:
	#curses.delay_output(1000)
	#nosound()
        pass

def tracktorpedo(origin, w, step, i, n, iquad):
    "Torpedo-track animation." 
    if not game.options & OPTION_CURSES:
	if step == 1:
	    if n != 1:
		skip(1)
		proutn(_("Track for torpedo number %d-  ") % (i+1))
	    else:
		skip(1)
		proutn(_("Torpedo track- "))
	elif step==4 or step==9: 
	    skip(1)
	proutn("%s   " % w)
    else:
	if not damaged(DSRSENS) or game.condition=="docked":
	    if i != 0 and step == 1:
		drawmaps(2)
		time.sleep(0.4)
	    if (iquad=='.') or (iquad==' '):
		put_srscan_sym(w, '+')
		#sound(step*10)
		#time.sleep(0.1)
		#nosound()
		put_srscan_sym(w, iquad)
	    else:
		curwnd.attron(curses.A_REVERSE)
		put_srscan_sym(w, iquad)
		#sound(500)
		#time.sleep(1.0)
		#nosound()
		curwnd.attroff(curses.A_REVERSE)
		put_srscan_sym(w, iquad)
	else:
	    proutn("%s   " % w)

def makechart():
    "Display the current galaxy chart."
    if game.options & OPTION_CURSES:
	setwnd(message_window)
	message_window.clear()
    chart()
    if game.options & OPTION_TTY:
	skip(1)

NSYM	= 14

def prstat(txt, data):
    proutn(txt)
    if game.options & OPTION_CURSES:
	skip(1)
	setwnd(status_window)
    else:
        proutn(" " * (NSYM - len(txt)))
    proutn(data)
    skip(1)
    if game.options & OPTION_CURSES:
	setwnd(report_window)

# Code from moving.c begins here

def imove(course=None, noattack=False):
    "Movement execution for warp, impulse, supernova, and tractor-beam events."
    w = coord()

    def newquadrant(noattack):
        # Leaving quadrant -- allow final enemy attack 
        # Don't do it if being pushed by Nova 
        if len(game.enemies) != 0 and not noattack:
            newcnd()
            for enemy in game.enemies:
                finald = (w - enemy.location).distance()
                enemy.kavgd = 0.5 * (finald + enemy.kdist)
            # Stas Sergeev added the condition
            # that attacks only happen if Klingons
            # are present and your skill is good.
            if game.skill > SKILL_GOOD and game.klhere > 0 and not game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova:
                attack(torps_ok=False)
            if game.alldone:
                return
        # check for edge of galaxy 
        kinks = 0
        while True:
            kink = False
            if course.final.i < 0:
                course.final.i = -course.final.i
                kink = True
            if course.final.j < 0:
                course.final.j = -course.final.j
                kink = True
            if course.final.i >= GALSIZE*QUADSIZE:
                course.final.i = (GALSIZE*QUADSIZE*2) - course.final.i
                kink = True
            if course.final.j >= GALSIZE*QUADSIZE:
                course.final.j = (GALSIZE*QUADSIZE*2) - course.final.j
                kink = True
            if kink:
                kinks += 1
            else:
                break
        if kinks:
            game.nkinks += 1
            if game.nkinks == 3:
                # Three strikes -- you're out! 
                finish(FNEG3)
                return
            skip(1)
            prout(_("YOU HAVE ATTEMPTED TO CROSS THE NEGATIVE ENERGY BARRIER"))
            prout(_("AT THE EDGE OF THE GALAXY.  THE THIRD TIME YOU TRY THIS,"))
            prout(_("YOU WILL BE DESTROYED."))
        # Compute final position in new quadrant 
        if trbeam: # Don't bother if we are to be beamed 
            return
        game.quadrant = course.final.quadrant()
        game.sector = course.final.sector()
        skip(1)
        prout(_("Entering Quadrant %s.") % game.quadrant)
        game.quad[game.sector.i][game.sector.j] = game.ship
        newqad()
        if game.skill>SKILL_NOVICE:
            attack(torps_ok=False)  

    def check_collision(h):
        iquad = game.quad[h.i][h.j]
        if iquad != '.':
            # object encountered in flight path 
            stopegy = 50.0*course.distance/game.optime
            if iquad in ('T', 'K', 'C', 'S', 'R', '?'):
                for enemy in game.enemies:
                    if enemy.location == game.sector:
                        break
                collision(rammed=False, enemy=enemy)
                return True
            elif iquad == ' ':
                skip(1)
                prouts(_("***RED ALERT!  RED ALERT!"))
                skip(1)
                proutn("***" + crmshp())
                proutn(_(" pulled into black hole at Sector %s") % h)
                # Getting pulled into a black hole was certain
                # death in Almy's original.  Stas Sergeev added a
                # possibility that you'll get timewarped instead.
                n=0
                for m in range(NDEVICES):
                    if game.damage[m]>0: 
                        n += 1
                probf=math.pow(1.4,(game.energy+game.shield)/5000.0-1.0)*math.pow(1.3,1.0/(n+1)-1.0)
                if (game.options & OPTION_BLKHOLE) and withprob(1-probf): 
                    timwrp()
                else: 
                    finish(FHOLE)
                return True
            else:
                # something else 
                skip(1)
                proutn(crmshp())
                if iquad == '#':
                    prout(_(" encounters Tholian web at %s;") % h)
                else:
                    prout(_(" blocked by object at %s;") % h)
                proutn(_("Emergency stop required "))
                prout(_("%2d units of energy.") % int(stopegy))
                game.energy -= stopegy
                if game.energy <= 0:
                    finish(FNRG)
                return True
        return False

    trbeam = False
    if game.inorbit:
	prout(_("Helmsman Sulu- \"Leaving standard orbit.\""))
	game.inorbit = False
    # If tractor beam is to occur, don't move full distance 
    if game.state.date+game.optime >= scheduled(FTBEAM):
	trbeam = True
	game.condition = "red"
	course.distance = course.distance*(scheduled(FTBEAM)-game.state.date)/game.optime + 0.1
	game.optime = scheduled(FTBEAM) - game.state.date + 1e-5
    # Move out
    game.quad[game.sector.i][game.sector.j] = '.'
    for m in range(course.moves):
        course.next()
        w = course.sector()
        if course.origin.quadrant() != course.location.quadrant():
            newquadrant(noattack)
            break
        elif check_collision(w):
            print "Collision detected"
            break
        else:
            game.sector = w
    # We're in destination quadrant -- compute new average enemy distances
    game.quad[game.sector.i][game.sector.j] = game.ship
    if game.enemies:
        for enemy in game.enemies:
            finald = (w-enemy.location).distance()
            enemy.kavgd = 0.5 * (finald + enemy.kdist)
            enemy.kdist = finald
        game.enemies.sort(lambda x, y: cmp(x.kdist, y.kdist))
        if not game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova:
            attack(torps_ok=False)
        for enemy in game.enemies:
            enemy.kavgd = enemy.kdist
    newcnd()
    drawmaps(0)
    setwnd(message_window)
    return

def dock(verbose):
    "Dock our ship at a starbase."
    scanner.chew()
    if game.condition == "docked" and verbose:
	prout(_("Already docked."))
	return
    if game.inorbit:
	prout(_("You must first leave standard orbit."))
	return
    if not game.base.is_valid() or abs(game.sector.i-game.base.i) > 1 or abs(game.sector.j-game.base.j) > 1:
	prout(crmshp() + _(" not adjacent to base."))
	return
    game.condition = "docked"
    if "verbose":
	prout(_("Docked."))
    game.ididit = True
    if game.energy < game.inenrg:
	game.energy = game.inenrg
    game.shield = game.inshld
    game.torps = game.intorps
    game.lsupres = game.inlsr
    game.state.crew = FULLCREW
    if not damaged(DRADIO) and \
	((is_scheduled(FCDBAS) or game.isatb == 1) and not game.iseenit):
	# get attack report from base 
	prout(_("Lt. Uhura- \"Captain, an important message from the starbase:\""))
	attackreport(False)
	game.iseenit = True

def cartesian(loc1=None, loc2=None):
    if loc1 is None:
        return game.quadrant * QUADSIZE + game.sector
    elif loc2 is None:
        return game.quadrant * QUADSIZE + loc1
    else:
        return loc1 * QUADSIZE + loc2

def getcourse(isprobe):
    "Get a course and distance from the user."
    key = 0
    dquad = copy.copy(game.quadrant)
    navmode = "unspecified"
    itemp = "curt"
    dsect = coord()
    iprompt = False
    if game.landed and not isprobe:
	prout(_("Dummy! You can't leave standard orbit until you"))
	proutn(_("are back aboard the ship."))
	scanner.chew()
	raise TrekError
    while navmode == "unspecified":
	if damaged(DNAVSYS):
	    if isprobe:
		prout(_("Computer damaged; manual navigation only"))
	    else:
		prout(_("Computer damaged; manual movement only"))
	    scanner.chew()
	    navmode = "manual"
	    key = "IHEOL"
	    break
        key = scanner.next()
	if key == "IHEOL":
	    proutn(_("Manual or automatic- "))
	    iprompt = True
	    scanner.chew()
	elif key == "IHALPHA":
            if scanner.sees("manual"):
		navmode = "manual"
		key = scanner.next()
		break
            elif scanner.sees("automatic"):
		navmode = "automatic"
		key = scanner.next()
		break
	    else:
		huh()
		scanner.chew()
		raise TrekError
	else: # numeric 
	    if isprobe:
		prout(_("(Manual navigation assumed.)"))
	    else:
		prout(_("(Manual movement assumed.)"))
	    navmode = "manual"
	    break
    delta = coord()
    if navmode == "automatic":
	while key == "IHEOL":
	    if isprobe:
		proutn(_("Target quadrant or quadrant&sector- "))
	    else:
		proutn(_("Destination sector or quadrant&sector- "))
	    scanner.chew()
	    iprompt = True
	    key = scanner.next()
	if key != "IHREAL":
	    huh()
	    raise TrekError
	xi = int(round(scanner.real))-1
	key = scanner.next()
	if key != "IHREAL":
	    huh()
	    raise TrekError
	xj = int(round(scanner.real))-1
	key = scanner.next()
	if key == "IHREAL":
	    # both quadrant and sector specified 
	    xk = int(round(scanner.real))-1
	    key = scanner.next()
	    if key != "IHREAL":
		huh()
		raise TrekError
	    xl = int(round(scanner.real))-1
	    dquad.i = xi
	    dquad.j = xj
	    dsect.i = xk
	    dsect.j = xl
	else:
            # only one pair of numbers was specified
	    if isprobe:
		# only quadrant specified -- go to center of dest quad 
		dquad.i = xi
		dquad.j = xj
		dsect.j = dsect.i = 4	# preserves 1-origin behavior
	    else:
                # only sector specified
		dsect.i = xi
		dsect.j = xj
	    itemp = "normal"
	if not dquad.valid_quadrant() or not dsect.valid_sector():
	    huh()
	    raise TrekError
	skip(1)
	if not isprobe:
	    if itemp > "curt":
		if iprompt:
		    prout(_("Helmsman Sulu- \"Course locked in for Sector %s.\"") % dsect)
	    else:
		prout(_("Ensign Chekov- \"Course laid in, Captain.\""))
        # the actual deltas get computed here
	delta.j = dquad.j-game.quadrant.j + (dsect.j-game.sector.j)/(QUADSIZE*1.0)
	delta.i = game.quadrant.i-dquad.i + (game.sector.i-dsect.i)/(QUADSIZE*1.0)
    else: # manual 
	while key == "IHEOL":
	    proutn(_("X and Y displacements- "))
	    scanner.chew()
	    iprompt = True
	    key = scanner.next()
	itemp = "verbose"
	if key != "IHREAL":
	    huh()
	    raise TrekError
	delta.j = scanner.real
	key = scanner.next()
	if key != "IHREAL":
	    huh()
	    raise TrekError
	delta.i = scanner.real
    # Check for zero movement 
    if delta.i == 0 and delta.j == 0:
	scanner.chew()
	raise TrekError
    if itemp == "verbose" and not isprobe:
	skip(1)
	prout(_("Helmsman Sulu- \"Aye, Sir.\""))
    scanner.chew()
    return course(bearing=delta.bearing(), distance=delta.distance())

class course:
    def __init__(self, bearing, distance, origin=None): 
        self.distance = distance
        self.bearing = bearing
        if origin is None:
            self.origin = cartesian(game.quadrant, game.sector)
        else:
            self.origin = origin
        # The bearing() code we inherited from FORTRAN is actually computing
        # clockface directions!
        if self.bearing < 0.0:
            self.bearing += 12.0
        self.angle = ((15.0 - self.bearing) * 0.5235988)
        if origin is None:
            self.origin = cartesian(game.quadrant, game.sector)
        else:
            self.origin = cartesian(game.quadrant, origin)
        self.increment = coord(-math.sin(self.angle), math.cos(self.angle))
        bigger = max(abs(self.increment.i), abs(self.increment.j))
        self.increment /= bigger
        self.moves = int(round(10*self.distance*bigger))
        self.reset()
        self.final = (self.location + self.moves*self.increment).roundtogrid()
    def reset(self):
        self.location = self.origin
        self.step = 0
    def arrived(self):
        return self.location.roundtogrid() == self.final
    def next(self):
        "Next step on course."
        self.step += 1
        self.nextlocation = self.location + self.increment
        samequad = (self.location.quadrant() == self.nextlocation.quadrant())
        self.location = self.nextlocation
        return samequad
    def quadrant(self):
        return self.location.quadrant()
    def sector(self):
        return self.location.sector()
    def power(self, warp):
	return self.distance*(warp**3)*(game.shldup+1)
    def time(self, warp):
        return 10.0*self.distance/warp**2

def impulse():
    "Move under impulse power."
    game.ididit = False
    if damaged(DIMPULS):
	scanner.chew()
	skip(1)
	prout(_("Engineer Scott- \"The impulse engines are damaged, Sir.\""))
	return
    if game.energy > 30.0:
        try:
            course = getcourse(isprobe=False)
        except TrekError:
	    return
	power = 20.0 + 100.0*course.distance
    else:
	power = 30.0
    if power >= game.energy:
	# Insufficient power for trip 
	skip(1)
	prout(_("First Officer Spock- \"Captain, the impulse engines"))
	prout(_("require 20.0 units to engage, plus 100.0 units per"))
	if game.energy > 30:
	    proutn(_("quadrant.  We can go, therefore, a maximum of %d") %
                     int(0.01 * (game.energy-20.0)-0.05))
	    prout(_(" quadrants.\""))
	else:
	    prout(_("quadrant.  They are, therefore, useless.\""))
	scanner.chew()
	return
    # Make sure enough time is left for the trip 
    game.optime = course.dist/0.095
    if game.optime >= game.state.remtime:
	prout(_("First Officer Spock- \"Captain, our speed under impulse"))
	prout(_("power is only 0.95 sectors per stardate. Are you sure"))
	proutn(_("we dare spend the time?\" "))
	if ja() == False:
	    return
    # Activate impulse engines and pay the cost 
    imove(course, noattack=False)
    game.ididit = True
    if game.alldone:
	return
    power = 20.0 + 100.0*course.dist
    game.energy -= power
    game.optime = course.dist/0.095
    if game.energy <= 0:
	finish(FNRG)
    return

def warp(course, involuntary):
    "ove under warp drive."
    blooey = False; twarp = False
    if not involuntary: # Not WARPX entry 
	game.ididit = False
	if game.damage[DWARPEN] > 10.0:
	    scanner.chew()
	    skip(1)
	    prout(_("Engineer Scott- \"The warp engines are damaged, Sir.\""))
	    return
	if damaged(DWARPEN) and game.warpfac > 4.0:
	    scanner.chew()
	    skip(1)
	    prout(_("Engineer Scott- \"Sorry, Captain. Until this damage"))
	    prout(_("  is repaired, I can only give you warp 4.\""))
	    return
       	# Read in course and distance
        if course==None:
            try:
                course = getcourse(isprobe=False)
            except TrekError:
                return
	# Make sure starship has enough energy for the trip
        # Note: this formula is slightly different from the C version,
        # and lets you skate a bit closer to the edge.
	if course.power(game.warpfac) >= game.energy:
	    # Insufficient power for trip 
	    game.ididit = False
	    skip(1)
	    prout(_("Engineering to bridge--"))
	    if not game.shldup or 0.5*power > game.energy:
		iwarp = (game.energy/(course.dist+0.05)) ** 0.333333333
		if iwarp <= 0:
		    prout(_("We can't do it, Captain. We don't have enough energy."))
		else:
		    proutn(_("We don't have enough energy, but we could do it at warp %d") % iwarp)
		    if game.shldup:
			prout(",")
			prout(_("if you'll lower the shields."))
		    else:
			prout(".")
	    else:
		prout(_("We haven't the energy to go that far with the shields up."))
	    return				
	# Make sure enough time is left for the trip 
	game.optime = course.time(game.warpfac)
	if game.optime >= 0.8*game.state.remtime:
	    skip(1)
	    prout(_("First Officer Spock- \"Captain, I compute that such"))
	    proutn(_("  a trip would require approximately %2.0f") %
		   (100.0*game.optime/game.state.remtime))
	    prout(_(" percent of our"))
	    proutn(_("  remaining time.  Are you sure this is wise?\" "))
	    if ja() == False:
		game.ididit = False
		game.optime=0 
		return
    # Entry WARPX 
    if game.warpfac > 6.0:
	# Decide if engine damage will occur
        # ESR: Seems wrong. Probability of damage goes *down* with distance? 
	prob = course.distance*(6.0-game.warpfac)**2/66.666666666
	if prob > randreal():
	    blooey = True
	    course.distance = randreal(course.distance)
	# Decide if time warp will occur 
	if 0.5*course.distance*math.pow(7.0,game.warpfac-10.0) > randreal():
	    twarp = True
	if idebug and game.warpfac==10 and not twarp:
	    blooey = False
	    proutn("=== Force time warp? ")
	    if ja() == True:
		twarp = True
	if blooey or twarp:
	    # If time warp or engine damage, check path 
	    # If it is obstructed, don't do warp or damage 
            for m in range(course.moves):
                course.next()
                w = course.sector()
                if not w.valid_sector():
                    break
		if game.quad[w.i][w.j] != '.':
		    blooey = False
		    twarp = False
            course.reset()
    # Activate Warp Engines and pay the cost 
    imove(course, noattack=False)
    if game.alldone:
	return
    game.energy -= course.power(game.warpfac)
    if game.energy <= 0:
	finish(FNRG)
    game.optime = course.time(game.warpfac)
    if twarp:
	timwrp()
    if blooey:
	game.damage[DWARPEN] = game.damfac * randreal(1.0, 4.0)
	skip(1)
	prout(_("Engineering to bridge--"))
	prout(_("  Scott here.  The warp engines are damaged."))
	prout(_("  We'll have to reduce speed to warp 4."))
    game.ididit = True
    return

def setwarp():
    "Change the warp factor."
    while True:
        key=scanner.next()
        if key != "IHEOL":
            break
	scanner.chew()
	proutn(_("Warp factor- "))
    scanner.chew()
    if key != "IHREAL":
	huh()
	return
    if game.damage[DWARPEN] > 10.0:
	prout(_("Warp engines inoperative."))
	return
    if damaged(DWARPEN) and scanner.real > 4.0:
	prout(_("Engineer Scott- \"I'm doing my best, Captain,"))
	prout(_("  but right now we can only go warp 4.\""))
	return
    if scanner.real > 10.0:
	prout(_("Helmsman Sulu- \"Our top speed is warp 10, Captain.\""))
	return
    if scanner.real < 1.0:
	prout(_("Helmsman Sulu- \"We can't go below warp 1, Captain.\""))
	return
    oldfac = game.warpfac
    game.warpfac = scanner.real
    if game.warpfac <= oldfac or game.warpfac <= 6.0:
	prout(_("Helmsman Sulu- \"Warp factor %d, Captain.\"") %
	       int(game.warpfac))
	return
    if game.warpfac < 8.00:
	prout(_("Engineer Scott- \"Aye, but our maximum safe speed is warp 6.\""))
	return
    if game.warpfac == 10.0:
	prout(_("Engineer Scott- \"Aye, Captain, we'll try it.\""))
	return
    prout(_("Engineer Scott- \"Aye, Captain, but our engines may not take it.\""))
    return

def atover(igrab):
    "Cope with being tossed out of quadrant by supernova or yanked by beam."
    scanner.chew()
    # is captain on planet? 
    if game.landed:
	if damaged(DTRANSP):
	    finish(FPNOVA)
	    return
	prout(_("Scotty rushes to the transporter controls."))
	if game.shldup:
	    prout(_("But with the shields up it's hopeless."))
	    finish(FPNOVA)
	prouts(_("His desperate attempt to rescue you . . ."))
	if withprob(0.5):
	    prout(_("fails."))
	    finish(FPNOVA)
	    return
	prout(_("SUCCEEDS!"))
	if game.imine:
	    game.imine = False
	    proutn(_("The crystals mined were "))
	    if withprob(0.25):
		prout(_("lost."))
	    else:
		prout(_("saved."))
		game.icrystl = True
    if igrab:
	return
    # Check to see if captain in shuttle craft 
    if game.icraft:
	finish(FSTRACTOR)
    if game.alldone:
	return
    # Inform captain of attempt to reach safety 
    skip(1)
    while True:
	if game.justin:
	    prouts(_("***RED ALERT!  RED ALERT!"))
	    skip(1)
	    proutn(_("The %s has stopped in a quadrant containing") % crmshp())
	    prouts(_("   a supernova."))
	    skip(2)
	prout(_("***Emergency automatic override attempts to hurl ")+crmshp())
	prout(_("safely out of quadrant."))
	if not damaged(DRADIO):
	    game.state.galaxy[game.quadrant.i][game.quadrant.j].charted = True
	# Try to use warp engines 
	if damaged(DWARPEN):
	    skip(1)
	    prout(_("Warp engines damaged."))
	    finish(FSNOVAED)
	    return
	game.warpfac = randreal(6.0, 8.0)
	prout(_("Warp factor set to %d") % int(game.warpfac))
	power = 0.75*game.energy
	dist = power/(game.warpfac*game.warpfac*game.warpfac*(game.shldup+1))
	dist = max(dist, randreal(math.sqrt(2)))
        bugout = course(bearing=randreal(12), distance=dist)	# How dumb!
	game.optime = bugout.time(game.warpfac)
	game.justin = False
	game.inorbit = False
	warp(bugout, involuntary=True)
	if not game.justin:
	    # This is bad news, we didn't leave quadrant. 
	    if game.alldone:
		return
	    skip(1)
	    prout(_("Insufficient energy to leave quadrant."))
	    finish(FSNOVAED)
	    return
	# Repeat if another snova
        if not game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova:
            break
    if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)==0: 
	finish(FWON) # Snova killed remaining enemy. 

def timwrp():
    "Let's do the time warp again."
    prout(_("***TIME WARP ENTERED."))
    if game.state.snap and withprob(0.5):
	# Go back in time 
	prout(_("You are traveling backwards in time %d stardates.") %
	      int(game.state.date-game.snapsht.date))
	game.state = game.snapsht
	game.state.snap = False
	if len(game.state.kcmdr):
	    schedule(FTBEAM, expran(game.intime/len(game.state.kcmdr)))
	    schedule(FBATTAK, expran(0.3*game.intime))
	schedule(FSNOVA, expran(0.5*game.intime))
	# next snapshot will be sooner 
	schedule(FSNAP, expran(0.25*game.state.remtime))
				
	if game.state.nscrem:
	    schedule(FSCMOVE, 0.2777)	    
	game.isatb = 0
	unschedule(FCDBAS)
	unschedule(FSCDBAS)
	game.battle.invalidate()
	# Make sure Galileo is consistant -- Snapshot may have been taken
        # when on planet, which would give us two Galileos! 
	gotit = False
	for l in range(game.inplan):
	    if game.state.planets[l].known == "shuttle_down":
		gotit = True
		if game.iscraft == "onship" and game.ship=='E':
		    prout(_("Chekov-  \"Security reports the Galileo has disappeared, Sir!"))
		    game.iscraft = "offship"
	# Likewise, if in the original time the Galileo was abandoned, but
	# was on ship earlier, it would have vanished -- let's restore it.
	if game.iscraft == "offship" and not gotit and game.damage[DSHUTTL] >= 0.0:
	    prout(_("Chekov-  \"Security reports the Galileo has reappeared in the dock!\""))
	    game.iscraft = "onship"
        # There used to be code to do the actual reconstrction here,
        # but the starchart is now part of the snapshotted galaxy state.
	prout(_("Spock has reconstructed a correct star chart from memory"))
    else:
	# Go forward in time 
	game.optime = expran(0.5*game.intime)
	prout(_("You are traveling forward in time %d stardates.") % int(game.optime))
	# cheat to make sure no tractor beams occur during time warp 
	postpone(FTBEAM, game.optime)
	game.damage[DRADIO] += game.optime
    newqad()
    events()	# Stas Sergeev added this -- do pending events 

def probe():
    "Launch deep-space probe." 
    # New code to launch a deep space probe 
    if game.nprobes == 0:
	scanner.chew()
	skip(1)
	if game.ship == 'E': 
	    prout(_("Engineer Scott- \"We have no more deep space probes, Sir.\""))
	else:
	    prout(_("Ye Faerie Queene has no deep space probes."))
	return
    if damaged(DDSP):
	scanner.chew()
	skip(1)
	prout(_("Engineer Scott- \"The probe launcher is damaged, Sir.\""))
	return
    if is_scheduled(FDSPROB):
	scanner.chew()
	skip(1)
	if damaged(DRADIO) and game.condition != "docked":
	    prout(_("Spock-  \"Records show the previous probe has not yet"))
	    prout(_("   reached its destination.\""))
	else:
	    prout(_("Uhura- \"The previous probe is still reporting data, Sir.\""))
	return
    key = scanner.next()
    if key == "IHEOL":
        if game.nprobes == 1:
            prout(_("1 probe left."))
        else:
            prout(_("%d probes left") % game.nprobes)
	proutn(_("Are you sure you want to fire a probe? "))
	if ja() == False:
	    return
    game.isarmed = False
    if key == "IHALPHA" and scanner.token == "armed":
	game.isarmed = True
	key = scanner.next()
    elif key == "IHEOL":
	proutn(_("Arm NOVAMAX warhead? "))
	game.isarmed = ja()
    elif key == "IHREAL":		# first element of course
        scanner.push(scanner.token)
    try:
        game.probe = getcourse(isprobe=True)
    except TrekError:
        return
    game.nprobes -= 1
    schedule(FDSPROB, 0.01) # Time to move one sector
    prout(_("Ensign Chekov-  \"The deep space probe is launched, Captain.\""))
    game.ididit = True
    return

def mayday():
    "Yell for help from nearest starbase."
    # There's more than one way to move in this game! 
    scanner.chew()
    # Test for conditions which prevent calling for help 
    if game.condition == "docked":
	prout(_("Lt. Uhura-  \"But Captain, we're already docked.\""))
	return
    if damaged(DRADIO):
	prout(_("Subspace radio damaged."))
	return
    if not game.state.baseq:
	prout(_("Lt. Uhura-  \"Captain, I'm not getting any response from Starbase.\""))
	return
    if game.landed:
	prout(_("You must be aboard the %s.") % crmshp())
	return
    # OK -- call for help from nearest starbase 
    game.nhelp += 1
    if game.base.i!=0:
	# There's one in this quadrant 
	ddist = (game.base - game.sector).distance()
    else:
	ddist = FOREVER
        for ibq in game.state.baseq:
	    xdist = QUADSIZE * (ibq - game.quadrant).distance()
	    if xdist < ddist:
		ddist = xdist
	# Since starbase not in quadrant, set up new quadrant 
	game.quadrant = ibq
	newqad()
    # dematerialize starship 
    game.quad[game.sector.i][game.sector.j]='.'
    proutn(_("Starbase in Quadrant %s responds--%s dematerializes") \
           % (game.quadrant, crmshp()))
    game.sector.invalidate()
    for m in range(1, 5+1):
        w = game.base.scatter() 
	if w.valid_sector() and game.quad[w.i][w.j]=='.':
	    # found one -- finish up 
            game.sector = w
	    break
    if not game.sector.is_valid():
	prout(_("You have been lost in space..."))
	finish(FMATERIALIZE)
	return
    # Give starbase three chances to rematerialize starship 
    probf = math.pow((1.0 - math.pow(0.98,ddist)), 0.33333333)
    for m in range(1, 3+1):
	if m == 1: proutn(_("1st"))
	elif m == 2: proutn(_("2nd"))
	elif m == 3: proutn(_("3rd"))
	proutn(_(" attempt to re-materialize ") + crmshp())
	game.quad[ix][iy]=('-','o','O')[m-1]
	warble()
	if randreal() > probf:
	    break
	prout(_("fails."))
	curses.delay_output(500)
    if m > 3:
	game.quad[ix][iy]='?'
	game.alive = False
	drawmaps(1)
	setwnd(message_window)
	finish(FMATERIALIZE)
	return
    game.quad[ix][iy]=game.ship
    prout(_("succeeds."))
    dock(False)
    skip(1)
    prout(_("Lt. Uhura-  \"Captain, we made it!\""))

def abandon():
    "Abandon ship."
    scanner.chew()
    if game.condition=="docked":
	if game.ship!='E':
	    prout(_("You cannot abandon Ye Faerie Queene."))
	    return
    else:
	# Must take shuttle craft to exit 
	if game.damage[DSHUTTL]==-1:
	    prout(_("Ye Faerie Queene has no shuttle craft."))
	    return
	if game.damage[DSHUTTL]<0:
	    prout(_("Shuttle craft now serving Big Macs."))
	    return
	if game.damage[DSHUTTL]>0:
	    prout(_("Shuttle craft damaged."))
	    return
	if game.landed:
	    prout(_("You must be aboard the ship."))
	    return
	if game.iscraft != "onship":
	    prout(_("Shuttle craft not currently available."))
	    return
	# Emit abandon ship messages 
	skip(1)
	prouts(_("***ABANDON SHIP!  ABANDON SHIP!"))
	skip(1)
	prouts(_("***ALL HANDS ABANDON SHIP!"))
	skip(2)
	prout(_("Captain and crew escape in shuttle craft."))
	if not game.state.baseq:
	    # Oops! no place to go... 
	    finish(FABANDN)
	    return
	q = game.state.galaxy[game.quadrant.i][game.quadrant.j]
	# Dispose of crew 
	if not (game.options & OPTION_WORLDS) and not damaged(DTRANSP):
	    prout(_("Remainder of ship's complement beam down"))
	    prout(_("to nearest habitable planet."))
	elif q.planet != None and not damaged(DTRANSP):
	    prout(_("Remainder of ship's complement beam down to %s.") %
		    q.planet)
	else:
	    prout(_("Entire crew of %d left to die in outer space.") %
		    game.state.crew)
	    game.casual += game.state.crew
	    game.abandoned += game.state.crew
	# If at least one base left, give 'em the Faerie Queene 
	skip(1)
	game.icrystl = False # crystals are lost 
	game.nprobes = 0 # No probes 
	prout(_("You are captured by Klingons and released to"))
	prout(_("the Federation in a prisoner-of-war exchange."))
	nb = randrange(len(game.state.baseq))
	# Set up quadrant and position FQ adjacient to base 
	if not game.quadrant == game.state.baseq[nb]:
	    game.quadrant = game.state.baseq[nb]
	    game.sector.i = game.sector.j = 5
	    newqad()
	while True:
	    # position next to base by trial and error 
	    game.quad[game.sector.i][game.sector.j] = '.'
	    for l in range(QUADSIZE):
		game.sector = game.base.scatter()
		if game.sector.valid_sector() and \
                       game.quad[game.sector.i][game.sector.j] == '.':
                    break
	    if l < QUADSIZE+1:
		break # found a spot 
	    game.sector.i=QUADSIZE/2
	    game.sector.j=QUADSIZE/2
	    newqad()
    # Get new commission 
    game.quad[game.sector.i][game.sector.j] = game.ship = 'F'
    game.state.crew = FULLCREW
    prout(_("Starfleet puts you in command of another ship,"))
    prout(_("the Faerie Queene, which is antiquated but,"))
    prout(_("still useable."))
    if game.icrystl:
	prout(_("The dilithium crystals have been moved."))
    game.imine = False
    game.iscraft = "offship" # Galileo disappears 
    # Resupply ship 
    game.condition="docked"
    for l in range(NDEVICES): 
	game.damage[l] = 0.0
    game.damage[DSHUTTL] = -1
    game.energy = game.inenrg = 3000.0
    game.shield = game.inshld = 1250.0
    game.torps = game.intorps = 6
    game.lsupres=game.inlsr=3.0
    game.shldup=False
    game.warpfac=5.0
    return

# Code from planets.c begins here.

def consumeTime():
    "Abort a lengthy operation if an event interrupts it." 
    game.ididit = True
    events()
    if game.alldone or game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova or game.justin: 
	return True
    return False

def survey():
    "Report on (uninhabited) planets in the galaxy."
    iknow = False
    skip(1)
    scanner.chew()
    prout(_("Spock-  \"Planet report follows, Captain.\""))
    skip(1)
    for i in range(game.inplan):
	if game.state.planets[i].pclass == "destroyed":
	    continue
	if (game.state.planets[i].known != "unknown" \
            and not game.state.planets[i].inhabited) \
            or idebug:
	    iknow = True
	    if idebug and game.state.planets[i].known=="unknown":
		proutn("(Unknown) ")
	    proutn(_("Quadrant %s") % game.state.planets[i].quadrant)
	    proutn(_("   class "))
	    proutn(game.state.planets[i].pclass)
	    proutn("   ")
	    if game.state.planets[i].crystals != present:
		proutn(_("no "))
	    prout(_("dilithium crystals present."))
	    if game.state.planets[i].known=="shuttle_down": 
		prout(_("    Shuttle Craft Galileo on surface."))
    if not iknow:
	prout(_("No information available."))

def orbit():
    "Enter standard orbit." 
    skip(1)
    scanner.chew()
    if game.inorbit:
	prout(_("Already in standard orbit."))
	return
    if damaged(DWARPEN) and damaged(DIMPULS):
	prout(_("Both warp and impulse engines damaged."))
	return
    if not game.plnet.is_valid():
        prout("There is no planet in this sector.")
        return
    if abs(game.sector.i-game.plnet.i)>1 or abs(game.sector.j-game.plnet.j)>1:
	prout(crmshp() + _(" not adjacent to planet."))
	skip(1)
	return
    game.optime = randreal(0.02, 0.05)
    prout(_("Helmsman Sulu-  \"Entering standard orbit, Sir.\""))
    newcnd()
    if consumeTime():
	return
    game.height = randreal(1400, 8600)
    prout(_("Sulu-  \"Entered orbit at altitude %.2f kilometers.\"") % game.height)
    game.inorbit = True
    game.ididit = True

def sensor():
    "Examine planets in this quadrant."
    if damaged(DSRSENS):
	if game.options & OPTION_TTY:
	    prout(_("Short range sensors damaged."))
	return
    if game.iplnet == None:
	if game.options & OPTION_TTY:
	    prout(_("Spock- \"No planet in this quadrant, Captain.\""))
	return
    if game.iplnet.known == "unknown":
	prout(_("Spock-  \"Sensor scan for Quadrant %s-") % game.quadrant)
	skip(1)
	prout(_("         Planet at Sector %s is of class %s.") %
	      (game.plnet, game.iplnet.pclass))
	if game.iplnet.known=="shuttle_down": 
	    prout(_("         Sensors show Galileo still on surface."))
	proutn(_("         Readings indicate"))
	if game.iplnet.crystals != "present":
	    proutn(_(" no"))
	prout(_(" dilithium crystals present.\""))
	if game.iplnet.known == "unknown":
	    game.iplnet.known = "known"
    elif game.iplnet.inhabited:
        prout(_("Spock-  \"The inhabited planet %s ") % game.iplnet.name)
        prout(_("        is located at Sector %s, Captain.\"") % game.plnet)

def beam():
    "Use the transporter."
    nrgneed = 0
    scanner.chew()
    skip(1)
    if damaged(DTRANSP):
	prout(_("Transporter damaged."))
	if not damaged(DSHUTTL) and (game.iplnet.known=="shuttle_down" or game.iscraft == "onship"):
	    skip(1)
	    proutn(_("Spock-  \"May I suggest the shuttle craft, Sir?\" "))
	    if ja() == True:
		shuttle()
	return
    if not game.inorbit:
	prout(crmshp() + _(" not in standard orbit."))
	return
    if game.shldup:
	prout(_("Impossible to transport through shields."))
	return
    if game.iplnet.known=="unknown":
	prout(_("Spock-  \"Captain, we have no information on this planet"))
	prout(_("  and Starfleet Regulations clearly state that in this situation"))
	prout(_("  you may not go down.\""))
	return
    if not game.landed and game.iplnet.crystals=="absent":
	prout(_("Spock-  \"Captain, I fail to see the logic in"))
	prout(_("  exploring a planet with no dilithium crystals."))
	proutn(_("  Are you sure this is wise?\" "))
	if ja() == False:
	    scanner.chew()
	    return
    if not (game.options & OPTION_PLAIN):
	nrgneed = 50 * game.skill + game.height / 100.0
	if nrgneed > game.energy:
    	    prout(_("Engineering to bridge--"))
	    prout(_("  Captain, we don't have enough energy for transportation."))
	    return
	if not game.landed and nrgneed * 2 > game.energy:
    	    prout(_("Engineering to bridge--"))
	    prout(_("  Captain, we have enough energy only to transport you down to"))
	    prout(_("  the planet, but there wouldn't be an energy for the trip back."))
	    if game.iplnet.known == "shuttle_down":
		prout(_("  Although the Galileo shuttle craft may still be on a surface."))
	    proutn(_("  Are you sure this is wise?\" "))
	    if ja() == False:
		scanner.chew()
		return
    if game.landed:
	# Coming from planet 
	if game.iplnet.known=="shuttle_down":
	    proutn(_("Spock-  \"Wouldn't you rather take the Galileo?\" "))
	    if ja() == True:
		scanner.chew()
		return
	    prout(_("Your crew hides the Galileo to prevent capture by aliens."))
	prout(_("Landing party assembled, ready to beam up."))
	skip(1)
	prout(_("Kirk whips out communicator..."))
	prouts(_("BEEP  BEEP  BEEP"))
	skip(2)
	prout(_("\"Kirk to enterprise-  Lock on coordinates...energize.\""))
    else:
	# Going to planet 
	prout(_("Scotty-  \"Transporter room ready, Sir.\""))
	skip(1)
	prout(_("Kirk and landing party prepare to beam down to planet surface."))
	skip(1)
	prout(_("Kirk-  \"Energize.\""))
    game.ididit = True
    skip(1)
    prouts("WWHOOOIIIIIRRRRREEEE.E.E.  .  .  .  .   .    .")
    skip(2)
    if withprob(0.98):
	prouts("BOOOIIIOOOIIOOOOIIIOIING . . .")
	skip(2)
	prout(_("Scotty-  \"Oh my God!  I've lost them.\""))
	finish(FLOST)
	return
    prouts(".    .   .  .  .  .  .E.E.EEEERRRRRIIIIIOOOHWW")
    game.landed = not game.landed
    game.energy -= nrgneed
    skip(2)
    prout(_("Transport complete."))
    if game.landed and game.iplnet.known=="shuttle_down":
	prout(_("The shuttle craft Galileo is here!"))
    if not game.landed and game.imine:
	game.icrystl = True
	game.cryprob = 0.05
    game.imine = False
    return

def mine():
    "Strip-mine a world for dilithium."
    skip(1)
    scanner.chew()
    if not game.landed:
	prout(_("Mining party not on planet."))
	return
    if game.iplnet.crystals == "mined":
	prout(_("This planet has already been strip-mined for dilithium."))
	return
    elif game.iplnet.crystals == "absent":
	prout(_("No dilithium crystals on this planet."))
	return
    if game.imine:
	prout(_("You've already mined enough crystals for this trip."))
	return
    if game.icrystl and game.cryprob == 0.05:
	prout(_("With all those fresh crystals aboard the ") + crmshp())
	prout(_("there's no reason to mine more at this time."))
	return
    game.optime = randreal(0.1, 0.3)*(ord(game.iplnet.pclass)-ord("L"))
    if consumeTime():
	return
    prout(_("Mining operation complete."))
    game.iplnet.crystals = "mined"
    game.imine = game.ididit = True

def usecrystals():
    "Use dilithium crystals."
    game.ididit = False
    skip(1)
    scanner.chew()
    if not game.icrystl:
	prout(_("No dilithium crystals available."))
	return
    if game.energy >= 1000:
	prout(_("Spock-  \"Captain, Starfleet Regulations prohibit such an operation"))
	prout(_("  except when Condition Yellow exists."))
	return
    prout(_("Spock- \"Captain, I must warn you that loading"))
    prout(_("  raw dilithium crystals into the ship's power"))
    prout(_("  system may risk a severe explosion."))
    proutn(_("  Are you sure this is wise?\" "))
    if ja() == False:
	scanner.chew()
	return
    skip(1)
    prout(_("Engineering Officer Scott-  \"(GULP) Aye Sir."))
    prout(_("  Mr. Spock and I will try it.\""))
    skip(1)
    prout(_("Spock-  \"Crystals in place, Sir."))
    prout(_("  Ready to activate circuit.\""))
    skip(1)
    prouts(_("Scotty-  \"Keep your fingers crossed, Sir!\""))
    skip(1)
    if withprob(game.cryprob):
	prouts(_("  \"Activating now! - - No good!  It's***"))
	skip(2)
	prouts(_("***RED ALERT!  RED A*L********************************"))
	skip(1)
	stars()
	prouts(_("******************   KA-BOOM!!!!   *******************"))
	skip(1)
	kaboom()
	return
    game.energy += randreal(5000.0, 5500.0)
    prouts(_("  \"Activating now! - - "))
    prout(_("The instruments"))
    prout(_("   are going crazy, but I think it's"))
    prout(_("   going to work!!  Congratulations, Sir!\""))
    game.cryprob *= 2.0
    game.ididit = True

def shuttle():
    "Use shuttlecraft for planetary jaunt."
    scanner.chew()
    skip(1)
    if damaged(DSHUTTL):
	if game.damage[DSHUTTL] == -1.0:
	    if game.inorbit and game.iplnet.known == "shuttle_down":
		prout(_("Ye Faerie Queene has no shuttle craft bay to dock it at."))
	    else:
		prout(_("Ye Faerie Queene had no shuttle craft."))
	elif game.damage[DSHUTTL] > 0:
	    prout(_("The Galileo is damaged."))
	else: # game.damage[DSHUTTL] < 0  
	    prout(_("Shuttle craft is now serving Big Macs."))
	return
    if not game.inorbit:
	prout(crmshp() + _(" not in standard orbit."))
	return
    if (game.iplnet.known != "shuttle_down") and game.iscraft != "onship":
	prout(_("Shuttle craft not currently available."))
	return
    if not game.landed and game.iplnet.known=="shuttle_down":
	prout(_("You will have to beam down to retrieve the shuttle craft."))
	return
    if game.shldup or game.condition == "docked":
	prout(_("Shuttle craft cannot pass through shields."))
	return
    if game.iplnet.known=="unknown":
	prout(_("Spock-  \"Captain, we have no information on this planet"))
	prout(_("  and Starfleet Regulations clearly state that in this situation"))
	prout(_("  you may not fly down.\""))
	return
    game.optime = 3.0e-5*game.height
    if game.optime >= 0.8*game.state.remtime:
	prout(_("First Officer Spock-  \"Captain, I compute that such"))
	proutn(_("  a maneuver would require approximately %2d%% of our") % \
	       int(100*game.optime/game.state.remtime))
	prout(_("remaining time."))
	proutn(_("Are you sure this is wise?\" "))
	if ja() == False:
	    game.optime = 0.0
	    return
    if game.landed:
	# Kirk on planet 
	if game.iscraft == "onship":
	    # Galileo on ship! 
	    if not damaged(DTRANSP):
		proutn(_("Spock-  \"Would you rather use the transporter?\" "))
		if ja() == True:
		    beam()
		    return
		proutn(_("Shuttle crew"))
	    else:
		proutn(_("Rescue party"))
	    prout(_(" boards Galileo and swoops toward planet surface."))
	    game.iscraft = "offship"
	    skip(1)
	    if consumeTime():
		return
	    game.iplnet.known="shuttle_down"
	    prout(_("Trip complete."))
	    return
	else:
	    # Ready to go back to ship 
	    prout(_("You and your mining party board the"))
	    prout(_("shuttle craft for the trip back to the Enterprise."))
	    skip(1)
	    prouts(_("The short hop begins . . ."))
	    skip(1)
	    game.iplnet.known="known"
	    game.icraft = True
	    skip(1)
	    game.landed = False
	    if consumeTime():
		return
	    game.iscraft = "onship"
	    game.icraft = False
	    if game.imine:
		game.icrystl = True
		game.cryprob = 0.05
	    game.imine = False
	    prout(_("Trip complete."))
	    return
    else:
	# Kirk on ship and so is Galileo 
	prout(_("Mining party assembles in the hangar deck,"))
	prout(_("ready to board the shuttle craft \"Galileo\"."))
	skip(1)
	prouts(_("The hangar doors open; the trip begins."))
	skip(1)
	game.icraft = True
	game.iscraft = "offship"
	if consumeTime():
	    return
	game.iplnet.known = "shuttle_down"
	game.landed = True
	game.icraft = False
	prout(_("Trip complete."))
	return

def deathray():
    "Use the big zapper."
    game.ididit = False
    skip(1)
    scanner.chew()
    if game.ship != 'E':
	prout(_("Ye Faerie Queene has no death ray."))
	return
    if len(game.enemies)==0:
	prout(_("Sulu-  \"But Sir, there are no enemies in this quadrant.\""))
	return
    if damaged(DDRAY):
	prout(_("Death Ray is damaged."))
	return
    prout(_("Spock-  \"Captain, the 'Experimental Death Ray'"))
    prout(_("  is highly unpredictible.  Considering the alternatives,"))
    proutn(_("  are you sure this is wise?\" "))
    if ja() == False:
	return
    prout(_("Spock-  \"Acknowledged.\""))
    skip(1)
    game.ididit = True
    prouts(_("WHOOEE ... WHOOEE ... WHOOEE ... WHOOEE"))
    skip(1)
    prout(_("Crew scrambles in emergency preparation."))
    prout(_("Spock and Scotty ready the death ray and"))
    prout(_("prepare to channel all ship's power to the device."))
    skip(1)
    prout(_("Spock-  \"Preparations complete, sir.\""))
    prout(_("Kirk-  \"Engage!\""))
    skip(1)
    prouts(_("WHIRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"))
    skip(1)
    dprob = 0.30
    if game.options & OPTION_PLAIN:
	dprob = 0.5
    r = randreal()
    if r > dprob:
	prouts(_("Sulu- \"Captain!  It's working!\""))
	skip(2)
	while len(game.enemies) > 0:
	    deadkl(game.enemies[1].location, game.quad[game.enemies[1].location.i][game.enemies[1].location.j],game.enemies[1].location)
	prout(_("Ensign Chekov-  \"Congratulations, Captain!\""))
	if (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem) == 0:
	    finish(FWON)    
	if (game.options & OPTION_PLAIN) == 0:
	    prout(_("Spock-  \"Captain, I believe the `Experimental Death Ray'"))
	    if withprob(0.05):
		prout(_("   is still operational.\""))
	    else:
		prout(_("   has been rendered nonfunctional.\""))
		game.damage[DDRAY] = 39.95
	return
    r = randreal()	# Pick failure method 
    if r <= 0.30:
	prouts(_("Sulu- \"Captain!  It's working!\""))
	skip(1)
	prouts(_("***RED ALERT!  RED ALERT!"))
	skip(1)
	prout(_("***MATTER-ANTIMATTER IMPLOSION IMMINENT!"))
	skip(1)
	prouts(_("***RED ALERT!  RED A*L********************************"))
	skip(1)
	stars()
	prouts(_("******************   KA-BOOM!!!!   *******************"))
	skip(1)
	kaboom()
	return
    if r <= 0.55:
	prouts(_("Sulu- \"Captain!  Yagabandaghangrapl, brachriigringlanbla!\""))
	skip(1)
	prout(_("Lt. Uhura-  \"Graaeek!  Graaeek!\""))
	skip(1)
	prout(_("Spock-  \"Fascinating!  . . . All humans aboard"))
	prout(_("  have apparently been transformed into strange mutations."))
	prout(_("  Vulcans do not seem to be affected."))
	skip(1)
	prout(_("Kirk-  \"Raauch!  Raauch!\""))
	finish(FDRAY)
	return
    if r <= 0.75:
	intj
	prouts(_("Sulu- \"Captain!  It's   --WHAT?!?!\""))
	skip(2)
	proutn(_("Spock-  \"I believe the word is"))
	prouts(_(" *ASTONISHING*"))
	prout(_(" Mr. Sulu."))
	for i in range(QUADSIZE):
	    for j in range(QUADSIZE):
		if game.quad[i][j] == '.':
		    game.quad[i][j] = '?'
	prout(_("  Captain, our quadrant is now infested with"))
	prouts(_(" - - - - - -  *THINGS*."))
	skip(1)
	prout(_("  I have no logical explanation.\""))
	return
    prouts(_("Sulu- \"Captain!  The Death Ray is creating tribbles!\""))
    skip(1)
    prout(_("Scotty-  \"There are so many tribbles down here"))
    prout(_("  in Engineering, we can't move for 'em, Captain.\""))
    finish(FTRIBBLE)
    return

# Code from reports.c begins here

def attackreport(curt):
    "eport status of bases under attack."
    if not curt:
	if is_scheduled(FCDBAS):
	    prout(_("Starbase in Quadrant %s is currently under Commander attack.") % game.battle)
	    prout(_("It can hold out until Stardate %d.") % int(scheduled(FCDBAS)))
	elif game.isatb == 1:
	    prout(_("Starbase in Quadrant %s is under Super-commander attack.") % game.state.kscmdr)
	    prout(_("It can hold out until Stardate %d.") % int(scheduled(FSCDBAS)))
	else:
	    prout(_("No Starbase is currently under attack."))
    else:
        if is_scheduled(FCDBAS):
	    proutn(_("Base in %s attacked by C. Alive until %.1f") % (game.battle, scheduled(FCDBAS)))
        if game.isatb:
	    proutn(_("Base in %s attacked by S. Alive until %.1f") % (game.state.kscmdr, scheduled(FSCDBAS)))
	clreol()

def report():
    # report on general game status 
    scanner.chew()
    s1 = "" and game.thawed and _("thawed ")
    s2 = {1:"short", 2:"medium", 4:"long"}[game.length]
    s3 = (None, _("novice"). _("fair"),
          _("good"), _("expert"), _("emeritus"))[game.skill]
    prout(_("You %s a %s%s %s game.") % ((_("were playing"), _("are playing"))[game.alldone], s1, s2, s3))
    if game.skill>SKILL_GOOD and game.thawed and not game.alldone:
	prout(_("No plaque is allowed."))
    if game.tourn:
	prout(_("This is tournament game %d.") % game.tourn)
    prout(_("Your secret password is \"%s\"") % game.passwd)
    proutn(_("%d of %d Klingons have been killed") % (((game.inkling + game.incom + game.inscom) - (game.state.remkl + len(game.state.kcmdr) + game.state.nscrem)), 
	   (game.inkling + game.incom + game.inscom)))
    if game.incom - len(game.state.kcmdr):
	prout(_(", including %d Commander%s.") % (game.incom - len(game.state.kcmdr), (_("s"), "")[(game.incom - len(game.state.kcmdr))==1]))
    elif game.inkling - game.state.remkl + (game.inscom - game.state.nscrem) > 0:
	prout(_(", but no Commanders."))
    else:
	prout(".")
    if game.skill > SKILL_FAIR:
	prout(_("The Super Commander has %sbeen destroyed.") % ("", _("not "))[game.state.nscrem])
    if len(game.state.baseq) != game.inbase:
	proutn(_("There "))
	if game.inbase-len(game.state.baseq)==1:
	    proutn(_("has been 1 base"))
	else:
	    proutn(_("have been %d bases") % (game.inbase-len(game.state.baseq)))
	prout(_(" destroyed, %d remaining.") % len(game.state.baseq))
    else:
	prout(_("There are %d bases.") % game.inbase)
    if communicating() or game.iseenit:
	# Don't report this if not seen and
	# either the radio is dead or not at base!
	attackreport(False)
	game.iseenit = True
    if game.casual: 
	prout(_("%d casualt%s suffered so far.") % (game.casual, ("y", "ies")[game.casual!=1]))
    if game.nhelp:
	prout(_("There were %d call%s for help.") % (game.nhelp,  ("" , _("s"))[game.nhelp!=1]))
    if game.ship == 'E':
	proutn(_("You have "))
	if game.nprobes:
	    proutn("%d" % (game.nprobes))
	else:
	    proutn(_("no"))
	proutn(_(" deep space probe"))
	if game.nprobes!=1:
	    proutn(_("s"))
	prout(".")
    if communicating() and is_scheduled(FDSPROB):
	if game.isarmed: 
	    proutn(_("An armed deep space probe is in "))
	else:
	    proutn(_("A deep space probe is in "))
	prout("Quadrant %s." % game.probec)
    if game.icrystl:
	if game.cryprob <= .05:
	    prout(_("Dilithium crystals aboard ship... not yet used."))
	else:
	    i=0
	    ai = 0.05
	    while game.cryprob > ai:
		ai *= 2.0
		i += 1
	    prout(_("Dilithium crystals have been used %d time%s.") % \
                  (i, (_("s"), "")[i==1]))
    skip(1)
	
def lrscan(silent):
    "Long-range sensor scan."
    if damaged(DLRSENS):
	# Now allow base's sensors if docked 
	if game.condition != "docked":
            if not silent:
                prout(_("LONG-RANGE SENSORS DAMAGED."))
	    return
        if not silent:
            prout(_("Starbase's long-range scan"))
    elif not silent:
	prout(_("Long-range scan"))
    for x in range(game.quadrant.i-1, game.quadrant.i+2):
        if not silent:
            proutn(" ")
        for y in range(game.quadrant.j-1, game.quadrant.j+2):
	    if not coord(x, y).valid_quadrant():
                if not silent:
                    proutn("  -1")
	    else:
		if not damaged(DRADIO):
		    game.state.galaxy[x][y].charted = True
		game.state.chart[x][y].klingons = game.state.galaxy[x][y].klingons
		game.state.chart[x][y].starbase = game.state.galaxy[x][y].starbase
		game.state.chart[x][y].stars = game.state.galaxy[x][y].stars
		if not silent and game.state.galaxy[x][y].supernova: 
		    proutn(" ***")
		elif not silent:
		    proutn(" %3d" % (game.state.chart[x][y].klingons*100 + game.state.chart[x][y].starbase * 10 + game.state.chart[x][y].stars))
	prout(" ")

def damagereport():
    "Damage report."
    jdam = False
    scanner.chew()
    for i in range(NDEVICES):
	if damaged(i):
	    if not jdam:
		prout(_("\tDEVICE\t\t\t-REPAIR TIMES-"))
		prout(_("\t\t\tIN FLIGHT\t\tDOCKED"))
		jdam = True
	    prout("  %-26s\t%8.2f\t\t%8.2f" % (device[i],
                                               game.damage[i]+0.05,
                                               DOCKFAC*game.damage[i]+0.005))
    if not jdam:
	prout(_("All devices functional."))

def rechart():
    "Update the chart in the Enterprise's computer from galaxy data."
    game.lastchart = game.state.date
    for i in range(GALSIZE):
	for j in range(GALSIZE):
	    if game.state.galaxy[i][j].charted:
		game.state.chart[i][j].klingons = game.state.galaxy[i][j].klingons
		game.state.chart[i][j].starbase = game.state.galaxy[i][j].starbase
		game.state.chart[i][j].stars = game.state.galaxy[i][j].stars

def chart():
    "Display the star chart."
    scanner.chew()
    if (game.options & OPTION_AUTOSCAN):
        lrscan(silent=True)
    if not damaged(DRADIO):
	rechart()
    if game.lastchart < game.state.date and game.condition == "docked":
	prout(_("Spock-  \"I revised the Star Chart from the starbase's records.\""))
	rechart()
    prout(_("       STAR CHART FOR THE KNOWN GALAXY"))
    if game.state.date > game.lastchart:
	prout(_("(Last surveillance update %d stardates ago).") % ((int)(game.state.date-game.lastchart)))
    prout("      1    2    3    4    5    6    7    8")
    for i in range(GALSIZE):
	proutn("%d |" % (i+1))
	for j in range(GALSIZE):
	    if (game.options & OPTION_SHOWME) and i == game.quadrant.i and j == game.quadrant.j:
		proutn("<")
	    else:
		proutn(" ")
	    if game.state.galaxy[i][j].supernova:
		show = "***"
	    elif not game.state.galaxy[i][j].charted and game.state.galaxy[i][j].starbase:
		show = ".1."
	    elif game.state.galaxy[i][j].charted:
		show = "%3d" % (game.state.chart[i][j].klingons*100 + game.state.chart[i][j].starbase * 10 + game.state.chart[i][j].stars)
	    else:
		show = "..."
	    proutn(show)
	    if (game.options & OPTION_SHOWME) and i == game.quadrant.i and j == game.quadrant.j:
		proutn(">")
	    else:
		proutn(" ")
	proutn("  |")
	if i<GALSIZE:
	    skip(1)

def sectscan(goodScan, i, j):
    "Light up an individual dot in a sector."
    if goodScan or (abs(i-game.sector.i)<= 1 and abs(j-game.sector.j) <= 1):
	proutn("%c " % game.quad[i][j])
    else:
	proutn("- ")

def status(req=0):
    "Emit status report lines"
    if not req or req == 1:
	prstat(_("Stardate"), _("%.1f, Time Left %.2f") \
               % (game.state.date, game.state.remtime))
    if not req or req == 2:
	if game.condition != "docked":
	    newcnd()
	prstat(_("Condition"), _("%s, %i DAMAGES") % \
               (game.condition.upper(), sum(map(lambda x: x > 0, game.damage))))
    if not req or req == 3:
	prstat(_("Position"), "%s , %s" % (game.quadrant, game.sector))
    if not req or req == 4:
	if damaged(DLIFSUP):
	    if game.condition == "docked":
		s = _("DAMAGED, Base provides")
	    else:
		s = _("DAMAGED, reserves=%4.2f") % game.lsupres
	else:
	    s = _("ACTIVE")
	prstat(_("Life Support"), s)
    if not req or req == 5:
	prstat(_("Warp Factor"), "%.1f" % game.warpfac)
    if not req or req == 6:
        extra = ""
        if game.icrystl and (game.options & OPTION_SHOWME):
            extra = _(" (have crystals)")
	prstat(_("Energy"), "%.2f%s" % (game.energy, extra))
    if not req or req == 7:
	prstat(_("Torpedoes"), "%d" % (game.torps))
    if not req or req == 8:
	if damaged(DSHIELD):
	    s = _("DAMAGED,")
	elif game.shldup:
	    s = _("UP,")
	else:
	    s = _("DOWN,")
	data = _(" %d%% %.1f units") \
               % (int((100.0*game.shield)/game.inshld + 0.5), game.shield)
	prstat(_("Shields"), s+data)
    if not req or req == 9:
        prstat(_("Klingons Left"), "%d" \
               % (game.state.remkl+len(game.state.kcmdr)+game.state.nscrem))
    if not req or req == 10:
	if game.options & OPTION_WORLDS:
	    plnet = game.state.galaxy[game.quadrant.i][game.quadrant.j].planet
	    if plnet and plnet.inhabited:
		prstat(_("Major system"), plnet.name)
	    else:
		prout(_("Sector is uninhabited"))
    elif not req or req == 11:
	attackreport(not req)

def request():
    "Request specified status data, a historical relic from slow TTYs."
    requests = ("da","co","po","ls","wa","en","to","sh","kl","sy", "ti")
    while scanner.next() == "IHEOL":
	proutn(_("Information desired? "))
    scanner.chew()
    if scanner.token in requests:
        status(requests.index(scanner.token))
    else:
	prout(_("UNRECOGNIZED REQUEST. Legal requests are:"))
	prout(("  date, condition, position, lsupport, warpfactor,"))
	prout(("  energy, torpedoes, shields, klingons, system, time."))
		
def srscan():
    "Short-range scan." 
    goodScan=True
    if damaged(DSRSENS):
	# Allow base's sensors if docked 
	if game.condition != "docked":
	    prout(_("   S.R. SENSORS DAMAGED!"))
	    goodScan=False
	else:
	    prout(_("  [Using Base's sensors]"))
    else:
	prout(_("     Short-range scan"))
    if goodScan and not damaged(DRADIO): 
	game.state.chart[game.quadrant.i][game.quadrant.j].klingons = game.state.galaxy[game.quadrant.i][game.quadrant.j].klingons
	game.state.chart[game.quadrant.i][game.quadrant.j].starbase = game.state.galaxy[game.quadrant.i][game.quadrant.j].starbase
	game.state.chart[game.quadrant.i][game.quadrant.j].stars = game.state.galaxy[game.quadrant.i][game.quadrant.j].stars
	game.state.galaxy[game.quadrant.i][game.quadrant.j].charted = True
    prout("    1 2 3 4 5 6 7 8 9 10")
    if game.condition != "docked":
	newcnd()
    for i in range(QUADSIZE):
	proutn("%2d  " % (i+1))
	for j in range(QUADSIZE):
	    sectscan(goodScan, i, j)
	skip(1)
		
def eta():
    "Use computer to get estimated time of arrival for a warp jump."
    w1 = coord(); w2 = coord()
    prompt = False
    if damaged(DCOMPTR):
	prout(_("COMPUTER DAMAGED, USE A POCKET CALCULATOR."))
	skip(1)
	return
    if scanner.next() != "IHREAL":
	prompt = True
	scanner.chew()
	proutn(_("Destination quadrant and/or sector? "))
	if scanner.next()!="IHREAL":
	    huh()
	    return
    w1.j = int(scanner.real-0.5)
    if scanner.next() != "IHREAL":
	huh()
	return
    w1.i = int(scanner.real-0.5)
    if scanner.next() == "IHREAL":
	w2.j = int(scanner.real-0.5)
	if scanner.next() != "IHREAL":
	    huh()
	    return
	w2.i = int(scanner.real-0.5)
    else:
	if game.quadrant.j>w1.i:
	    w2.i = 0
	else:
	    w2.i=QUADSIZE-1
	if game.quadrant.i>w1.j:
	    w2.j = 0
	else:
	    w2.j=QUADSIZE-1
    if not w1.valid_quadrant() or not w2.valid_sector():
	huh()
	return
    dist = math.sqrt((w1.j-game.quadrant.j+(w2.j-game.sector.j)/(QUADSIZE*1.0))**2+
		(w1.i-game.quadrant.i+(w2.i-game.sector.i)/(QUADSIZE*1.0))**2)
    wfl = False
    if prompt:
	prout(_("Answer \"no\" if you don't know the value:"))
    while True:
	scanner.chew()
	proutn(_("Time or arrival date? "))
	if scanner.next()=="IHREAL":
	    ttime = scanner.real
	    if ttime > game.state.date:
		ttime -= game.state.date # Actually a star date
            twarp=(math.floor(math.sqrt((10.0*dist)/ttime)*10.0)+1.0)/10.0
            if ttime <= 1e-10 or twarp > 10:
		prout(_("We'll never make it, sir."))
		scanner.chew()
		return
	    if twarp < 1.0:
		twarp = 1.0
	    break
	scanner.chew()
	proutn(_("Warp factor? "))
	if scanner.next()== "IHREAL":
	    wfl = True
	    twarp = scanner.real
	    if twarp<1.0 or twarp > 10.0:
		huh()
		return
	    break
	prout(_("Captain, certainly you can give me one of these."))
    while True:
	scanner.chew()
	ttime = (10.0*dist)/twarp**2
	tpower = dist*twarp*twarp*twarp*(game.shldup+1)
	if tpower >= game.energy:
	    prout(_("Insufficient energy, sir."))
	    if not game.shldup or tpower > game.energy*2.0:
		if not wfl:
		    return
		proutn(_("New warp factor to try? "))
		if scanner.next() == "IHREAL":
		    wfl = True
		    twarp = scanner.real
		    if twarp<1.0 or twarp > 10.0:
			huh()
			return
		    continue
		else:
		    scanner.chew()
		    skip(1)
		    return
	    prout(_("But if you lower your shields,"))
	    proutn(_("remaining"))
	    tpower /= 2
	else:
	    proutn(_("Remaining"))
	prout(_(" energy will be %.2f.") % (game.energy-tpower))
	if wfl:
	    prout(_("And we will arrive at stardate %.2f.") % (game.state.date+ttime))
	elif twarp==1.0:
	    prout(_("Any warp speed is adequate."))
	else:
	    prout(_("Minimum warp needed is %.2f,") % (twarp))
	    prout(_("and we will arrive at stardate %.2f.") % (game.state.date+ttime))
	if game.state.remtime < ttime:
	    prout(_("Unfortunately, the Federation will be destroyed by then."))
	if twarp > 6.0:
	    prout(_("You'll be taking risks at that speed, Captain"))
	if (game.isatb==1 and game.state.kscmdr == w1 and \
	     scheduled(FSCDBAS)< ttime+game.state.date) or \
	    (scheduled(FCDBAS)<ttime+game.state.date and game.battle == w1):
	    prout(_("The starbase there will be destroyed by then."))
	proutn(_("New warp factor to try? "))
	if scanner.next() == "IHREAL":
	    wfl = True
	    twarp = scanner.real
	    if twarp<1.0 or twarp > 10.0:
		huh()
		return
	else:
	    scanner.chew()
	    skip(1)
	    return

# Code from setup.c begins here

def prelim():
    "Issue a historically correct banner."
    skip(2)
    prout(_("-SUPER- STAR TREK"))
    skip(1)
# From the FORTRAN original
#    prout(_("Latest update-21 Sept 78"))
#    skip(1)

def freeze(boss):
    "Save game."
    if boss:
	scanner.push("emsave.trk")
    key = scanner.next()
    if key == "IHEOL":
        proutn(_("File name: "))
        key = scanner.next()
    if key != "IHALPHA":
        huh()
        return
    scanner.chew()
    if '.' not in scanner.token:
        scanner.token += ".trk"
    try:
        fp = open(scanner.token, "wb")
    except IOError:
	prout(_("Can't freeze game as file %s") % scanner.token)
	return
    cPickle.dump(game, fp)
    fp.close()

def thaw():
    "Retrieve saved game." 
    game.passwd[0] = '\0'
    key = scanner.next()
    if key == "IHEOL":
	proutn(_("File name: "))
	key = scanner.next()
    if key != "IHALPHA":
	huh()
	return True
    scanner.chew()
    if '.' not in scanner.token:
        scanner.token += ".trk"
    try:
        fp = open(scanner.token, "rb")
    except IOError:
	prout(_("Can't thaw game in %s") % scanner.token)
	return
    game = cPickle.load(fp)
    fp.close()
    return False

# I used <http://www.memory-alpha.org> to find planets
# with references in ST:TOS.  Eath and the Alpha Centauri
# Colony have been omitted.
# 
# Some planets marked Class G and P here will be displayed as class M
# because of the way planets are generated. This is a known bug.
systnames = (
    # Federation Worlds 
    _("Andoria (Fesoan)"),	# several episodes 
    _("Tellar Prime (Miracht)"),	# TOS: "Journey to Babel" 
    _("Vulcan (T'Khasi)"),	# many episodes 
    _("Medusa"),		# TOS: "Is There in Truth No Beauty?" 
    _("Argelius II (Nelphia)"),	# TOS: "Wolf in the Fold" ("IV" in BSD) 
    _("Ardana"),		# TOS: "The Cloud Minders" 
    _("Catulla (Cendo-Prae)"),	# TOS: "The Way to Eden" 
    _("Gideon"),		# TOS: "The Mark of Gideon" 
    _("Aldebaran III"),		# TOS: "The Deadly Years" 
    _("Alpha Majoris I"),	# TOS: "Wolf in the Fold" 
    _("Altair IV"),		# TOS: "Amok Time 
    _("Ariannus"),		# TOS: "Let That Be Your Last Battlefield" 
    _("Benecia"),		# TOS: "The Conscience of the King" 
    _("Beta Niobe I (Sarpeidon)"),	# TOS: "All Our Yesterdays" 
    _("Alpha Carinae II"),	# TOS: "The Ultimate Computer" 
    _("Capella IV (Kohath)"),	# TOS: "Friday's Child" (Class G) 
    _("Daran V"),		# TOS: "For the World is Hollow and I Have Touched the Sky" 
    _("Deneb II"),		# TOS: "Wolf in the Fold" ("IV" in BSD) 
    _("Eminiar VII"),		# TOS: "A Taste of Armageddon" 
    _("Gamma Canaris IV"),	# TOS: "Metamorphosis" 
    _("Gamma Tranguli VI (Vaalel)"),	# TOS: "The Apple" 
    _("Ingraham B"),		# TOS: "Operation: Annihilate" 
    _("Janus IV"),		# TOS: "The Devil in the Dark" 
    _("Makus III"),		# TOS: "The Galileo Seven" 
    _("Marcos XII"),		# TOS: "And the Children Shall Lead", 
    _("Omega IV"),		# TOS: "The Omega Glory" 
    _("Regulus V"),		# TOS: "Amok Time 
    _("Deneva"),		# TOS: "Operation -- Annihilate!" 
    # Worlds from BSD Trek 
    _("Rigel II"),		# TOS: "Shore Leave" ("III" in BSD) 
    _("Beta III"),		# TOS: "The Return of the Archons" 
    _("Triacus"),		# TOS: "And the Children Shall Lead", 
    _("Exo III"),		# TOS: "What Are Little Girls Made Of?" (Class P) 
#	# Others 
#    _("Hansen's Planet"),	# TOS: "The Galileo Seven" 
#    _("Taurus IV"),		# TOS: "The Galileo Seven" (class G) 
#    _("Antos IV (Doraphane)"),	# TOS: "Whom Gods Destroy", "Who Mourns for Adonais?" 
#    _("Izar"),			# TOS: "Whom Gods Destroy" 
#    _("Tiburon"),		# TOS: "The Way to Eden" 
#    _("Merak II"),		# TOS: "The Cloud Minders" 
#    _("Coridan (Desotriana)"),	# TOS: "Journey to Babel" 
#    _("Iotia"),		# TOS: "A Piece of the Action" 
)

device = (
	_("S. R. Sensors"), \
	_("L. R. Sensors"), \
	_("Phasers"), \
	_("Photon Tubes"), \
	_("Life Support"), \
	_("Warp Engines"), \
	_("Impulse Engines"), \
	_("Shields"), \
	_("Subspace Radio"), \
	_("Shuttle Craft"), \
	_("Computer"), \
	_("Navigation System"), \
	_("Transporter"), \
	_("Shield Control"), \
	_("Death Ray"), \
	_("D. S. Probe"), \
)

def setup():
    "Prepare to play, set up cosmos."
    w = coord()
    #  Decide how many of everything
    if choose():
	return # frozen game
    # Prepare the Enterprise
    game.alldone = game.gamewon = game.shldchg = game.shldup = False
    game.ship = 'E'
    game.state.crew = FULLCREW
    game.energy = game.inenrg = 5000.0
    game.shield = game.inshld = 2500.0
    game.inlsr = 4.0
    game.lsupres = 4.0
    game.quadrant = randplace(GALSIZE)
    game.sector = randplace(QUADSIZE)
    game.torps = game.intorps = 10
    game.nprobes = randrange(2, 5)
    game.warpfac = 5.0
    for i in range(NDEVICES): 
	game.damage[i] = 0.0
    # Set up assorted game parameters
    game.battle = coord()
    game.state.date = game.indate = 100.0 * randreal(20, 51)
    game.nkinks = game.nhelp = game.casual = game.abandoned = 0
    game.iscate = game.resting = game.imine = game.icrystl = game.icraft = False
    game.isatb = game.state.nplankl = 0
    game.state.starkl = game.state.basekl = 0
    game.iscraft = "onship"
    game.landed = False
    game.alive = True
    # Starchart is functional but we've never seen it
    game.lastchart = FOREVER
    # Put stars in the galaxy
    game.instar = 0
    for i in range(GALSIZE):
	for j in range(GALSIZE):
	    k = randrange(1, QUADSIZE**2/10+1)
	    game.instar += k
	    game.state.galaxy[i][j].stars = k
    # Locate star bases in galaxy
    for i in range(game.inbase):
        while True:
            while True:
                w = randplace(GALSIZE)
                if not game.state.galaxy[w.i][w.j].starbase:
                    break
	    contflag = False
            # C version: for (j = i-1; j > 0; j--)
            # so it did them in the opposite order.
            for j in range(1, i):
		# Improved placement algorithm to spread out bases
		distq = (w - game.state.baseq[j]).distance()
		if distq < 6.0*(BASEMAX+1-game.inbase) and withprob(0.75):
		    contflag = True
		    if idebug:
			prout("=== Abandoning base #%d at %s" % (i, w))
		    break
		elif distq < 6.0 * (BASEMAX+1-game.inbase):
		    if idebug:
			prout("=== Saving base #%d, close to #%d" % (i, j))
            if not contflag:
                break
	game.state.baseq.append(w)
	game.state.galaxy[w.i][w.j].starbase = game.state.chart[w.i][w.j].starbase = True
    # Position ordinary Klingon Battle Cruisers
    krem = game.inkling
    klumper = 0.25*game.skill*(9.0-game.length)+1.0
    if klumper > MAXKLQUAD: 
	klumper = MAXKLQUAD
    while True:
	r = randreal()
	klump = (1.0 - r*r)*klumper
	if klump > krem:
	    klump = krem
	krem -= klump
        while True:
            w = randplace(GALSIZE)
            if not game.state.galaxy[w.i][w.j].supernova and \
               game.state.galaxy[w.i][w.j].klingons + klump <= MAXKLQUAD:
                break
	game.state.galaxy[w.i][w.j].klingons += int(klump)
        if krem <= 0:
            break
    # Position Klingon Commander Ships
    for i in range(game.incom):
        while True:
            w = randplace(GALSIZE)
            if not welcoming(w) or w in game.state.kcmdr:
                continue
            if (game.state.galaxy[w.i][w.j].klingons or withprob(0.25)):
                break
	game.state.galaxy[w.i][w.j].klingons += 1
	game.state.kcmdr.append(w)
    # Locate planets in galaxy
    for i in range(game.inplan):
        while True:
            w = randplace(GALSIZE) 
            if game.state.galaxy[w.i][w.j].planet == None:
                break
        new = planet()
	new.quadrant = w
        new.crystals = "absent"
	if (game.options & OPTION_WORLDS) and i < NINHAB:
	    new.pclass = "M"	# All inhabited planets are class M
	    new.crystals = "absent"
	    new.known = "known"
            new.name = systnames[i]
	    new.inhabited = True
	else:
	    new.pclass = ("M", "N", "O")[randrange(0, 3)]
            if withprob(0.33):
                new.crystals = "present"
	    new.known = "unknown"
	    new.inhabited = False
	game.state.galaxy[w.i][w.j].planet = new
        game.state.planets.append(new)
    # Locate Romulans
    for i in range(game.state.nromrem):
	w = randplace(GALSIZE)
	game.state.galaxy[w.i][w.j].romulans += 1
    # Place the Super-Commander if needed
    if game.state.nscrem > 0:
        while True:
            w = randplace(GALSIZE)
            if welcoming(w):
                break
	game.state.kscmdr = w
	game.state.galaxy[w.i][w.j].klingons += 1
    # Initialize times for extraneous events
    schedule(FSNOVA, expran(0.5 * game.intime))
    schedule(FTBEAM, expran(1.5 * (game.intime / len(game.state.kcmdr))))
    schedule(FSNAP, randreal(1.0, 2.0)) # Force an early snapshot
    schedule(FBATTAK, expran(0.3*game.intime))
    unschedule(FCDBAS)
    if game.state.nscrem:
	schedule(FSCMOVE, 0.2777)
    else:
	unschedule(FSCMOVE)
    unschedule(FSCDBAS)
    unschedule(FDSPROB)
    if (game.options & OPTION_WORLDS) and game.skill >= SKILL_GOOD:
	schedule(FDISTR, expran(1.0 + game.intime))
    else:
	unschedule(FDISTR)
    unschedule(FENSLV)
    unschedule(FREPRO)
    # Place thing (in tournament game, we don't want one!)
    # New in SST2K: never place the Thing near a starbase.
    # This makes sense and avoids a special case in the old code.
    global thing
    if game.tourn is None:
        while True:
            thing = randplace(GALSIZE)
            if thing not in game.state.baseq:
                break
    skip(2)
    game.state.snap = False
    if game.skill == SKILL_NOVICE:
	prout(_("It is stardate %d. The Federation is being attacked by") % int(game.state.date))
	prout(_("a deadly Klingon invasion force. As captain of the United"))
	prout(_("Starship U.S.S. Enterprise, it is your mission to seek out"))
	prout(_("and destroy this invasion force of %d battle cruisers.") % ((game.inkling + game.incom + game.inscom)))
	prout(_("You have an initial allotment of %d stardates to complete") % int(game.intime))
	prout(_("your mission.  As you proceed you may be given more time."))
	skip(1)
	prout(_("You will have %d supporting starbases.") % (game.inbase))
	proutn(_("Starbase locations-  "))
    else:
	prout(_("Stardate %d.") % int(game.state.date))
	skip(1)
	prout(_("%d Klingons.") % (game.inkling + game.incom + game.inscom))
	prout(_("An unknown number of Romulans."))
	if game.state.nscrem:
	    prout(_("And one (GULP) Super-Commander."))
	prout(_("%d stardates.") % int(game.intime))
	proutn(_("%d starbases in ") % game.inbase)
    for i in range(game.inbase):
	proutn(`game.state.baseq[i]`)
	proutn("  ")
    skip(2)
    proutn(_("The Enterprise is currently in Quadrant %s") % game.quadrant)
    proutn(_(" Sector %s") % game.sector)
    skip(2)
    prout(_("Good Luck!"))
    if game.state.nscrem:
	prout(_("  YOU'LL NEED IT."))
    waitfor()
    newqad()
    if len(game.enemies) - (thing == game.quadrant) - (game.tholian != None):
	game.shldup = True
    if game.neutz:	# bad luck to start in a Romulan Neutral Zone
	attack(torps_ok=False)

def choose():
    "Choose your game type."
    while True:
	game.tourn = game.length = 0
	game.thawed = False
	game.skill = SKILL_NONE
	if not scanner.inqueue: # Can start with command line options 
	    proutn(_("Would you like a regular, tournament, or saved game? "))
        scanner.next()
        if scanner.sees("tournament"):
	    while scanner.next() == "IHEOL":
		proutn(_("Type in tournament number-"))
	    if scanner.real == 0:
		scanner.chew()
		continue # We don't want a blank entry
	    game.tourn = int(round(scanner.real))
	    random.seed(scanner.real)
            if logfp:
                logfp.write("# random.seed(%d)\n" % scanner.real)
	    break
        if scanner.sees("saved") or scanner.sees("frozen"):
	    if thaw():
		continue
	    scanner.chew()
	    if game.passwd == None:
		continue
	    if not game.alldone:
		game.thawed = True # No plaque if not finished
	    report()
	    waitfor()
	    return True
        if scanner.sees("regular"):
	    break
	proutn(_("What is \"%s\"?") % scanner.token)
	scanner.chew()
    while game.length==0 or game.skill==SKILL_NONE:
	if scanner.next() == "IHALPHA":
            if scanner.sees("short"):
		game.length = 1
	    elif scanner.sees("medium"):
		game.length = 2
	    elif scanner.sees("long"):
		game.length = 4
	    elif scanner.sees("novice"):
		game.skill = SKILL_NOVICE
	    elif scanner.sees("fair"):
		game.skill = SKILL_FAIR
	    elif scanner.sees("good"):
		game.skill = SKILL_GOOD
	    elif scanner.sees("expert"):
		game.skill = SKILL_EXPERT
	    elif scanner.sees("emeritus"):
		game.skill = SKILL_EMERITUS
	    else:
		proutn(_("What is \""))
		proutn(scanner.token)
		prout("\"?")
	else:
	    scanner.chew()
	    if game.length==0:
		proutn(_("Would you like a Short, Medium, or Long game? "))
	    elif game.skill == SKILL_NONE:
		proutn(_("Are you a Novice, Fair, Good, Expert, or Emeritus player? "))
    # Choose game options -- added by ESR for SST2K
    if scanner.next() != "IHALPHA":
	scanner.chew()
	proutn(_("Choose your game style (plain, almy, fancy or just press enter): "))
	scanner.next()
    if scanner.sees("plain"):
	# Approximates the UT FORTRAN version.
	game.options &=~ (OPTION_THOLIAN | OPTION_PLANETS | OPTION_THINGY | OPTION_PROBE | OPTION_RAMMING | OPTION_MVBADDY | OPTION_BLKHOLE | OPTION_BASE | OPTION_WORLDS)
	game.options |= OPTION_PLAIN
    elif scanner.sees("almy"):
	# Approximates Tom Almy's version.
	game.options &=~ (OPTION_THINGY | OPTION_BLKHOLE | OPTION_BASE | OPTION_WORLDS)
	game.options |= OPTION_ALMY
    elif scanner.sees("fancy") or scanner.sees("\n"):
	pass
    elif len(scanner.token):
        proutn(_("What is \"%s\"?") % scanner.token)
    setpassword()
    if game.passwd == "debug":
	idebug = True
	prout("=== Debug mode enabled.")
    # Use parameters to generate initial values of things
    game.damfac = 0.5 * game.skill
    game.inbase = randrange(BASEMIN, BASEMAX+1)
    game.inplan = 0
    if game.options & OPTION_PLANETS:
	game.inplan += randrange(MAXUNINHAB/2, MAXUNINHAB+1)
    if game.options & OPTION_WORLDS:
	game.inplan += int(NINHAB)
    game.state.nromrem = game.inrom = randrange(2 *game.skill)
    game.state.nscrem = game.inscom = (game.skill > SKILL_FAIR)
    game.state.remtime = 7.0 * game.length
    game.intime = game.state.remtime
    game.state.remkl = game.inkling = 2.0*game.intime*((game.skill+1 - 2*randreal())*game.skill*0.1+.15)
    game.incom = min(MINCMDR, int(game.skill + 0.0625*game.inkling*randreal()))
    game.state.remres = (game.inkling+4*game.incom)*game.intime
    game.inresor = game.state.remres
    if game.inkling > 50:
        game.state.inbase += 1
    return False

def dropin(iquad=None):
    "Drop a feature on a random dot in the current quadrant."
    while True:
        w = randplace(QUADSIZE)
        if game.quad[w.i][w.j] == '.':
            break
    if iquad is not None:
        game.quad[w.i][w.j] = iquad
    return w

def newcnd():
    "Update our alert status."
    game.condition = "green"
    if game.energy < 1000.0:
	game.condition = "yellow"
    if game.state.galaxy[game.quadrant.i][game.quadrant.j].klingons or game.state.galaxy[game.quadrant.i][game.quadrant.j].romulans:
	game.condition = "red"
    if not game.alive:
	game.condition="dead"

def newkling():
    "Drop new Klingon into current quadrant."
    return enemy('K', loc=dropin(), power=randreal(300,450)+25.0*game.skill)

def newqad():
    "Set up a new state of quadrant, for when we enter or re-enter it."
    game.justin = True
    game.iplnet = None
    game.neutz = game.inorbit = game.landed = False
    game.ientesc = game.iseenit = False
    # Create a blank quadrant
    game.quad = fill2d(QUADSIZE, lambda i, j: '.')
    if game.iscate:
	# Attempt to escape Super-commander, so tbeam back!
	game.iscate = False
	game.ientesc = True
    q = game.state.galaxy[game.quadrant.i][game.quadrant.j]
    # cope with supernova
    if q.supernova:
	return
    game.klhere = q.klingons
    game.irhere = q.romulans
    # Position Starship
    game.quad[game.sector.i][game.sector.j] = game.ship
    game.enemies = []
    if q.klingons:
	# Position ordinary Klingons
	for i in range(game.klhere):
            newkling()
	# If we need a commander, promote a Klingon
        for cmdr in game.state.kcmdr:
	    if cmdr == game.quadrant:
                e = game.enemies[game.klhere-1]
                game.quad[e.location.i][e.location.j] = 'C'
                e.power = randreal(950,1350) + 50.0*game.skill
		break	
	# If we need a super-commander, promote a Klingon
	if game.quadrant == game.state.kscmdr:
            e = game.enemies[0]
	    game.quad[e.location.i][e.location.j] = 'S'
	    e.power = randreal(1175.0,  1575.0) + 125.0*game.skill
	    game.iscate = (game.state.remkl > 1)
    # Put in Romulans if needed
    for i in range(q.romulans):
        enemy('R', loc=dropin(), power=randreal(400.0,850.0)+50.0*game.skill)
    # If quadrant needs a starbase, put it in
    if q.starbase:
	game.base = dropin('B')
    # If quadrant needs a planet, put it in
    if q.planet:
	game.iplnet = q.planet
	if not q.planet.inhabited:
	    game.plnet = dropin('P')
	else:
	    game.plnet = dropin('@')
    # Check for condition
    newcnd()
    # Check for RNZ
    if game.irhere > 0 and game.klhere == 0:
	game.neutz = True
	if not damaged(DRADIO):
	    skip(1)
	    prout(_("LT. Uhura- \"Captain, an urgent message."))
	    prout(_("  I'll put it on audio.\"  CLICK"))
	    skip(1)
	    prout(_("INTRUDER! YOU HAVE VIOLATED THE ROMULAN NEUTRAL ZONE."))
	    prout(_("LEAVE AT ONCE, OR YOU WILL BE DESTROYED!"))
    # Put in THING if needed
    if thing == game.quadrant:
        enemy(type='?', loc=dropin(),
                  power=randreal(6000,6500.0)+250.0*game.skill)
        if not damaged(DSRSENS):
            skip(1)
            prout(_("Mr. Spock- \"Captain, this is most unusual."))
            prout(_("    Please examine your short-range scan.\""))
    # Decide if quadrant needs a Tholian; lighten up if skill is low 
    if game.options & OPTION_THOLIAN:
	if (game.skill < SKILL_GOOD and withprob(0.02)) or \
	    (game.skill == SKILL_GOOD and withprob(0.05)) or \
            (game.skill > SKILL_GOOD and withprob(0.08)):
            w = coord()
            while True:
		w.i = withprob(0.5) * (QUADSIZE-1)
		w.j = withprob(0.5) * (QUADSIZE-1)
                if game.quad[w.i][w.j] == '.':
                    break
            game.tholian = enemy(type='T', loc=w,
                                 power=randrange(100, 500) + 25.0*game.skill)
	    # Reserve unoccupied corners 
	    if game.quad[0][0]=='.':
		game.quad[0][0] = 'X'
	    if game.quad[0][QUADSIZE-1]=='.':
		game.quad[0][QUADSIZE-1] = 'X'
	    if game.quad[QUADSIZE-1][0]=='.':
		game.quad[QUADSIZE-1][0] = 'X'
	    if game.quad[QUADSIZE-1][QUADSIZE-1]=='.':
		game.quad[QUADSIZE-1][QUADSIZE-1] = 'X'
    game.enemies.sort(lambda x, y: cmp(x.kdist, y.kdist))
    # And finally the stars
    for i in range(q.stars):
	dropin('*')
    # Put in a few black holes
    for i in range(1, 3+1):
	if withprob(0.5): 
	    dropin(' ')
    # Take out X's in corners if Tholian present
    if game.tholian:
	if game.quad[0][0]=='X':
	    game.quad[0][0] = '.'
	if game.quad[0][QUADSIZE-1]=='X':
	    game.quad[0][QUADSIZE-1] = '.'
	if game.quad[QUADSIZE-1][0]=='X':
	    game.quad[QUADSIZE-1][0] = '.'
	if game.quad[QUADSIZE-1][QUADSIZE-1]=='X':
	    game.quad[QUADSIZE-1][QUADSIZE-1] = '.'

def setpassword():
    "Set the self-destruct password."
    if game.options & OPTION_PLAIN:
	while True:
	    scanner.chew()
	    proutn(_("Please type in a secret password- "))
	    scanner.next()
	    game.passwd = scanner.token
	    if game.passwd != None:
		break
    else:
        game.passwd = ""
        for i in range(8):
	    game.passwd += chr(ord('a')+randrange(26))

# Code from sst.c begins here

commands = {
    "SRSCAN":   	OPTION_TTY,
    "STATUS":   	OPTION_TTY,
    "REQUEST":  	OPTION_TTY,
    "LRSCAN":   	OPTION_TTY,
    "PHASERS":  	0,
    "TORPEDO":  	0,
    "PHOTONS":  	0,
    "MOVE":     	0,
    "SHIELDS":   	0,
    "DOCK":     	0,
    "DAMAGES":   	0,
    "CHART":    	0,
    "IMPULSE":  	0,
    "REST":     	0,
    "WARP":     	0,
    "SCORE":    	0,
    "SENSORS":  	OPTION_PLANETS,
    "ORBIT":		OPTION_PLANETS,
    "TRANSPORT":	OPTION_PLANETS,
    "MINE":		OPTION_PLANETS,
    "CRYSTALS":  	OPTION_PLANETS,
    "SHUTTLE":  	OPTION_PLANETS,
    "PLANETS":  	OPTION_PLANETS,
    "REPORT":   	0,
    "COMPUTER": 	0,
    "COMMANDS": 	0,
    "EMEXIT":		0,
    "PROBE":		OPTION_PROBE,
    "SAVE":		0,
    "FREEZE":		0,	# Synonym for SAVE
    "ABANDON":  	0,
    "DESTRUCT": 	0,
    "DEATHRAY": 	0,
    "DEBUG":    	0,
    "MAYDAY":		0,
    "SOS":		0,	# Synonym for MAYDAY
    "CALL":		0,	# Synonym for MAYDAY
    "QUIT":		0,
    "HELP":		0,
}

def listCommands():
    "Generate a list of legal commands."
    prout(_("LEGAL COMMANDS ARE:"))
    emitted = 0
    for key in commands:
	if not commands[key] or (commands[key] & game.options):
            proutn("%-12s " % key)
            emitted += 1
            if emitted % 5 == 4:
                skip(1)
    skip(1)

def helpme():
    "Browse on-line help."
    key = scanner.next()
    while True:
	if key == "IHEOL":
	    setwnd(prompt_window)
	    proutn(_("Help on what command? "))
	    key = scanner.next()
	setwnd(message_window)
	if key == "IHEOL":
	    return
        if scanner.token in commands or scanner.token == "ABBREV":
	    break
	skip(1)
	listCommands()
	key = "IHEOL"
	scanner.chew()
	skip(1)
    cmd = scanner.token.upper()
    try:
        fp = open(SSTDOC, "r")
    except IOError:
        try:
            fp = open(DOC_NAME, "r")
        except IOError:
            prout(_("Spock-  \"Captain, that information is missing from the"))
            proutn(_("   computer. You need to find "))
            proutn(DOC_NAME)
            prout(_(" and put it in the"))
            proutn(_("   current directory or to "))
            proutn(SSTDOC)
            prout(".\"")
            # This used to continue: "You need to find SST.DOC and put 
            # it in the current directory."
            return
    while True:
        linebuf = fp.readline()
	if linebuf == '':
	    prout(_("Spock- \"Captain, there is no information on that command.\""))
	    fp.close()
	    return
	if linebuf[0] == '%' and linebuf[1] == '%' and linebuf[2] == ' ':
            linebuf = linebuf[3:].strip()
            if cmd == linebuf:
		break
    skip(1)
    prout(_("Spock- \"Captain, I've found the following information:\""))
    skip(1)
    while linebuf in fp:
        if "******" in linebuf:
	    break
	proutn(linebuf)
    fp.close()

def makemoves():
    "Command-interpretation loop."
    clrscr()
    setwnd(message_window)
    while True: 	# command loop 
	drawmaps(1)
        while True:	# get a command 
	    hitme = False
	    game.optime = game.justin = False
	    scanner.chew()
	    setwnd(prompt_window)
	    clrscr()
	    proutn("COMMAND> ")
	    if scanner.next() == "IHEOL":
		if game.options & OPTION_CURSES:
		    makechart()
		continue
            elif scanner.token == "":
                continue
	    game.ididit = False
	    clrscr()
	    setwnd(message_window)
	    clrscr()
            candidates = filter(lambda x: x.startswith(scanner.token.upper()),
                                commands)
            if len(candidates) == 1:
                cmd = candidates[0]
                break
            elif candidates and not (game.options & OPTION_PLAIN):
                prout("Commands with prefix '%s': %s" % (scanner.token, " ".join(candidates)))
            else:
                listCommands()
                continue
	if cmd == "SRSCAN":		# srscan
	    srscan()
	elif cmd == "STATUS":		# status
	    status()
	elif cmd == "REQUEST":		# status request 
	    request()
	elif cmd == "LRSCAN":		# long range scan
	    lrscan(silent=False)
	elif cmd == "PHASERS":		# phasers
	    phasers()
	    if game.ididit:
		hitme = True
	elif cmd == "TORPEDO":		# photon torpedos
	    photon()
	    if game.ididit:
		hitme = True
	elif cmd == "MOVE":		# move under warp
	    warp(course=None, involuntary=False)
	elif cmd == "SHIELDS":		# shields
	    doshield(shraise=False)
	    if game.ididit:
		hitme = True
		game.shldchg = False
	elif cmd == "DOCK":		# dock at starbase
	    dock(True)
	    if game.ididit:
		attack(torps_ok=False)		
	elif cmd == "DAMAGES":		# damage reports
	    damagereport()
	elif cmd == "CHART":		# chart
	    makechart()
	elif cmd == "IMPULSE":		# impulse
	    impulse()
	elif cmd == "REST":		# rest
	    wait()
	    if game.ididit:
		hitme = True
	elif cmd == "WARP":		# warp
	    setwarp()
	elif cmd == "SCORE":		# score
	    score()
	elif cmd == "SENSORS":		# sensors
	    sensor()
	elif cmd == "ORBIT":		# orbit
	    orbit()
	    if game.ididit:
		hitme = True
	elif cmd == "TRANSPORT":		# transport "beam"
	    beam()
	elif cmd == "MINE":		# mine
	    mine()
	    if game.ididit:
		hitme = True
	elif cmd == "CRYSTALS":		# crystals
	    usecrystals()
	    if game.ididit:
		hitme = True
	elif cmd == "SHUTTLE":		# shuttle
	    shuttle()
	    if game.ididit:
		hitme = True
	elif cmd == "PLANETS":		# Planet list
	    survey()
	elif cmd == "REPORT":		# Game Report 
	    report()
	elif cmd == "COMPUTER":		# use COMPUTER!
	    eta()
	elif cmd == "COMMANDS":
	    listCommands()
	elif cmd == "EMEXIT":		# Emergency exit
	    clrscr()			# Hide screen
	    freeze(True)		# forced save
	    raise SysExit,1			# And quick exit
	elif cmd == "PROBE":
	    probe()			# Launch probe
	    if game.ididit:
		hitme = True
	elif cmd == "ABANDON":		# Abandon Ship
	    abandon()
	elif cmd == "DESTRUCT":		# Self Destruct
	    selfdestruct()
	elif cmd == "SAVE":		# Save Game
	    freeze(False)
	    clrscr()
	    if game.skill > SKILL_GOOD:
		prout(_("WARNING--Saved games produce no plaques!"))
	elif cmd == "DEATHRAY":		# Try a desparation measure
	    deathray()
	    if game.ididit:
		hitme = True
	elif cmd == "DEBUGCMD":		# What do we want for debug???
	    debugme()
	elif cmd == "MAYDAY":		# Call for help
	    mayday()
	    if game.ididit:
		hitme = True
	elif cmd == "QUIT":
	    game.alldone = True		# quit the game
	elif cmd == "HELP":
	    helpme()			# get help
	while True:
	    if game.alldone:
		break		# Game has ended
	    if game.optime != 0.0:
		events()
		if game.alldone:
		    break	# Events did us in
	    if game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova:
		atover(False)
		continue
	    if hitme and not game.justin:
		attack(torps_ok=True)
		if game.alldone:
		    break
		if game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova:
		    atover(False)
		    hitme = True
		    continue
	    break
	if game.alldone:
	    break
    if idebug:
	prout("=== Ending")

def cramen(type):
    "Emit the name of an enemy or feature." 
    if   type == 'R': s = _("Romulan")
    elif type == 'K': s = _("Klingon")
    elif type == 'C': s = _("Commander")
    elif type == 'S': s = _("Super-commander")
    elif type == '*': s = _("Star")
    elif type == 'P': s = _("Planet")
    elif type == 'B': s = _("Starbase")
    elif type == ' ': s = _("Black hole")
    elif type == 'T': s = _("Tholian")
    elif type == '#': s = _("Tholian web")
    elif type == '?': s = _("Stranger")
    elif type == '@': s = _("Inhabited World")
    else: s = "Unknown??"
    return s

def crmena(stars, enemy, loctype, w):
    "Emit the name of an enemy and his location."
    buf = ""
    if stars:
	buf += "***"
    buf += cramen(enemy) + _(" at ")
    if loctype == "quadrant":
	buf += _("Quadrant ")
    elif loctype == "sector":
	buf += _("Sector ")
    return buf + `w`

def crmshp():
    "Emit our ship name." 
    return{'E':_("Enterprise"),'F':_("Faerie Queene")}.get(game.ship,"Ship???")

def stars():
    "Emit a line of stars" 
    prouts("******************************************************")
    skip(1)

def expran(avrage):
    return -avrage*math.log(1e-7 + randreal())

def randplace(size):
    "Choose a random location."
    w = coord()
    w.i = randrange(size) 
    w.j = randrange(size)
    return w

class sstscanner:
    def __init__(self):
        self.type = None
        self.token = None
        self.real = 0.0
        self.inqueue = []
    def next(self):
        # Get a token from the user
        self.real = 0.0
        self.token = ''
        # Fill the token quue if nothing here
        while not self.inqueue:
            line = cgetline()
            if curwnd==prompt_window:
                clrscr()
                setwnd(message_window)
                clrscr()
            if line == '':
                return None
            if not line:
                continue
            else:
                self.inqueue = line.lstrip().split() + ["\n"]
        # From here on in it's all looking at the queue
        self.token = self.inqueue.pop(0)
        if self.token == "\n":
            self.type = "IHEOL"
            return "IHEOL"
        try:
            self.real = float(self.token)
            self.type = "IHREAL"
            return "IHREAL"
        except ValueError:
            pass
        # Treat as alpha
        self.token = self.token.lower()
        self.type = "IHALPHA"
        self.real = None
        return "IHALPHA"
    def append(self, tok):
        self.inqueue.append(tok)
    def push(self, tok):
        self.inqueue.insert(0, tok)
    def waiting(self):
        return self.inqueue
    def chew(self):
        # Demand input for next scan
        self.inqueue = []
        self.real = self.token = None
    def sees(self, s):
        # compares s to item and returns true if it matches to the length of s
        return s.startswith(self.token)
    def int(self):
        # Round token value to nearest integer
        return int(round(scanner.real))
    def getcoord(self):
        s = coord()
        scanner.next()
    	if scanner.type != "IHREAL":
	    huh()
	    return None
	s.i = scanner.int()-1
        scanner.next()
	if scanner.type != "IHREAL":
	    huh()
	    return None
	s.j = scanner.int()-1
        return s
    def __repr__(str):
        return "<sstcanner: token=%s, type=%s, queue=%s>" % (scanner.token, scanner.type, scanner.inqueue)

def ja():
    "Yes-or-no confirmation."
    scanner.chew()
    while True:
	scanner.next()
	if scanner.token == 'y':
	    return True
	if scanner.token == 'n':
	    return False
	scanner.chew()
	proutn(_("Please answer with \"y\" or \"n\": "))

def huh():
    "Complain about unparseable input."
    scanner.chew()
    skip(1)
    prout(_("Beg your pardon, Captain?"))

def debugme():
    "Access to the internals for debugging."
    proutn("Reset levels? ")
    if ja() == True:
	if game.energy < game.inenrg:
	    game.energy = game.inenrg
	game.shield = game.inshld
	game.torps = game.intorps
	game.lsupres = game.inlsr
    proutn("Reset damage? ")
    if ja() == True:
	for i in range(NDEVICES): 
	    if game.damage[i] > 0.0: 
		game.damage[i] = 0.0
    proutn("Toggle debug flag? ")
    if ja() == True:
	idebug = not idebug
	if idebug:
	    prout("Debug output ON")	    
	else:
	    prout("Debug output OFF")
    proutn("Cause selective damage? ")
    if ja() == True:
	for i in range(NDEVICES):
	    proutn("Kill %s?" % device[i])
	    scanner.chew()
	    key = scanner.next()
            if key == "IHALPHA" and scanner.sees("y"):
		game.damage[i] = 10.0
    proutn("Examine/change events? ")
    if ja() == True:
	ev = event()
	w = coord()
        legends = {
            FSNOVA:  "Supernova       ",
            FTBEAM:  "T Beam          ",
            FSNAP:   "Snapshot        ",
            FBATTAK: "Base Attack     ",
            FCDBAS:  "Base Destroy    ",
            FSCMOVE: "SC Move         ",
            FSCDBAS: "SC Base Destroy ",
            FDSPROB: "Probe Move      ",
            FDISTR:  "Distress Call   ",
            FENSLV:  "Enslavement     ",
            FREPRO:  "Klingon Build   ",
        }
	for i in range(1, NEVENTS):
            proutn(legends[i])
	    if is_scheduled(i):
		proutn("%.2f" % (scheduled(i)-game.state.date))
		if i == FENSLV or i == FREPRO:
		    ev = findevent(i)
		    proutn(" in %s" % ev.quadrant)
	    else:
		proutn("never")
	    proutn("? ")
	    scanner.chew()
	    key = scanner.next()
	    if key == 'n':
		unschedule(i)
		scanner.chew()
	    elif key == "IHREAL":
		ev = schedule(i, scanner.real)
		if i == FENSLV or i == FREPRO:
		    scanner.chew()
		    proutn("In quadrant- ")
		    key = scanner.next()
		    # "IHEOL" says to leave coordinates as they are 
		    if key != "IHEOL":
			if key != "IHREAL":
			    prout("Event %d canceled, no x coordinate." % (i))
			    unschedule(i)
			    continue
			w.i = int(round(scanner.real))
			key = scanner.next()
			if key != "IHREAL":
			    prout("Event %d canceled, no y coordinate." % (i))
			    unschedule(i)
			    continue
			w.j = int(round(scanner.real))
			ev.quadrant = w
	scanner.chew()
    proutn("Induce supernova here? ")
    if ja() == True:
	game.state.galaxy[game.quadrant.i][game.quadrant.j].supernova = True
	atover(True)

if __name__ == '__main__':
    import getopt, socket
    try:
        global line, thing, game, idebug
        game = None
        thing = coord()
        thing.angry = False
        game = gamestate()
        idebug = 0
        game.options = OPTION_ALL &~ (OPTION_IOMODES | OPTION_PLAIN | OPTION_ALMY)
        if os.getenv("TERM"):
            game.options |= OPTION_CURSES
        else:
            game.options |= OPTION_TTY
        seed = int(time.time())
        (options, arguments) = getopt.getopt(sys.argv[1:], "r:s:tx")
        for (switch, val) in options:
            if switch == '-r':
                try:
                    replayfp = open(val, "r")
                except IOError:
                    sys.stderr.write("sst: can't open replay file %s\n" % val)
                    raise SystemExit, 1
                try:
                    line = replayfp.readline().strip()
                    (leader, key, seed) = line.split()
                    seed = eval(seed)
                    sys.stderr.write("sst2k: seed set to %s\n" % seed)
                    line = replayfp.readline().strip()
                    arguments += line.split()[2:]
                except ValueError:
                    sys.stderr.write("sst: replay file %s is ill-formed\n"% val)
                    raise SystemExit(1)
                game.options |= OPTION_TTY
                game.options &=~ OPTION_CURSES
            elif switch == '-s':
                seed = int(val)
            elif switch == '-t':
                game.options |= OPTION_TTY
                game.options &=~ OPTION_CURSES
            elif switch == '-x':
                idebug = True
            else:
                sys.stderr.write("usage: sst [-t] [-x] [startcommand...].\n")
                raise SystemExit, 1
        # where to save the input in case of bugs
        if "TMPDIR" in os.environ:
            tmpdir = os.environ['TMPDIR']
        else:
            tmpdir = "/tmp"
        try:
            logfp = open(os.path.join(tmpdir, "sst-input.log"), "w")
        except IOError:
            sys.stderr.write("sst: warning, can't open logfile\n")
            sys.exit(1)
        if logfp:
            logfp.write("# seed %s\n" % seed)
            logfp.write("# options %s\n" % " ".join(arguments))
            logfp.write("# recorded by %s@%s on %s\n" % \
                    (getpass.getuser(),socket.gethostname(),time.ctime()))
        random.seed(seed)
        scanner = sstscanner()
        map(scanner.append, arguments)
        try:
            iostart()
            while True: # Play a game 
                setwnd(fullscreen_window)
                clrscr()
                prelim()
                setup()
                if game.alldone:
                    score()
                    game.alldone = False
                else:
                    makemoves()
                skip(1)
                stars()
                skip(1)
                if game.tourn and game.alldone:
                    proutn(_("Do you want your score recorded?"))
                    if ja() == True:
                        scanner.chew()
                        scanner.push("\n")
                        freeze(False)
                scanner.chew()
                proutn(_("Do you want to play again? "))
                if not ja():
                    break
            skip(1)
            prout(_("May the Great Bird of the Galaxy roost upon your home planet."))
        finally:
            ioend()
        raise SystemExit, 0
    except KeyboardInterrupt:
        if logfp:
            logfp.close()
        print ""
