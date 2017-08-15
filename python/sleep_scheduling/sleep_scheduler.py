import numpy as np
import logging
from time import time

import config as cf
from python.utils.grid import *
from python.utils.regions_converter import *
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
    cluster.update_sleep_prob()
    self._cluster = cluster
    
    grid = Grid()
    for node in cluster.get_sensor_nodes():
      grid.add_node(node, cf.COVERAGE_RADIUS)
    regions_converter = RegionsConverter(grid)
    regions_buff = regions_converter.convert()    

    ids = [node.id for node in cluster.get_sensor_nodes()] 
    config_int = {}
    config_int['NB_INDIVIDUALS'] = cf.NB_INDIVIDUALS
    config_int['MAX_ITERATIONS'] = cf.MAX_ITERATIONS
    config_float = {}
    config_float['FITNESS_ALPHA'] = cf.FITNESS_ALPHA
    config_float['FITNESS_BETA'] = cf.FITNESS_BETA
    config_float['WMAX'] = cf.WMAX
    config_float['WMIN'] = cf.WMIN

    configuration = (config_int, config_float)
    
    self._pso = optimizer_class(regions_buff[0], regions_buff[1],
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
    head_id  = (self._cluster.get_heads())[0].id

    best_configuration = self._pso.Run(energies, head_id)
    learning_trace     = self._pso.GetLearningTrace()
    best_coverage      = self._pso.GetBestCoverage()
    best_overlapping   = self._pso.GetBestOverlapping()

    #print("best cov: %f, best over: %f" %(best_coverage, best_overlapping))
    #print("init: %f, final: %f" %(learning_trace[0], learning_trace[-1]))
    #print(sum(ord(x) for x in best_configuration))

    #logging.info('search finished.')
    #print(self._best_configuration)
    # actually put nodes to sleep
    nb_alive = len(self._cluster.get_alive_nodes())
    log = {}
    log['coverage']        = best_coverage
    log['overlapping']     = best_overlapping
    log['nb_sleeping']     = float(sum(ord(x) for x in best_configuration)/float(nb_alive))
    log['initial_fitness'] = learning_trace[0]
    log['final_fitness']   = learning_trace[-1]

    #print("sleeping nodes %d out of %d" %(nb_sleeping_nodes, len(best_configuration)))
    #print([x.id for x in self._cluster if x.alive])

    # set cluster's nodes to sleep accordingly to optimization algorithm
    for idx, node in enumerate(sensor_nodes):
      node.is_sleeping = ord(best_configuration[idx])

    return log

