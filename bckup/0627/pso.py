import numpy as np
from config import *
import logging
from time import time

def crossover(individual1, individual2, crossover_rate):
  """Resulting individual has part of the genes of both parents."""
  random = np.random.uniform(0,1)
  if random >= crossover_rate:
    return individual1[:]
  split_gene = int(np.random.uniform(0,len(individual1)-1))
  new_individual = individual1[:]
  # before gene split_idx, genes are from individual1, after from
  # individual2
  new_individual[split_gene:] = individual2[split_gene:]
  return new_individual

def mutation(individual, mutation_rate, cluster):
  """Add a mutation in a single gene with a certain probability."""
  random = np.random.uniform(0,1)
  if random >= mutation_rate: # skip mutation
    return individual[:] # return a copy
  new_individual = individual[:]
  # probability of mutation per gene
  prob_mutation = 1/len(individual)
  for gene in range(0, len(individual)):
    random = np.random.uniform(0,1)
    if random < prob_mutation: # mutate
      random = np.random.uniform(0,1)
      if random < cluster[gene].sleep_prob:
          new_individual[gene] = 0
      else:
          new_individual[gene] = 1
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
  time1 = time()
  individual_coverage, individual_overlapping = cluster.get_both(sleep)
  time2 = time()
  print("both")
  print(time2-time1)
  #individual_coverage = cluster.get_network_coverage(sleep)
  #individual_overlapping = cluster.get_network_overlapping(sleep)

  term1 = PSO_FITNESS_ALPHA*(1-individual_energy/initial_energy)
  term2 = PSO_FITNESS_BETA*(individual_coverage/total_coverage)
  term3 = PSO_FITNESS_GAMMA*(total_overlapping/individual_overlapping)
  return term1 + term2 + term3

def calculate_mutation_rate(iteration):
  return PSO_WMAX - (PSO_WMAX-PSO_WMIN)*iteration/PSO_MAX_ITERATIONS

def sleep_scheduling_pso(cluster, base_station):
  """Runs PSO to decide which nodes in the cluster will sleep. The cur-
  rent cluster head should not be put to sleep, otherwise all informa-
  tion for that node is lost.
  """
  logging.info('running sleep scheduling')
  cluster_size = len(cluster)

  # calculate sleep probability for each node
  time1 = time()
  cluster.calculate_sleep_prob()
  time2 = time()
  print("sleep_prob")
  print(time2-time1)
  sleep_prob = [node.sleep_prob for node in cluster]

  # paint grid and generate regions
  #time1 = time()
  cluster.paint_grid()
  #time2 = time()
  #print(time2-time1)

  # initialize particles, best local and best global
  particles  = []
  best_locals = []
  best_global = np.zeros((cluster_size), dtype=np.uint8)
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
    elif fitness(best_local, cluster) > fitness(best_global, cluster):
      best_global = best_local[:]

    particles.append(particle)      
    best_locals.append(best_local)

  for it in range(0, PSO_MAX_ITERATIONS):
    logging.debug("pso iteration %d" % (it))
    for idx_particle in range(0, PSO_NB_PARTICLES):
      particle = particles[idx_particle]
      best_local = best_locals[idx_particle]

      mutation_rate   = calculate_mutation_rate(it)
      crossover_rate1 = 1 - it/PSO_MAX_ITERATIONS
      crossover_rate2 = 1 - crossover_rate1
      new_particle = mutation(particle, mutation_rate, cluster)
      new_particle = crossover(particle, best_local, crossover_rate1)
      new_particle = crossover(particle, best_global,crossover_rate2)

      if should_update_particle(new_particle, best_global):
        #print('updating particle')
        particle = new_particle[:]

      # update best locals and best global
      if fitness(particle, cluster) > fitness(best_local, cluster):
        #print('found best_local')
        best_local = particle[:]
      if fitness(particle, cluster) > fitness(best_global, cluster):
        #print('found best_global')
        best_global = particle[:]

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

