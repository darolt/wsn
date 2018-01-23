import numpy as np
import logging
from time import time

import config as cf
from python.utils.grid import *
from python.utils.regions_converter import *
from python.utils.utils import *
from cc.genetic_algorithm import *
from cc.pso import *
from cc.modified_pso import *
from cc.ecca import *
from multiprocessing.dummy import Pool as ThreadPool

"""Wraps the C++ instance that executes the PSO and also calculates
all coverage information.
"""
class SleepScheduler(object):

  def __init__(self, cluster, optimizer_class):
    # need to update neighbors through this method, so grid can be
    # generated faster
    cluster.update_neighbors()
    self._cluster = cluster
    
    grid = Grid()
    for node in cluster.get_sensor_nodes():
      grid.add_node(node, cf.COVERAGE_RADIUS)
    regions_converter = RegionsConverter(grid)
    exclusive_regions, overlapping_regions = regions_converter.convert()

    config_int =   {'NB_INDIVIDUALS': cf.NB_INDIVIDUALS,
                    'MAX_ITERATIONS': cf.MAX_ITERATIONS}
    config_float = {'FITNESS_ALPHA' : cf.FITNESS_ALPHA,
                    'FITNESS_BETA'  : cf.FITNESS_BETA,
                    'FITNESS_GAMMA' : cf.FITNESS_GAMMA,
                    'WMAX'          : cf.WMAX,
                    'WMIN'          : cf.WMIN}

    configuration = (config_int, config_float)
    ids = [node.id for node in cluster.get_sensor_nodes()] 
    
    self._optimizer = optimizer_class(exclusive_regions, overlapping_regions,
                                ids, configuration)

  def schedule(self):
    """Runs PSO to decide which nodes in the cluster will sleep. The cur-
    rent cluster head should not be put to sleep, otherwise all informa-
    tion for that node is lost.
    """
    # when a single node (CH) is alive you must keep it awake
    if (self._cluster.count_alive_nodes() <= 1):
      return {}
    membership = self._cluster[0].membership
    logging.debug("running sleep scheduling for cluster %d" % (membership))
    # no need to run sleep scheduling if all nodes are dead
  
    # calculate sleep probability for each node
    self._cluster.update_sleep_prob()
    sensor_nodes = self._cluster.get_sensor_nodes()
    node_ids = [node.id for node in sensor_nodes]
    energies = [node.energy_source.energy for node in sensor_nodes]
    #head_id  = (self._cluster.get_heads())[0].id

    best_configuration = self._optimizer.Run(energies)
    best_coverage      = self._optimizer.GetBestCoverage()
    best_overlapping   = self._optimizer.GetBestOverlapping()
    learning_trace     = self._optimizer.GetLearningTrace()
    term1_trace        = self._optimizer.GetTerm1Trace()
    term2_trace        = self._optimizer.GetTerm2Trace()

    #print("best cov: %f, best over: %f" %(best_coverage, best_overlapping))
    #print("init: %f, final: %f" %(learning_trace[0], learning_trace[-1]))
    #print(sum(ord(x) for x in best_configuration))
    
    #plot_curves({'scenario': learning_trace})
    #logging.info('search finished.')
    #print(self._best_configuration)
    # actually put nodes to sleep
    nb_alive = len(self._cluster.get_alive_nodes())
    nb_sleeping = sum(ord(y) for x, y in zip(self._cluster, best_configuration) if x.alive)
    sleeping_rate = float(nb_sleeping)/float(nb_alive)
    #print("coverage %f, active rate %f" %(best_coverage, 1-sleeping_rate))
    log = {}
    log['coverage']        = best_coverage
    log['overlapping']     = best_overlapping
    log['nb_sleeping']     = sleeping_rate 
    log['initial_fitness'] = learning_trace[0]
    log['final_fitness']   = learning_trace[-1]
    log['term1_initial']   = term1_trace[0]
    log['term1_final']     = term1_trace[-1]
    log['term2_initial']   = term2_trace[0]
    log['term2_final']     = term2_trace[-1]

    #print("sleeping nodes %d out of %d" %(nb_sleeping_nodes, len(best_configuration)))
    #print([x.id for x in self._cluster if x.alive])

    # set cluster's nodes to sleep accordingly to optimization algorithm
    for idx, node in enumerate(sensor_nodes):
      node.is_sleeping = ord(best_configuration[idx])

    return log

