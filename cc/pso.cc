#include "pso.h"
#include <stdio.h>

Pso::Pso(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config)
    :Optimizer(exclusive, overlapping, ids, config) {
}

Pso::~Pso() {
}

void
Pso::Optimize(float_v energies, const vector<u_int> &can_sleep,
              float total_energy) {
  uniform_real_distribution<float> distribution(0.0, 1.0);
  uniform_real_distribution<float> distribution2(-0.5, 0.5);

  // initialize velocity_
  velocity_ = vector<float_v>(NB_INDIVIDUALS_, float_v(nb_nodes_, 0.0));

  for (u_int individual_idx = 0; individual_idx < NB_INDIVIDUALS_; individual_idx++) {
    for(u_int gene_idx = 0; gene_idx < nb_nodes_; gene_idx++) {
      velocity_[individual_idx][gene_idx] = distribution2(generator_);
    }
  }

  float acceleration = 1.0, phi1 = 2.0, phi2 = 2.0;
  for (u_int it = 0; it < MAX_ITERATIONS_; it++) {
    learning_trace_.push_back(best_global_fitness_);
    for (u_int individual_idx = 0; individual_idx < NB_INDIVIDUALS_; individual_idx++) {
      individual_t &individual = individuals_[individual_idx];
      
      //for(u_int gene_idx = 0; gene_idx < nb_nodes_; gene_idx++) {
      for(auto const &gene_idx: can_sleep) {
        float r1 = distribution(generator_);
        float r2 = distribution(generator_);
        float r3 = distribution(generator_);

        int diff_to_global = best_global_[gene_idx] - individual[gene_idx];
        int diff_to_local  = best_locals_[individual_idx][gene_idx] - individual[gene_idx];
        velocity_[individual_idx][gene_idx] = acceleration*velocity_[individual_idx][gene_idx] +
                                              phi1*r1*diff_to_global + 
                                              phi2*r2*diff_to_local;

        float velocity_norm = 1 / (1 + exp(-velocity_[individual_idx][gene_idx]));
        //printf("r3: %f, v: %f\n\n", r3, velocity_[individual_idx][gene_idx]);
        
        individual[gene_idx] = (r3 < velocity_norm) ? 1 : 0;
      }
    }

    UpdateGenerationFitness(energies, total_energy);

  }
}

