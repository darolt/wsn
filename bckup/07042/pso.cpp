#include "pso.h"
#include <random>
#include <stdio.h>

PSO::PSO(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config)
    :_ids(ids) {
  _regions = new Regions(exclusive, overlapping);


  _NB_PARTICLES       = config.first[0];
  _MAX_ITERATIONS = config.first[1];
  _FITNESS_ALPHA  = config.second[0];
  _FITNESS_BETA   = config.second[1];
  _FITNESS_GAMMA  = config.second[2];
  _WMAX           = config.second[3];
  _WMIN           = config.second[4];
  
  _nb_nodes = ids.size();
  // initialize particles structure
  _particles   = vector<individual_t>(_NB_PARTICLES, individual_t(_nb_nodes, 0));
  _best_locals = vector<individual_t>(_NB_PARTICLES, individual_t(_nb_nodes, 0));
  _best_global = individual_t(_nb_nodes, 0);
  _best_local_fitness = vector<float>(_NB_PARTICLES, 0.0);
}

PSO::~PSO() {
  delete _regions;
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
  _best_global_fitness = 0.0;
  //printf("number of particles %d\n", _NB_PARTICLES);
  for (u_int particle_idx = 0; particle_idx < _NB_PARTICLES; particle_idx++) {
    individual_t particle = _particles[particle_idx];
    for (u_int node_idx = 0; node_idx < _nb_nodes; node_idx++) {
      float random = distribution(generator);
      particle[node_idx] = (random < sleep_probs[node_idx]) ? 1 : 0;
    } 
    _best_locals[particle_idx] = particle;

    _best_local_fitness[particle_idx] = fitness(particle,
                                                energies, total_energy);

    if ((particle_idx == 0) ||
        (_best_local_fitness[particle_idx] > _best_global_fitness)) {
      _best_global = particle;
      _best_global_fitness = _best_local_fitness[particle_idx];
    }
  }
  // search for solution
  float mutation_rate, crossover_rate1, crossover_rate2;
  for (u_int it = 0; it < _MAX_ITERATIONS; it++) {
    for (u_int particle_idx = 0; particle_idx < _NB_PARTICLES; particle_idx++) {
      individual_t &particle = _particles[particle_idx];
      mutation_rate   = _WMAX - (_WMAX-_WMIN)*it/float(_MAX_ITERATIONS);
      crossover_rate1 = 1.0 - it/float(_MAX_ITERATIONS);
      crossover_rate2 = 1.0 - crossover_rate1;

      individual_t new_particle = particle;
      if (distribution(generator) < mutation_rate) {
        //printf("mutating particle\n");
        new_particle = mutation(particle, sleep_probs);
      }
      if (distribution(generator) < crossover_rate1) {
        //printf("crossing1 particle\n");
        new_particle = crossover(particle, _best_locals[particle_idx]);
      }
      if (distribution(generator) < crossover_rate2) {
        //printf("crossing2 particle\n");
        new_particle = crossover(particle, _best_global);
      }
      if (distribution(generator) < should_update(new_particle, _best_global)) {
        //printf("updated particle\n");
        particle = new_particle;

        float particle_fitness = fitness(particle,
                                         energies, total_energy);
        if (particle_fitness > _best_local_fitness[particle_idx]) {
          _best_locals[particle_idx] = particle;
          _best_local_fitness[particle_idx] = particle_fitness;
        }
        if (particle_fitness > _best_global_fitness) {
          _best_global = particle;
          _best_global_fitness = particle_fitness;
        }
      }
    }
  }

  //printf("%f\n", _best_global_fitness);
  return _best_global;
}

individual_t
PSO::mutation(individual_t individual, vector<float> sleep_probs) {
  individual_t new_individual = individual;
  float threshold = 0.005;
  // find all nodes that are susceptible to sleep (not dead neither ch)
  vector<u_int> can_sleep;
  for(u_int idx = 0; idx < sleep_probs.size(); idx++) {
    float prob = sleep_probs[idx];
    if (prob > threshold) {
      can_sleep.push_back(idx);
    }
  }
  if (can_sleep.size() > 0) {
    default_random_engine generator;
    uniform_int_distribution<int> distribution(0, can_sleep.size());
    u_int mutated_gene = distribution(generator);
    if (individual[mutated_gene] == 1) {
      new_individual[mutated_gene] = 0;
    } else {
      new_individual[mutated_gene] = 1;
    }
  }
  return new_individual;
}

individual_t
PSO::crossover(individual_t individual1, individual_t individual2) {
  default_random_engine generator;
  uniform_int_distribution<int> distribution(0, individual1.size());

  individual_t new_individual = individual1;
  int split_gene = distribution(generator);
  for (u_int gene = split_gene; gene < individual1.size(); gene++) {
    new_individual[gene] = individual2[gene];
  }
  return new_individual;
}

float
PSO::should_update(individual_t particle1, individual_t particle2) {
  float nb_differences = 0.0;
  for (u_int gene = 0; gene < particle1.size(); gene++) {
    //printf("%d", particle1[gene]);
    //printf("%d", particle2[gene]);
    if (particle1[gene] != particle2[gene]) {
      nb_differences += 1.0;
    }
  }
  return nb_differences/float(particle1.size());
}

float
PSO::fitness(individual_t individual, vector<float> energies,
             float total_energy) {
  vector<int> sleep_nodes;
  float partial_energy = 0.0;
  for (u_int gene_idx = 0; gene_idx < _nb_nodes; gene_idx++) {
    // push nodes that supposed to sleep plus nodes that are dead
    if (individual[gene_idx] == 1) {
      sleep_nodes.push_back(_ids[gene_idx]);
    } else {
      partial_energy += energies[gene_idx];
    }
  }
  vector<int> dead_nodes;
  float threshold = 0.005;
  for (u_int gene_idx = 0; gene_idx < _nb_nodes; gene_idx++) {
    // push nodes that supposed to sleep plus nodes that are dead
    if (energies[gene_idx] < threshold) {
      dead_nodes.push_back(_ids[gene_idx]);
    }
  }

  auto coverage_info = _regions->get_all(sleep_nodes, dead_nodes);

  float term1;
  if (total_energy == 0) {
    term1 = 0.0;
  } else {
    term1 = _FITNESS_ALPHA*(1-partial_energy/total_energy);
  }
  float term2;
  if (coverage_info[0] == 0) {
    term2 = 0.0;
  } else { // beta*partial_coverage/total_coverage
    term2 = _FITNESS_BETA*(coverage_info[1]/coverage_info[0]);
  }
  float term3;
  if (coverage_info[3] == 0) {
    term3 = 0.0;
  } else { // gamma*total_overlapping/partial_overlapping
    term3 = _FITNESS_GAMMA*(coverage_info[2]/coverage_info[3]);
  }

  printf("fitness %f, 1: %f, 2: %f, 3: %f\n", term1 + term2 + term3, term1, term2, term3);
  return term1 + term2 + term3;
}


