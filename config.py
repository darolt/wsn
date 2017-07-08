import math

# number of nodes
NB_NODES = 300

MAX_ROUNDS = 15000
# area definition
AREA_WIDTH = 250.0
AREA_LENGTH = 250.0
# base station position
BS_POS_X = 125.0
BS_POS_Y = -75.0
# packet configs
MSG_LENGTH = 4000
HEADER_LENGTH = 150
# number of transmissions of sensed information to cluster heads or to
# base station (per round)
MAX_TX_PER_ROUND = 1

# ENERGY MODEL
# energy dissipated at the transceiver electronic (/bit)
E_ELEC = 50e-9 # Joules
# energy dissipated at the data aggregation (/bit)
E_DA = 5e-9 # Joules
# energy dissipated at the power amplifier (supposing a multi-path
# fading channel) (/bin/m^4)
E_MP = 0.0013e-12 # Joules
# energy dissipated at the power amplifier (supposing a line-of-sight
# free-space channel (/bin/m^2)
E_FS = 10e-12 # Joules
THRESHOLD_DIST = math.sqrt(E_FS/E_MP) # meters
BSID = -1
LEACH_NB_CLUSTERS = 5
# initial energy at every node's battery
INITIAL_ENERGY = 2 # Joules
# fuzzyness coeficient
FUZZY_M = 2

# useful constants (for readability)
INFINITY = float('inf')
MINUS_INFINITY = float('-inf')

# runtime configuration
RUN_DC    = 0
RUN_MTE   = 1
RUN_LEACH = 1
RUN_FCM   = 1
RUN_PSO   = 0

# PSO sleep scheduling configuration
PSO_E = 0.5
PSO_F = 0.5
PSO_NB_PARTICLES   = 30
COVERAGE_RADIUS    = 20 # meters 
# ALPHA, BETA and GAMMA are the fitness function' weights
# where ALPHA optimizes energy lifetime, BETA the coverage and GAMMA reduces the overlapping
PSO_FITNESS_ALPHA  = 1
PSO_FITNESS_BETA   = 0
PSO_FITNESS_GAMMA  = 0
PSO_MAX_ITERATIONS = 50
PSO_WMAX = 0.9
PSO_WMIN = 0.4

# GRID
GRID_PRECISION = 0.1 # in meters
