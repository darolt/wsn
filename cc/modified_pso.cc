#include "modified_pso.h"
#include <stdio.h>

using namespace std;

ModifiedPso::ModifiedPso(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config)
    :Optimizer(exclusive, overlapping, ids, config) {
}

ModifiedPso::~ModifiedPso() {
}

void
ModifiedPso::Optimize(const vector<u_int> &can_sleep) {

  uniform_real_distribution<float> distribution(0.0, 1.0);
  float mutation_rate, crossover_rate1, crossover_rate2;
  for (u_int it = 0; it < max_iterations_; it++) {
    learning_trace_.push_back(best_global_fitness_);
    for (u_int particle_idx = 0; particle_idx < nb_individuals_; particle_idx++) {
      individual_t &particle = population_[particle_idx];
      mutation_rate   = wmax_ - (wmax_-wmin_)*it/float(max_iterations_);
      crossover_rate1 = 1.0 - it/float(max_iterations_);
      crossover_rate2 = 1.0 - crossover_rate1;

      Mutate(particle, can_sleep, mutation_rate);
      if (distribution(generator_) < crossover_rate1) {
        Crossover(particle, best_locals_[particle_idx]);
      }
      if (distribution(generator_) < crossover_rate2) {
        Crossover(particle, best_global_);
      }
    }

    UpdateFitness();
  }
}

void
ModifiedPso::Mutate(individual_t &individual, vector<u_int> can_sleep,
              float mutation_rate) {
  // mutate mutation_rate percent of particles' genes
  u_int nb_mutations = u_int(float(can_sleep.size())*mutation_rate);
  for(u_int count = 0; count < nb_mutations; count++) {
    uniform_int_distribution<int> distribution(0, can_sleep.size()-1);
    u_int mutated_idx = distribution(generator_);
    u_int mutated_gene = can_sleep[mutated_idx];
    //printf("%d %d\n", mutated_idx, mutated_gene);

    // flips gene
    individual[mutated_gene] = (individual[mutated_gene] == 0) ? 1 : 0;
    can_sleep.erase(can_sleep.begin() + mutated_idx);
  }

  return;
}

void
ModifiedPso::Crossover(individual_t &individual1, individual_t &individual2) {
  uniform_real_distribution<float> distribution(0.0, 1.0);

  for (u_int gene = 0; gene < nb_nodes_; gene++) {
    u_int origin = (distribution(generator_) < 0.5) ? 0 : 1;
    individual1[gene] = (origin) ? individual1[gene] : individual2[gene];
  }
  return;
}

