import math

## Runtime configuration
MAX_ROUNDS = 15000
# number of transmissions of sensed information to cluster heads or to
# base station (per round)
MAX_TX_PER_ROUND = 1

NOTIFY_POSITION = 0

# Describe the scenarios that will be simulated
# scenarios should be described in the following format:
# scenario_name = (routing_topology, sleep_scheduling)
# where routing_topology may be:
#   'DC'   : Direct Communication
#   'MTE'  : Minimum Transmission Energy
#   'LEACH': LEACH
#   'FCM  ': Fuzzy C-Means
# and sleep_scheduling may be:
#   None                 : No sleep scheduling (mind that None is not a string)
#   'Pso'                : Particle Swarm Optimization
#   'ModifiedPso'        : Modified PSO
#   'GeneticAlgorithm'   : Genetic Algorithm
# and aggregation_model may be:
#   'zero'  : Zero cost
#   'total' : 100% cost
#   'linear': TODO spec
#   'log'   : log cost

scenario_0 = ('DC',    None,         'zero')
scenario_1 = ('LEACH', None,         'zero')
scenario_2 = ('MTE',   None,         'total')
scenario_3 = ('FCM',   None,         'zero')
scenario_4 = ('FCM',  'ModifiedPso', 'zero')
scenario_5 = ('FCM',  'Pso',         'zero')
# list with all scenarios to simulate
scenarios = [
              #scenario_0,
              #scenario_1,
              #scenario_2,
              #scenario_3,
              "cf.FITNESS_ALPHA=0.5",
              "cf.FITNESS_BETA=0.5",
              scenario_4,
              scenario_5,
              "cf.FITNESS_ALPHA=1.0",
              "cf.FITNESS_BETA=0.0",
              scenario_4,
              scenario_5,
              "cf.FITNESS_ALPHA=0.0",
              "cf.FITNESS_BETA=1.0",
              scenario_4,
              scenario_5
            ]

## tracer options
TRACE_ENERGY         = 0
TRACE_ALIVE_NODES    = 1
TRACE_COVERAGE       = 1
TRACE_LEARNING_CURVE = 0


## Network configurations:
# number of nodes
NB_NODES = 300
# node sensor range
COVERAGE_RADIUS    = 10 # meters 
# node transmission range
TX_RANGE = 30 # meters
BSID = -1
# area definition
AREA_WIDTH = 250.0
AREA_LENGTH = 250.0
# base station position
BS_POS_X = 125.0
BS_POS_Y = 125.0
# packet configs
MSG_LENGTH = 4000 # bits
HEADER_LENGTH = 150 # bits
# initial energy at every node's battery
INITIAL_ENERGY = 2 # Joules


## Energy Configurations
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


## Routing configurations:
NB_CLUSTERS = 5
# FCM fuzzyness coeficient
FUZZY_M = 2


## Sleep Scheduling configurations:
NB_INDIVIDUALS   = 20
MAX_ITERATIONS = 100
# ALPHA and BETA are the fitness function' weights
# where ALPHA optimizes energy lifetime, BETA the coverage
FITNESS_ALPHA  = 0.5
FITNESS_BETA   = 0.5
WMAX = 0.6
WMIN = 0.1


## Other configurations:
# GRID
GRID_PRECISION = 0.1 # in meters
# useful constants (for readability)
INFINITY = float('inf')
MINUS_INFINITY = float('-inf')

