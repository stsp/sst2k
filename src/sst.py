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
    # We know this if either short or long range sensors are working
    if not damaged(DSRSENS) or not damaged(DLRSENS) or \
	game.condition == docked:
	crmena(True, ienm, sector, game.ks[loccom])
	prout(_(" escapes to %s (and regains strength)."),
	      cramlc(quadrant, iq))
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
# 3. If Enterprise is not docked, an agressive action is taken if enemy
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
    mdist = dist1 + 0.5; # Nearest integer distance 

    # If SC, check with spy to see if should hi-tail it 
    if ienm==IHS and \
	(game.kpower[loccom] <= 500.0 or (game.condition==docked and not damaged(DPHOTON))):
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
        if forces <= 1000.0 and game.condition != docked: # Typical situation 
	    motion = ((forces+200.0*Rand())/150.0) - 5.0
	else:
            if forces > 1000.0: # Very strong -- move in for kill 
		motion = (1.0-square(Rand()))*dist1 + 1.0
	    if game.condition=="docked" and (game.options & OPTION_BASE): # protected by base -- back off ! 
		motion -= game.skill*(2.0-square(Rand()))
	if idebug:
	    proutn("=== MOTION = %d, FORCES = %1.2f, ", motion, forces)
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
	proutn("NSTEPS = %d:", nsteps)
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
	    proutn(" %d", ll+1)
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
		proutn(cramlc(neither, next))
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
	    proutn(_(" from %s"), cramlc(2, com))
	    if game.kdist[loccom] < dist1:
		proutn(_(" advances to "))
	    else:
		proutn(_(" retreats to "))
	    prout(cramlc(sector, next))

def moveklings():
    # Klingon tactical movement 
    w = coord(); 

    if idebug:
	prout("== MOVCOM")

    # Figure out which Klingon is the commander (or Supercommander)
    #   and do move
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
    # if skill level is high, move other Klingons and Romulans too!
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
		pause_game(True)
		prout(_("Lt. Uhura-  \"Captain, Starfleet Intelligence reports"))
		proutn(_("   a planet in "))
		proutn(cramlc(quadrant, game.state.kscmdr))
		prout(_(" has been destroyed"))
		prout(_("   by the Super-commander.\""))
	    break
    return False; # looks good! 
			
def supercommander():
    # move the Super Commander 
    iq = coord(); sc = coord(); ibq = coord()
    basetbl = []

    if idebug:
	prout("== SUPERCOMMANDER")

    # Decide on being active or passive 
    avoid = ((game.incom - game.state.remcom + game.inkling - game.state.remkl)/(game.state.date+0.01-game.indate) < 0.1*game.skill*(game.skill+1.0) or \
	    (game.state.date-game.indate) < 3.0)
    if not game.iscate and avoid:
	# compute move away from Enterprise 
	ideltax = game.state.kscmdr.x-game.quadrant.x
	ideltay = game.state.kscmdr.y-game.quadrant.y
	if math.sqrt(ideltax*ideltax+ideltay*ideltay) > 2.0:
	    # circulate in space 
	    ideltax = game.state.kscmdr.y-game.quadrant.y
	    ideltay = game.quadrant.x-game.state.kscmdr.x
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
	ideltax = ibq.x - game.state.kscmdr.x
	ideltay = ibq.y - game.state.kscmdr.y
    # Maximum movement is 1 quadrant in either or both axis 
    if ideltax > 1:
	ideltax = 1
    if ideltax < -1:
	ideltax = -1
    if ideltay > 1:
	ideltay = 1
    if ideltay < -1:
	ideltay = -1

    # try moving in both x and y directions 
    iq.x = game.state.kscmdr.x + ideltax
    iq.y = game.state.kscmdr.y + ideltax
    if movescom(iq, avoid):
	# failed -- try some other maneuvers 
	if ideltax==0 or ideltay==0:
	    # attempt angle move 
	    if ideltax != 0:
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
		iq.y = game.state.kscmdr.y + ideltay
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
		pause_game(True)
		proutn(_("Lt. Uhura-  \"Captain, the starbase in "))
		proutn(cramlc(quadrant, game.state.kscmdr))
		skip(1)
		prout(_("   reports that it is under attack from the Klingon Super-commander."))
		proutn(_("   It can survive until stardate %d.\""),
		       int(scheduled(FSCDBAS)))
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
    pause_game(True)
    prout(_("Lt. Uhura-  \"Captain, Starfleet Intelligence reports"))
    proutn(_("   the Super-commander is in "))
    proutn(cramlc(quadrant, game.state.kscmdr))
    prout(".\"")
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
    crmena(True, IHT, sector, game.tholian)
    prout(_(" completes web."))
    game.ithere = False
    game.nenhere -= 1
    return
