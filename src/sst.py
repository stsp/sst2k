"""
sst.py =-- Super Star Trek in Python

"""
import os, sys, math, curses

SSTDOC = "/usr/share/doc/sst/sst.doc"

# Stub to be replaced
def _(str): return str

PHASEFAC	= 2.0
GALSIZE 	= 8
NINHAB  	= (GALSIZE * GALSIZE / 2)
MAXUNINHAB	= 10
PLNETMAX	= (NINHAB + MAXUNINHAB)
QUADSIZE	= 10
BASEMAX 	= (GALSIZE * GALSIZE / 12)
MAXKLGAME	= 127
MAXKLQUAD	= 9
FULLCREW	= 428	# BSD Trek was 387, that's wrong 
FOREVER 	= 1e30

# These functions hide the difference between 0-origin and 1-origin addressing.
def VALID_QUADRANT(x, y):	return ((x)>=1 and (x)<=GALSIZE and (y)>=1 and (y)<=GALSIZE)
def VALID_SECTOR(x, y):	return ((x)>=1 and (x)<=QUADSIZE and (y)>=1 and (y)<=QUADSIZE)

def square(i):		return ((i)*(i))
def distance(c1, c2):	return math.sqrt(square(c1.x - c2.x) + square(c1.y - c2.y))
def invalidate(w):	w.x = w.y = 0
def is_valid(w):	return (w.x != 0 and w.y != 0)

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

class planet:
    def __init(self):
        self.name = None	# string-valued if inhabited
        self.w = coord()	# quadrant located
        self.pclass = None	# could be ""M", "N", "O", or "destroyed"
        self.crystals = None	# could be "mined", "present", "absent"
        self.known = None	# could be "unknown", "known", "shuttle_down"

# How to represent features
IHR = 'R',
IHK = 'K',
IHC = 'C',
IHS = 'S',
IHSTAR = '*',
IHP = 'P',
IHW = '@',
IHB = 'B',
IHBLANK = ' ',
IHDOT = '.',
IHQUEST = '?',
IHE = 'E',
IHF = 'F',
IHT = 'T',
IHWEB = '#',
IHMATER0 = '-',
IHMATER1 = 'o',
IHMATER2 = '0'

NOPLANET = None
class quadrant:
    def __init(self):
        self.stars = None
        self.planet = None
	self.starbase = None
	self.klingons = None
	self.romulans = None
	self.supernova = None
	self.charted = None
        self.status = None	# Could be "secure", "distressed", "enslaved"

class page:
    def __init(self):
	self.stars = None
	self.starbase = None
	self.klingons = None

class snapshot:
    def __init(self):
        self.snap = False	# snapshot taken
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
        self.planets = []	# Planet information
        for i in range(PLNETMAX):
            self.planets.append(planet())
        self.date = None	# stardate
	self.remres = None	# remaining resources
	self.remtime = None	# remaining time
        self.baseq = [] 	# Base quadrant coordinates
        for i in range(BASEMAX+1):
            self.baseq.append(coord())
        self.kcmdr = [] 	# Commander quadrant coordinates
        for i in range(QUADSIZE+1):
            self.kcmdr.append(coord())
	self.kscmdr = coord()	# Supercommander quadrant coordinates
        self.galaxy = [] 	# The Galaxy (subscript 0 not used)
        for i in range(GALSIZE+1):
            self.chart.append([])
            for j in range(GALSIZE+1):
                self.galaxy[i].append(quadrant())
    	self.chart = [] 	# the starchart (subscript 0 not used)
        for i in range(GALSIZE+1):
            self.chart.append([])
            for j in range(GALSIZE+1):
                self.chart[i].append(page())

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
OPTION_THOLIAN	= 0x00000008	# Tholians and their webs 
OPTION_THINGY	= 0x00000010	# Space Thingy can shoot back 
OPTION_PROBE	= 0x00000020	# deep-space probes 
OPTION_SHOWME	= 0x00000040	# bracket Enterprise in chart 
OPTION_RAMMING	= 0x00000080	# enemies may ram Enterprise 
OPTION_MVBADDY	= 0x00000100	# more enemies can move 
OPTION_BLKHOLE	= 0x00000200	# black hole may timewarp you 
OPTION_BASE	= 0x00000400	# bases have good shields 
OPTION_WORLDS	= 0x00000800	# logic for inhabited worlds 
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

def damaged(dev):	return (game.damage[dev] != 0.0)

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

#
# abstract out the event handling -- underlying data structures will change
# when we implement stateful events
# 
def findevent(evtype):	return game.future[evtype]

class gamestate:
    def __init__(self):
        self.options = None	# Game options
        self.state = None	# A snapshot structure
        self.snapsht = None	# Last snapshot taken for time-travel purposes
        self.quad = [[IHDOT * (QUADSIZE+1)] * (QUADSIZE+1)]	# contents of our quadrant
        self.kpower = [[0 * (QUADSIZE+1)] * (QUADSIZE+1)]	# enemy energy levels
        self.kdist = [[0 * (QUADSIZE+1)] * (QUADSIZE+1)]	# enemy distances
        self.kavgd = [[0 * (QUADSIZE+1)] * (QUADSIZE+1)]	# average distances
        self.damage = [0] * NDEVICES	# damage encountered
        self.future = [0.0] * NEVENTS	# future events
        for i in range(NEVENTS):
            self.future.append(event())
        self.passwd  = None;		# Self Destruct password
        self.ks = [[None * (QUADSIZE+1)] * (QUADSIZE+1)]	# enemy sector locations
        self.quadrant = None	# where we are in the large
        self.sector = None	# where we are in the small
        self.tholian = None	# coordinates of Tholian
        self.base = None	# position of base in current quadrant
        self.battle = None	# base coordinates being attacked
        self.plnet = None	# location of planet in quadrant
        self.probec = None	# current probe quadrant
        self.gamewon = False	# Finished!
        self.ididit = False	# action taken -- allows enemy to attack
        self.alive = False	# we are alive (not killed)
        self.justin = False	# just entered quadrant
        self.shldup = False	# shields are up
        self.shldchg = False	# shield is changing (affects efficiency)
        self.comhere = False	# commander here
        self.ishere = False	# super-commander in quadrant
        self.iscate = False	# super commander is here
        self.ientesc = False	# attempted escape from supercommander
        self.ithere = False	# Tholian is here 
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
        self.iplnet = 0		# planet # in quadrant
        self.inplan = 0		# initial planets
        self.nenhere = 0	# number of enemies in quadrant
        self.irhere = 0		# Romulans in quadrant
        self.isatb = 0		# =1 if super commander is attacking base
        self.tourn = 0		# tournament number
        self.proben = 0		# number of moves for probe
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
        self.wfacsq = 0.0	# squared warp factor
        self.lsupres = 0.0	# life support reserves
        self.dist = 0.0		# movement distance
        self.direc = 0.0	# movement direction
        self.optime = 0.0	# time taken by current operation
        self.docfac = 0.0	# repair factor when docking (constant?)
        self.damfac = 0.0	# damage factor
        self.lastchart = 0.0	# time star chart was last updated
        self.cryprob = 0.0	# probability that crystal will work
        self.probex = 0.0	# location of probe
        self.probey = 0.0	#
        self.probeinx = 0.0	# probe x,y increment
        self.probeiny = 0.0	#
        self.height = 0.0	# height of orbit around planet

# From enumerated type 'feature'
IHR = 'R'
IHK = 'K'
IHC = 'C'
IHS = 'S'
IHSTAR = '*'
IHP = 'P'
IHW = '@'
IHB = 'B'
IHBLANK = ' '
IHDOT = '.'
IHQUEST = '?'
IHE = 'E'
IHF = 'F'
IHT = 'T'
IHWEB = '#'
IHMATER0 = '-'
IHMATER1 = 'o'
IHMATER2 = '0'


# From enumerated type 'FINTYPE'
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

# From enumerated type 'COLORS'
DEFAULT = 0
BLACK = 1
BLUE = 2
GREEN = 3
CYAN = 4
RED = 5
MAGENTA = 6
BROWN = 7
LIGHTGRAY = 8
DARKGRAY = 9
LIGHTBLUE = 10
LIGHTGREEN = 11
LIGHTCYAN = 12
LIGHTRED = 13
LIGHTMAGENTA = 14
YELLOW = 15
WHITE = 16

# Code from ai.c begins here

def tryexit(look, ienm, loccom, irun):
    # a bad guy attempts to bug out 
    iq = coord()
    iq.x = game.quadrant.x+(look.x+(QUADSIZE-1))/QUADSIZE - 1
    iq.y = game.quadrant.y+(look.y+(QUADSIZE-1))/QUADSIZE - 1
    if not VALID_QUADRANT(iq.x,iq.y) or \
	game.state.galaxy[iq.x][iq.y].supernova or \
	game.state.galaxy[iq.x][iq.y].klingons > MAXKLQUAD-1:
	return False; # no can do -- neg energy, supernovae, or >MAXKLQUAD-1 Klingons 
    if ienm == IHR:
	return False; # Romulans cannot escape! 
    if not irun:
	# avoid intruding on another commander's territory 
	if ienm == IHC:
	    for n in range(1, game.state.remcom+1):
		if same(game.state.kcmdr[n],iq):
		    return False
	    # refuse to leave if currently attacking starbase 
	    if same(game.battle, game.quadrant):
		return False
	# don't leave if over 1000 units of energy 
	if game.kpower[loccom] > 1000.0:
	    return False
    # print escape message and move out of quadrant.
    # we know this if either short or long range sensors are working
    if not damaged(DSRSENS) or not damaged(DLRSENS) or \
	game.condition == docked:
	crmena(True, ienm, "sector", game.ks[loccom])
	prout(_(" escapes to Quadrant %s (and regains strength).") % q)
    # handle local matters related to escape 
    game.quad[game.ks[loccom].x][game.ks[loccom].y] = IHDOT
    game.ks[loccom] = game.ks[game.nenhere]
    game.kavgd[loccom] = game.kavgd[game.nenhere]
    game.kpower[loccom] = game.kpower[game.nenhere]
    game.kdist[loccom] = game.kdist[game.nenhere]
    game.klhere -= 1
    game.nenhere -= 1
    if game.condition != docked:
	newcnd()
    # Handle global matters related to escape 
    game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons -= 1
    game.state.galaxy[iq.x][iq.y].klingons += 1
    if ienm==IHS:
	game.ishere = False
	game.iscate = False
	game.ientesc = False
	game.isatb = 0
	schedule(FSCMOVE, 0.2777)
	unschedule(FSCDBAS)
	game.state.kscmdr=iq
    else:
	for n in range(1, game.state.remcom+1):
	    if same(game.state.kcmdr[n], game.quadrant):
		game.state.kcmdr[n]=iq
		break
	game.comhere = False
    return True; # success 

#
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
# 

def movebaddy(com, loccom, ienm):
    # tactical movement for the bad guys 
    next = coord(); look = coord()
    irun = False
    # This should probably be just game.comhere + game.ishere 
    if game.skill >= SKILL_EXPERT:
	nbaddys = ((game.comhere*2 + game.ishere*2+game.klhere*1.23+game.irhere*1.5)/2.0)
    else:
	nbaddys = game.comhere + game.ishere

    dist1 = game.kdist[loccom]
    mdist = int(dist1 + 0.5); # Nearest integer distance 

    # If SC, check with spy to see if should hi-tail it 
    if ienm==IHS and \
	(game.kpower[loccom] <= 500.0 or (game.condition=="docked" and not damaged(DPHOTON))):
	irun = True
	motion = -QUADSIZE
    else:
	# decide whether to advance, retreat, or hold position 
	forces = game.kpower[loccom]+100.0*game.nenhere+400*(nbaddys-1)
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
	    motion = ((forces+200.0*Rand())/150.0) - 5.0
	else:
            if forces > 1000.0: # Very strong -- move in for kill 
		motion = (1.0-square(Rand()))*dist1 + 1.0
	    if game.condition=="docked" and (game.options & OPTION_BASE): # protected by base -- back off ! 
		motion -= game.skill*(2.0-square(Rand()))
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
    if motion < 0:
        msteps = -motion
    else:
        msteps = motion
    if motion > 0 and nsteps > mdist:
	nsteps = mdist; # don't overshoot 
    if nsteps > QUADSIZE:
	nsteps = QUADSIZE; # This shouldn't be necessary 
    if nsteps < 1:
	nsteps = 1; # This shouldn't be necessary 
    if idebug:
	proutn("NSTEPS = %d:" % nsteps)
    # Compute preferred values of delta X and Y 
    mx = game.sector.x - com.x
    my = game.sector.y - com.y
    if 2.0 * abs(mx) < abs(my):
	mx = 0
    if 2.0 * abs(my) < abs(game.sector.x-com.x):
	my = 0
    if mx != 0:
        if mx*motion < 0:
            mx = -1
        else:
            mx = 1
    if my != 0:
        if my*motion < 0:
            my = -1
        else:
            my = 1
    next = com
    # main move loop 
    for ll in range(nsteps):
	if idebug:
	    proutn(" %d" % (ll+1))
	# Check if preferred position available 
	look.x = next.x + mx
	look.y = next.y + my
        if mx < 0:
            krawlx = 1
        else:
            krawlx = -1
        if my < 0:
            krawly = 1
        else:
            krawly = -1
	success = False
	attempts = 0; # Settle mysterious hang problem 
	while attempts < 20 and not success:
            attempts += 1
	    if look.x < 1 or look.x > QUADSIZE:
		if motion < 0 and tryexit(look, ienm, loccom, irun):
		    return
		if krawlx == mx or my == 0:
		    break
		look.x = next.x + krawlx
		krawlx = -krawlx
	    elif look.y < 1 or look.y > QUADSIZE:
		if motion < 0 and tryexit(look, ienm, loccom, irun):
		    return
		if krawly == my or mx == 0:
		    break
		look.y = next.y + krawly
		krawly = -krawly
	    elif (game.options & OPTION_RAMMING) and game.quad[look.x][look.y] != IHDOT:
		# See if we should ram ship 
		if game.quad[look.x][look.y] == game.ship and \
		    (ienm == IHC or ienm == IHS):
		    ram(True, ienm, com)
		    return
		if krawlx != mx and my != 0:
		    look.x = next.x + krawlx
		    krawlx = -krawlx
		elif krawly != my and mx != 0:
		    look.y = next.y + krawly
		    krawly = -krawly
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
    # Put commander in place within same quadrant 
    game.quad[com.x][com.y] = IHDOT
    game.quad[next.x][next.y] = ienm
    if not same(next, com):
	# it moved 
	game.ks[loccom] = next
	game.kdist[loccom] = game.kavgd[loccom] = distance(game.sector, next)
	if not damaged(DSRSENS) or game.condition == docked:
	    proutn("***")
	    cramen(ienm)
	    proutn(_(" from Sector %s") % com)
	    if game.kdist[loccom] < dist1:
		proutn(_(" advances to "))
	    else:
		proutn(_(" retreats to "))
	    prout("Sector %s." % next)

def moveklings():
    # Klingon tactical movement 
    if idebug:
	prout("== MOVCOM")
    # Figure out which Klingon is the commander (or Supercommander)
    # and do move
    if game.comhere:
	for i in range(1, game.nenhere+1):
	    w = game.ks[i]
	    if game.quad[w.x][w.y] == IHC:
		movebaddy(w, i, IHC)
		break
    if game.ishere:
	for i in range(1, game.nenhere+1):
	    w = game.ks[i]
	    if game.quad[w.x][w.y] == IHS:
		movebaddy(w, i, IHS)
		break
    # If skill level is high, move other Klingons and Romulans too!
    # Move these last so they can base their actions on what the
    # commander(s) do.
    if game.skill >= SKILL_EXPERT and (game.options & OPTION_MVBADDY):
	for i in range(1, game.nenhere+1):
	    w = game.ks[i]
	    if game.quad[w.x][w.y] == IHK or game.quad[w.x][w.y] == IHR:
		movebaddy(w, i, game.quad[w.x][w.y])
    sortklings();

def movescom(iq, avoid):
    # commander movement helper 
    if same(iq, game.quadrant) or not VALID_QUADRANT(iq.x, iq.y) or \
	game.state.galaxy[iq.x][iq.y].supernova or \
	game.state.galaxy[iq.x][iq.y].klingons > MAXKLQUAD-1:
	return 1
    if avoid:
	# Avoid quadrants with bases if we want to avoid Enterprise 
	for i in range(1, game.state.rembase+1):
	    if same(game.state.baseq[i], iq):
		return True
    if game.justin and not game.iscate:
	return True
    # do the move 
    game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].klingons -= 1
    game.state.kscmdr = iq
    game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].klingons += 1
    if game.ishere:
	# SC has scooted, Remove him from current quadrant 
	game.iscate=False
	game.isatb=0
	game.ishere = False
	game.ientesc = False
	unschedule(FSCDBAS)
	for i in range(1, game.nenhere+1):
	    if game.quad[game.ks[i].x][game.ks[i].y] == IHS:
		break
	game.quad[game.ks[i].x][game.ks[i].y] = IHDOT
	game.ks[i] = game.ks[game.nenhere]
	game.kdist[i] = game.kdist[game.nenhere]
	game.kavgd[i] = game.kavgd[game.nenhere]
	game.kpower[i] = game.kpower[game.nenhere]
	game.klhere -= 1
	game.nenhere -= 1
	if game.condition!=docked:
	    newcnd()
	sortklings()
    # check for a helpful planet 
    for i in range(game.inplan):
	if same(game.state.planets[i].w, game.state.kscmdr) and \
	    game.state.planets[i].crystals == present:
	    # destroy the planet 
	    game.state.planets[i].pclass = destroyed
	    game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].planet = NOPLANET
	    if not damaged(DRADIO) or game.condition == docked:
		announce()
		prout(_("Lt. Uhura-  \"Captain, Starfleet Intelligence reports"))
		proutn(_("   a planet in Quadrant %s has been destroyed") % game.state.kscmdr)
		prout(_("   by the Super-commander.\""))
	    break
    return False; # looks good! 
			
def supercommander():
    # move the Super Commander 
    iq = coord(); sc = coord(); ibq = coord(); idelta = coord()
    basetbl = []
    if idebug:
	prout("== SUPERCOMMANDER")
    # Decide on being active or passive 
    avoid = ((game.incom - game.state.remcom + game.inkling - game.state.remkl)/(game.state.date+0.01-game.indate) < 0.1*game.skill*(game.skill+1.0) or \
	    (game.state.date-game.indate) < 3.0)
    if not game.iscate and avoid:
	# compute move away from Enterprise 
	idelta = game.state.kscmdr-game.quadrant
	if math.sqrt(idelta.x*idelta.x+idelta.y*idelta.y) > 2.0:
	    # circulate in space 
	    idelta.x = game.state.kscmdr.y-game.quadrant.y
	    idelta.y = game.quadrant.x-game.state.kscmdr.x
    else:
	# compute distances to starbases 
	if game.state.rembase <= 0:
	    # nothing left to do 
	    unschedule(FSCMOVE)
	    return
	sc = game.state.kscmdr
	for i in range(1, game.state.rembase+1):
	    basetbl.append((i, distance(game.state.baseq[i], sc)))
	if game.state.rembase > 1:
            basetbl.sort(lambda x, y: cmp(x[1]. y[1]))
	# look for nearest base without a commander, no Enterprise, and
        # without too many Klingons, and not already under attack. 
	ifindit = iwhichb = 0
	for i2 in range(1, game.state.rembase+1):
	    i = basetbl[i2][0];	# bug in original had it not finding nearest
	    ibq = game.state.baseq[i]
	    if same(ibq, game.quadrant) or same(ibq, game.battle) or \
		game.state.galaxy[ibq.x][ibq.y].supernova or \
		game.state.galaxy[ibq.x][ibq.y].klingons > MAXKLQUAD-1:
		continue
	    # if there is a commander, and no other base is appropriate,
	    #   we will take the one with the commander
	    for j in range(1, game.state.remcom+1):
		if same(ibq, game.state.kcmdr[j]) and ifindit!= 2:
		    ifindit = 2
		    iwhichb = i
		    break
	    if j > game.state.remcom: # no commander -- use this one 
		ifindit = 1
		iwhichb = i
		break
	if ifindit==0:
	    return; # Nothing suitable -- wait until next time
	ibq = game.state.baseq[iwhichb]
	# decide how to move toward base 
	idelta = ibq - game.state.kscmdr
    # Maximum movement is 1 quadrant in either or both axes 
    idelta = idelta.sgn()
    # try moving in both x and y directions
    # there was what looked like a bug in the Almy C code here,
    # but it might be this translation is just wrong.
    iq = game.state.kscmdr + idelta
    if movescom(iq, avoid):
	# failed -- try some other maneuvers 
	if idelta.x==0 or idelta.y==0:
	    # attempt angle move 
	    if idelta.x != 0:
		iq.y = game.state.kscmdr.y + 1
		if movescom(iq, avoid):
		    iq.y = game.state.kscmdr.y - 1
		    movescom(iq, avoid)
	    else:
		iq.x = game.state.kscmdr.x + 1
		if movescom(iq, avoid):
		    iq.x = game.state.kscmdr.x - 1
		    movescom(iq, avoid)
	else:
	    # try moving just in x or y 
	    iq.y = game.state.kscmdr.y
	    if movescom(iq, avoid):
		iq.y = game.state.kscmdr.y + idelta.y
		iq.x = game.state.kscmdr.x
		movescom(iq, avoid)
    # check for a base 
    if game.state.rembase == 0:
	unschedule(FSCMOVE)
    else:
	for i in range(1, game.state.rembase+1):
	    ibq = game.state.baseq[i]
	    if same(ibq, game.state.kscmdr) and same(game.state.kscmdr, game.battle):
		# attack the base 
		if avoid:
		    return; # no, don't attack base! 
		game.iseenit = False
		game.isatb = 1
		schedule(FSCDBAS, 1.0 +2.0*Rand())
		if is_scheduled(FCDBAS):
		    postpone(FSCDBAS, scheduled(FCDBAS)-game.state.date)
		if damaged(DRADIO) and game.condition != docked:
		    return; # no warning 
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
	(Rand() > 0.2 or \
	 (damaged(DRADIO) and game.condition != docked) or \
	 not game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].charted):
	return
    announce()
    prout(_("Lt. Uhura-  \"Captain, Starfleet Intelligence reports"))
    proutn(_("   the Super-commander is in Quadrant %s,") % game.state.kscmdr)
    return;

def movetholian():
    # move the Tholian 
    if not game.ithere or game.justin:
	return

    if game.tholian.x == 1 and game.tholian.y == 1:
	idx = 1; idy = QUADSIZE
    elif game.tholian.x == 1 and game.tholian.y == QUADSIZE:
	idx = QUADSIZE; idy = QUADSIZE
    elif game.tholian.x == QUADSIZE and game.tholian.y == QUADSIZE:
	idx = QUADSIZE; idy = 1
    elif game.tholian.x == QUADSIZE and game.tholian.y == 1:
	idx = 1; idy = 1
    else:
	# something is wrong! 
	game.ithere = False
	return

    # do nothing if we are blocked 
    if game.quad[idx][idy]!= IHDOT and game.quad[idx][idy]!= IHWEB:
	return
    game.quad[game.tholian.x][game.tholian.y] = IHWEB

    if game.tholian.x != idx:
	# move in x axis 
	im = math.fabs(idx - game.tholian.x)*1.0/(idx - game.tholian.x)
	while game.tholian.x != idx:
	    game.tholian.x += im
	    if game.quad[game.tholian.x][game.tholian.y]==IHDOT:
		game.quad[game.tholian.x][game.tholian.y] = IHWEB
    elif game.tholian.y != idy:
	# move in y axis 
	im = math.fabs(idy - game.tholian.y)*1.0/(idy - game.tholian.y)
	while game.tholian.y != idy:
	    game.tholian.y += im
	    if game.quad[game.tholian.x][game.tholian.y]==IHDOT:
		game.quad[game.tholian.x][game.tholian.y] = IHWEB
    game.quad[game.tholian.x][game.tholian.y] = IHT
    game.ks[game.nenhere] = game.tholian

    # check to see if all holes plugged 
    for i in range(1, QUADSIZE+1):
	if game.quad[1][i]!=IHWEB and game.quad[1][i]!=IHT:
	    return
	if game.quad[QUADSIZE][i]!=IHWEB and game.quad[QUADSIZE][i]!=IHT:
	    return
	if game.quad[i][1]!=IHWEB and game.quad[i][1]!=IHT:
	    return
	if game.quad[i][QUADSIZE]!=IHWEB and game.quad[i][QUADSIZE]!=IHT:
	    return
    # All plugged up -- Tholian splits 
    game.quad[game.tholian.x][game.tholian.y]=IHWEB
    dropin(IHBLANK)
    crmena(True, IHT, "sector", game.tholian)
    prout(_(" completes web."))
    game.ithere = False
    game.nenhere -= 1
    return

# Code from battle.c begins here

def doshield(shraise):
    # change shield status 
    action = "NONE"
    game.ididit = False
    if shraise:
	action = "SHUP"
    else:
	key = scan()
	if key == IHALPHA:
	    if isit("transfer"):
		action = "NRG"
	    else:
		chew()
		if damaged(DSHIELD):
		    prout(_("Shields damaged and down."))
		    return
		if isit("up"):
		    action = "SHUP"
		elif isit("down"):
		    action = "SHDN"
	if action=="NONE":
	    proutn(_("Do you wish to change shield energy? "))
	    if ja() == True:
		proutn(_("Energy to transfer to shields- "))
		action = "NRG"
	    elif damaged(DSHIELD):
		prout(_("Shields damaged and down."))
		return
	    elif game.shldup:
		proutn(_("Shields are up. Do you want them down? "))
		if ja() == True:
		    action = "SHDN"
		else:
		    chew()
		    return
	    else:
		proutn(_("Shields are down. Do you want them up? "))
		if ja() == True:
		    action = "SHUP"
		else:
		    chew()
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
	while scan() != IHREAL:
	    chew()
	    proutn(_("Energy to transfer to shields- "))
	chew()
	if aaitem==0:
	    return
	if aaitem > game.energy:
	    prout(_("Insufficient ship energy."))
	    return
	game.ididit = True
	if game.shield+aaitem >= game.inshld:
	    prout(_("Shield energy maximized."))
	    if game.shield+aaitem > game.inshld:
		prout(_("Excess energy requested returned to ship energy"))
	    game.energy -= game.inshld-game.shield
	    game.shield = game.inshld
	    return
	if aaitem < 0.0 and game.energy-aaitem > game.inenrg:
	    # Prevent shield drain loophole 
	    skip(1)
	    prout(_("Engineering to bridge--"))
	    prout(_("  Scott here. Power circuit problem, Captain."))
	    prout(_("  I can't drain the shields."))
	    game.ididit = False
	    return
	if game.shield+aaitem < 0:
	    prout(_("All shield energy transferred to ship."))
	    game.energy += game.shield
	    game.shield = 0.0
	    return
	proutn(_("Scotty- \""))
	if aaitem > 0:
	    prout(_("Transferring energy to shields.\""))
	else:
	    prout(_("Draining energy from shields.\""))
	game.shield += aaitem
	game.energy -= aaitem
	return

def randdevice():
    # choose a device to damage, at random. 
    #
    # Quoth Eric Allman in the code of BSD-Trek:
    # "Under certain conditions you can get a critical hit.  This
    # sort of hit damages devices.  The probability that a given
    # device is damaged depends on the device.  Well protected
    # devices (such as the computer, which is in the core of the
    # ship and has considerable redundancy) almost never get
    # damaged, whereas devices which are exposed (such as the
    # warp engines) or which are particularly delicate (such as
    # the transporter) have a much higher probability of being
    # damaged."
    # 
    # This is one place where OPTION_PLAIN does not restore the
    # original behavior, which was equiprobable damage across
    # all devices.  If we wanted that, we'd return NDEVICES*Rand()
    # and have done with it.  Also, in the original game, DNAVYS
    # and DCOMPTR were the same device. 
    # 
    # Instead, we use a table of weights similar to the one from BSD Trek.
    # BSD doesn't have the shuttle, shield controller, death ray, or probes. 
    # We don't have a cloaking device.  The shuttle got the allocation
    # for the cloaking device, then we shaved a half-percent off
    # everything to have some weight to give DSHCTRL/DDRAY/DDSP.
    # 
    weights = (
	105,	# DSRSENS: short range scanners	10.5% 
	105,	# DLRSENS: long range scanners		10.5% 
	120,	# DPHASER: phasers			12.0% 
	120,	# DPHOTON: photon torpedoes		12.0% 
	25,	# DLIFSUP: life support		 2.5% 
	65,	# DWARPEN: warp drive			 6.5% 
	70,	# DIMPULS: impulse engines		 6.5% 
	145,	# DSHIELD: deflector shields		14.5% 
	30,	# DRADIO:  subspace radio		 3.0% 
	45,	# DSHUTTL: shuttle			 4.5% 
	15,	# DCOMPTR: computer			 1.5% 
	20,	# NAVCOMP: navigation system		 2.0% 
	75,	# DTRANSP: transporter			 7.5% 
	20,	# DSHCTRL: high-speed shield controller 2.0% 
	10,	# DDRAY: death ray			 1.0% 
	30,	# DDSP: deep-space probes		 3.0% 
    )
    idx = Rand() * 1000.0	# weights must sum to 1000 
    sum = 0
    for (i, w) in enumerate(weights):
	sum += w
	if idx < sum:
	    return i
    return None;	# we should never get here

def ram(ibumpd, ienm, w):
    # make our ship ram something 
    prouts(_("***RED ALERT!  RED ALERT!"))
    skip(1)
    prout(_("***COLLISION IMMINENT."))
    skip(2)
    proutn("***")
    crmshp()
    hardness = {IHR:1.5, IHC:2.0, IHS:2.5, IHT:0.5, IHQUEST:4.0}.get(ienm, 1.0)
    if ibumpd:
        proutn(_(" rammed by "))
    else:
        proutn(_(" rams "))
    crmena(False, ienm, sector, w)
    if ibumpd:
	proutn(_(" (original position)"))
    skip(1)
    deadkl(w, ienm, game.sector)
    proutn("***")
    crmshp()
    prout(_(" heavily damaged."))
    icas = 10.0+20.0*Rand()
    prout(_("***Sickbay reports %d casualties"), icas)
    game.casual += icas
    game.state.crew -= icas
    #
    # In the pre-SST2K version, all devices got equiprobably damaged,
    # which was silly.  Instead, pick up to half the devices at
    # random according to our weighting table,
    # 
    ncrits = Rand() * (NDEVICES/2)
    for m in range(ncrits):
	dev = randdevice()
	if game.damage[dev] < 0:
	    continue
	extradm = (10.0*hardness*Rand()+1.0)*game.damfac
	# Damage for at least time of travel! 
	game.damage[dev] += game.optime + extradm
    game.shldup = False
    prout(_("***Shields are down."))
    if game.state.remkl + game.state.remcom + game.state.nscrem:
	announce()
	damagereport()
    else:
	finish(FWON)
    return;

def torpedo(course, r, incoming, i, n):
    # let a photon torpedo fly 
    iquad = 0
    shoved = False
    ac = course + 0.25*r
    angle = (15.0-ac)*0.5235988
    bullseye = (15.0 - course)*0.5235988
    deltax = -math.sin(angle);
    deltay = math.cos(angle);
    x = incoming.x; y = incoming.y
    w = coord(); jw = coord()
    w.x = w.y = jw.x = jw.y = 0
    bigger = max(math.fabs(deltax), math.fabs(deltay))
    deltax /= bigger
    deltay /= bigger
    if not damaged(DSRSENS) or game.condition=="docked":
	setwnd(srscan_window)
    else: 
	setwnd(message_window)
    # Loop to move a single torpedo 
    for l in range(1, 15+1):
	x += deltax
	w.x = x + 0.5
	y += deltay
	w.y = y + 0.5
	if not VALID_SECTOR(w.x, w.y):
	    break
	iquad=game.quad[w.x][w.y]
	tracktorpedo(w, l, i, n, iquad)
	if iquad==IHDOT:
	    continue
	# hit something 
	setwnd(message_window)
	if damaged(DSRSENS) and not game.condition=="docked":
	    skip(1);	# start new line after text track 
	if iquad in (IHE, IHF): # Hit our ship 
	    skip(1)
	    proutn(_("Torpedo hits "))
	    crmshp()
	    prout(".")
	    hit = 700.0 + 100.0*Rand() - \
		1000.0 * distance(w, incoming) * math.fabs(math.sin(bullseye-angle))
	    newcnd(); # we're blown out of dock 
	    # We may be displaced. 
	    if game.landed or game.condition=="docked":
		return hit # Cheat if on a planet 
	    ang = angle + 2.5*(Rand()-0.5)
	    temp = math.fabs(math.sin(ang))
	    if math.fabs(math.cos(ang)) > temp:
		temp = math.fabs(math.cos(ang))
	    xx = -math.sin(ang)/temp
	    yy = math.cos(ang)/temp
	    jw.x=w.x+xx+0.5
	    jw.y=w.y+yy+0.5
	    if not VALID_SECTOR(jw.x, jw.y):
		return hit
	    if game.quad[jw.x][jw.y]==IHBLANK:
		finish(FHOLE)
		return hit
	    if game.quad[jw.x][jw.y]!=IHDOT:
		# can't move into object 
		return hit
	    game.sector = jw
	    crmshp()
	    shoved = True
	elif iquad in (IHC, IHS): # Hit a commander 
	    if Rand() <= 0.05:
		crmena(True, iquad, sector, w)
		prout(_(" uses anti-photon device;"))
		prout(_("   torpedo neutralized."))
		return None
	elif iquad in (IHR, IHK): # Hit a regular enemy 
	    # find the enemy 
	    for ll in range(1, game.nenhere+1):
		if same(w, game.ks[ll]):
		    break
	    kp = math.fabs(game.kpower[ll])
	    h1 = 700.0 + 100.0*Rand() - \
		1000.0 * distance(w, incoming) * math.fabs(math.sin(bullseye-angle))
	    h1 = math.fabs(h1)
	    if kp < h1:
		h1 = kp
            if game.kpower[ll] < 0:
                game.kpower[ll] -= -h1
            else:
                game.kpower[ll] -= h1
	    if game.kpower[ll] == 0:
		deadkl(w, iquad, w)
		return None
	    crmena(True, iquad, "sector", w)
	    # If enemy damaged but not destroyed, try to displace 
	    ang = angle + 2.5*(Rand()-0.5)
	    temp = math.fabs(math.sin(ang))
	    if math.fabs(math.cos(ang)) > temp:
		temp = math.fabs(math.cos(ang))
	    xx = -math.sin(ang)/temp
	    yy = math.cos(ang)/temp
	    jw.x=w.x+xx+0.5
	    jw.y=w.y+yy+0.5
	    if not VALID_SECTOR(jw.x, jw.y):
		prout(_(" damaged but not destroyed."))
		return
	    if game.quad[jw.x][jw.y]==IHBLANK:
		prout(_(" buffeted into black hole."))
		deadkl(w, iquad, jw)
		return None
	    if game.quad[jw.x][jw.y]!=IHDOT:
		# can't move into object 
		prout(_(" damaged but not destroyed."))
		return None
	    proutn(_(" damaged--"))
	    game.ks[ll] = jw
	    shoved = True
	    break
	elif iquad == IHB: # Hit a base 
	    skip(1)
	    prout(_("***STARBASE DESTROYED.."))
	    for ll in range(1, game.state.rembase+1):
		if same(game.state.baseq[ll], game.quadrant):
		    game.state.baseq[ll]=game.state.baseq[game.state.rembase]
		    break
	    game.quad[w.x][w.y]=IHDOT
	    game.state.rembase -= 1
	    game.base.x=game.base.y=0
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].starbase -= 1
	    game.state.chart[game.quadrant.x][game.quadrant.y].starbase -= 1
	    game.state.basekl += 1
	    newcnd()
	    return None
	elif iquad == IHP: # Hit a planet 
	    crmena(True, iquad, sector, w)
	    prout(_(" destroyed."))
	    game.state.nplankl += 1
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].planet = NOPLANET
	    game.state.planets[game.iplnet].pclass = destroyed
	    game.iplnet = 0
	    invalidate(game.plnet)
	    game.quad[w.x][w.y] = IHDOT
	    if game.landed:
		# captain perishes on planet 
		finish(FDPLANET)
	    return None
	elif iquad == IHW: # Hit an inhabited world -- very bad! 
	    crmena(True, iquad, sector, w)
	    prout(_(" destroyed."))
	    game.state.nworldkl += 1
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].planet = NOPLANET
	    game.state.planets[game.iplnet].pclass = destroyed
	    game.iplnet = 0
	    invalidate(game.plnet)
	    game.quad[w.x][w.y] = IHDOT
	    if game.landed:
		# captain perishes on planet 
		finish(FDPLANET)
	    prout(_("You have just destroyed an inhabited planet."))
	    prout(_("Celebratory rallies are being held on the Klingon homeworld."))
	    return None
	elif iquad == IHSTAR: # Hit a star 
	    if Rand() > 0.10:
		nova(w)
		return None
	    crmena(True, IHSTAR, sector, w)
	    prout(_(" unaffected by photon blast."))
	    return None
	elif iquad == IHQUEST: # Hit a thingy 
	    if not (game.options & OPTION_THINGY) or Rand()>0.7:
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
		#
		# Stas Sergeev added the possibility that
		# you can shove the Thingy and piss it off.
		# It then becomes an enemy and may fire at you.
		# 
		iqengry = True
		shoved = True
	    return None
	elif iquad == IHBLANK: # Black hole 
	    skip(1)
	    crmena(True, IHBLANK, sector, w)
	    prout(_(" swallows torpedo."))
	    return None
	elif iquad == IHWEB: # hit the web 
	    skip(1)
	    prout(_("***Torpedo absorbed by Tholian web."))
	    return None
	elif iquad == IHT:  # Hit a Tholian 
	    h1 = 700.0 + 100.0*Rand() - \
		1000.0 * distance(w, incoming) * math.fabs(math.sin(bullseye-angle))
	    h1 = math.fabs(h1)
	    if h1 >= 600:
		game.quad[w.x][w.y] = IHDOT
		game.ithere = False
		deadkl(w, iquad, w)
		return None
	    skip(1)
	    crmena(True, IHT, sector, w)
	    if Rand() > 0.05:
		prout(_(" survives photon blast."))
		return None
	    prout(_(" disappears."))
	    game.quad[w.x][w.y] = IHWEB
	    game.ithere = False
	    game.nenhere -= 1
	    dropin(IHBLANK)
	    return None
        else: # Problem!
	    skip(1)
	    proutn("Don't know how to handle torpedo collision with ")
	    crmena(True, iquad, sector, w)
	    skip(1)
	    return None
	break
    if curwnd!=message_window:
	setwnd(message_window)
    if shoved:
	game.quad[w.x][w.y]=IHDOT
	game.quad[jw.x][jw.y]=iquad
	prout(_(" displaced by blast to %s "), cramlc(sector, jw))
	for ll in range(1, game.nenhere+1):
	    game.kdist[ll] = game.kavgd[ll] = distance(game.sector,game.ks[ll])
	sortklings()
	return None
    skip(1)
    prout(_("Torpedo missed."))
    return None;

def fry(hit):
    # critical-hit resolution 
    ktr=1
    # a critical hit occured 
    if hit < (275.0-25.0*game.skill)*(1.0+0.5*Rand()):
	return

    ncrit = 1.0 + hit/(500.0+100.0*Rand())
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
	extradm = (hit*game.damfac)/(ncrit*(75.0+25.0*Rand()))
	game.damage[j] += extradm
	if loop1 > 0:
            for loop2 in range(loop1):
                if j == cdam[loop2]:
                    break
	    if loop2 < loop1:
		continue
	    ktr += 1
	    if ktr==3:
		skip(1)
	    proutn(_(" and "))
	proutn(device[j])
    prout(_(" damaged."))
    if damaged(DSHIELD) and game.shldup:
	prout(_("***Shields knocked down."))
	game.shldup=False

def attack(torps_ok):
    # bad guy attacks us 
    # torps_ok == false forces use of phasers in an attack 
    atackd = False; attempt = False; ihurt = False;
    hitmax=0.0; hittot=0.0; chgfac=1.0
    jay = coord()
    where = "neither"

    # game could be over at this point, check 
    if game.alldone:
	return

    if idebug:
	prout("=== ATTACK!")

    # Tholian gewts to move before attacking 
    if game.ithere:
	movetholian()

    # if you have just entered the RNZ, you'll get a warning 
    if game.neutz: # The one chance not to be attacked 
	game.neutz = False
	return

    # commanders get a chance to tac-move towards you 
    if (((game.comhere or game.ishere) and not game.justin) or game.skill == SKILL_EMERITUS) and torps_ok:
	moveklings()

    # if no enemies remain after movement, we're done 
    if game.nenhere==0 or (game.nenhere==1 and iqhere and not iqengry):
	return

    # set up partial hits if attack happens during shield status change 
    pfac = 1.0/game.inshld
    if game.shldchg:
	chgfac = 0.25+0.5*Rand()

    skip(1)

    # message verbosity control 
    if game.skill <= SKILL_FAIR:
	where = "sector"

    for loop in range(1, game.nenhere+1):
	if game.kpower[loop] < 0:
	    continue;	# too weak to attack 
	# compute hit strength and diminish shield power 
	r = Rand()
	# Increase chance of photon torpedos if docked or enemy energy low 
	if game.condition == "docked":
	    r *= 0.25
	if game.kpower[loop] < 500:
	    r *= 0.25; 
	jay = game.ks[loop]
	iquad = game.quad[jay.x][jay.y]
	if iquad==IHT or (iquad==IHQUEST and not iqengry):
	    continue
	# different enemies have different probabilities of throwing a torp 
	usephasers = not torps_ok or \
	    (iquad == IHK and r > 0.0005) or \
	    (iquad==IHC and r > 0.015) or \
	    (iquad==IHR and r > 0.3) or \
	    (iquad==IHS and r > 0.07) or \
	    (iquad==IHQUEST and r > 0.05)
	if usephasers:	    # Enemy uses phasers 
	    if game.condition == "docked":
		continue; # Don't waste the effort! 
	    attempt = True; # Attempt to attack 
	    dustfac = 0.8+0.05*Rand()
	    hit = game.kpower[loop]*math.pow(dustfac,game.kavgd[loop])
	    game.kpower[loop] *= 0.75
	else: # Enemy uses photon torpedo 
	    course = 1.90985*math.atan2(game.sector.y-jay.y, jay.x-game.sector.x)
	    hit = 0
	    proutn(_("***TORPEDO INCOMING"))
	    if not damaged(DSRSENS):
		proutn(_(" From "))
		crmena(False, iquad, where, jay)
	    attempt = True
	    prout("  ")
	    r = (Rand()+Rand())*0.5 -0.5
	    r += 0.002*game.kpower[loop]*r
	    hit = torpedo(course, r, jay, 1, 1)
	    if (game.state.remkl + game.state.remcom + game.state.nscrem)==0:
		finish(FWON); # Klingons did themselves in! 
	    if game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova or game.alldone:
		return; # Supernova or finished 
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
	    proutn(_(" on the "))
	    crmshp()
	if not damaged(DSRSENS) and usephasers:
	    proutn(_(" from "))
	    crmena(False, iquad, where, jay)
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
    if not atackd:
	return
    percent = 100.0*pfac*game.shield+0.5
    if not ihurt:
	# Shields fully protect ship 
	proutn(_("Enemy attack reduces shield strength to "))
    else:
	# Print message if starship suffered hit(s) 
	skip(1)
	proutn(_("Energy left %2d    shields ") % int(game.energy))
	if game.shldup:
	    proutn(_("up "))
	elif not damaged(DSHIELD):
	    proutn(_("down "))
	else:
	    proutn(_("damaged, "))
    prout(_("%d%%,   torpedoes left %d"), percent, game.torps)
    # Check if anyone was hurt 
    if hitmax >= 200 or hittot >= 500:
	icas= hittot*Rand()*0.015
	if icas >= 2:
	    skip(1)
	    prout(_("Mc Coy-  \"Sickbay to bridge.  We suffered %d casualties") % icas)
	    prout(_("   in that last attack.\""))
	    game.casual += icas
	    game.state.crew -= icas
    # After attack, reset average distance to enemies 
    for loop in range(1, game.nenhere+1):
	game.kavgd[loop] = game.kdist[loop]
    sortklings()
    return;
		
def deadkl(w, type, mv):
    # kill a Klingon, Tholian, Romulan, or Thingy 
    # Added mv to allow enemy to "move" before dying 

    crmena(True, type, sector, mv)
    # Decide what kind of enemy it is and update appropriately 
    if type == IHR:
	# chalk up a Romulan 
	game.state.galaxy[game.quadrant.x][game.quadrant.y].romulans -= 1
	game.irhere -= 1
	game.state.nromrem -= 1
    elif type == IHT:
	# Killed a Tholian 
	game.ithere = False
    elif type == IHQUEST:
	# Killed a Thingy 
	iqhere = iqengry = False
	invalidate(thing)
    else:
	# Some type of a Klingon 
	game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons -= 1
	game.klhere -= 1
	if type == IHC:
	    game.comhere = False
	    for i in range(1, game.state.remcom+1):
		if same(game.state.kcmdr[i], game.quadrant):
		    break
	    game.state.kcmdr[i] = game.state.kcmdr[game.state.remcom]
	    game.state.kcmdr[game.state.remcom].x = 0
	    game.state.kcmdr[game.state.remcom].y = 0
	    game.state.remcom -= 1
	    unschedule(FTBEAM)
	    if game.state.remcom != 0:
		schedule(FTBEAM, expran(1.0*game.incom/game.state.remcom))
	elif type ==  IHK:
	    game.state.remkl -= 1
	elif type ==  IHS:
	    game.state.nscrem -= 1
	    game.ishere = False
	    game.state.kscmdr.x = game.state.kscmdr.y = game.isatb = 0
	    game.iscate = False
	    unschedule(FSCMOVE)
	    unschedule(FSCDBAS)
	else:
	    prout("*** Internal error, deadkl() called on %s\n" % type)

    # For each kind of enemy, finish message to player 
    prout(_(" destroyed."))
    game.quad[w.x][w.y] = IHDOT
    if (game.state.remkl + game.state.remcom + game.state.nscrem)==0:
	return

    game.state.remtime = game.state.remres/(game.state.remkl + 4*game.state.remcom)

    # Remove enemy ship from arrays describing local conditions 
    if is_scheduled(FCDBAS) and same(game.battle, game.quadrant) and type==IHC:
	unschedule(FCDBAS)
    for i in range(1, game.nenhere+1):
	if same(game.ks[i], w):
	    break
    game.nenhere -= 1
    if i <= game.nenhere:
        for j in range(i, game.nenhere+1):
	    game.ks[j] = game.ks[j+1]
	    game.kpower[j] = game.kpower[j+1]
	    game.kavgd[j] = game.kdist[j] = game.kdist[j+1]
    game.ks[game.nenhere+1].x = 0
    game.ks[game.nenhere+1].x = 0
    game.kdist[game.nenhere+1] = 0
    game.kavgd[game.nenhere+1] = 0
    game.kpower[game.nenhere+1] = 0
    return;

def targetcheck(x, y):
    # Return None if target is invalid 
    if not VALID_SECTOR(x, y):
	huh()
	return None
    deltx = 0.1*(y - game.sector.y)
    delty = 0.1*(x - game.sector.x)
    if deltx==0 and delty== 0:
	skip(1)
	prout(_("Spock-  \"Bridge to sickbay.  Dr. McCoy,"))
	prout(_("  I recommend an immediate review of"))
	prout(_("  the Captain's psychological profile.\""))
	chew()
	return None
    return 1.90985932*math.atan2(deltx, delty)

def photon():
    # launch photon torpedo 
    game.ididit = False
    if damaged(DPHOTON):
	prout(_("Photon tubes damaged."))
	chew()
	return
    if game.torps == 0:
	prout(_("No torpedoes left."))
	chew()
	return
    key = scan()
    while True:
	if key == IHALPHA:
	    huh()
	    return
	elif key == IHEOL:
	    prout(_("%d torpedoes left."), game.torps)
	    proutn(_("Number of torpedoes to fire- "))
	    key = scan()
	else: # key == IHREAL  {
	    n = aaitem + 0.5
	    if n <= 0: # abort command 
		chew()
		return
	    if n > 3:
		chew()
		prout(_("Maximum of 3 torpedoes per burst."))
		key = IHEOL
		return
	    if n <= game.torps:
		break
	    chew()
	    key = IHEOL
    for i in range(1, n+1):
	key = scan()
	if i==1 and key == IHEOL:
	    break;	# we will try prompting 
	if i==2 and key == IHEOL:
	    # direct all torpedoes at one target 
	    while i <= n:
		targ[i][1] = targ[1][1]
		targ[i][2] = targ[1][2]
		course[i] = course[1]
		i += 1
	    break
	if key != IHREAL:
	    huh()
	    return
	targ[i][1] = aaitem
	key = scan()
	if key != IHREAL:
	    huh()
	    return
	targ[i][2] = aaitem
	course[i] = targetcheck(targ[i][1], targ[i][2])
        if course[i] == None:
	    return
    chew()
    if i == 1 and key == IHEOL:
	# prompt for each one 
	for i in range(1, n+1):
	    proutn(_("Target sector for torpedo number %d- "), i)
	    key = scan()
	    if key != IHREAL:
		huh()
		return
	    targ[i][1] = aaitem
	    key = scan()
	    if key != IHREAL:
		huh()
		return
	    targ[i][2] = aaitem
	    chew()
            course[i] = targetcheck(targ[i][1], targ[i][2])
            if course[i] == None:
                return
    game.ididit = True
    # Loop for moving <n> torpedoes 
    for i in range(1, n+1):
	if game.condition != "docked":
	    game.torps -= 1
	r = (Rand()+Rand())*0.5 -0.5
	if math.fabs(r) >= 0.47:
	    # misfire! 
	    r = (Rand()+1.2) * r
	    if n>1:
		prouts(_("***TORPEDO NUMBER %d MISFIRES"), i)
	    else:
		prouts(_("***TORPEDO MISFIRES."))
	    skip(1)
	    if i < n:
		prout(_("  Remainder of burst aborted."))
	    if Rand() <= 0.2:
		prout(_("***Photon tubes damaged by misfire."))
		game.damage[DPHOTON] = game.damfac*(1.0+2.0*Rand())
	    break
	if game.shldup or game.condition == "docked":
	    r *= 1.0 + 0.0001*game.shield
	torpedo(course[i], r, game.sector, i, n)
	if game.alldone or game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
	    return
    if (game.state.remkl + game.state.remcom + game.state.nscrem)==0:
	finish(FWON);

def overheat(rpow):
    # check for phasers overheating 
    if rpow > 1500:
	chekbrn = (rpow-1500.)*0.00038
	if Rand() <= chekbrn:
	    prout(_("Weapons officer Sulu-  \"Phasers overheated, sir.\""))
	    game.damage[DPHASER] = game.damfac*(1.0 + Rand()) * (1.0+chekbrn)

def checkshctrl(rpow):
    # check shield control 
	
    skip(1)
    if Rand() < 0.998:
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
    icas = hit*Rand()*0.012
    skip(1)
    fry(0.8*hit)
    if icas:
	skip(1)
	prout(_("McCoy to bridge- \"Severe radiation burns, Jim."))
	prout(_("  %d casualties so far.\""), icas)
	game.casual += icas
	game.state.crew -= icas
    skip(1)
    prout(_("Phaser energy dispersed by shields."))
    prout(_("Enemy unaffected."))
    overheat(rpow)
    return True;

def hittem(doublehits):
    # register a phaser hit on Klingons and Romulans 
    nenhr2=game.nenhere; kk=1
    w = coord()
    skip(1)
    for k in range(1, nenhr2+1):
        wham = hits[k]
	if wham==0:
	    continue
	dustfac = 0.9 + 0.01*Rand()
	hit = wham*math.pow(dustfac,game.kdist[kk])
	kpini = game.kpower[kk]
	kp = math.fabs(kpini)
	if PHASEFAC*hit < kp:
	    kp = PHASEFAC*hit
        if game.kpower[kk] < 0:
            game.kpower[kk] -= -kp
        else:
            game.kpower[kk] -= kp
	kpow = game.kpower[kk]
	w = game.ks[kk]
	if hit > 0.005:
	    if not damaged(DSRSENS):
		boom(w)
	    proutn(_("%d unit hit on ") % int(hit))
	else:
	    proutn(_("Very small hit on "))
	ienm = game.quad[w.x][w.y]
	if ienm==IHQUEST:
	    iqengry = True
	crmena(False, ienm, "sector", w)
	skip(1)
	if kpow == 0:
	    deadkl(w, ienm, w)
	    if (game.state.remkl + game.state.remcom + game.state.nscrem)==0:
		finish(FWON);		
	    if game.alldone:
		return
	    kk -= 1; # don't do the increment 
	else: # decide whether or not to emasculate klingon 
	    if kpow > 0 and Rand() >= 0.9 and \
		kpow <= ((0.4 + 0.4*Rand())*kpini):
		prout(_("***Mr. Spock-  \"Captain, the vessel at %s"),
		      cramlc(sector, w))
		prout(_("   has just lost its firepower.\""))
		game.kpower[kk] = -kpow
        kk += 1
    return;

def phasers():
    # fire phasers 
    hits = []; rpow=0
    kz = 0; k = 1; irec=0 # Cheating inhibitor 
    ifast = False; no = False; itarg = True; msgflag = True
    automode = "NOTSET"
    key=0

    skip(1)
    # SR sensors and Computer are needed fopr automode 
    if damaged(DSRSENS) or damaged(DCOMPTR):
	itarg = False
    if game.condition == "docked":
	prout(_("Phasers can't be fired through base shields."))
	chew()
	return
    if damaged(DPHASER):
	prout(_("Phaser control damaged."))
	chew()
	return
    if game.shldup:
	if damaged(DSHCTRL):
	    prout(_("High speed shield control damaged."))
	    chew()
	    return
	if game.energy <= 200.0:
	    prout(_("Insufficient energy to activate high-speed shield control."))
	    chew()
	    return
	prout(_("Weapons Officer Sulu-  \"High-speed shield control enabled, sir.\""))
	ifast = True
		
    # Original code so convoluted, I re-did it all 
    while automode=="NOTSET":
	key=scan()
	if key == IHALPHA:
	    if isit("manual"):
		if game.nenhere==0:
		    prout(_("There is no enemy present to select."))
		    chew()
		    key = IHEOL
		    automode="AUTOMATIC"
		else:
		    automode = "MANUAL"
		    key = scan()
	    elif isit("automatic"):
		if (not itarg) and game.nenhere != 0:
		    automode = "FORCEMAN"
		else:
		    if game.nenhere==0:
			prout(_("Energy will be expended into space."))
		    automode = "AUTOMATIC"
		    key = scan()
	    elif isit("no"):
		no = True
	    else:
		huh()
		return
	elif key == IHREAL:
	    if game.nenhere==0:
		prout(_("Energy will be expended into space."))
		automode = "AUTOMATIC"
	    elif not itarg:
		automode = "FORCEMAN"
	    else:
		automode = "AUTOMATIC"
	else:
	    # IHEOL 
	    if game.nenhere==0:
		prout(_("Energy will be expended into space."))
		automode = "AUTOMATIC"
	    elif not itarg:
		automode = "FORCEMAN"
	    else: 
		proutn(_("Manual or automatic? "))			
    avail = game.energy
    if ifast:
        avail -= 200.0
    if automode == "AUTOMATIC":
	if key == IHALPHA and isit("no"):
	    no = True
	    key = scan()
	if key != IHREAL and game.nenhere != 0:
	    prout(_("Phasers locked on target. Energy available: %.2f"),
		  avail)
	irec=0
        while True:
	    chew()
	    if not kz:
		for i in range(1, game.nenhere+1):
		    irec += math.fabs(game.kpower[i])/(PHASEFAC*math.pow(0.90,game.kdist[i]))*(1.01+0.05*Rand()) + 1.0
	    kz=1
	    proutn(_("%d units required. "), irec)
	    chew()
	    proutn(_("Units to fire= "))
	    key = scan()
	    if key!=IHREAL:
		return
	    rpow = aaitem
	    if rpow > avail:
		proutn(_("Energy available= %.2f") % avail)
		skip(1)
		key = IHEOL
            if not rpow > avail:
                break
	if rpow<=0:
	    # chicken out 
	    chew()
	    return
        key=scan()
	if key == IHALPHA and isit("no"):
	    no = True
	if ifast:
	    game.energy -= 200; # Go and do it! 
	    if checkshctrl(rpow):
		return
	chew()
	game.energy -= rpow
	extra = rpow
	if game.nenhere:
	    extra = 0.0
	    powrem = rpow
	    for i in range(1, game.nenhere+1):
		hits[i] = 0.0
		if powrem <= 0:
		    continue
		hits[i] = math.fabs(game.kpower[i])/(PHASEFAC*math.pow(0.90,game.kdist[i]))
		over = (0.01 + 0.05*Rand())*hits[i]
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
	    if game.ithere:
		proutn(_("*** Tholian web absorbs "))
		if game.nenhere>0:
		    proutn(_("excess "))
		prout(_("phaser energy."))
	    else:
		prout(_("%d expended on empty space."), int(extra))
    elif automode == "FORCEMAN":
	chew()
	key = IHEOL
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
        for k in range(1, game.nenhere+1):
	    aim = game.ks[k]
	    ienm = game.quad[aim.x][aim.y]
	    if msgflag:
		proutn(_("Energy available= %.2f") % avail-0.006)
		skip(1)
		msgflag = False
		rpow = 0.0
	    if damaged(DSRSENS) and not (abs(game.sector.x-aim.x) < 2 and abs(game.sector.y-aim.y) < 2) and \
		(ienm == IHC or ienm == IHS):
		cramen(ienm)
		prout(_(" can't be located without short range scan."))
		chew()
		key = IHEOL
		hits[k] = 0; # prevent overflow -- thanks to Alexei Voitenko 
		k += 1
		continue
	    if key == IHEOL:
		chew()
		if itarg and k > kz:
		    irec=(abs(game.kpower[k])/(PHASEFAC*math.pow(0.9,game.kdist[k]))) *	(1.01+0.05*Rand()) + 1.0
		kz = k
		proutn("(")
		if not damaged(DCOMPTR):
		    proutn("%d", irec)
		else:
		    proutn("??")
		proutn(")  ")
		proutn(_("units to fire at "))
		crmena(False, ienm, sector, aim)
		proutn("-  ")
		key = scan()
	    if key == IHALPHA and isit("no"):
		no = True
		key = scan()
		continue
	    if key == IHALPHA:
		huh()
		return
	    if key == IHEOL:
		if k==1: # Let me say I'm baffled by this 
		    msgflag = True
		continue
	    if aaitem < 0:
		# abort out 
		chew()
		return
	    hits[k] = aaitem
	    rpow += aaitem
	    # If total requested is too much, inform and start over 
            if rpow > avail:
		prout(_("Available energy exceeded -- try again."))
		chew()
		return
	    key = scan(); # scan for next value 
	    k += 1
	if rpow == 0.0:
	    # zero energy -- abort 
	    chew()
	    return
	if key == IHALPHA and isit("no"):
	    no = True
	game.energy -= rpow
	chew()
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
	    if Rand() >= 0.99:
		prout(_("Sulu-  \"Sir, the high-speed shield control has malfunctioned . . ."))
		prouts(_("         CLICK   CLICK   POP  . . ."))
		prout(_(" No response, sir!"))
		game.shldup = False
	    else:
		prout(_("Shields raised."))
	else:
	    game.shldup = False
    overheat(rpow);

