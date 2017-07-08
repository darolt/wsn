import numpy as np
from config import *
import logging
from time import time

# TODO generate all random values at once to improve performance

def crossover(individual1, individual2, crossover_rate):
  """Resulting individual has part of the genes of both parents."""
  split_gene = int(np.random.uniform(0,len(individual1)-1))
  # before gene split_idx, genes are from individual1, after from
  # individual2
  return np.concatenate((individual1[:split_gene], individual2[split_gene:]))

def mutation(individual, mutation_rate, cluster):
  """Forces a mutation in a single gene."""
  # mutate one gene
  new_individual = individual[:]
  while 1:
    mutated_gene = int(np.random.uniform(0,len(individual)-1))
    if cluster[mutated_gene].sleep_prob != 0:
      if individual[mutated_gene]:
        new_individual[mutated_gene] = 0
      else:
        new_individual[mutated_gene] = 1
      break
  
  return new_individual

def fitness(individual, cluster):
  """Evaluates how fit is an individual."""
  sleep = []
  for idx, gene in enumerate(individual):
    if gene:
      sleep.append(cluster[idx])
  initial_energy    = cluster.initial_energy
  individual_energy = cluster.get_remaining_energy(sleep)
  total_coverage  = cluster.regions.initial_coverage
  total_overlapping = cluster.regions.initial_overlapping
  #time1 = time()
  individual_coverage, individual_overlapping = cluster.get_both(sleep)
  #time2 = time()
  #print(time2-time1)
  if initial_energy == 0:
    term1 = 0
  else:
    term1 = PSO_FITNESS_ALPHA*(1-individual_energy/initial_energy)

  if total_coverage == 0:
    term2 = 0
  else:
    term2 = PSO_FITNESS_BETA*(individual_coverage/total_coverage)

  if individual_overlapping == 0:
    term3 = 0
  else:
    term3 = PSO_FITNESS_GAMMA*(total_overlapping/individual_overlapping)

  return term1 + term2 + term3

def calculate_mutation_rate(iteration):
  return PSO_WMAX - (PSO_WMAX-PSO_WMIN)*iteration/PSO_MAX_ITERATIONS


def sleep_scheduling_pso(cluster):
  """Runs PSO to decide which nodes in the cluster will sleep. The cur-
  rent cluster head should not be put to sleep, otherwise all informa-
  tion for that node is lost.
  """
  logging.info('running sleep scheduling')
  cluster_size = len(cluster)

  # calculate sleep probability for each node
  cluster.calculate_sleep_prob()
  sleep_prob = [node.sleep_prob for node in cluster]

  # paint grid and generate regions
  cluster.paint_grid()

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
    particle = random > sleep_prob
    best_local = particle[:]

    if idx_particle == 0:
      best_global = best_local[:]
      best_global_fit = fitness(best_global, cluster)

    best_locals_fit.append(fitness(best_local, cluster))
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
        new_particle = mutation(new_particle, mutation_rate, cluster)
      if randoms[it][idx_particle][1] > crossover_rate1:
        new_particle = crossover(new_particle, best_local, crossover_rate1)
      if randoms[it][idx_particle][2] > crossover_rate2:
        new_particle = crossover(new_particle, best_global,crossover_rate2)

      if should_update_particle(new_particle, best_global):
        particle = new_particle[:]

      # update best locals and best global
      particle_fitness = fitness(particle, cluster)
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

def should_update_particle(particle1, particle2):
  differences = particle1 == particle2
  nb_differences = np.count_nonzero(differences)
  if nb_differences > len(particle1)/2.0:
    return 1
  else:
    return 0

