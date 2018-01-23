import numpy as np
from config import *
import logging
from time import time
from grid import *
from regions_converter import *
import pso


class PSOWrapper(object):

  def __init__(self, cluster):
    # need to update neighbors through this method, so grid can be
    # generated faster
    cluster.update_sleep_prob()
    self._cluster = cluster
    
    grid = Grid()
    for node in cluster.get_sensor_nodes():
      grid.add_node(node, COVERAGE_RADIUS)
    regions_converter = RegionsConverter(grid)
    regions_buff = regions_converter.convert()    

    ids = [node.id for node in cluster.get_sensor_nodes()] 
    config_int = [PSO_NB_PARTICLES, PSO_MAX_ITERATIONS]
    config_float = [PSO_FITNESS_ALPHA, PSO_FITNESS_BETA,
                    PSO_FITNESS_GAMMA, PSO_WMAX, PSO_WMIN]
    configuration = (config_int, config_float)
    self._pso = pso.PSO(regions_buff[0], regions_buff[1],
                        ids, configuration)
    
  def sleep_scheduling(self):
    """Runs PSO to decide which nodes in the cluster will sleep. The cur-
    rent cluster head should not be put to sleep, otherwise all informa-
    tion for that node is lost.
    """
    if (self._cluster.count_alive_nodes() == 0):
      return
    membership = self._cluster[0].membership
    logging.info("running sleep scheduling for cluster %d" % (membership))
    # no need to run sleep scheduling if all nodes are dead
  
    # calculate sleep probability for each node
    self._cluster.update_sleep_prob()
    sensor_nodes = self._cluster.get_sensor_nodes()
    sleep_probs = [node.sleep_prob for node in sensor_nodes]
    node_ids = [node.id for node in sensor_nodes]
    energies = [node.energy_source.energy for node in sensor_nodes]

    best_configuration, learning_trace = self._pso.run(energies, sleep_probs)
    #logging.info('search finished.')
    #print(self._best_configuration)
    # actually put nodes to sleep
    nb_sleeping_nodes = sum(ord(x) for x in best_configuration)
    #print("sleeping nodes %d out of %d" %(nb_sleeping_nodes, len(best_configuration)))
    sleeping_print = [ord(x) for x in best_configuration]
    mask = [x.is_head() for x in sensor_nodes]
    print(sleep_probs)
    print(sleeping_print)
    print(mask)
    print([x.id for x in self._cluster if x.alive])
    print(learning_trace)
    #print(sum(ord(x) for x in best_configuration))
    for idx, node in enumerate(sensor_nodes):
      node.is_sleeping = ord(best_configuration[idx])


