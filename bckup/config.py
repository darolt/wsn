import math

# number of nodes
NB_NODES = 300
# area definition
AREA_WIDTH = 250.0
AREA_LENGTH = 250.0
# base station position
BS_POS_X = 125.0
BS_POS_Y = -75.0
MSG_LENGTH = 4000
HEADER_LENGTH = 150
MAX_ROUNDS = 10000
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

RUN_DC    = 1
RUN_MTE   = 1
RUN_LEACH = 1
RUN_FCM   = 1
