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

