#!/usr/bin/env python
"""
sst.py =-- Super Star Trek in Python

This code is a Python translation of a C translation of a FORTRAN original.
The FORTRANness still shows in many ways, notably the use of a lot of
parallel arrays where a more modern language would use structures
or objects.  (However, 1-origin array indexing was fixed.)

Dave Matuszek says:

SRSCAN, MOVE, PHASERS, CALL, STATUS, IMPULSE, PHOTONS, ABANDON,
LRSCAN, WARP, SHIELDS, DESTRUCT, CHART, REST, DOCK, QUIT, and DAMAGE
were in the original non-"super" version of UT FORTRAN Star Trek.

Tholians were not in the original. Dave is dubious about their merits.
(They are now controlled by OPTION_THOLIAN and turned off if the game
type is "plain".)

Planets and dilithium crystals were not in the original.  Dave is OK
with this idea. (It's now controlled by OPTION_PLANETS and turned 
off if the game type is "plain".)

Dave says the bit about the Galileo getting turned into a
McDonald's is "consistant with our original vision".  (This has been
left permanently enabled, as it can only happen if OPTION_PLANETS
is on.)

Dave also says the Space Thingy should not be preserved across saved
games, so you can't prove to others that you've seen it.  He says it
shouldn't fire back, either.  It should do nothing except scream and
disappear when hit by photon torpedos.  It's OK that it may move
when attacked, but it didn't in the original.  (Whether the Thingy
can fire back is now controlled by OPTION_THINGY and turned off if the
game type is "plain" or "almy".  The no-save behavior has been restored.)

The Faerie Queen, black holes, and time warping were in the original.

Here are Tom Almy's changes:

In early 1997, I got the bright idea to look for references to
"Super Star Trek" on the World Wide Web. There weren't many hits,
but there was one that came up with 1979 Fortran sources! This
version had a few additional features that mine didn't have,
however mine had some feature it didn't have. So I merged its
features that I liked. I also took a peek at the DECUS version (a
port, less sources, to the PDP-10), and some other variations.

1, Compared to the original UT version, I've changed the "help" command to
"call" and the "terminate" command to "quit" to better match
user expectations. The DECUS version apparently made those changes
as well as changing "freeze" to "save". However I like "freeze".
(Both "freeze" and "save" work in SST2K.)

2. The experimental deathray originally had only a 5% chance of
success, but could be used repeatedly. I guess after a couple
years of use, it was less "experimental" because the 1979
version had a 70% success rate. However it was prone to breaking
after use. I upgraded the deathray, but kept the original set of
failure modes (great humor!).  (Now controlled by OPTION_DEATHRAY
and turned off if game type is "plain".)

3. The 1979 version also mentions srscan and lrscan working when
docked (using the starbase's scanners), so I made some changes here
to do this (and indicating that fact to the player), and then realized
the base would have a subspace radio as well -- doing a Chart when docked
updates the star chart, and all radio reports will be heard. The Dock
command will also give a report if a base is under attack.

4. Tholian Web from the 1979 version.  (Now controlled by
OPTION_THOLIAN and turned off if game type is "plain".)

5. Enemies can ram the Enterprise. (Now controlled by OPTION_RAMMING
and turned off if game type is "plain".)

6. Regular Klingons and Romulans can move in Expert and Emeritus games. 
This code could use improvement. (Now controlled by OPTION_MVBADDY
and turned off if game type is "plain".)

7. The deep-space probe feature from the DECUS version.  (Now controlled
by OPTION_PROBE and turned off if game type is "plain").

8. 'emexit' command from the 1979 version.

9. Bugfix: Klingon commander movements are no longer reported if long-range 
sensors are damaged.

10. Bugfix: Better base positioning at startup (more spread out).
That made sense to add because most people abort games with 
bad base placement.

In June 2002, I fixed two known bugs and a documentation typo.
In June 2004 I fixed a number of bugs involving: 1) parsing invalid
numbers, 2) manual phasers when SR scan is damaged and commander is
present, 3) time warping into the future, 4) hang when moving
klingons in crowded quadrants.  (These fixes are in SST2K.)

Here are Stas Sergeev's changes:

1. The Space Thingy can be shoved, if you ram it, and can fire back if 
fired upon. (Now controlled by OPTION_THINGY and turned off if game 
type is "plain" or "almy".)

2. When you are docked, base covers you with an almost invincible shield. 
(A commander can still ram you, or a Romulan can destroy the base,
or a SCom can even succeed with direct attack IIRC, but this rarely 
happens.)  (Now controlled by OPTION_BASE and turned off if game 
type is "plain" or "almy".)

3. Ramming a black hole is no longer instant death.  There is a
chance you might get timewarped instead. (Now controlled by 
OPTION_BLKHOLE and turned off if game type is "plain" or "almy".)

4. The Tholian can be hit with phasers.

5. SCom can't escape from you if no more enemies remain 
(without this, chasing SCom can take an eternity).

6. Probe target you enter is now the destination quadrant. Before I don't 
remember what it was, but it was something I had difficulty using.

7. Secret password is now autogenerated.

8. "Plaque" is adjusted for A4 paper :-)

9. Phasers now tells you how much energy needed, but only if the computer 
is alive.

10. Planets are auto-scanned when you enter the quadrant.

11. Mining or using crystals in presense of enemy now yields an attack.
There are other minor adjustments to what yields an attack
and what does not.

12. "freeze" command reverts to "save", most people will understand this
better anyway. (SST2K recognizes both.)

13. Screen-oriented interface, with sensor scans always up.  (SST2K
supports both screen-oriented and TTY modes.)

Eric Raymond's changes:

Mainly, I translated this C code out of FORTRAN into C -- created #defines
for a lot of magic numbers and refactored the heck out of it.

1. "sos" and "call" becomes "mayday", "freeze" and "save" are both good.

2. Status report now indicates when dilithium crystals are on board.

3. Per Dave Matuszek's remarks, Thingy state is never saved across games.

4. Added game option selection so you can play a close (but not bug-for-
bug identical) approximation of older versions.

5. Half the quadrants now have inhabited planets, from which one 
cannot mine dilithium (there will still be the same additional number
of dilithium-bearing planets).  Torpedoing an inhabited world is *bad*.
There is BSD-Trek-like logic for Klingons to attack and enslave 
inhabited worlds, producing more ships (only is skill is 'good' or 
better). (Controlled by OPTION_WORLDS and turned off if game 
type is "plain" or "almy".)

6. User input is now logged so we can do regression testing.

7. More BSD-Trek features: You can now lose if your entire crew
dies in battle.  When abandoning ship in a game with inhabited
worlds enabled, they must have one in the quadrant to beam down
to; otherwise they die in space and this counts heavily against
your score.  Docking at a starbase replenishes your crew.

8. Still more BSD-Trek: we now have a weighted damage table.
Also, the nav subsystem (enabling automatic course
setting) can be damaged separately from the main computer (which
handles weapons targeting, ETA calculation, and self-destruct).
"""
import os,sys,math,curses,time,atexit,readline,cPickle,random,getopt,copy

SSTDOC  	= "/usr/share/doc/sst/sst.doc"
DOC_NAME	= "sst.doc"

# Stub to be replaced
def _(str): return str

PHASEFAC	= 2.0
GALSIZE 	= 8
NINHAB  	= (GALSIZE * GALSIZE / 2)
MAXUNINHAB	= 10
PLNETMAX	= (NINHAB + MAXUNINHAB)
QUADSIZE	= 10
BASEMIN		= 2
BASEMAX 	= (GALSIZE * GALSIZE / 12)
MAXKLGAME	= 127
MAXKLQUAD	= 9
FULLCREW	= 428	# BSD Trek was 387, that's wrong 
FOREVER 	= 1e30

# These functions hide the difference between 0-origin and 1-origin addressing.
def VALID_QUADRANT(x, y):	return ((x)>=0 and (x)<GALSIZE and (y)>=0 and (y)<GALSIZE)
def VALID_SECTOR(x, y):	return ((x)>=0 and (x)<QUADSIZE and (y)>=0 and (y)<QUADSIZE)

def square(i):		return ((i)*(i))
def distance(c1, c2):	return math.sqrt(square(c1.x - c2.x) + square(c1.y - c2.y))
def invalidate(w):	w.x = w.y = 0
def is_valid(w):	return (w.x != 0 and w.y != 0)

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

IHEOL = '\n'
IHREAL = 0.0
IHALPHA = " "

class coord:
    def __init__(self, x=None, y=None):
        self.x = x
        self.y = y
    def invalidate(self):
        self.x = self.y = None
    def is_valid(self):
        return self.x != None and self.y != None
    def __eq__(self, other):
        return other != None and self.x == other.y and self.x == other.y
    def __add__(self, other):
        return coord(self.x+self.x, self.y+self.y)
    def __sub__(self, other):
        return coord(self.x-self.x, self.y-self.y)
    def distance(self, other):
        return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2)
    def sgn(self):
        s = coord()
        if self.x == 0:
            s.x = 0
        else:
            s.x = self.x / abs(self.x)
        if self.y == 0:
            s.y = 0
        else:
            s.y = self.y / abs(self.y)
        return s
    def scatter(self):
        s = coord()
        s.x = self.x + randrange(-1, 2)
        s.y = self.y + randrange(-1, 2)
        return s
    def __hash__(self):
        return hash((x, y))
    def __str__(self):
        return "%s - %s" % (self.x+1, self.y+1)
    __repr__ = __str__

class planet:
    def __init__(self):
        self.name = None	# string-valued if inhabited
        self.w = coord()	# quadrant located
        self.pclass = None	# could be ""M", "N", "O", or "destroyed"
        self.crystals = "absent"# could be "mined", "present", "absent"
        self.known = "unknown"	# could be "unknown", "known", "shuttle_down"
        self.inhabited = False	# is it inhabites?
    def __str__(self):
        return self.name

class quadrant:
    def __init__(self):
        self.stars = None
        self.planet = None
	self.starbase = None
	self.klingons = None
	self.romulans = None
	self.supernova = None
	self.charted = None
        self.status = None	# Could be "secure", "distressed", "enslaved"

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
	self.remcom = 0  	# remaining commanders
	self.nscrem = 0		# remaining super commanders
	self.rembase = 0	# remaining bases
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
        for i in range(BASEMAX):
            self.baseq.append(coord())
        self.kcmdr = [] 	# Commander quadrant coordinates
        for i in range(QUADSIZE):
            self.kcmdr.append(coord())
	self.kscmdr = coord()	# Supercommander quadrant coordinates
        # the galaxy (subscript 0 not used)
        self.galaxy = fill2d(GALSIZE, lambda i, j: quadrant())
        # the starchart (subscript 0 not used)
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

#
# abstract out the event handling -- underlying data structures will change
# when we implement stateful events
# 
def findevent(evtype):	return game.future[evtype]

class gamestate:
    def __init__(self):
        self.options = None	# Game options
        self.state = snapshot()	# A snapshot structure
        self.snapsht = snapshot()	# Last snapshot taken for time-travel purposes
        self.quad = fill2d(QUADSIZE, lambda i, j: IHDOT)	# contents of our quadrant
        self.kpower = fill2d(QUADSIZE, lambda i, j: 0.0)	# enemy energy levels
        self.kdist = fill2d(QUADSIZE, lambda i, j: 0.0)		# enemy distances
        self.kavgd = fill2d(QUADSIZE, lambda i, j: 0.0) 	# average distances
        self.damage = [0.0] * NDEVICES	# damage encountered
        self.future = []		# future events
        for i in range(NEVENTS):
            self.future.append(event())
        self.passwd  = None;		# Self Destruct password
        self.ks = fill2d(QUADSIZE, lambda i, j: coord())	# enemy sector locations
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
    def recompute(self):
        # Stas thinks this should be (C expression): 
        # game.state.remkl + game.state.remcom > 0 ?
	#	game.state.remres/(game.state.remkl + 4*game.state.remcom) : 99
        # He says the existing expression is prone to divide-by-zero errors
        # after killing the last klingon when score is shown -- perhaps also
        # if the only remaining klingon is SCOM.
        game.state.remtime = game.state.remres/(game.state.remkl + 4*game.state.remcom)
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

# Log the results of pulling random numbers so we can check determinism.

import traceback

def withprob(p):
    v = random.random()
    #logfp.write("# withprob(%s) -> %f (%s) at %s\n" % (p, v, v<p, traceback.extract_stack()[-2][1:]))
    return v < p

def randrange(*args):
    v = random.randrange(*args)
    #logfp.write("# randrange%s -> %s at %s\n" % (args, v, traceback.extract_stack()[-2][1:]))
    return v

def randreal(*args):
    v = random.random()
    if len(args) == 1:
        v *= args[0] 		# returns from [0, a1)
    elif len(args) == 2:
        v = args[0] + v*args[1]	# returns from [a1, a2)
    #logfp.write("# randreal%s -> %s at %s\n" % (args, v, traceback.extract_stack()[-2][1:]))
    return v

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
	    for n in range(game.state.remcom):
		if game.state.kcmdr[n] == iq:
		    return False
	    # refuse to leave if currently attacking starbase 
	    if game.battle == game.quadrant:
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
	for n in range(game.state.remcom):
	    if game.state.kcmdr[n] == game.quadrant:
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
	    motion = ((forces + randreal(200))/150.0) - 5.0
	else:
            if forces > 1000.0: # Very strong -- move in for kill 
		motion = (1.0-square(randreal()))*dist1 + 1.0
	    if game.condition=="docked" and (game.options & OPTION_BASE): # protected by base -- back off ! 
		motion -= game.skill*(2.0-square(randreal()))
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
	    if look.x < 0 or look.x >= QUADSIZE:
		if motion < 0 and tryexit(look, ienm, loccom, irun):
		    return
		if krawlx == mx or my == 0:
		    break
		look.x = next.x + krawlx
		krawlx = -krawlx
	    elif look.y < 0 or look.y >= QUADSIZE:
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
    if next != com:
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
	for i in range(game.nenhere):
	    w = game.ks[i]
	    if game.quad[w.x][w.y] == IHC:
		movebaddy(w, i, IHC)
		break
    if game.ishere:
	for i in range(game.nenhere):
	    w = game.ks[i]
	    if game.quad[w.x][w.y] == IHS:
		movebaddy(w, i, IHS)
		break
    # If skill level is high, move other Klingons and Romulans too!
    # Move these last so they can base their actions on what the
    # commander(s) do.
    if game.skill >= SKILL_EXPERT and (game.options & OPTION_MVBADDY):
	for i in range(game.nenhere):
	    w = game.ks[i]
	    if game.quad[w.x][w.y] == IHK or game.quad[w.x][w.y] == IHR:
		movebaddy(w, i, game.quad[w.x][w.y])
    sortklings();

def movescom(iq, avoid):
    # commander movement helper 
    if iq == game.quadrant or not VALID_QUADRANT(iq.x, iq.y) or \
	game.state.galaxy[iq.x][iq.y].supernova or \
	game.state.galaxy[iq.x][iq.y].klingons > MAXKLQUAD-1:
	return 1
    if avoid:
	# Avoid quadrants with bases if we want to avoid Enterprise 
	for i in range(game.state.rembase):
	    if game.state.baseq[i] == iq:
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
	for i in range(game.nenhere):
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
	if game.state.planets[i].w == game.state.kscmdr and \
	    game.state.planets[i].crystals == "present":
	    # destroy the planet 
	    game.state.planets[i].pclass = "destroyed"
	    game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].planet = None
	    if communicating():
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
	for i in range(game.state.rembase):
	    basetbl.append((i, distance(game.state.baseq[i], sc)))
	if game.state.rembase > 1:
            basetbl.sort(lambda x, y: cmp(x[1]. y[1]))
	# look for nearest base without a commander, no Enterprise, and
        # without too many Klingons, and not already under attack. 
	ifindit = iwhichb = 0
	for i2 in range(game.state.rembase):
	    i = basetbl[i2][0];	# bug in original had it not finding nearest
	    ibq = game.state.baseq[i]
	    if ibq == game.quadrant or ibq == game.battle or \
		game.state.galaxy[ibq.x][ibq.y].supernova or \
		game.state.galaxy[ibq.x][ibq.y].klingons > MAXKLQUAD-1:
		continue
	    # if there is a commander, and no other base is appropriate,
	    #   we will take the one with the commander
	    for j in range(game.state.remcom):
		if ibq == game.state.kcmdr[j] and ifindit!= 2:
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
	for i in range(game.state.rembase):
	    ibq = game.state.baseq[i]
	    if ibq == game.state.kscmdr and game.state.kscmdr == game.battle:
		# attack the base 
		if avoid:
		    return; # no, don't attack base! 
		game.iseenit = False
		game.isatb = 1
		schedule(FSCDBAS, randreal(1.0, 3.0))
		if is_scheduled(FCDBAS):
		    postpone(FSCDBAS, scheduled(FCDBAS)-game.state.date)
		if not communicating():
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
	(withprob(0.8) or \
	 (not communicating()) or \
	 not game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].charted):
	return
    announce()
    prout(_("Lt. Uhura-  \"Captain, Starfleet Intelligence reports"))
    proutn(_("   the Super-commander is in Quadrant %s,") % game.state.kscmdr)
    return;

def movetholian():
    # move the Tholian 
    if not game.tholian or game.justin:
	return
    if game.tholian.x == 0 and game.tholian.y == 0:
	idx = 0; idy = QUADSIZE-1
    elif game.tholian.x == 0 and game.tholian.y == QUADSIZE-1:
	idx = QUADSIZE-1; idy = QUADSIZE-1
    elif game.tholian.x == QUADSIZE-1 and game.tholian.y == QUADSIZE-1:
	idx = QUADSIZE-1; idy = 0
    elif game.tholian.x == QUADSIZE-1 and game.tholian.y == 0:
	idx = 0; idy = 0
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
    for i in range(QUADSIZE):
	if game.quad[0][i]!=IHWEB and game.quad[0][i]!=IHT:
	    return
	if game.quad[QUADSIZE][i]!=IHWEB and game.quad[QUADSIZE][i]!=IHT:
	    return
	if game.quad[i][0]!=IHWEB and game.quad[i][0]!=IHT:
	    return
	if game.quad[i][QUADSIZE]!=IHWEB and game.quad[i][QUADSIZE]!=IHT:
	    return
    # All plugged up -- Tholian splits 
    game.quad[game.tholian.x][game.tholian.y]=IHWEB
    dropin(IHBLANK)
    crmena(True, IHT, "sector", game.tholian)
    prout(_(" completes web."))
    game.tholian = None
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
	if aaitem == 0:
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
    # all devices.  If we wanted that, we'd return randrange(NDEVICES)
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
    idx = randrange(1000)	# weights must sum to 1000 
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
    crmena(False, ienm, "sector", w)
    if ibumpd:
	proutn(_(" (original position)"))
    skip(1)
    deadkl(w, ienm, game.sector)
    proutn("***")
    crmshp()
    prout(_(" heavily damaged."))
    icas = randrange(10, 30)
    prout(_("***Sickbay reports %d casualties"), icas)
    game.casual += icas
    game.state.crew -= icas
    #
    # In the pre-SST2K version, all devices got equiprobably damaged,
    # which was silly.  Instead, pick up to half the devices at
    # random according to our weighting table,
    # 
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
	    hit = 700.0 + randreal(100) - \
		1000.0 * distance(w, incoming) * math.fabs(math.sin(bullseye-angle))
	    newcnd(); # we're blown out of dock 
	    # We may be displaced. 
	    if game.landed or game.condition=="docked":
		return hit # Cheat if on a planet 
	    ang = angle + 2.5*(randreal()-0.5)
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
	    if withprob(0.05):
		crmena(True, iquad, "sector", w)
		prout(_(" uses anti-photon device;"))
		prout(_("   torpedo neutralized."))
		return None
	elif iquad in (IHR, IHK): # Hit a regular enemy 
	    # find the enemy 
	    for ll in range(game.nenhere):
		if w == game.ks[ll]:
		    break
	    kp = math.fabs(game.kpower[ll])
	    h1 = 700.0 + randrange(100) - \
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
	    ang = angle + 2.5*(randreal()-0.5)
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
	    for ll in range(game.state.rembase):
		if game.state.baseq[ll] == game.quadrant:
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
	    crmena(True, iquad, "sector", w)
	    prout(_(" destroyed."))
	    game.state.nplankl += 1
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].planet = None
	    game.iplnet.pclass = "destroyed"
	    game.iplnet = None
	    invalidate(game.plnet)
	    game.quad[w.x][w.y] = IHDOT
	    if game.landed:
		# captain perishes on planet 
		finish(FDPLANET)
	    return None
	elif iquad == IHW: # Hit an inhabited world -- very bad! 
	    crmena(True, iquad, "sector", w)
	    prout(_(" destroyed."))
	    game.state.nworldkl += 1
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].planet = None
	    game.iplnet.pclass = "destroyed"
	    game.iplnet = None
	    invalidate(game.plnet)
	    game.quad[w.x][w.y] = IHDOT
	    if game.landed:
		# captain perishes on planet 
		finish(FDPLANET)
	    prout(_("You have just destroyed an inhabited planet."))
	    prout(_("Celebratory rallies are being held on the Klingon homeworld."))
	    return None
	elif iquad == IHSTAR: # Hit a star 
	    if withprob(0.9):
		nova(w)
            else:
                crmena(True, IHSTAR, "sector", w)
                prout(_(" unaffected by photon blast."))
	    return None
	elif iquad == IHQUEST: # Hit a thingy 
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
		#
		# Stas Sergeev added the possibility that
		# you can shove the Thingy and piss it off.
		# It then becomes an enemy and may fire at you.
		#
                global iqengry
		iqengry = True
		shoved = True
	    return None
	elif iquad == IHBLANK: # Black hole 
	    skip(1)
	    crmena(True, IHBLANK, "sector", w)
	    prout(_(" swallows torpedo."))
	    return None
	elif iquad == IHWEB: # hit the web 
	    skip(1)
	    prout(_("***Torpedo absorbed by Tholian web."))
	    return None
	elif iquad == IHT:  # Hit a Tholian 
	    h1 = 700.0 + randrange(100) - \
		1000.0 * distance(w, incoming) * math.fabs(math.sin(bullseye-angle))
	    h1 = math.fabs(h1)
	    if h1 >= 600:
		game.quad[w.x][w.y] = IHDOT
		game.tholian = None
		deadkl(w, iquad, w)
		return None
	    skip(1)
	    crmena(True, IHT, "sector", w)
	    if withprob(0.05):
		prout(_(" survives photon blast."))
		return None
	    prout(_(" disappears."))
	    game.quad[w.x][w.y] = IHWEB
	    game.tholian = None
	    game.nenhere -= 1
	    dropin(IHBLANK)
	    return None
        else: # Problem!
	    skip(1)
	    proutn("Don't know how to handle torpedo collision with ")
	    crmena(True, iquad, "sector", w)
	    skip(1)
	    return None
	break
    if curwnd!=message_window:
	setwnd(message_window)
    if shoved:
	game.quad[w.x][w.y]=IHDOT
	game.quad[jw.x][jw.y]=iquad
	prout(_(" displaced by blast to Sector %s ") % jw)
	for ll in range(game.nenhere):
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
    # Tholian gets to move before attacking 
    if game.tholian:
	movetholian()
    # if you have just entered the RNZ, you'll get a warning 
    if game.neutz: # The one chance not to be attacked 
	game.neutz = False
	return
    # commanders get a chance to tac-move towards you 
    if (((game.comhere or game.ishere) and not game.justin) or game.skill == SKILL_EMERITUS) and torps_ok:
	moveklings()
    # if no enemies remain after movement, we're done 
    if game.nenhere==0 or (game.nenhere==1 and thing == game.quadrant and not iqengry):
	return
    # set up partial hits if attack happens during shield status change 
    pfac = 1.0/game.inshld
    if game.shldchg:
	chgfac = 0.25 + randreal(0.5)
    skip(1)
    # message verbosity control 
    if game.skill <= SKILL_FAIR:
	where = "sector"
    for loop in range(game.nenhere):
	if game.kpower[loop] < 0:
	    continue;	# too weak to attack 
	# compute hit strength and diminish shield power 
	r = randreal()
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
	    dustfac = 0.8 + randreal(0.5)
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
	    r = (randreal()+randreal())*0.5 - 0.5
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
    prout(_("%d%%,   torpedoes left %d") % (percent, game.torps))
    # Check if anyone was hurt 
    if hitmax >= 200 or hittot >= 500:
	icas = randrange(hittot * 0.015)
	if icas >= 2:
	    skip(1)
	    prout(_("Mc Coy-  \"Sickbay to bridge.  We suffered %d casualties") % icas)
	    prout(_("   in that last attack.\""))
	    game.casual += icas
	    game.state.crew -= icas
    # After attack, reset average distance to enemies 
    for loop in range(game.nenhere):
	game.kavgd[loop] = game.kdist[loop]
    sortklings()
    return;
		
def deadkl(w, type, mv):
    # kill a Klingon, Tholian, Romulan, or Thingy 
    # Added mv to allow enemy to "move" before dying 
    crmena(True, type, "sector", mv)
    # Decide what kind of enemy it is and update appropriately 
    if type == IHR:
	# chalk up a Romulan 
	game.state.galaxy[game.quadrant.x][game.quadrant.y].romulans -= 1
	game.irhere -= 1
	game.state.nromrem -= 1
    elif type == IHT:
	# Killed a Tholian 
	game.tholian = None
    elif type == IHQUEST:
	# Killed a Thingy
        global iqengry
	iqengry = False
	invalidate(thing)
    else:
	# Some type of a Klingon 
	game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons -= 1
	game.klhere -= 1
	if type == IHC:
	    game.comhere = False
	    for i in range(game.state.remcom):
		if game.state.kcmdr[i] == game.quadrant:
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
    game.recompute()
    # Remove enemy ship from arrays describing local conditions 
    if is_scheduled(FCDBAS) and game.battle == game.quadrant and type==IHC:
	unschedule(FCDBAS)
    for i in range(game.nenhere):
	if game.ks[i] == w:
            for j in range(i, game.nenhere):
                game.ks[j] = game.ks[j+1]
                game.kpower[j] = game.kpower[j+1]
                game.kavgd[j] = game.kdist[j] = game.kdist[j+1]
            game.ks[game.nenhere].x = 0
            game.ks[game.nenhere].y = 0
            game.kdist[game.nenhere] = 0
            game.kavgd[game.nenhere] = 0
            game.kpower[game.nenhere] = 0
            game.nenhere -= 1
	    break
        break
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
	    prout(_("%d torpedoes left.") % game.torps)
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
	    proutn(_("Target sector for torpedo number %d- ") % i)
	    key = scan()
	    if key != IHREAL:
		huh()
		return
	    targ[i][1] = int(aaitem-0.5)
	    key = scan()
	    if key != IHREAL:
		huh()
		return
	    targ[i][2] = int(aaitem-0.5)
	    chew()
            course[i] = targetcheck(targ[i][1], targ[i][2])
            if course[i] == None:
                return
    game.ididit = True
    # Loop for moving <n> torpedoes 
    for i in range(n):
	if game.condition != "docked":
	    game.torps -= 1
	r = (randreal()+randreal())*0.5 -0.5
	if math.fabs(r) >= 0.47:
	    # misfire! 
	    r *= randreal(1.2, 2.2)
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
	    r *= 1.0 + 0.0001*game.shield
	torpedo(course[i], r, game.sector, i, n)
	if game.alldone or game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
	    return
    if (game.state.remkl + game.state.remcom + game.state.nscrem)==0:
	finish(FWON);

def overheat(rpow):
    # check for phasers overheating 
    if rpow > 1500:
        checkburn = (rpow-1500.0)*0.00038
        if withprob(checkburn):
	    prout(_("Weapons officer Sulu-  \"Phasers overheated, sir.\""))
	    game.damage[DPHASER] = game.damfac* randreal(1.0, 2.0) * (1.0+checkburn)

def checkshctrl(rpow):
    # check shield control 
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
    icas = randrange(hit*0.012)
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
    # register a phaser hit on Klingons and Romulans 
    nenhr2 = game.nenhere; kk=1
    w = coord()
    skip(1)
    for k in range(nenhr2):
        wham = hits[k]
	if wham==0:
	    continue
	dustfac = randreal(0.9, 1.0)
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
            global iqengry
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
	    if kpow>0 and withprob(0.9) and kpow <= randreal(0.4, 0.8)*kpini:
		prout(_("***Mr. Spock-  \"Captain, the vessel at Sector %s")%w)
		prout(_("   has just lost its firepower.\""))
		game.kpower[kk] = -kpow
        kk += 1
    return;

def phasers():
    # fire phasers 
    hits = []
    kz = 0; k = 1; irec=0 # Cheating inhibitor 
    ifast = False; no = False; itarg = True; msgflag = True; rpow=0
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
    # (That was Tom Almy talking about the C code, I think -- ESR)
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
                chew()
    avail = game.energy
    if ifast:
        avail -= 200.0
    if automode == "AUTOMATIC":
	if key == IHALPHA and isit("no"):
	    no = True
	    key = scan()
	if key != IHREAL and game.nenhere != 0:
	    prout(_("Phasers locked on target. Energy available: %.2f")%avail)
	irec=0
        while True:
	    chew()
	    if not kz:
		for i in range(game.nenhere):
		    irec += math.fabs(game.kpower[i])/(PHASEFAC*math.pow(0.90,game.kdist[i]))*randreal(1.01, 1.06) + 1.0
	    kz=1
	    proutn(_("%d units required. ") % irec)
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
	    for i in range(game.nenhere):
		hits.append(0.0)
		if powrem <= 0:
		    continue
		hits[i] = math.fabs(game.kpower[i])/(PHASEFAC*math.pow(0.90,game.kdist[i]))
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
		if game.nenhere>0:
		    proutn(_("excess "))
		prout(_("phaser energy."))
	    else:
		prout(_("%d expended on empty space.") % int(extra))
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
        for k in range(game.nenhere):
	    aim = game.ks[k]
	    ienm = game.quad[aim.x][aim.y]
	    if msgflag:
		proutn(_("Energy available= %.2f") % (avail-0.006))
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
		    irec=(abs(game.kpower[k])/(PHASEFAC*math.pow(0.9,game.kdist[k]))) *	randreal(1.01, 1.06) + 1.0
		kz = k
		proutn("(")
		if not damaged(DCOMPTR):
		    proutn("%d" % irec)
		else:
		    proutn("??")
		proutn(")  ")
		proutn(_("units to fire at "))
		crmena(False, ienm, "sector", aim)
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
	    if withprob(0.99):
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
    # remove an event from the schedule 
    game.future[evtype].date = FOREVER
    return game.future[evtype]

def is_scheduled(evtype):
    # is an event of specified type scheduled 
    return game.future[evtype].date != FOREVER

def scheduled(evtype):
    # when will this event happen? 
    return game.future[evtype].date

def schedule(evtype, offset):
    # schedule an event of specified type
    game.future[evtype].date = game.state.date + offset
    return game.future[evtype]

def postpone(evtype, offset):
    # postpone a scheduled event 
    game.future[evtype].date += offset

def cancelrest():
    # rest period is interrupted by event 
    if game.resting:
	skip(1)
	proutn(_("Mr. Spock-  \"Captain, shall we cancel the rest period?\""))
	if ja() == True:
	    game.resting = False
	    game.optime = 0.0
	    return True
    return False

def events():
    # run through the event queue looking for things to do 
    i=0
    fintim = game.state.date + game.optime; yank=0
    ictbeam = False; istract = False
    w = coord(); hold = coord()
    ev = event(); ev2 = event()

    def tractorbeam(yank):
        # tractor beaming cases merge here 
        announce()
        game.optime = (10.0/(7.5*7.5))*yank # 7.5 is yank rate (warp 7.5) 
        skip(1)
        proutn("***")
        crmshp()
        prout(_(" caught in long range tractor beam--"))
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
        crmshp()
        prout(_(" is pulled to Quadrant %s, Sector %s") \
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
        newqad(False)
        # Adjust finish time to time of tractor beaming 
        fintim = game.state.date+game.optime
        attack(False)
        if game.state.remcom <= 0:
            unschedule(FTBEAM)
        else: 
            schedule(FTBEAM, game.optime+expran(1.5*game.intime/game.state.remcom))

    def destroybase():
        # Code merges here for any commander destroying base 
        # Not perfect, but will have to do 
        # Handle case where base is in same quadrant as starship 
        if game.battle == game.quadrant:
            game.state.chart[game.battle.x][game.battle.y].starbase = False
            game.quad[game.base.x][game.base.y] = IHDOT
            game.base.x=game.base.y=0
            newcnd()
            skip(1)
            prout(_("Spock-  \"Captain, I believe the starbase has been destroyed.\""))
        elif game.state.rembase != 1 and communicating():
            # Get word via subspace radio 
            announce()
            skip(1)
            prout(_("Lt. Uhura-  \"Captain, Starfleet Command reports that"))
            proutn(_("   the starbase in Quadrant %s has been destroyed by") % game.battle)
            if game.isatb == 2: 
                prout(_("the Klingon Super-Commander"))
            else:
                prout(_("a Klingon Commander"))
            game.state.chart[game.battle.x][game.battle.y].starbase = False
        # Remove Starbase from galaxy 
        game.state.galaxy[game.battle.x][game.battle.y].starbase = False
        for i in range(1, game.state.rembase+1):
            if game.state.baseq[i] == game.battle:
                game.state.baseq[i] = game.state.baseq[game.state.rembase]
        game.state.rembase -= 1
        if game.isatb == 2:
            # reinstate a commander's base attack 
            game.battle = hold
            game.isatb = 0
        else:
            invalidate(game.battle)

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
    hold.x = hold.y = 0
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
	game.state.remres -= (game.state.remkl+4*game.state.remcom)*xtime
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
	    repair /= game.docfac
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
	    supernova(False)
	    schedule(FSNOVA, expran(0.5*game.intime))
	    if game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
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
                tractorbeam(distance(game.state.kscmdr, game.quadrant))
	    else:
		return
	elif evcode == FTBEAM: # Tractor beam 
            if game.state.remcom == 0:
                unschedule(FTBEAM)
                continue
            i = randrange(game.state.remcom)
            yank = distance(game.state.kcmdr[i], game.quadrant)
            if istract or game.condition == "docked" or yank == 0:
                # Drats! Have to reschedule 
                schedule(FTBEAM, 
                         game.optime + expran(1.5*game.intime/game.state.remcom))
                continue
            ictbeam = True
            tractorbeam(yank)
	elif evcode == FSNAP: # Snapshot of the universe (for time warp) 
	    game.snapsht = copy.deepcopy(game.state)
	    game.state.snap = True
	    schedule(FSNAP, expran(0.5 * game.intime))
	elif evcode == FBATTAK: # Commander attacks starbase 
	    if game.state.remcom==0 or game.state.rembase==0:
		# no can do 
		unschedule(FBATTAK)
		unschedule(FCDBAS)
                continue
	    i = 0
	    for j in range(game.state.rembase):
		for k in range(game.state.remcom):
		    if game.state.baseq[j] == game.state.kcmdr[k] and \
			not game.state.baseq[j] == game.quadrant and \
                        not game.state.baseq[j] == game.state.kscmdr:
			i = 1
		if i == 1:
		    continue
	    if j>game.state.rembase:
		# no match found -- try later 
		schedule(FBATTAK, expran(0.3*game.intime))
		unschedule(FCDBAS)
		continue
	    # commander + starbase combination found -- launch attack 
	    game.battle = game.state.baseq[j]
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
	    proutn(_("Lt. Uhura-  \"Captain, the starbase in Quadrant %s") % game.battle)
	    prout(_("   reports that it is under attack and that it can"))
	    proutn(_("   hold out only until stardate %d") % (int(scheduled(FCDBAS))))
            prout(".\"")
	    if cancelrest():
                return
	elif evcode == FSCDBAS: # Supercommander destroys base 
	    unschedule(FSCDBAS)
	    game.isatb = 2
	    if not game.state.galaxy[game.state.kscmdr.x][game.state.kscmdr.y].starbase: 
		continue # WAS RETURN! 
	    hold = game.battle
	    game.battle = game.state.kscmdr
	    destroybase()
	elif evcode == FCDBAS: # Commander succeeds in destroying base 
	    if evcode==FCDBAS:
		unschedule(FCDBAS)
		# find the lucky pair 
		for i in range(game.state.remcom):
		    if game.state.kcmdr[i] == game.battle: 
			break
		if i > game.state.remcom or game.state.rembase == 0 or \
		    not game.state.galaxy[game.battle.x][game.battle.y].starbase:
		    # No action to take after all 
		    invalidate(game.battle)
		    continue
            destroybase()
	elif evcode == FSCMOVE: # Supercommander moves 
	    schedule(FSCMOVE, 0.2777)
	    if not game.ientesc and not istract and game.isatb != 1 and \
                   (not game.iscate or not game.justin): 
		supercommander()
	elif evcode == FDSPROB: # Move deep space probe 
	    schedule(FDSPROB, 0.01)
	    game.probex += game.probeinx
	    game.probey += game.probeiny
	    i = (int)(game.probex/QUADSIZE +0.05)
	    j = (int)(game.probey/QUADSIZE + 0.05)
	    if game.probec.x != i or game.probec.y != j:
		game.probec.x = i
		game.probec.y = j
		if not VALID_QUADRANT(i, j) or \
		    game.state.galaxy[game.probec.x][game.probec.y].supernova:
		    # Left galaxy or ran into supernova
                    if comunicating():
			announce()
			skip(1)
			proutn(_("Lt. Uhura-  \"The deep space probe "))
			if not VALID_QUADRANT(j, i):
			    proutn(_("has left the galaxy"))
			else:
			    proutn(_("is no longer transmitting"))
			prout(".\"")
		    unschedule(FDSPROB)
		    continue
                if not communicating():
		    announce()
		    skip(1)
		    proutn(_("Lt. Uhura-  \"The deep space probe is now in Quadrant %s.\"") % game.probec)
	    pdest = game.state.galaxy[game.probec.x][game.probec.y]
	    # Update star chart if Radio is working or have access to radio
	    if communicating():
		chp = game.state.chart[game.probec.x][game.probec.y]
		chp.klingons = pdest.klingons
		chp.starbase = pdest.starbase
		chp.stars = pdest.stars
		pdest.charted = True
	    game.proben -= 1 # One less to travel
	    if game.proben == 0 and game.isarmed and pdest.stars:
		# lets blow the sucker! 
		supernova(True, game.probec)
		unschedule(FDSPROB)
		if game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova: 
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
		q = game.state.galaxy[w.x][w.y]
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
	    q.status = distressed

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
	    q = game.state.galaxy[ev.quadrant.x][ev.quadrant.y]
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
	    q = game.state.galaxy[ev.quadrant.x][ev.quadrant.y]
	    if q.klingons <= 0:
		q.status = "secure"
		continue
	    if game.state.remkl >=MAXKLGAME:
		continue		# full right now 
	    # reproduce one Klingon 
	    w = ev.quadrant
	    if game.klhere >= MAXKLQUAD:
                try:
                    # this quadrant not ok, pick an adjacent one 
                    for i in range(w.x - 1, w.x + 2):
                        for j in range(w.y - 1, w.y + 2):
                            if not VALID_QUADRANT(i, j):
                                continue
                            q = game.state.galaxy[w.x][w.y]
                            # check for this quad ok (not full & no snova) 
                            if q.klingons >= MAXKLQUAD or q.supernova:
                                continue
                            raise "FOUNDIT"
                    else:
                        continue	# search for eligible quadrant failed
                except "FOUNDIT":
                    w.x = i
                    w.y = j
	    # deliver the child 
	    game.state.remkl += 1
	    q.klingons += 1
	    if game.quadrant == w:
                game.klhere += 1
		newkling(game.klhere)
	    # recompute time left
            game.recompute()
	    # report the disaster if we can 
	    if communicating():
		if game.quadrant == w:
		    prout(_("Spock- sensors indicate the Klingons have"))
		    prout(_("launched a warship from %s.") % q.planet)
		else:
		    prout(_("Uhura- Starfleet reports increased Klingon activity"))
		    if q.planet != None:
			proutn(_("near %s") % q.planet)
		    prout(_("in Quadrant %s.") % w)
				
def wait():
    # wait on events 
    game.ididit = False
    while True:
	key = scan()
	if key  != IHEOL:
	    break
	proutn(_("How long? "))
    chew()
    if key != IHREAL:
	huh()
	return
    origTime = delay = aaitem
    if delay <= 0.0:
	return
    if delay >= game.state.remtime or game.nenhere != 0:
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
	if game.nenhere:
	    rtime = randreal(1.0, 2.0)
	    if rtime < temp:
		temp = rtime
	    game.optime = temp
	if game.optime < delay:
	    attack(False)
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
        if game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
            break
    game.resting = False
    game.optime = 0

# A nova occurs.  It is the result of having a star hit with a
# photon torpedo, or possibly of a probe warhead going off.
# Stars that go nova cause stars which surround them to undergo
# the same probabilistic process.  Klingons next to them are
# destroyed.  And if the starship is next to it, it gets zapped.
# If the zap is too much, it gets destroyed.
        
def nova(nov):
    # star goes nova 
    course = (0.0, 10.5, 12.0, 1.5, 9.0, 0.0, 3.0, 7.5, 6.0, 4.5)
    newc = coord(); scratch = coord()
    if withprob(0.05):
	# Wow! We've supernova'ed 
	supernova(False, nov)
	return
    # handle initial nova 
    game.quad[nov.x][nov.y] = IHDOT
    crmena(False, IHSTAR, "sector", nov)
    prout(_(" novas."))
    game.state.galaxy[game.quadrant.x][game.quadrant.y].stars -= 1
    game.state.starkl += 1
	
    # Set up stack to recursively trigger adjacent stars 
    bot = top = top2 = 1
    kount = 0
    icx = icy = 0
    hits[1][1] = nov.x
    hits[1][2] = nov.y
    while True:
	for mm in range(bot, top+1): 
	    for nn in range(1, 3+1):  # nn,j represents coordinates around current 
		for j in range(1, 3+1):
		    if j==2 and nn== 2:
			continue
		    scratch.x = hits[mm][1]+nn-2
		    scratch.y = hits[mm][2]+j-2
		    if not VALID_SECTOR(scratch.y, scratch.x):
			continue
		    iquad = game.quad[scratch.x][scratch.y]
                    # Empty space ends reaction
                    if iquad in (IHDOT, IHQUEST, IHBLANK, IHT, IHWEB):
			break
		    elif iquad == IHSTAR: # Affect another star 
			if wthprob(0.05):
			    # This star supernovas 
			    scratch = supernova(False)
			    return
			top2 += 1
			hits[top2][1]=scratch.x
			hits[top2][2]=scratch.y
			game.state.galaxy[game.quadrant.x][game.quadrant.y].stars -= 1
			game.state.starkl += 1
			crmena(True, IHSTAR, "sector", scratch)
			prout(_(" novas."))
			game.quad[scratch.x][scratch.y] = IHDOT
		    elif iquad == IHP: # Destroy planet 
			game.state.galaxy[game.quadrant.x][game.quadrant.y].planet = None
			game.state.nplankl += 1
			crmena(True, IHP, "sector", scratch)
			prout(_(" destroyed."))
			game.iplnet.pclass = "destroyed"
			game.iplnet = None
			invalidate(game.plnet)
			if game.landed:
			    finish(FPNOVA)
			    return
			game.quad[scratch.x][scratch.y] = IHDOT
		    elif iquad == IHB: # Destroy base 
			game.state.galaxy[game.quadrant.x][game.quadrant.y].starbase = False
			for i in range(game.state.rembase):
			    if game.state.baseq[i] == game.quadrant: 
				break
			game.state.baseq[i] = game.state.baseq[game.state.rembase]
			game.state.rembase -= 1
			invalidate(game.base)
			game.state.basekl += 1
			newcnd()
			crmena(True, IHB, "sector", scratch)
			prout(_(" destroyed."))
			game.quad[scratch.x][scratch.y] = IHDOT
		    elif iquad in (IHE, IHF): # Buffet ship 
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
			icx += game.sector.x-hits[mm][1]
			icy += game.sector.y-hits[mm][2]
			kount += 1
		    elif iquad == IHK: # kill klingon 
			deadkl(scratch,iquad, scratch)
                    elif iquad in (IHC,IHS,IHR): # Damage/destroy big enemies 
			for ll in range(game.nenhere):
			    if game.ks[ll] == scratch:
				break
			game.kpower[ll] -= 800.0 # If firepower is lost, die 
			if game.kpower[ll] <= 0.0:
			    deadkl(scratch, iquad, scratch)
			    break
			newc.x = scratch.x + scratch.x - hits[mm][1]
			newc.y = scratch.y + scratch.y - hits[mm][2]
			crmena(True, iquad, "sector", scratch)
			proutn(_(" damaged"))
			if not VALID_SECTOR(newc.x, newc.y):
			    # can't leave quadrant 
			    skip(1)
			    break
			iquad1 = game.quad[newc.x][newc.y]
			if iquad1 == IHBLANK:
			    proutn(_(", blasted into "))
			    crmena(False, IHBLANK, "sector", newc)
			    skip(1)
			    deadkl(scratch, iquad, newc)
			    break
			if iquad1 != IHDOT:
			    # can't move into something else 
			    skip(1)
			    break
			proutn(_(", buffeted to Sector %s") % newc)
			game.quad[scratch.x][scratch.y] = IHDOT
			game.quad[newc.x][newc.y] = iquad
			game.ks[ll] = newc
			game.kdist[ll] = game.kavgd[ll] = distance(game.sector, newc)
			skip(1)
	if top == top2: 
	    break
	bot = top + 1
	top = top2
    if kount==0: 
	return

    # Starship affected by nova -- kick it away. 
    game.dist = kount*0.1
    icx = sgn(icx)
    icy = sgn(icy)
    game.direc = course[3*(icx+1)+icy+2]
    if game.direc == 0.0:
	game.dist = 0.0
    if game.dist == 0.0:
	return
    game.optime = 10.0*game.dist/16.0
    skip(1)
    prout(_("Force of nova displaces starship."))
    imove(novapush=True)
    game.optime = 10.0*game.dist/16.0
    return
	
def supernova(induced, w=None):
    # star goes supernova 
    num = 0; npdead = 0
    nq = coord()
    if w != None: 
	nq = w
    else:
	stars = 0
	# Scheduled supernova -- select star 
	# logic changed here so that we won't favor quadrants in top
        # left of universe 
	for nq.x in range(GALSIZE):
	    for nq.y in range(GALSIZE):
		stars += game.state.galaxy[nq.x][nq.y].stars
	if stars == 0:
	    return # nothing to supernova exists 
	num = randrange(stars) + 1
	for nq.x in range(GALSIZE):
	    for nq.y in range(GALSIZE):
		num -= game.state.galaxy[nq.x][nq.y].stars
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
	num = randrange(game.state.galaxy[nq.x][nq.y].stars) + 1
	for ns.x in range(QUADSIZE):
	    for ns.y in range(QUADSIZE):
		if game.quad[ns.x][ns.y]==IHSTAR:
		    num -= 1
		    if num==0:
			break
	    if num==0:
		break
	skip(1)
	prouts(_("***RED ALERT!  RED ALERT!"))
	skip(1)
	prout(_("***Incipient supernova detected at Sector %s") % ns)
	if square(ns.x-game.sector.x) + square(ns.y-game.sector.y) <= 2.1:
	    proutn(_("Emergency override attempts t"))
	    prouts("***************")
	    skip(1)
	    stars()
	    game.alldone = True

    # destroy any Klingons in supernovaed quadrant 
    kldead = game.state.galaxy[nq.x][nq.y].klingons
    game.state.galaxy[nq.x][nq.y].klingons = 0
    if nq == game.state.kscmdr:
	# did in the Supercommander! 
	game.state.nscrem = game.state.kscmdr.x = game.state.kscmdr.y = game.isatb =  0
	game.iscate = False
	unschedule(FSCMOVE)
	unschedule(FSCDBAS)
    if game.state.remcom:
	maxloop = game.state.remcom
	for l in range(maxloop):
	    if game.state.kcmdr[l] == nq:
		game.state.kcmdr[l] = game.state.kcmdr[game.state.remcom]
		invalidate(game.state.kcmdr[game.state.remcom])
		game.state.remcom -= 1
		kldead -= 1
		if game.state.remcom==0:
		    unschedule(FTBEAM)
		break
    game.state.remkl -= kldead
    # destroy Romulans and planets in supernovaed quadrant 
    nrmdead = game.state.galaxy[nq.x][nq.y].romulans
    game.state.galaxy[nq.x][nq.y].romulans = 0
    game.state.nromrem -= nrmdead
    # Destroy planets 
    for loop in range(game.inplan):
	if game.state.planets[loop].w == nq:
	    game.state.planets[loop].pclass = "destroyed"
	    npdead += 1
    # Destroy any base in supernovaed quadrant 
    if game.state.rembase:
	maxloop = game.state.rembase
	for loop in range(maxloop):
	    if game.state.baseq[loop] == nq:
		game.state.baseq[loop] = game.state.baseq[game.state.rembase]
		invalidate(game.state.baseq[game.state.rembase])
		game.state.rembase -= 1
		break
    # If starship caused supernova, tally up destruction 
    if induced:
	game.state.starkl += game.state.galaxy[nq.x][nq.y].stars
	game.state.basekl += game.state.galaxy[nq.x][nq.y].starbase
	game.state.nplankl += npdead
    # mark supernova in galaxy and in star chart 
    if game.quadrant == nq or communicating():
	game.state.galaxy[nq.x][nq.y].supernova = True
    # If supernova destroys last Klingons give special message 
    if (game.state.remkl + game.state.remcom + game.state.nscrem)==0 and not nq == game.quadrant:
	skip(2)
	if not induced:
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
    # self-destruct maneuver 
    # Finish with a BANG! 
    chew()
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
    scan()
    chew()
    if game.passwd != citem:
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
    if game.ship==IHE:
	prouts("***")
    prouts(_("********* Entropy of "))
    crmshp()
    prouts(_(" maximized *********"))
    skip(1)
    stars()
    skip(1)
    if game.nenhere != 0:
	whammo = 25.0 * game.energy
	l=1
	while l <= game.nenhere:
	    if game.kpower[l]*game.kdist[l] <= whammo: 
		deadkl(game.ks[l], game.quad[game.ks[l].x][game.ks[l].y], game.ks[l])
	    l += 1
    finish(FDILITHIUM)
				
def killrate():
    "Compute our rate of kils over time."
    elapsed = game.state.date - game.indate
    if elapsed == 0:	# Avoid divide-by-zero error if calculated on turn 0
        return 0
    else:
        starting = (game.inkling + game.incom + game.inscom)
        remaining = (game.state.remkl + game.state.remcom + game.state.nscrem)
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
    if game.ship == IHF:
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
			chew()
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
	if (game.state.remkl + game.state.remcom + game.state.nscrem)*3.0 > (game.inkling + game.incom + game.inscom):
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
	proutn(_("The "))
	crmshp()
	prout(_("has been destroyed in battle."))
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
	proutn(_("The "))
	crmshp()
	prout(_(" has been fried by a supernova."))
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
	proutn(_("The "))
	crmshp()
	prout(_(" has been cremated by its own phasers."))
    elif ifin == FLOST:
	prout(_("You and your landing party have been"))
	prout(_("converted to energy, disipating through space."))
    elif ifin == FMINING:
	prout(_("You are left with your landing party on"))
	prout(_("a wild jungle planet inhabited by primitive cannibals."))
	skip(1)
	prout(_("They are very fond of \"Captain Kirk\" soup."))
	skip(1)
	proutn(_("Without your leadership, the "))
	crmshp()
	prout(_(" is destroyed."))
    elif ifin == FDPLANET:
	prout(_("You and your mining party perish."))
	skip(1)
	prout(_("That was a great shot."))
	skip(1)
    elif ifin == FSSC:
	prout(_("The Galileo is instantly annihilated by the supernova."))
	prout(_("You and your mining party are atomized."))
	skip(1)
	proutn(_("Mr. Spock takes command of the "))
	crmshp()
	prout(_(" and"))
	prout(_("joins the Romulans, reigning terror on the Federation."))
    elif ifin == FPNOVA:
	prout(_("You and your mining party are atomized."))
	skip(1)
	proutn(_("Mr. Spock takes command of the "))
	crmshp()
	prout(_(" and"))
	prout(_("joins the Romulans, reigning terror on the Federation."))
    elif ifin == FSTRACTOR:
	prout(_("The shuttle craft Galileo is also caught,"))
	prout(_("and breaks up under the strain."))
	skip(1)
	prout(_("Your debris is scattered for millions of miles."))
	proutn(_("Without your leadership, the "))
	crmshp()
	prout(_(" is destroyed."))
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
    if game.ship == IHF:
	game.ship = None
    elif game.ship == IHE:
	game.ship = IHF
    game.alive = False
    if (game.state.remkl + game.state.remcom + game.state.nscrem) != 0:
	goodies = game.state.remres/game.inresor
	baddies = (game.state.remkl + 2.0*game.state.remcom)/(game.inkling+2.0*game.incom)
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
    # compute player's score 
    timused = game.state.date - game.indate
    iskill = game.skill
    if (timused == 0 or (game.state.remkl + game.state.remcom + game.state.nscrem) != 0) and timused < 5.0:
	timused = 5.0
    perdate = killrate()
    ithperd = 500*perdate + 0.5
    iwon = 0
    if game.gamewon:
	iwon = 100*game.skill
    if game.ship == IHE: 
	klship = 0
    elif game.ship == IHF: 
	klship = 1
    else:
	klship = 2
    if not game.gamewon:
	game.state.nromrem = 0 # None captured if no win
    iscore = 10*(game.inkling - game.state.remkl) \
             + 50*(game.incom - game.state.remcom) \
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
    if game.state.nromrem:
	prout(_("%6d Romulans captured                  %5d") %
	      (game.state.nromrem, game.state.nromrem))
    if game.inkling - game.state.remkl:
	prout(_("%6d ordinary Klingons destroyed        %5d") %
	      (game.inkling - game.state.remkl, 10*(game.inkling - game.state.remkl)))
    if game.incom - game.state.remcom:
	prout(_("%6d Klingon commanders destroyed       %5d") %
	      (game.incom - game.state.remcom, 50*(game.incom - game.state.remcom)))
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
	      (game.state.nplankl, -300*game.state.nworldkl))
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
    # emit winner's commemmorative plaque 
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
    timestring = ctime()
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

def outro():
    "wrap up, either normally or due to signal"
    if game.options & OPTION_CURSES:
	#clear()
	#curs_set(1)
	#refresh()
	#resetterm()
	#echo()
	curses.endwin()
	sys.stdout.write('\n')
    if logfp:
	logfp.close()

def iostart():
    global stdscr, rows
    #setlocale(LC_ALL, "")
    #bindtextdomain(PACKAGE, LOCALEDIR)
    #textdomain(PACKAGE)
    if atexit.register(outro):
	sys.stderr.write("Unable to register outro(), exiting...\n")
	raise SysExit,1
    if not (game.options & OPTION_CURSES):
	ln_env = os.getenv("LINES")
        if ln_env:
            rows = ln_env
        else:
            rows = 25
    else:
	stdscr = curses.initscr()
	stdscr.keypad(True)
	#saveterm()
	curses.nonl()
	curses.cbreak()
        curses.start_color()
        curses.init_pair(curses.COLOR_BLACK, curses.COLOR_BLACK, curses.COLOR_BLACK)
        curses.init_pair(curses.COLOR_GREEN, curses.COLOR_GREEN, curses.COLOR_BLACK)
        curses.init_pair(curses.COLOR_RED, curses.COLOR_RED, curses.COLOR_BLACK)
        curses.init_pair(curses.COLOR_CYAN, curses.COLOR_CYAN, curses.COLOR_BLACK)
        curses.init_pair(curses.COLOR_WHITE, curses.COLOR_WHITE, curses.COLOR_BLACK)
        curses.init_pair(curses.COLOR_MAGENTA, curses.COLOR_MAGENTA, curses.COLOR_BLACK)
        curses.init_pair(curses.COLOR_BLUE, curses.COLOR_BLUE, curses.COLOR_BLACK)
        curses.init_pair(curses.COLOR_YELLOW, curses.COLOR_YELLOW, curses.COLOR_BLACK)
	#noecho()
        global fullscreen_window, srscan_window, report_window, status_window
        global lrscan_window, message_window, prompt_window
	fullscreen_window = stdscr
	srscan_window     = curses.newwin(12, 25, 0,       0)
	report_window     = curses.newwin(11, 0,  1,       25)
	status_window     = curses.newwin(10, 0,  1,       39)
	lrscan_window     = curses.newwin(5,  0,  0,       64) 
	message_window    = curses.newwin(0,  0,  12,      0)
	prompt_window     = curses.newwin(1,  0,  rows-2,  0) 
	message_window.scrollok(True)
	setwnd(fullscreen_window)
	textcolor(DEFAULT)

def waitfor():
    "wait for user action -- OK to do nothing if on a TTY"
    if game.options & OPTION_CURSES:
	stsdcr.getch()

def announce():
    skip(1)
    if game.skill > SKILL_FAIR:
	prouts(_("[ANOUNCEMENT ARRIVING...]"))
    else:
	prouts(_("[IMPORTANT ANNOUNCEMENT ARRIVING -- PRESS ENTER TO CONTINUE]"))
    skip(1)

def pause_game():
    if game.skill > SKILL_FAIR:
        prompt = _("[CONTINUE?]")
    else:
        prompt = _("[PRESS ENTER TO CONTINUE]")

    if game.options & OPTION_CURSES:
        drawmaps(0)
        setwnd(prompt_window)
        prompt_window.wclear()
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
		proutn("\n")
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
    "print slowly!" 
    for c in line:
	time.sleep(0.03)
	proutn(c)
	if game.options & OPTION_CURSES:
	    wrefresh(curwnd)
	else:
	    sys.stdout.flush()
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
                if line == '':
                    prout("*** Replay finished")
                    replayfp.close()
                    break
                elif line[0] != "#":
                    break
	else:
	    line = raw_input()
    if logfp:
	logfp.write(line + "\n")
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
        wclrtoeol(curwnd)
        wrefresh(curwnd)

def clrscr():
    "Clear screen -- can be a no-op in tty mode."
    global linecount
    if game.options & OPTION_CURSES:
       curwnd.clear()
       curwnd.move(0, 0)
       curwnd.refresh()
    linecount = 0

def textcolor(color):
    "Set the current text color"
    if game.options & OPTION_CURSES:
	if color == DEFAULT: 
	    curwnd.attrset(0)
	elif color == BLACK: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_BLACK))
	elif color == BLUE: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_BLUE))
	elif color == GREEN: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_GREEN))
	elif color == CYAN: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_CYAN))
	elif color == RED: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_RED))
	elif color == MAGENTA: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_MAGENTA))
	elif color == BROWN: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_YELLOW))
	elif color == LIGHTGRAY: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_WHITE))
	elif color == DARKGRAY: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_BLACK) | curses.A_BOLD)
	elif color == LIGHTBLUE: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_BLUE) | curses.A_BOLD)
	elif color == LIGHTGREEN: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_GREEN) | curses.A_BOLD)
	elif color == LIGHTCYAN: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_CYAN) | curses.A_BOLD)
	elif color == LIGHTRED: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_RED) | curses.A_BOLD)
	elif color == LIGHTMAGENTA: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_MAGENTA) | curses.A_BOLD)
	elif color == YELLOW: 
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_YELLOW) | curses.A_BOLD)
	elif color == WHITE:
	    curwnd.attron(curses.COLOR_PAIR(curses.COLOR_WHITE) | curses.A_BOLD)

def highvideo():
    "Set highlight video, if this is reasonable."
    if game.options & OPTION_CURSES:
	curwnd.attron(curses.A_REVERSE)
 
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
	    lrscan()

def put_srscan_sym(w, sym):
    "Emit symbol for short-range scan."
    srscan_window.move(w.x+1, w.y*2+2)
    srscan_window.addch(sym)
    srscan_window.refresh()

def boom(w):
    "Enemy fall down, go boom."  
    if game.options & OPTION_CURSES:
	drawmaps(2)
	setwnd(srscan_window)
	srscan_window.attron(curses.A_REVERSE)
	put_srscan_sym(w, game.quad[w.x][w.y])
	#sound(500)
	#time.sleep(1.0)
	#nosound()
	srscan_window.attroff(curses.A_REVERSE)
	put_srscan_sym(w, game.quad[w.x][w.y])
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

def tracktorpedo(w, l, i, n, iquad):
    "Torpedo-track animation." 
    if not game.options & OPTION_CURSES:
	if l == 1:
	    if n != 1:
		skip(1)
		proutn(_("Track for torpedo number %d-  ") % i)
	    else:
		skip(1)
		proutn(_("Torpedo track- "))
	elif l==4 or l==9: 
	    skip(1)
	proutn("%d - %d   " % (w.x, w.y))
    else:
	if not damaged(DSRSENS) or game.condition=="docked":
	    if i != 1 and l == 1:
		drawmaps(2)
		time.sleep(0.4)
	    if (iquad==IHDOT) or (iquad==IHBLANK):
		put_srscan_sym(w, '+')
		#sound(l*10)
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
	    proutn("%d - %d   " % (w.x, w.y))

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

def imove(novapush):
    # movement execution for warp, impulse, supernova, and tractor-beam events 
    w = coord(); final = coord()
    trbeam = False

    def no_quad_change():
        # No quadrant change -- compute new average enemy distances 
        game.quad[game.sector.x][game.sector.y] = game.ship
        if game.nenhere:
            for m in range(game.nenhere):
                finald = distance(w, game.ks[m])
                game.kavgd[m] = 0.5 * (finald+game.kdist[m])
                game.kdist[m] = finald
            sortklings()
            if not game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
                attack(False)
            for m in range(game.nenhere):
                game.kavgd[m] = game.kdist[m]
        newcnd()
        drawmaps(0)
        setwnd(message_window)
    w.x = w.y = 0
    if game.inorbit:
	prout(_("Helmsman Sulu- \"Leaving standard orbit.\""))
	game.inorbit = False
    angle = ((15.0 - game.direc) * 0.5235988)
    deltax = -math.sin(angle)
    deltay = math.cos(angle)
    if math.fabs(deltax) > math.fabs(deltay):
	bigger = math.fabs(deltax)
    else:
	bigger = math.fabs(deltay)
    deltay /= bigger
    deltax /= bigger
    # If tractor beam is to occur, don't move full distance 
    if game.state.date+game.optime >= scheduled(FTBEAM):
	trbeam = True
	game.condition = "red"
	game.dist = game.dist*(scheduled(FTBEAM)-game.state.date)/game.optime + 0.1
	game.optime = scheduled(FTBEAM) - game.state.date + 1e-5
    # Move within the quadrant 
    game.quad[game.sector.x][game.sector.y] = IHDOT
    x = game.sector.x
    y = game.sector.y
    n = int(10.0*game.dist*bigger+0.5)
    if n > 0:
	for m in range(1, n+1):
            x += deltax
            y += deltay
	    w.x = int(round(x))
	    w.y = int(round(y))
	    if not VALID_SECTOR(w.x, w.y):
		# Leaving quadrant -- allow final enemy attack 
		# Don't do it if being pushed by Nova 
		if game.nenhere != 0 and not novapush:
		    newcnd()
		    for m in range(game.nenhere):
			finald = distance(w, game.ks[m])
			game.kavgd[m] = 0.5 * (finald + game.kdist[m])
		    #
		    # Stas Sergeev added the condition
		    # that attacks only happen if Klingons
		    # are present and your skill is good.
		    # 
		    if game.skill > SKILL_GOOD and game.klhere > 0 and not game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
			attack(False)
		    if game.alldone:
			return
		# compute final position -- new quadrant and sector 
		x = (QUADSIZE*game.quadrant.x)+game.sector.x
		y = (QUADSIZE*game.quadrant.y)+game.sector.y
		w.x = int(round(x+10.0*game.dist*bigger*deltax))
		w.y = int(round(y+10.0*game.dist*bigger*deltay))
		# check for edge of galaxy 
		kinks = 0
                while True:
		    kink = False
		    if w.x < 0:
			w.x = -w.x
			kink = True
		    if w.y < 0:
			w.y = -w.y
			kink = True
		    if w.x >= GALSIZE*QUADSIZE:
			w.x = (GALSIZE*QUADSIZE*2) - w.x
			kink = True
		    if w.y >= GALSIZE*QUADSIZE:
			w.y = (GALSIZE*QUADSIZE*2) - w.y
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
		game.quadrant.x = w.x/QUADSIZE
		game.quadrant.y = w.y/QUADSIZE
		game.sector.x = w.x - (QUADSIZE*game.quadrant.x)
		game.sector.y = w.y - (QUADSIZE*game.quadrant.y)
		skip(1)
		prout(_("Entering Quadrant %s.") % game.quadrant)
		game.quad[game.sector.x][game.sector.y] = game.ship
		newqad(False)
		if game.skill>SKILL_NOVICE:
		    attack(False)  
		return
	    iquad = game.quad[w.x][w.y]
	    if iquad != IHDOT:
		# object encountered in flight path 
		stopegy = 50.0*game.dist/game.optime
		game.dist = distance(game.sector, w) / (QUADSIZE * 1.0)
                if iquad in (IHT, IHK, IHC, IHS, IHR, IHQUEST):
		    game.sector = w
		    ram(False, iquad, game.sector)
		    final = game.sector
		elif iquad == IHBLANK:
		    skip(1)
		    prouts(_("***RED ALERT!  RED ALERT!"))
		    skip(1)
		    proutn("***")
		    crmshp()
		    proutn(_(" pulled into black hole at Sector %s") % w)
		    #
		    # Getting pulled into a black hole was certain
		    # death in Almy's original.  Stas Sergeev added a
		    # possibility that you'll get timewarped instead.
		    # 
		    n=0
		    for m in range(NDEVICES):
			if game.damage[m]>0: 
			    n += 1
		    probf=math.pow(1.4,(game.energy+game.shield)/5000.0-1.0)*math.pow(1.3,1.0/(n+1)-1.0)
		    if (game.options & OPTION_BLKHOLE) and withprob(1-probf): 
			timwrp()
		    else: 
			finish(FHOLE)
		    return
		else:
		    # something else 
		    skip(1)
		    crmshp()
		    if iquad == IHWEB:
			proutn(_(" encounters Tholian web at %s;") % w)
		    else:
			proutn(_(" blocked by object at %s;") % w)
		    proutn(_("Emergency stop required "))
		    prout(_("%2d units of energy.") % int(stopegy))
		    game.energy -= stopegy
		    final.x = x-deltax+0.5
		    final.y = y-deltay+0.5
		    game.sector = final
		    if game.energy <= 0:
			finish(FNRG)
			return
                # We're here!
		no_quad_change()
                return
	game.dist = distance(game.sector, w) / (QUADSIZE * 1.0)
	game.sector = w
    final = game.sector
    no_quad_change()
    return

def dock(verbose):
    # dock our ship at a starbase 
    chew()
    if game.condition == "docked" and verbose:
	prout(_("Already docked."))
	return
    if game.inorbit:
	prout(_("You must first leave standard orbit."))
	return
    if not is_valid(game.base) or abs(game.sector.x-game.base.x) > 1 or abs(game.sector.y-game.base.y) > 1:
	crmshp()
	prout(_(" not adjacent to base."))
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
 
# This program originally required input in terms of a (clock)
# direction and distance. Somewhere in history, it was changed to
# cartesian coordinates. So we need to convert.  Probably
# "manual" input should still be done this way -- it's a real
# pain if the computer isn't working! Manual mode is still confusing
# because it involves giving x and y motions, yet the coordinates
# are always displayed y - x, where +y is downward!

def getcourse(isprobe, akey):
    # get course and distance
    key = 0
    dquad = copy.copy(game.quadrant)
    navmode = "unspecified"
    itemp = "curt"
    dsect = coord()
    iprompt = False
    if game.landed and not isprobe:
	prout(_("Dummy! You can't leave standard orbit until you"))
	proutn(_("are back aboard the ship."))
	chew()
	return False
    while navmode == "unspecified":
	if damaged(DNAVSYS):
	    if isprobe:
		prout(_("Computer damaged; manual navigation only"))
	    else:
		prout(_("Computer damaged; manual movement only"))
	    chew()
	    navmode = "manual"
	    key = IHEOL
	    break
	if isprobe and akey != -1:
	    # For probe launch, use pre-scanned value first time 
	    key = akey
	    akey = -1
	else: 
	    key = scan()
	if key == IHEOL:
	    proutn(_("Manual or automatic- "))
	    iprompt = True
	    chew()
	elif key == IHALPHA:
            if isit("manual"):
		navmode = "manual"
		key = scan()
		break
            elif isit("automatic"):
		navmode = "automatic"
		key = scan()
		break
	    else:
		huh()
		chew()
		return False
	else: # numeric 
	    if isprobe:
		prout(_("(Manual navigation assumed.)"))
	    else:
		prout(_("(Manual movement assumed.)"))
	    navmode = "manual"
	    break
    if navmode == "automatic":
	while key == IHEOL:
	    if isprobe:
		proutn(_("Target quadrant or quadrant&sector- "))
	    else:
		proutn(_("Destination sector or quadrant&sector- "))
	    chew()
	    iprompt = True
	    key = scan()
	if key != IHREAL:
	    huh()
	    return False
	xi = int(round(aaitem))-1
	key = scan()
	if key != IHREAL:
	    huh()
	    return False
	xj = int(round(aaitem))-1
	key = scan()
	if key == IHREAL:
	    # both quadrant and sector specified 
	    xk = int(round(aaitem))-1
	    key = scan()
	    if key != IHREAL:
		huh()
		return False
	    xl = int(round(aaitem))-1
	    dquad.x = xi
	    dquad.y = xj
	    dsect.y = xk
	    dsect.x = xl
	else:
            # only one pair of numbers was specified
	    if isprobe:
		# only quadrant specified -- go to center of dest quad 
		dquad.x = xi
		dquad.y = xj
		dsect.y = dsect.x = 4	# preserves 1-origin behavior
	    else:
                # only sector specified
		dsect.y = xi
		dsect.x = xj
	    itemp = "normal"
	if not VALID_QUADRANT(dquad.y,dquad.x) or not VALID_SECTOR(dsect.x,dsect.y):
	    huh()
	    return False
	skip(1)
	if not isprobe:
	    if itemp > "curt":
		if iprompt:
		    prout(_("Helmsman Sulu- \"Course locked in for Sector %s.\"") % dsect)
	    else:
		prout(_("Ensign Chekov- \"Course laid in, Captain.\""))
        # the actual deltas get computed here
	deltax = dquad.y-game.quadrant.y + 0.1*(dsect.x-game.sector.y)
	deltay = game.quadrant.x-dquad.x + 0.1*(game.sector.x-dsect.y)
    else: # manual 
	while key == IHEOL:
	    proutn(_("X and Y displacements- "))
	    chew()
	    iprompt = True
	    key = scan()
	itemp = "verbose"
	if key != IHREAL:
	    huh()
	    return False
	deltax = aaitem
	key = scan()
	if key != IHREAL:
	    huh()
	    return False
	deltay = aaitem
    # Check for zero movement 
    if deltax == 0 and deltay == 0:
	chew()
	return False
    if itemp == "verbose" and not isprobe:
	skip(1)
	prout(_("Helmsman Sulu- \"Aye, Sir.\""))
    # Course actually laid in.
    game.dist = math.sqrt(deltax*deltax + deltay*deltay)
    game.direc = math.atan2(deltax, deltay)*1.90985932
    if game.direc < 0.0:
	game.direc += 12.0
    chew()
    return True

def impulse():
    # move under impulse power 
    game.ididit = False
    if damaged(DIMPULS):
	chew()
	skip(1)
	prout(_("Engineer Scott- \"The impulse engines are damaged, Sir.\""))
	return
    if game.energy > 30.0:
        if not getcourse(isprobe=False, akey=0):
	    return
	power = 20.0 + 100.0*game.dist
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
	chew()
	return
    # Make sure enough time is left for the trip 
    game.optime = game.dist/0.095
    if game.optime >= game.state.remtime:
	prout(_("First Officer Spock- \"Captain, our speed under impulse"))
	prout(_("power is only 0.95 sectors per stardate. Are you sure"))
	proutn(_("we dare spend the time?\" "))
	if ja() == False:
	    return
    # Activate impulse engines and pay the cost 
    imove(novapush=False)
    game.ididit = True
    if game.alldone:
	return
    power = 20.0 + 100.0*game.dist
    game.energy -= power
    game.optime = game.dist/0.095
    if game.energy <= 0:
	finish(FNRG)
    return

def warp(timewarp):
    # move under warp drive 
    blooey = False; twarp = False
    if not timewarp: # Not WARPX entry 
	game.ididit = False
	if game.damage[DWARPEN] > 10.0:
	    chew()
	    skip(1)
	    prout(_("Engineer Scott- \"The impulse engines are damaged, Sir.\""))
	    return
	if damaged(DWARPEN) and game.warpfac > 4.0:
	    chew()
	    skip(1)
	    prout(_("Engineer Scott- \"Sorry, Captain. Until this damage"))
	    prout(_("  is repaired, I can only give you warp 4.\""))
	    return
       	# Read in course and distance 
        if not getcourse(isprobe=False, akey=0):
	    return
	# Make sure starship has enough energy for the trip 
	power = (game.dist+0.05)*game.warpfac*game.warpfac*game.warpfac*(game.shldup+1)
	if power >= game.energy:
	    # Insufficient power for trip 
	    game.ididit = False
	    skip(1)
	    prout(_("Engineering to bridge--"))
	    if not game.shldup or 0.5*power > game.energy:
		iwarp = math.pow((game.energy/(game.dist+0.05)), 0.333333333)
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
	game.optime = 10.0*game.dist/game.wfacsq
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
	prob = game.dist*square(6.0-game.warpfac)/66.666666666
	if prob > randreal():
	    blooey = True
	    game.dist = randreal(game.dist)
	# Decide if time warp will occur 
	if 0.5*game.dist*math.pow(7.0,game.warpfac-10.0) > randreal():
	    twarp = True
	if idebug and game.warpfac==10 and not twarp:
	    blooey = False
	    proutn("=== Force time warp? ")
	    if ja() == True:
		twarp = True
	if blooey or twarp:
	    # If time warp or engine damage, check path 
	    # If it is obstructed, don't do warp or damage 
	    angle = ((15.0-game.direc)*0.5235998)
	    deltax = -math.sin(angle)
	    deltay = math.cos(angle)
	    if math.fabs(deltax) > math.fabs(deltay):
		bigger = math.fabs(deltax)
	    else:
		bigger = math.fabs(deltay)
			
	    deltax /= bigger
	    deltay /= bigger
	    n = 10.0 * game.dist * bigger +0.5
	    x = game.sector.x
	    y = game.sector.y
	    for l in range(1, n+1):
		x += deltax
		ix = x + 0.5
		y += deltay
		iy = y +0.5
		if not VALID_SECTOR(ix, iy):
		    break
		if game.quad[ix][iy] != IHDOT:
		    blooey = False
		    twarp = False
    # Activate Warp Engines and pay the cost 
    imove(novapush=False)
    if game.alldone:
	return
    game.energy -= game.dist*game.warpfac*game.warpfac*game.warpfac*(game.shldup+1)
    if game.energy <= 0:
	finish(FNRG)
    game.optime = 10.0*game.dist/game.wfacsq
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
    # change the warp factor 	
    while True:
        key=scan()
        if key != IHEOL:
            break
	chew()
	proutn(_("Warp factor- "))
    chew()
    if key != IHREAL:
	huh()
	return
    if game.damage[DWARPEN] > 10.0:
	prout(_("Warp engines inoperative."))
	return
    if damaged(DWARPEN) and aaitem > 4.0:
	prout(_("Engineer Scott- \"I'm doing my best, Captain,"))
	prout(_("  but right now we can only go warp 4.\""))
	return
    if aaitem > 10.0:
	prout(_("Helmsman Sulu- \"Our top speed is warp 10, Captain.\""))
	return
    if aaitem < 1.0:
	prout(_("Helmsman Sulu- \"We can't go below warp 1, Captain.\""))
	return
    oldfac = game.warpfac
    game.warpfac = aaitem
    game.wfacsq=game.warpfac*game.warpfac
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
    # cope with being tossed out of quadrant by supernova or yanked by beam 
    chew()
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
	    proutn(_("The "))
	    crmshp()
	    prout(_(" has stopped in a quadrant containing"))
	    prouts(_("   a supernova."))
	    skip(2)
	proutn(_("***Emergency automatic override attempts to hurl "))
	crmshp()
	skip(1)
	prout(_("safely out of quadrant."))
	if not damaged(DRADIO):
	    game.state.galaxy[game.quadrant.x][game.quadrant.y].charted = True
	# Try to use warp engines 
	if damaged(DWARPEN):
	    skip(1)
	    prout(_("Warp engines damaged."))
	    finish(FSNOVAED)
	    return
	game.warpfac = randreal(6.0, 8.0)
	game.wfacsq = game.warpfac * game.warpfac
	prout(_("Warp factor set to %d") % int(game.warpfac))
	power = 0.75*game.energy
	game.dist = power/(game.warpfac*game.warpfac*game.warpfac*(game.shldup+1))
	distreq = randreal(math.sqrt(2))
	if distreq < game.dist:
	    game.dist = distreq
	game.optime = 10.0*game.dist/game.wfacsq
	game.direc = randreal(12)	# How dumb! 
	game.justin = False
	game.inorbit = False
	warp(True)
	if not game.justin:
	    # This is bad news, we didn't leave quadrant. 
	    if game.alldone:
		return
	    skip(1)
	    prout(_("Insufficient energy to leave quadrant."))
	    finish(FSNOVAED)
	    return
	# Repeat if another snova
        if not game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
            break
    if (game.state.remkl + game.state.remcom + game.state.nscrem)==0: 
	finish(FWON) # Snova killed remaining enemy. 

def timwrp():
    # let's do the time warp again 
    prout(_("***TIME WARP ENTERED."))
    if game.state.snap and withprob(0.5):
	# Go back in time 
	prout(_("You are traveling backwards in time %d stardates.") %
	      int(game.state.date-game.snapsht.date))
	game.state = game.snapsht
	game.state.snap = False
	if game.state.remcom:
	    schedule(FTBEAM, expran(game.intime/game.state.remcom))
	    schedule(FBATTAK, expran(0.3*game.intime))
	schedule(FSNOVA, expran(0.5*game.intime))
	# next snapshot will be sooner 
	schedule(FSNAP, expran(0.25*game.state.remtime))
				
	if game.state.nscrem:
	    schedule(FSCMOVE, 0.2777)	    
	game.isatb = 0
	unschedule(FCDBAS)
	unschedule(FSCDBAS)
	invalidate(game.battle)

	# Make sure Galileo is consistant -- Snapshot may have been taken
        # when on planet, which would give us two Galileos! 
	gotit = False
	for l in range(game.inplan):
	    if game.state.planets[l].known == "shuttle_down":
		gotit = True
		if game.iscraft == "onship" and game.ship==IHE:
		    prout(_("Chekov-  \"Security reports the Galileo has disappeared, Sir!"))
		    game.iscraft = "offship"
	# Likewise, if in the original time the Galileo was abandoned, but
	# was on ship earlier, it would have vanished -- let's restore it.
	if game.iscraft == "offship" and not gotit and game.damage[DSHUTTL] >= 0.0:
	    prout(_("Checkov-  \"Security reports the Galileo has reappeared in the dock!\""))
	    game.iscraft = "onship"
	# 
#	 * There used to be code to do the actual reconstrction here,
#	 * but the starchart is now part of the snapshotted galaxy state.
#	 
	prout(_("Spock has reconstructed a correct star chart from memory"))
    else:
	# Go forward in time 
	game.optime = -0.5*game.intime*math.log(randreal())
	prout(_("You are traveling forward in time %d stardates.") % int(game.optime))
	# cheat to make sure no tractor beams occur during time warp 
	postpone(FTBEAM, game.optime)
	game.damage[DRADIO] += game.optime
    newqad(False)
    events()	# Stas Sergeev added this -- do pending events 

def probe():
    # launch deep-space probe 
    # New code to launch a deep space probe 
    if game.nprobes == 0:
	chew()
	skip(1)
	if game.ship == IHE: 
	    prout(_("Engineer Scott- \"We have no more deep space probes, Sir.\""))
	else:
	    prout(_("Ye Faerie Queene has no deep space probes."))
	return
    if damaged(DDSP):
	chew()
	skip(1)
	prout(_("Engineer Scott- \"The probe launcher is damaged, Sir.\""))
	return
    if is_scheduled(FDSPROB):
	chew()
	skip(1)
	if damaged(DRADIO) and game.condition != "docked":
	    prout(_("Spock-  \"Records show the previous probe has not yet"))
	    prout(_("   reached its destination.\""))
	else:
	    prout(_("Uhura- \"The previous probe is still reporting data, Sir.\""))
	return
    key = scan()
    if key == IHEOL:
	# slow mode, so let Kirk know how many probes there are left
        if game.nprobes == 1:
            prout(_("1 probe left."))
        else:
            prout(_("%d probes left") % game.nprobes)
	proutn(_("Are you sure you want to fire a probe? "))
	if ja() == False:
	    return
    game.isarmed = False
    if key == IHALPHA and citem == "armed":
	game.isarmed = True
	key = scan()
    elif key == IHEOL:
	proutn(_("Arm NOVAMAX warhead? "))
	game.isarmed = ja()
    if not getcourse(isprobe=True, akey=key):
	return
    game.nprobes -= 1
    angle = ((15.0 - game.direc) * 0.5235988)
    game.probeinx = -math.sin(angle)
    game.probeiny = math.cos(angle)
    if math.fabs(game.probeinx) > math.fabs(game.probeiny):
	bigger = math.fabs(game.probeinx)
    else:
	bigger = math.fabs(game.probeiny)
    game.probeiny /= bigger
    game.probeinx /= bigger
    game.proben = 10.0*game.dist*bigger +0.5
    game.probex = game.quadrant.x*QUADSIZE + game.sector.x - 1	# We will use better packing than original
    game.probey = game.quadrant.y*QUADSIZE + game.sector.y - 1
    game.probec = game.quadrant
    schedule(FDSPROB, 0.01) # Time to move one sector
    prout(_("Ensign Chekov-  \"The deep space probe is launched, Captain.\""))
    game.ididit = True
    return

# Here's how the mayday code works:
# 
# First, the closest starbase is selected.  If there is a a starbase
# in your own quadrant, you are in good shape.  This distance takes
# quadrant distances into account only.
#
# A magic number is computed based on the distance which acts as the
# probability that you will be rematerialized.  You get three tries.
#
# When it is determined that you should be able to be rematerialized
# (i.e., when the probability thing mentioned above comes up
# positive), you are put into that quadrant (anywhere).  Then, we try
# to see if there is a spot adjacent to the star- base.  If not, you
# can't be rematerialized!!!  Otherwise, it drops you there.  It only
# tries five times to find a spot to drop you.  After that, it's your
# problem.

def mayday():
    # yell for help from nearest starbase 
    # There's more than one way to move in this game! 
    line = 0
    chew()
    # Test for conditions which prevent calling for help 
    if game.condition == "docked":
	prout(_("Lt. Uhura-  \"But Captain, we're already docked.\""))
	return
    if damaged(DRADIO):
	prout(_("Subspace radio damaged."))
	return
    if game.state.rembase==0:
	prout(_("Lt. Uhura-  \"Captain, I'm not getting any response from Starbase.\""))
	return
    if game.landed:
	proutn(_("You must be aboard the "))
	crmshp()
	prout(".")
	return
    # OK -- call for help from nearest starbase 
    game.nhelp += 1
    if game.base.x!=0:
	# There's one in this quadrant 
	ddist = distance(game.base, game.sector)
    else:
	ddist = FOREVER
	for m in range(game.state.rembase):
	    xdist = QUADSIZE * distance(game.state.baseq[m], game.quadrant)
	    if xdist < ddist:
		ddist = xdist
		line = m
	# Since starbase not in quadrant, set up new quadrant 
	game.quadrant = game.state.baseq[line]
	newqad(True)
    # dematerialize starship 
    game.quad[game.sector.x][game.sector.y]=IHDOT
    proutn(_("Starbase in Quadrant %s responds--") % game.quadrant)
    crmshp()
    prout(_(" dematerializes."))
    game.sector.x=0
    for m in range(1, 5+1):
        w = game.base.scatter() 
	if VALID_SECTOR(ix,iy) and game.quad[ix][iy]==IHDOT:
	    # found one -- finish up 
            game.sector = w
	    break
    if not is_valid(game.sector):
	prout(_("You have been lost in space..."))
	finish(FMATERIALIZE)
	return
    # Give starbase three chances to rematerialize starship 
    probf = math.pow((1.0 - math.pow(0.98,ddist)), 0.33333333)
    for m in range(1, 3+1):
	if m == 1: proutn(_("1st"))
	elif m == 2: proutn(_("2nd"))
	elif m == 3: proutn(_("3rd"))
	proutn(_(" attempt to re-materialize "))
	crmshp()
	game.quad[ix][iy]=(IHMATER0,IHMATER1,IHMATER2)[m-1]
	textcolor(RED)
	warble()
	if randreal() > probf:
	    break
	prout(_("fails."))
	curses.delay_output(500)
	textcolor(DEFAULT)
    if m > 3:
	game.quad[ix][iy]=IHQUEST
	game.alive = False
	drawmaps(1)
	setwnd(message_window)
	finish(FMATERIALIZE)
	return
    game.quad[ix][iy]=game.ship
    textcolor(GREEN)
    prout(_("succeeds."))
    textcolor(DEFAULT)
    dock(False)
    skip(1)
    prout(_("Lt. Uhura-  \"Captain, we made it!\""))

# Abandon Ship (the BSD-Trek description)
# 
# The ship is abandoned.  If your current ship is the Faire
# Queene, or if your shuttlecraft is dead, you're out of
# luck.  You need the shuttlecraft in order for the captain
# (that's you!!) to escape.
# 
# Your crew can beam to an inhabited starsystem in the
# quadrant, if there is one and if the transporter is working.
# If there is no inhabited starsystem, or if the transporter
# is out, they are left to die in outer space.
# 
# If there are no starbases left, you are captured by the
# Klingons, who torture you mercilessly.  However, if there
# is at least one starbase, you are returned to the
# Federation in a prisoner of war exchange.  Of course, this
# can't happen unless you have taken some prisoners.

def abandon():
    # abandon ship 
    chew()
    if game.condition=="docked":
	if game.ship!=IHE:
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
	# Print abandon ship messages 
	skip(1)
	prouts(_("***ABANDON SHIP!  ABANDON SHIP!"))
	skip(1)
	prouts(_("***ALL HANDS ABANDON SHIP!"))
	skip(2)
	prout(_("Captain and crew escape in shuttle craft."))
	if game.state.rembase==0:
	    # Oops! no place to go... 
	    finish(FABANDN)
	    return
	q = game.state.galaxy[game.quadrant.x][game.quadrant.y]
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
	nb = randrange(game.state.rembase)
	# Set up quadrant and position FQ adjacient to base 
	if not game.quadrant == game.state.baseq[nb]:
	    game.quadrant = game.state.baseq[nb]
	    game.sector.x = game.sector.y = 5
	    newqad(True)
	while True:
	    # position next to base by trial and error 
	    game.quad[game.sector.x][game.sector.y] = IHDOT
	    for l in range(QUADSIZE):
		game.sector = game.base.scatter()
		if VALID_SECTOR(game.sector.x, game.sector.y) and \
                       game.quad[game.sector.x][game.sector.y] == IHDOT:
                    break
	    if l < QUADSIZE+1:
		break # found a spot 
	    game.sector.x=QUADSIZE/2
	    game.sector.y=QUADSIZE/2
	    newqad(True)
    # Get new commission 
    game.quad[game.sector.x][game.sector.y] = game.ship = IHF
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
    game.wfacsq=25.0
    return

# Code from planets.c begins here.

def consumeTime():
    # abort a lengthy operation if an event interrupts it 
    game.ididit = True
    events()
    if game.alldone or game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova or game.justin: 
	return True
    return False

def survey():
    # report on (uninhabited) planets in the galaxy 
    iknow = False
    skip(1)
    chew()
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
	    proutn(_("Quadrant %s") % game.state.planets[i].w)
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
    # enter standard orbit 
    skip(1)
    chew()
    if game.inorbit:
	prout(_("Already in standard orbit."))
	return
    if damaged(DWARPEN) and damaged(DIMPULS):
	prout(_("Both warp and impulse engines damaged."))
	return
    if not is_valid(game.plnet) or abs(game.sector.x-game.plnet.x) > 1 or abs(game.sector.y-game.plnet.y) > 1:
	crmshp()
	prout(_(" not adjacent to planet."))
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
    # examine planets in this quadrant 
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
    # use the transporter 
    nrgneed = 0
    chew()
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
	crmshp()
	prout(_(" not in standard orbit."))
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
	    chew()
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
		chew()
		return
    if game.landed:
	# Coming from planet 
	if game.iplnet.known=="shuttle_down":
	    proutn(_("Spock-  \"Wouldn't you rather take the Galileo?\" "))
	    if ja() == True:
		chew()
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
    # strip-mine a world for dilithium 
    skip(1)
    chew()
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
	proutn(_("With all those fresh crystals aboard the "))
	crmshp()
	skip(1)
	prout(_("there's no reason to mine more at this time."))
	return
    game.optime = randreal(0.1, 0.3)*(ord(game.iplnet.pclass)-ord("L"))
    if consumeTime():
	return
    prout(_("Mining operation complete."))
    game.iplnet.crystals = "mined"
    game.imine = game.ididit = True

def usecrystals():
    # use dilithium crystals 
    game.ididit = False
    skip(1)
    chew()
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
	chew()
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
    if with(game.cryprob):
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
    # use shuttlecraft for planetary jaunt 
    chew()
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
	crmshp()
	prout(_(" not in standard orbit."))
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
	# Kirk on ship 
	# and so is Galileo 
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
    # use the big zapper 
    game.ididit = False
    skip(1)
    chew()
    if game.ship != IHE:
	prout(_("Ye Faerie Queene has no death ray."))
	return
    if game.nenhere==0:
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
	while game.nenhere > 0:
	    deadkl(game.ks[1], game.quad[game.ks[1].x][game.ks[1].y],game.ks[1])
	prout(_("Ensign Chekov-  \"Congratulations, Captain!\""))
	if (game.state.remkl + game.state.remcom + game.state.nscrem) == 0:
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
		if game.quad[i][j] == IHDOT:
		    game.quad[i][j] = IHQUEST
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
    # report status of bases under attack 
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
    chew()
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
    proutn(_("%d of %d Klingons have been killed") % (((game.inkling + game.incom + game.inscom) - (game.state.remkl + game.state.remcom + game.state.nscrem)), 
	   (game.inkling + game.incom + game.inscom)))
    if game.incom - game.state.remcom:
	prout(_(", including %d Commander%s.") % (game.incom - game.state.remcom, (_("s"), "")[(game.incom - game.state.remcom)==1]))
    elif game.inkling - game.state.remkl + (game.inscom - game.state.nscrem) > 0:
	prout(_(", but no Commanders."))
    else:
	prout(".")
    if game.skill > SKILL_FAIR:
	prout(_("The Super Commander has %sbeen destroyed.") % ("", _("not "))[game.state.nscrem])
    if game.state.rembase != game.inbase:
	proutn(_("There "))
	if game.inbase-game.state.rembase==1:
	    proutn(_("has been 1 base"))
	else:
	    proutn(_("have been %d bases") % (game.inbase-game.state.rembase))
	prout(_(" destroyed, %d remaining.") % game.state.rembase)
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
    if game.ship == IHE:
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
	
def lrscan():
    # long-range sensor scan 
    if damaged(DLRSENS):
	# Now allow base's sensors if docked 
	if game.condition != "docked":
	    prout(_("LONG-RANGE SENSORS DAMAGED."))
	    return
	prout(_("Starbase's long-range scan"))
    else:
	prout(_("Long-range scan"))
    for x in range(game.quadrant.x-1, game.quadrant.x+2):
        proutn(" ")
        for y in range(game.quadrant.y-1, game.quadrant.y+2):
	    if not VALID_QUADRANT(x, y):
		proutn("  -1")
	    else:
		if not damaged(DRADIO):
		    game.state.galaxy[x][y].charted = True
		game.state.chart[x][y].klingons = game.state.galaxy[x][y].klingons
		game.state.chart[x][y].starbase = game.state.galaxy[x][y].starbase
		game.state.chart[x][y].stars = game.state.galaxy[x][y].stars
		if game.state.galaxy[x][y].supernova: 
		    proutn(" ***")
		else:
		    proutn(" %3d" % (game.state.chart[x][y].klingons*100 + game.state.chart[x][y].starbase * 10 + game.state.chart[x][y].stars))
	prout(" ")

def damagereport():
    # damage report 
    jdam = False
    chew()

    for i in range(NDEVICES):
	if damaged(i):
	    if not jdam:
		prout(_("\tDEVICE\t\t\t-REPAIR TIMES-"))
		prout(_("\t\t\tIN FLIGHT\t\tDOCKED"))
		jdam = True
	    prout("  %-26s\t%8.2f\t\t%8.2f" % (device[i],
                                               game.damage[i]+0.05,
                                               game.docfac*game.damage[i]+0.005))
    if not jdam:
	prout(_("All devices functional."))

def rechart():
    # update the chart in the Enterprise's computer from galaxy data 
    game.lastchart = game.state.date
    for i in range(GALSIZE):
	for j in range(GALSIZE):
	    if game.state.galaxy[i][j].charted:
		game.state.chart[i][j].klingons = game.state.galaxy[i][j].klingons
		game.state.chart[i][j].starbase = game.state.galaxy[i][j].starbase
		game.state.chart[i][j].stars = game.state.galaxy[i][j].stars

def chart():
    # display the star chart  
    chew()
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
	    if (game.options & OPTION_SHOWME) and i == game.quadrant.x and j == game.quadrant.y:
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
	    if (game.options & OPTION_SHOWME) and i == game.quadrant.x and j == game.quadrant.y:
		proutn(">")
	    else:
		proutn(" ")
	proutn("  |")
	if i<GALSIZE:
	    skip(1)

def sectscan(goodScan, i, j):
    # light up an individual dot in a sector 
    if goodScan or (abs(i-game.sector.x)<= 1 and abs(j-game.sector.y) <= 1):
	if (game.quad[i][j]==IHMATER0) or (game.quad[i][j]==IHMATER1) or (game.quad[i][j]==IHMATER2) or (game.quad[i][j]==IHE) or (game.quad[i][j]==IHF):
	    if game.condition   == "red": textcolor(RED)
	    elif game.condition == "green": textcolor(GREEN)
	    elif game.condition == "yellow": textcolor(YELLOW)
	    elif game.condition == "docked": textcolor(CYAN)
	    elif game.condition == "dead": textcolor(BROWN)
	    if game.quad[i][j] != game.ship: 
		highvideo()
	proutn("%c " % game.quad[i][j])
	textcolor(DEFAULT)
    else:
	proutn("- ")

def status(req=0):
    # print status report lines 

    if not req or req == 1:
	prstat(_("Stardate"), _("%.1f, Time Left %.2f") \
               % (game.state.date, game.state.remtime))
    if not req or req == 2:
	if game.condition != "docked":
	    newcnd()
        dam = 0
	for t in range(NDEVICES):
	    if game.damage[t]>0: 
		dam += 1
	prstat(_("Condition"), _("%s, %i DAMAGES") % (game.condition.upper(), dam))
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
               % (game.state.remkl + game.state.remcom + game.state.nscrem))
    if not req or req == 10:
	if game.options & OPTION_WORLDS:
	    plnet = game.state.galaxy[game.quadrant.x][game.quadrant.y].planet
	    if plnet and plnet.inhabited:
		prstat(_("Major system"), plnet.name)
	    else:
		prout(_("Sector is uninhabited"))
    elif not req or req == 11:
	attackreport(not req)

def request():
    requests = ("da","co","po","ls","wa","en","to","sh","kl","sy", "ti")
    while scan() == IHEOL:
	proutn(_("Information desired? "))
    chew()
    if citem in requests:
        status(requests.index(citem))
    else:
	prout(_("UNRECOGNIZED REQUEST. Legal requests are:"))
	prout(("  date, condition, position, lsupport, warpfactor,"))
	prout(("  energy, torpedoes, shields, klingons, system, time."))
		
def srscan():
    # short-range scan 
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
	game.state.chart[game.quadrant.x][game.quadrant.y].klingons = game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons
	game.state.chart[game.quadrant.x][game.quadrant.y].starbase = game.state.galaxy[game.quadrant.x][game.quadrant.y].starbase
	game.state.chart[game.quadrant.x][game.quadrant.y].stars = game.state.galaxy[game.quadrant.x][game.quadrant.y].stars
	game.state.galaxy[game.quadrant.x][game.quadrant.y].charted = True
    prout("    1 2 3 4 5 6 7 8 9 10")
    if game.condition != "docked":
	newcnd()
    for i in range(QUADSIZE):
	proutn("%2d  " % (i+1))
	for j in range(QUADSIZE):
	    sectscan(goodScan, i, j)
	skip(1)
			
			
def eta():
    # use computer to get estimated time of arrival for a warp jump 
    w1 = coord(); w2 = coord()
    prompt = False
    if damaged(DCOMPTR):
	prout(_("COMPUTER DAMAGED, USE A POCKET CALCULATOR."))
	skip(1)
	return
    if scan() != IHREAL:
	prompt = True
	chew()
	proutn(_("Destination quadrant and/or sector? "))
	if scan()!=IHREAL:
	    huh()
	    return
    w1.y = int(aaitem-0.5)
    if scan() != IHREAL:
	huh()
	return
    w1.x = int(aaitem-0.5)
    if scan() == IHREAL:
	w2.y = int(aaitem-0.5)
	if scan() != IHREAL:
	    huh()
	    return
	w2.x = int(aaitem-0.5)
    else:
	if game.quadrant.y>w1.x:
	    w2.x = 0
	else:
	    w2.x=QUADSIZE-1
	if game.quadrant.x>w1.y:
	    w2.y = 0
	else:
	    w2.y=QUADSIZE-1
    if not VALID_QUADRANT(w1.x, w1.y) or not VALID_SECTOR(w2.x, w2.y):
	huh()
	return
    game.dist = math.sqrt(square(w1.y-game.quadrant.y+0.1*(w2.y-game.sector.y))+
		square(w1.x-game.quadrant.x+0.1*(w2.x-game.sector.x)))
    wfl = False
    if prompt:
	prout(_("Answer \"no\" if you don't know the value:"))
    while True:
	chew()
	proutn(_("Time or arrival date? "))
	if scan()==IHREAL:
	    ttime = aaitem
	    if ttime > game.state.date:
		ttime -= game.state.date # Actually a star date
            twarp=(math.floor(math.sqrt((10.0*game.dist)/ttime)*10.0)+1.0)/10.0
            if ttime <= 1e-10 or twarp > 10:
		prout(_("We'll never make it, sir."))
		chew()
		return
	    if twarp < 1.0:
		twarp = 1.0
	    break
	chew()
	proutn(_("Warp factor? "))
	if scan()== IHREAL:
	    wfl = True
	    twarp = aaitem
	    if twarp<1.0 or twarp > 10.0:
		huh()
		return
	    break
	prout(_("Captain, certainly you can give me one of these."))
    while True:
	chew()
	ttime = (10.0*game.dist)/square(twarp)
	tpower = game.dist*twarp*twarp*twarp*(game.shldup+1)
	if tpower >= game.energy:
	    prout(_("Insufficient energy, sir."))
	    if not game.shldup or tpower > game.energy*2.0:
		if not wfl:
		    return
		proutn(_("New warp factor to try? "))
		if scan() == IHREAL:
		    wfl = True
		    twarp = aaitem
		    if twarp<1.0 or twarp > 10.0:
			huh()
			return
		    continue
		else:
		    chew()
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
	if scan() == IHREAL:
	    wfl = True
	    twarp = aaitem
	    if twarp<1.0 or twarp > 10.0:
		huh()
		return
	else:
	    chew()
	    skip(1)
	    return
			

# Code from setup.c begins here

def prelim():
    # issue a historically correct banner 
    skip(2)
    prout(_("-SUPER- STAR TREK"))
    skip(1)
# From the FORTRAN original
#    prout(_("Latest update-21 Sept 78"))
#    skip(1)

def freeze(boss):
    # save game 
    if boss:
	citem = "emsave.trk"
    else:
        key = scan()
	if key == IHEOL:
	    proutn(_("File name: "))
	    key = scan()
	if key != IHALPHA:
	    huh()
	    return
	chew()
        if '.' not in citem:
	    citem += ".trk"
    try:
        fp = open(citem, "wb")
    except IOError:
	prout(_("Can't freeze game as file %s") % citem)
	return
    cPickle.dump(game, fp)
    fp.close()

def thaw():
    # retrieve saved game 
    game.passwd[0] = '\0'
    key = scan()
    if key == IHEOL:
	proutn(_("File name: "))
	key = scan()
    if key != IHALPHA:
	huh()
	return True
    chew()
    if '.' not in citem:
        citem += ".trk"
    try:
        fp = open(citem, "rb")
    except IOError:
	prout(_("Can't thaw game in %s") % citem)
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
    _("Argelius II (Nelphia)"),# TOS: "Wolf in the Fold" ("IV" in BSD) 
    _("Ardana"),		# TOS: "The Cloud Minders" 
    _("Catulla (Cendo-Prae)"),	# TOS: "The Way to Eden" 
    _("Gideon"),		# TOS: "The Mark of Gideon" 
    _("Aldebaran III"),	# TOS: "The Deadly Years" 
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

def setup(needprompt):
    # prepare to play, set up cosmos 
    w = coord()
    #  Decide how many of everything
    if choose(needprompt):
	return # frozen game
    # Prepare the Enterprise
    game.alldone = game.gamewon = False
    game.ship = IHE
    game.state.crew = FULLCREW
    game.energy = game.inenrg = 5000.0
    game.shield = game.inshld = 2500.0
    game.shldchg = False
    game.shldup = False
    game.inlsr = 4.0
    game.lsupres = 4.0
    game.quadrant = randplace(GALSIZE)
    game.sector = randplace(QUADSIZE)
    game.torps = game.intorps = 10
    game.nprobes = randrange(2, 5)
    game.warpfac = 5.0
    game.wfacsq = game.warpfac * game.warpfac
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
    game.docfac = 0.25
    for i in range(GALSIZE):
	for j in range(GALSIZE):
	    quad = game.state.galaxy[i][j]
	    quad.charted = 0
	    quad.planet = None
	    quad.romulans = 0
	    quad.klingons = 0
	    quad.starbase = False
	    quad.supernova = False
	    quad.status = "secure"
    # Initialize times for extraneous events
    schedule(FSNOVA, expran(0.5 * game.intime))
    schedule(FTBEAM, expran(1.5 * (game.intime / game.state.remcom)))
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
                if not game.state.galaxy[w.x][w.y].starbase:
                    break
	    contflag = False
            # C version: for (j = i-1; j > 0; j--)
            # so it did them in the opposite order.
            for j in range(1, i):
		# Improved placement algorithm to spread out bases
		distq = w.distance(game.state.baseq[j])
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
	game.state.baseq[i] = w
	game.state.galaxy[w.x][w.y].starbase = True
	game.state.chart[w.x][w.y].starbase = True
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
            if not game.state.galaxy[w.x][w.y].supernova and \
               game.state.galaxy[w.x][w.y].klingons + klump <= MAXKLQUAD:
                break
	game.state.galaxy[w.x][w.y].klingons += int(klump)
        if krem <= 0:
            break
    # Position Klingon Commander Ships
    for i in range(1, game.incom+1):
        while True:
            w = randplace(GALSIZE)
	    if (game.state.galaxy[w.x][w.y].klingons or withprob(0.25)) and \
		   not game.state.galaxy[w.x][w.y].supernova and \
		   game.state.galaxy[w.x][w.y].klingons <= MAXKLQUAD-1 and \
                   not w in game.state.kcmdr[:i]:
                break
	game.state.galaxy[w.x][w.y].klingons += 1
	game.state.kcmdr[i] = w
    # Locate planets in galaxy
    for i in range(game.inplan):
        while True:
            w = randplace(GALSIZE) 
            if game.state.galaxy[w.x][w.y].planet == None:
                break
        new = planet()
	new.w = w
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
	game.state.galaxy[w.x][w.y].planet = new
        game.state.planets.append(new)
    # Locate Romulans
    for i in range(game.state.nromrem):
	w = randplace(GALSIZE)
	game.state.galaxy[w.x][w.y].romulans += 1
    # Locate the Super Commander
    if game.state.nscrem > 0:
        while True:
            w = randplace(GALSIZE)
            if not game.state.galaxy[w.x][w.y].supernova and game.state.galaxy[w.x][w.y].klingons <= MAXKLQUAD:
                break
	game.state.kscmdr = w
	game.state.galaxy[w.x][w.y].klingons += 1
    # Place thing (in tournament game, thingx == -1, don't want one!)
    global thing
    if thing == None:
	thing = randplace(GALSIZE)
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
    newqad(False)
    if game.nenhere - (thing == game.quadrant) - (game.tholian != None):
	game.shldup = True
    if game.neutz:	# bad luck to start in a Romulan Neutral Zone
	attack(False)

def choose(needprompt):
    # choose your game type
    global thing
    while True:
	game.tourn = 0
	game.thawed = False
	game.skill = SKILL_NONE
	game.length = 0
	if needprompt: # Can start with command line options 
	    proutn(_("Would you like a regular, tournament, or saved game? "))
	scan()
	if len(citem)==0: # Try again
	    continue
        if isit("tournament"):
	    while scan() == IHEOL:
		proutn(_("Type in tournament number-"))
	    if aaitem == 0:
		chew()
		continue # We don't want a blank entry
	    game.tourn = int(round(aaitem))
	    random.seed(aaitem)
            if logfp:
                logfp.write("# random.seed(%d)\n" % aaitem)
	    break
        if isit("saved") or isit("frozen"):
	    if thaw():
		continue
	    chew()
	    if game.passwd == None:
		continue
	    if not game.alldone:
		game.thawed = True # No plaque if not finished
	    report()
	    waitfor()
	    return True
        if isit("regular"):
	    break
	proutn(_("What is \"%s\"?"), citem)
	chew()
    while game.length==0 or game.skill==SKILL_NONE:
	if scan() == IHALPHA:
            if isit("short"):
		game.length = 1
	    elif isit("medium"):
		game.length = 2
	    elif isit("long"):
		game.length = 4
	    elif isit("novice"):
		game.skill = SKILL_NOVICE
	    elif isit("fair"):
		game.skill = SKILL_FAIR
	    elif isit("good"):
		game.skill = SKILL_GOOD
	    elif isit("expert"):
		game.skill = SKILL_EXPERT
	    elif isit("emeritus"):
		game.skill = SKILL_EMERITUS
	    else:
		proutn(_("What is \""))
		proutn(citem)
		prout("\"?")
	else:
	    chew()
	    if game.length==0:
		proutn(_("Would you like a Short, Medium, or Long game? "))
	    elif game.skill == SKILL_NONE:
		proutn(_("Are you a Novice, Fair, Good, Expert, or Emeritus player? "))
    # Choose game options -- added by ESR for SST2K
    if scan() != IHALPHA:
	chew()
	proutn(_("Choose your game style (or just press enter): "))
	scan()
    if isit("plain"):
	# Approximates the UT FORTRAN version.
	game.options &=~ (OPTION_THOLIAN | OPTION_PLANETS | OPTION_THINGY | OPTION_PROBE | OPTION_RAMMING | OPTION_MVBADDY | OPTION_BLKHOLE | OPTION_BASE | OPTION_WORLDS)
	game.options |= OPTION_PLAIN
    elif isit("almy"):
	# Approximates Tom Almy's version.
	game.options &=~ (OPTION_THINGY | OPTION_BLKHOLE | OPTION_BASE | OPTION_WORLDS)
	game.options |= OPTION_ALMY
    elif isit("fancy"):
	pass
    elif len(citem):
        proutn(_("What is \"%s\"?") % citem)
    setpassword()
    if game.passwd == "debug":
	idebug = True
	fputs("=== Debug mode enabled\n", sys.stdout)

    # Use parameters to generate initial values of things
    game.damfac = 0.5 * game.skill
    game.state.rembase = randrange(BASEMIN, BASEMAX+1)
    game.inbase = game.state.rembase
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
    game.incom = int(game.skill + 0.0625*game.inkling*randreal())
    game.state.remcom = min(10, game.incom)
    game.incom = game.state.remcom
    game.state.remres = (game.inkling+4*game.incom)*game.intime
    game.inresor = game.state.remres
    if game.inkling > 50:
        game.state.rembase += 1
	game.inbase = game.state.rembase
    return False

def dropin(iquad):
    # drop a feature on a random dot in the current quadrant 
    w = coord()
    while True:
        w = randplace(QUADSIZE)
        if game.quad[w.x][w.y] == IHDOT:
            break
    game.quad[w.x][w.y] = iquad
    return w

def newcnd():
    # update our alert status 
    game.condition = "green"
    if game.energy < 1000.0:
	game.condition = "yellow"
    if game.state.galaxy[game.quadrant.x][game.quadrant.y].klingons or game.state.galaxy[game.quadrant.x][game.quadrant.y].romulans:
	game.condition = "red"
    if not game.alive:
	game.condition="dead"

def newkling(i):
    # drop new Klingon into current quadrant 
    pi = dropin(IHK)
    game.ks[i] = pi
    game.kdist[i] = game.kavgd[i] = distance(game.sector, pi)
    game.kpower[i] = randreal(300, 450) + 25.0*game.skill
    return pi

def newqad(shutup):
    # set up a new state of quadrant, for when we enter or re-enter it 
    w = coord()
    game.justin = True
    game.klhere = 0
    game.comhere = False
    game.ishere = False
    game.irhere = 0
    game.iplnet = 0
    game.nenhere = 0
    game.neutz = False
    game.inorbit = False
    game.landed = False
    game.ientesc = False
    global iqengry
    iqengry = False
    game.iseenit = False
    if game.iscate:
	# Attempt to escape Super-commander, so tbeam back!
	game.iscate = False
	game.ientesc = True
    q = game.state.galaxy[game.quadrant.x][game.quadrant.y]
    # cope with supernova
    if q.supernova:
	return
    game.klhere = q.klingons
    game.irhere = q.romulans
    game.nenhere = game.klhere + game.irhere
    # Position Starship
    game.quad[game.sector.x][game.sector.y] = game.ship
    if q.klingons:
	w.x = w.y = 0	# quiet a gcc warning 
	# Position ordinary Klingons
	for i in range(game.klhere):
	    w = newkling(i)
	# If we need a commander, promote a Klingon
	for i in range(game.state.remcom):
	    if game.state.kcmdr[i] == game.quadrant:
		break
			
	if i <= game.state.remcom:
	    game.quad[w.x][w.y] = IHC
	    game.kpower[game.klhere] = randreal(950, 1350) + 50.0*game.skill
	    game.comhere = True
	# If we need a super-commander, promote a Klingon
	if game.quadrant == game.state.kscmdr:
	    game.quad[game.ks[0].x][game.ks[0].y] = IHS
	    game.kpower[0] = randreal(1175.0,  1575.0) + 125.0*game.skill
	    game.iscate = (game.state.remkl > 1)
	    game.ishere = True
    # Put in Romulans if needed
    for i in range(game.klhere, game.nenhere):
	w = dropin(IHR)
	game.ks[i] = w
	game.kdist[i] = game.kavgd[i] = distance(game.sector, w)
	game.kpower[i] = randreal(400.0, 850.0) + 50.0*game.skill
    # If quadrant needs a starbase, put it in
    if q.starbase:
	game.base = dropin(IHB)
    # If quadrant needs a planet, put it in
    if q.planet:
	game.iplnet = q.planet
	if not q.planet.inhabited:
	    game.plnet = dropin(IHP)
	else:
	    game.plnet = dropin(IHW)
    # Check for condition
    newcnd()
    # And finally the stars
    for i in range(q.stars): 
	dropin(IHSTAR)

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
    if shutup==0:
	# Put in THING if needed
        global thing
	if thing == game.quadrant:
	    w = dropin(IHQUEST)
	    thing = randplace(GALSIZE)
	    game.nenhere += 1
	    game.ks[game.nenhere] = w
	    game.kdist[game.nenhere] = game.kavgd[game.nenhere] = \
		distance(game.sector, w)
	    game.kpower[game.nenhere] = randreal(6000,6500.0)+250.0*game.skill
	    if not damaged(DSRSENS):
		skip(1)
		prout(_("Mr. Spock- \"Captain, this is most unusual."))
		prout(_("    Please examine your short-range scan.\""))
    # Decide if quadrant needs a Tholian; lighten up if skill is low 
    if game.options & OPTION_THOLIAN:
	if (game.skill < SKILL_GOOD and withprob(0.02)) or \
	    (game.skill == SKILL_GOOD and withprob(0.05)) or \
            (game.skill > SKILL_GOOD and withprob(0.08)):
            game.tholian = coord()
            while True:
		game.tholian.x = withprob(0.5) * (QUADSIZE-1)
		game.tholian.y = withprob(0.5) * (QUADSIZE-1)
                if game.quad[game.tholian.x][game.tholian.y] == IHDOT:
                    break
	    game.quad[game.tholian.x][game.tholian.y] = IHT
	    game.nenhere += 1
	    game.ks[game.nenhere] = game.tholian
	    game.kdist[game.nenhere] = game.kavgd[game.nenhere] = \
		distance(game.sector, game.tholian)
	    game.kpower[game.nenhere] = randrange(100, 500) + 25.0*game.skill
	    # Reserve unoccupied corners 
	    if game.quad[0][0]==IHDOT:
		game.quad[0][0] = 'X'
	    if game.quad[0][QUADSIZE-1]==IHDOT:
		game.quad[0][QUADSIZE-1] = 'X'
	    if game.quad[QUADSIZE-1][0]==IHDOT:
		game.quad[QUADSIZE-1][0] = 'X'
	    if game.quad[QUADSIZE-1][QUADSIZE-1]==IHDOT:
		game.quad[QUADSIZE-1][QUADSIZE-1] = 'X'
    sortklings()
    # Put in a few black holes
    for i in range(1, 3+1):
	if withprob(0.5): 
	    dropin(IHBLANK)
    # Take out X's in corners if Tholian present
    if game.tholian:
	if game.quad[0][0]=='X':
	    game.quad[0][0] = IHDOT
	if game.quad[0][QUADSIZE-1]=='X':
	    game.quad[0][QUADSIZE-1] = IHDOT
	if game.quad[QUADSIZE-1][0]=='X':
	    game.quad[QUADSIZE-1][0] = IHDOT
	if game.quad[QUADSIZE-1][QUADSIZE-1]=='X':
	    game.quad[QUADSIZE-1][QUADSIZE-1] = IHDOT

def sortklings():
    # sort Klingons by distance from us 
    # The author liked bubble sort. So we will use it. :-(
    if game.nenhere-(thing==game.quadrant)-(game.tholian!=None) < 2:
	return
    while True:
	sw = False
	for j in range(game.nenhere):
	    if game.kdist[j] > game.kdist[j+1]:
		sw = True
		t = game.kdist[j]
		game.kdist[j] = game.kdist[j+1]
		game.kdist[j+1] = t
		t = game.kavgd[j]
		game.kavgd[j] = game.kavgd[j+1]
		game.kavgd[j+1] = t
		k = game.ks[j].x
		game.ks[j].x = game.ks[j+1].x
		game.ks[j+1].x = k
		k = game.ks[j].y
		game.ks[j].y = game.ks[j+1].y
		game.ks[j+1].y = k
		t = game.kpower[j]
		game.kpower[j] = game.kpower[j+1]
		game.kpower[j+1] = t
        if not sw:
            break

def setpassword():
    # set the self-destruct password 
    if game.options & OPTION_PLAIN:
	while True:
	    chew()
	    proutn(_("Please type in a secret password- "))
	    scan()
	    game.passwd = citem
	    if game.passwd != None:
		break
    else:
        game.passwd = ""
        for i in range(3):
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
    "SEED":		0,
    "VISUAL":		0,
}

def ACCEPT(cmd):	return (not commands[cmd] or (commands[cmd] & game.options))

def listCommands():
    # generate a list of legal commands 
    k = 0
    proutn(_("LEGAL COMMANDS ARE:"))
    for key in commands:
	if ACCEPT(key):
            if k % 5 == 0:
                skip(1)
            proutn("%-12s " % key) 
            k += 1
    skip(1)

def helpme():
    # browse on-line help 
    # Give help on commands 
    key = scan()
    while True:
	if key == IHEOL:
	    setwnd(prompt_window)
	    proutn(_("Help on what command? "))
	    key = scan()
	setwnd(message_window)
	if key == IHEOL:
	    return
        if citem in commands or citem == "ABBREV":
	    break
	skip(1)
	listCommands()
	key = IHEOL
	chew()
	skip(1)
    cmd = citem.upper()
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
            #
            # This used to continue: "You need to find SST.DOC and put 
            # it in the current directory."
            # 
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
    # command-interpretation loop 
    v = 0
    clrscr()
    setwnd(message_window)
    while True: 	# command loop 
	drawmaps(1)
        while True:	# get a command 
	    hitme = False
	    game.justin = False
	    game.optime = 0.0
	    chew()
	    setwnd(prompt_window)
	    clrscr()
	    proutn("COMMAND> ")
	    if scan() == IHEOL:
		if game.options & OPTION_CURSES:
		    makechart()
		continue
	    game.ididit = False
	    clrscr()
	    setwnd(message_window)
	    clrscr()
            candidates = filter(lambda x: x.startswith(citem.upper()),
                                commands)
            if len(candidates) == 1:
                cmd = candidates[0]
                break
            elif candidates and not (game.options & OPTION_PLAIN):
                prout("Commands with that prefix: " + " ".join(candidates))
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
	    lrscan()
	elif cmd == "PHASERS":		# phasers
	    phasers()
	    if game.ididit:
		hitme = True
	elif cmd == "TORPEDO":		# photon torpedos
	    photon()
	    if game.ididit:
		hitme = True
	elif cmd == "MOVE":		# move under warp
	    warp(False)
	elif cmd == "SHIELDS":		# shields
	    doshield(shraise=False)
	    if game.ididit:
		hitme = True
		game.shldchg = False
	elif cmd == "DOCK":		# dock at starbase
	    dock(True)
	    if game.ididit:
		attack(False)		
	elif cmd == "DAMAGES":		# damage reports
	    damagereport()
	elif cmd == "CHART":		# chart
	    makechart()
	elif cmd == "IMPULSE":		# impulse
	    impulse()
	elif cmd == "REST":		# rest
	    os.wait()
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
	    if game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
		atover(False)
		continue
	    if hitme and not game.justin:
		attack(True)
		if game.alldone:
		    break
		if game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova:
		    atover(False)
		    hitme = True
		    continue
	    break
	if game.alldone:
	    break
    if idebug:
	prout("=== Ending")

def cramen(cmd):
    # return an enemy 
    if   cmd == IHR: s = _("Romulan")
    elif cmd == IHK: s = _("Klingon")
    elif cmd == IHC: s = _("Commander")
    elif cmd == IHS: s = _("Super-commander")
    elif cmd == IHSTAR: s = _("Star")
    elif cmd == IHP: s = _("Planet")
    elif cmd == IHB: s = _("Starbase")
    elif cmd == IHBLANK: s = _("Black hole")
    elif cmd == IHT: s = _("Tholian")
    elif cmd == IHWEB: s = _("Tholian web")
    elif cmd == IHQUEST: s = _("Stranger")
    elif cmd == IHW: s = _("Inhabited World")
    else: s = "Unknown??"
    proutn(s)

def crmena(stars, enemy, loctype, w):
    # print an enemy and his location 
    if stars:
	proutn("***")
    cramen(enemy)
    proutn(_(" at "))
    buf = ""
    if loctype == "quadrant":
	buf = _("Quadrant ")
    elif loctype == "sector":
	buf = _("Sector ")
    proutn(buf + `w`)

def crmshp():
    # print our ship name 
    if game.ship == IHE:
        s = _("Enterprise")
    elif game.ship == IHF:
        s = _("Faerie Queene")
    else:
        s = "Ship???"
    proutn(s)

def stars():
    # print a line of stars 
    prouts("******************************************************")
    skip(1)

def expran(avrage):
    return -avrage*math.log(1e-7 + randreal())

def randplace(size):
    # choose a random location  
    w = coord()
    w.x = randrange(size) 
    w.y = randrange(size)
    return w

def chew():
    # Demand input for next scan
    global inqueue
    inqueue = None

def chew2():
    # return IHEOL next time 
    global inqueue
    inqueue = []

def scan():
    # Get a token from the user
    global inqueue, line, citem, aaitem
    aaitem = 0.0
    citem = ''

    # Read a line if nothing here
    if inqueue == None:
	line = cgetline()
	if curwnd==prompt_window:
	    clrscr()
	    setwnd(message_window)
	    clrscr()
        # Skip leading white space
        line = line.lstrip()
        if line:
            inqueue = line.split()
        else:
            inqueue = []
            return IHEOL
    elif not inqueue:
        return IHEOL
    # From here on in it's all looking at the queue
    citem = inqueue.pop(0)
    if citem == IHEOL:
        return IHEOL
    try:
        aaitem = float(citem)
        return IHREAL
    except ValueError:
        pass
    # Treat as alpha
    citem = citem.lower()
    return IHALPHA

def ja():
    # yes-or-no confirmation 
    chew()
    while True:
	scan()
	chew()
	if citem == 'y':
	    return True
	if citem == 'n':
	    return False
	proutn(_("Please answer with \"y\" or \"n\": "))

def huh():
    # complain about unparseable input 
    chew()
    skip(1)
    prout(_("Beg your pardon, Captain?"))

def isit(s):
    # compares s to citem and returns true if it matches to the length of s
    return s.startswith(citem)

def debugme():
    # access to the internals for debugging 
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
	    proutn("Kill ")
	    proutn(device[i])
	    proutn("? ")
	    chew()
	    key = scan()
            if key == IHALPHA and isit("y"):
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
	    chew()
	    key = scan()
	    if key == 'n':
		unschedule(i)
		chew()
	    elif key == IHREAL:
		ev = schedule(i, aaitem)
		if i == FENSLV or i == FREPRO:
		    chew()
		    proutn("In quadrant- ")
		    key = scan()
		    # IHEOL says to leave coordinates as they are 
		    if key != IHEOL:
			if key != IHREAL:
			    prout("Event %d canceled, no x coordinate." % (i))
			    unschedule(i)
			    continue
			w.x = int(round(aaitem))
			key = scan()
			if key != IHREAL:
			    prout("Event %d canceled, no y coordinate." % (i))
			    unschedule(i)
			    continue
			w.y = int(round(aaitem))
			ev.quadrant = w
	chew()
    proutn("Induce supernova here? ")
    if ja() == True:
	game.state.galaxy[game.quadrant.x][game.quadrant.y].supernova = True
	atover(True)

if __name__ == '__main__':
    global line, thing, game, idebug, iqengry
    game = citem = aaitem = inqueue = None
    line = ''
    thing = coord()
    iqengry = False
    game = gamestate()
    idebug = 0
    game.options = OPTION_ALL &~ (OPTION_IOMODES | OPTION_SHOWME | OPTION_PLAIN | OPTION_ALMY)
    # Disable curses mode until the game logic is working.
    #    if os.getenv("TERM"):
    #	game.options |= OPTION_CURSES | OPTION_SHOWME
    #    else:
    game.options |= OPTION_TTY
    seed = int(time.time())
    (options, arguments) = getopt.getopt(sys.argv[1:], "r:tx")
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
		os.exit(1)
	    game.options |= OPTION_TTY
	    game.options &=~ OPTION_CURSES
	elif switch == '-t':
	    game.options |= OPTION_TTY
	    game.options &=~ OPTION_CURSES
	elif switch == '-x':
	    idebug = True
	else:
	    sys.stderr.write("usage: sst [-t] [-x] [startcommand...].\n")
	    os.exit(0)
    # where to save the input in case of bugs
    try:
        logfp = open("/usr/tmp/sst-input.log", "w")
    except IOError:
        sys.stderr.write("sst: warning, can't open logfile\n")
    if logfp:
	logfp.write("# seed %s\n" % seed)
	logfp.write("# options %s\n" % " ".join(arguments))
    random.seed(seed)
    iostart()
    if arguments:
        inqueue = arguments
    else:
        inqueue = None
    while True: # Play a game 
	setwnd(fullscreen_window)
	clrscr()
	prelim()
	setup(needprompt=not inqueue)
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
		chew2()
		freeze(False)
        chew()
	proutn(_("Do you want to play again? "))
	if not ja():
	    break
    skip(1)
    prout(_("May the Great Bird of the Galaxy roost upon your home planet."))
    raise SystemExit, 0
