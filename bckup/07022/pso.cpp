#include "pso.h"
#include <random>

PSO::PSO(dict_t exclusive, regions_t overlapping, vector<int> ids, int nb_particles)
    :_ids(ids),
     _nb_particles(nb_particles) {
  _regions = new Regions(exclusive, overlapping);

  _nb_nodes = ids.size();
  // initialize particles structure
  vector<individual_t> zeros0(nb_particles, individual_t(_nb_nodes, 0))
  individual_t zeros1(_nb_nodes, 0);
  _particles = zeros0;
  _best_locals zeros0;
  _best_global = zeros1;
  _best_local_fitness = vector<float>(nb_particles, 0.0);
}

PSO::~PSO() {
}

individual_t
PSO::run(vector<float> energies, vector<float> sleep_probs) {
  default_random_engine generator;
  uniform_real_distribution<float> distribution(0.0, 1.0);

  float total_energy = 0.0;
  for (auto const &energy: energies) {
    total_energy += energy;
  }

  // initialize data
  int particle_idx = 0;
  for (auto const &particle: _particles) {
    for (int node_idx = 0; node_idx < _nb_nodes; node_idx++) {
      float random = distribution(generate);
      particle[node_idx] = (random < sleep_probs[node_idx]) ? 1 : 0;
    } 
    _best_locals[particle_idx] = particle;

    _best_local_fitness[particle_idx] = fitness(particle, sleep_probs,
                                                energies, total_energy);
    if (particle_idx == 0) {
      _best_global = particle;
      _best_global_fitness = _best_local_fitness[particle_idx];
    }

    if (_best_local_fitness[particle_idx] > _best_global_fitness) {
      _best_global = particle;
      _best_global_fitness = _best_local_fitness[particle_idx];
    }

    particle_idx++;
  }
  // search for solution
  float mutation_rate, crossover_rate1, crossover_rate2;
  for (int it = 0; it < 50; it++) {
    particle_idx = 0;
    for (auto const &particle: _particles) {
      mutation_rate   = 0.9 - (0.9-0.4)*it/50;  
      crossover_rate1 = 1 - it/50;
      crossover_rate2 = 1 - crossover_rate1;

      individual_t new_particle = particle;
      if (distribution(generator) < mutation_rate) {
        new_particle = mutation(particle, sleep_probs);
      }
      if (distribution(generator) < crossover_rate1) {
        new_particle = crossover(particle, _best_locals[particle_idx]);
      }
      if (distribution(generator) < crossover_rate2) {
        new_particle = crossover(particle, _best_global);
      }
      if (should_update(new_particle, _best_global) == 1) {
        particle = new_particle;

        particle_fitness = fitness(particle, sleep_probs,
                                   energies, total_energy)
        if (particle_fitness > _best_local_fitness[particle_idx]) {
          _best_locals[particle_idx] = particle;
        }
        if (particle_fitness > _best_global_fitness) {
          _best_global = particle;
          _best_global_fitness = particle_fitness;
        }
      }

    }
  }

  return _best_global;
}

individual_t
PSO::mutation(individual_t individual, vector<float> sleep_probs) {
  default_random_engine generator;
  uniform_int_distribution<int> distribution(0, individual.size());

  individual_t new_individual = individual;
  while (1) {
    float mutated_gene = distribution(generator);
    if (sleep_probs[mutated_gene] != 0) {
      if (individual[mutated_gene] == 1) {
        new_individual[mutated_gene] = 0;
      } else {
        new_individual[mutated_gene] = 1;
      }
      break;
    }
  }
  return new_individual;
}

individual_t
PSO::crossover(individual_t individual1, individual_t individual2) {
  default_random_engine generator;
  uniform_int_distribution<int> distribution(0, individual1.size());

  individual_t new_individual = individual1;
  split_gene = distribution(generator);
  for (int gene = split_gene; gene < individual1.size(); gene++) {
    new_individual[gene] = individual2[gene];
  }
}

int
PSO::should_update(individual_t particle1, individual_t particle2) {
  int nb_differences = 0;
  for (int gene = 0; gene < individual1.size(); gene++) {
    if (individual1[gene] != individual2[gene]) {
      nb_differences++;
    }
  }
  if (2*nb_differences > individual1.size()) {
    return 1;
  } else {
    return 0;
  }
}

float
PSO::fitness(individual_t individual, vector<float> sleep_probs,
             vector<float> energies, float total_energy) {
  vector<int> sleep_nodes;
  int gene_idx = 0;
  float partial_energy = 0.0;
  for (auto const &gene: individual) {
    if (gene == 1) {
      sleep_nodes.push_back(_ids[gene_idx]);
    } else {
      partial_energy += energies[gene_idx];
    }
  }

  vector<float> coverage_info = _regions->get_all(sleep_nodes);

  float term1;
  if (total_energy == 0) {
    term1 = 0.0;
  } else {
    term1 = 0.4*(1-partial_energy/total_energy);
  }
  float term2;
  if (coverage_info[0] == 0) {
    term2 = 0.0;
  } else {
    term2 = 0.3*(coverage_info[1]/coverage_info[0]);
  }
  float term3;
  if (coverage_info[3] == 0) {
    term3 = 0.0;
  } else {
    term3 = 0.3*(coverage_info[2]/coverage_info[3]);
  }

  return term1 + term2 + term3;
}


