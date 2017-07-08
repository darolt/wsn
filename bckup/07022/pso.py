import numpy as np
from config import *
import logging
from time import time
from grid import *
from regions_converter import *
import regions

# TODO generate all random values at once to improve performance
# TODO go to C++
def calculate_mutation_rate(iteration):
  return PSO_WMAX - (PSO_WMAX-PSO_WMIN)*iteration/PSO_MAX_ITERATIONS

# TODO go to c++
def should_update_particle(particle1, particle2):
  differences = particle1 == particle2
  nb_differences = np.count_nonzero(differences)
  if nb_differences > len(particle1)/2.0:
    return 1
  else:
    return 0
# TODO go to C++
def crossover(individual1, individual2, crossover_rate):
  """Resulting individual has part of the genes of both parents."""
  split_gene = int(np.random.uniform(0,len(individual1)-1))
  # before gene split_idx, genes are from individual1, after from
  # individual2
  return np.concatenate((individual1[:split_gene], individual2[split_gene:]))

# TODO go to C++
def mutation(individual, mutation_rate, sleep_probs):
  """Forces a mutation in a single gene."""
  # mutate one gene
  new_individual = individual[:]
  while 1:
    mutated_gene = int(np.random.uniform(0,len(individual)-1))
    if sleep_probs[mutated_gene] != 0:
      if individual[mutated_gene]:
        new_individual[mutated_gene] = 0
      else:
        new_individual[mutated_gene] = 1
      break
  
  return new_individual

class PSOWrapper():

  def __init__(self, cluster):
    # need to update neighbors through this method, so grid can be
    # generated faster
    cluster.calculate_sleep_prob()

    grid = Grid()
    for node in cluster[0:-1]:
      grid.add_node(node, COVERAGE_RADIUS)
    regions_converter = RegionsConverter(grid)
    regions_buff = regions_converter.convert()    

    self._regions = regions.Regions(regions_buff[0], regions_buff[1])
    
  def sleep_scheduling(self, cluster):
    """Runs PSO to decide which nodes in the cluster will sleep. The cur-
    rent cluster head should not be put to sleep, otherwise all informa-
    tion for that node is lost.
    """
    logging.info('running sleep scheduling')
    cluster_size = len(cluster)
  
    # calculate sleep probability for each node
    cluster.calculate_sleep_prob()
    sleep_probs = [node.sleep_prob for node in cluster]
    node_ids = [node.id for node in cluster]
    energies = [node.energy_source.energy for node in cluster]

    # TODO C++ here
    # self._pso.run(energies, sleep_probs)
  
    # initialize particles, best local and best global
    particles  = []
    best_locals = []
    best_locals_fit = []
    best_global = np.zeros((cluster_size), dtype=np.uint8)
    best_global_fit = 0.0
    # a single particle is a array with 0 (sleep) or 1 (awake) for every
    # node in the cluster
    for idx_particle in range(0, PSO_NB_PARTICLES):
      # each particle contains information about the state of all nodes
      # in this cluster, indicating if they are active (1) /inactive (0)
      random = np.random.uniform(0, 1, (cluster_size))
      particle = random > sleep_probs
      best_local = particle[:]
  
      if idx_particle == 0:
        best_global = best_local[:]
        best_global_fit = self.fitness(best_global, node_ids, cluster)
  
      best_locals_fit.append(self.fitness(best_local, node_ids, cluster))
      if best_locals_fit[idx_particle] > best_global_fit:
        best_global = best_local[:]
        best_global_fit = best_locals_fit[idx_particle]
  
      particles.append(particle)      
      best_locals.append(best_local)
  
    shape_randoms = (PSO_MAX_ITERATIONS, PSO_NB_PARTICLES, 3)
    randoms = np.random.uniform(0, 1, shape_randoms)
    for it in range(0, PSO_MAX_ITERATIONS):
      logging.debug("pso iteration %d" % (it))
      for idx_particle in range(0, PSO_NB_PARTICLES):
        particle = particles[idx_particle]
        best_local = best_locals[idx_particle]
  
        mutation_rate   = calculate_mutation_rate(it)
        crossover_rate1 = 1 - it/PSO_MAX_ITERATIONS
        crossover_rate2 = 1 - crossover_rate1
  
        new_particle = particle[:]
        if randoms[it][idx_particle][0] > mutation_rate:
          new_particle = mutation(new_particle, mutation_rate, sleep_probs)
        if randoms[it][idx_particle][1] > crossover_rate1:
          new_particle = crossover(new_particle, best_local, crossover_rate1)
        if randoms[it][idx_particle][2] > crossover_rate2:
          new_particle = crossover(new_particle, best_global,crossover_rate2)
  
        if should_update_particle(new_particle, best_global):
          particle = new_particle[:]
  
        # update best locals and best global
        particle_fitness = self.fitness(particle, node_ids, cluster)
        if particle_fitness > best_locals_fit[idx_particle]:
          #print('found best_local')
          best_local = particle[:]
          best_locals_fit[idx_particle] = particle_fitness
        if particle_fitness > best_global_fit:
          #print('found best_global')
          best_global = particle[:]
          best_global_fit = particle_fitness
  
    # actually put nodes to sleep
    for idx, node in enumerate(cluster):
      node.is_sleeping = best_global[idx]

  # TODO go to C++
  # energies enter here
  def fitness(self, individual, node_ids,  cluster):
    """Evaluates how fit is an individual."""
    sleep_nodes = []
    for idx, gene in enumerate(individual):
      if gene:
        sleep_nodes.append(node_ids[idx])

    coverage_info = self._regions.get_all(sleep_nodes)

    initial_energy      = cluster.initial_energy
    individual_energy   = cluster.get_remaining_energy(sleep_nodes)
    total_coverage      = coverage_info[0]
    partial_coverage    = coverage_info[1]
    total_overlapping   = coverage_info[2]
    partial_overlapping = coverage_info[3]

    if initial_energy == 0:
      term1 = 0
    else:
      term1 = PSO_FITNESS_ALPHA*(1-individual_energy/initial_energy)
  
    if total_coverage == 0:
      term2 = 0
    else:
      term2 = PSO_FITNESS_BETA*(partial_coverage/total_coverage)
  
    if partial_overlapping == 0:
      term3 = 0
    else:
      term3 = PSO_FITNESS_GAMMA*(total_overlapping/partial_overlapping)
  
    return term1 + term2 + term3
  

