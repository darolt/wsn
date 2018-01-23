#include "pso.h"
#include <random>
#include <stdio.h>

PSO::PSO(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config)
    :_ids(ids) {

  _regions = new Regions(exclusive, overlapping);

  _NB_PARTICLES   = config.first[0];
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

void
PSO::print_particle(individual_t particle) {
  for (auto const &gene: particle) {
    printf("%d", gene);
  }
  printf("\n");
}

return_t
PSO::run(vector<float> energies, vector<float> sleep_probs) {
  // for all random behavior
  default_random_engine generator;
  uniform_real_distribution<float> distribution(0.0, 1.0);

  // for tracing the learning of pso
  vector<float> learning_trace;

  float total_energy = 0.0;
  for (auto const &energy: energies) {
    total_energy += energy;
  }

  // initialize data
  _best_global_fitness = 0.0;
  //printf("number of particles %d\n", _NB_PARTICLES);
  for (u_int particle_idx = 0; particle_idx < _NB_PARTICLES; particle_idx++) {
    individual_t &particle = _particles[particle_idx];
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
    printf("iteration %d\n", it);
    learning_trace.push_back(_best_global_fitness);
    for (u_int particle_idx = 0; particle_idx < _NB_PARTICLES; particle_idx++) {
      printf("particle %d\n", particle_idx);
      individual_t &particle = _particles[particle_idx];
      mutation_rate   = _WMAX - (_WMAX-_WMIN)*it/float(_MAX_ITERATIONS);
      crossover_rate1 = 1.0 - it/float(_MAX_ITERATIONS);
      crossover_rate2 = 1.0 - crossover_rate1;

      //printf("mutating particle\n");
      //printf("Particle before");
      //print_particle(particle);
      mutation(particle, sleep_probs, mutation_rate);
      //printf("Particle after mutation");
      //print_particle(particle);
      if (distribution(generator) < crossover_rate1) {
        //printf("crossing1 particle\n");
        crossover(particle, _best_locals[particle_idx]);
        //printf("Particle after crossover1");
        //print_particle(particle);
      }
      if (distribution(generator) < crossover_rate2) {
        //printf("crossing2 particle\n");
        crossover(particle, _best_global);
        //printf("Particle after crossover2");
        //print_particle(particle);
      }

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

  //printf("%f\n", _best_global_fitness);
  return_t return_info;
  return_info.first  = _best_global;
  return_info.second = learning_trace;
 
  return return_info;
}

void
PSO::mutation(individual_t &individual, vector<float> sleep_probs,
              float mutation_rate) {
  float threshold = 0.005;
  // find all nodes that are susceptible to sleep (not dead neither ch)
  vector<u_int> can_sleep;
  for(u_int idx = 0; idx < sleep_probs.size(); idx++) {
    float prob = sleep_probs[idx];
    if (prob > threshold) {
      can_sleep.push_back(idx);
    }
  }

  // mutate mutation_rate percent of particles' genes
  u_int nb_mutations = u_int(float(can_sleep.size())*mutation_rate);
  //printf("nb_mutations %d\n", nb_mutations);
  uniform_int_distribution<int> distribution2(0.0, 1.0);
  default_random_engine generator;
  for(u_int count = 0; count < nb_mutations; count++) {
    //printf("can_sleep size %d\n", can_sleep.size());
    uniform_int_distribution<int> distribution(0, can_sleep.size()-1);
    u_int mutated_idx = distribution(generator);
    u_int mutated_gene = can_sleep[mutated_idx];
    //printf("%d %d\n", mutated_idx, mutated_gene);

    float random = distribution2(generator);
    // flips gene
    individual[mutated_gene] = (individual[mutated_gene] == 0) ? 1 : 0;
    can_sleep.erase(can_sleep.begin() + mutated_idx);
  }

  return;
}

void
PSO::crossover(individual_t &individual1, individual_t &individual2) {
  default_random_engine generator;
  uniform_real_distribution<float> distribution2(0.0, 1.0);

  for (u_int gene = 0; gene < _nb_nodes; gene++) {
    u_int origin = (distribution2(generator) < 0.5) ? 0 : 1;
    individual1[gene] = (origin) ? individual1[gene] : individual2[gene];
  }
  return;
}

float
PSO::should_update(individual_t particle1) {
  float nb_differences = 0.0;
  for (u_int gene = 0; gene < _nb_nodes; gene++) {
    //printf("%d", particle1[gene]);
    //printf("%d", particle2[gene]);
    if (particle1[gene] != _best_global[gene]) {
      nb_differences += 1.0;
    }
  }
  return nb_differences/float(_nb_nodes);
}

float
PSO::fitness(individual_t individual, vector<float> energies,
             float total_energy) {
  vector<u_int> sleep_nodes;
  float partial_energy = 0.0;
  vector<u_int> dead_nodes;
  float threshold = 0.005;
  u_int alive_nodes = 0;
  for (u_int gene_idx = 0; gene_idx < _nb_nodes; gene_idx++) {
    // push nodes that are dead
    if (energies[gene_idx] < threshold) {
      dead_nodes.push_back(_ids[gene_idx]);
    } else if (individual[gene_idx] == 1) { // sleeping nodes
      sleep_nodes.push_back(_ids[gene_idx]);
      alive_nodes++;
    } else {
      partial_energy += energies[gene_idx];
      alive_nodes++;
    }
  }

  // new term1
  float average_energy = total_energy/alive_nodes;
  float energies_dev = 0.0, neg_energy_dev = 0.0, pos_energy_dev = 0.0;
  for (u_int gene_idx = 0; gene_idx < _nb_nodes; gene_idx++) {
    if (individual[gene_idx] == 0) {
      float energy_dev = energies[gene_idx] - average_energy;
      if (energy_dev < 0) {
        neg_energy_dev += energy_dev;
      } else {
        pos_energy_dev += energy_dev;
      }
      energies_dev += energy_dev;
    }
  }

  auto coverage_info = _regions->get_all(sleep_nodes, dead_nodes);

  float term1;
  //if (total_energy == 0) {
  if (pos_energy_dev-neg_energy_dev == 0) {
    term1 = 0.0;
  } else {
    //term1 = _FITNESS_ALPHA*(1.0-partial_energy/total_energy);
    term1 = _FITNESS_ALPHA*(energies_dev-neg_energy_dev)/(pos_energy_dev-neg_energy_dev);
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
  } else { // gamma*(1- partial_overlapping/total_overlapping)
    term3 = _FITNESS_GAMMA*(1.0 - coverage_info[3]/coverage_info[2]);
  }

  printf("fitness %f, 1: %f, 2: %f, 3: %f\n", term1 + term2 + term3, term1, term2, term3);
  return term1 + term2 + term3;
}


