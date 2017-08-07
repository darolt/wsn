#include "my_pso.h"
#include <stdio.h>

MyPso::MyPso(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config)
    :Optimizer(exclusive, overlapping, ids, config) {
}

MyPso::~MyPso() {
}

void
MyPso::Optimize(float_v energies, const vector<u_int> &can_sleep,
              float total_energy) {
  uniform_real_distribution<float> distribution(0.0, 1.0);

  float acceleration, phi1, phi2;
  for (u_int it = 0; it < MAX_ITERATIONS_; it++) {
    learning_trace_.push_back(best_global_fitness_);
    for (u_int individual_idx = 0; individual_idx < NB_INDIVIDUALS_; individual_idx++) {
      individual_t &individual = individuals_[individual_idx];
      acceleration   = WMAX_ - (WMAX_-WMIN_)*it/float(MAX_ITERATIONS_);
      phi1 = 1.0 - it/float(MAX_ITERATIONS_);
      phi2 = 1.0 - phi1;

      float r1 = distribution(generator_);
      float r2 = distribution(generator_);

      auto new_individual = individuals_[individual_idx];
      Influence(individual, best_locals_[individual_idx],
                new_individual, can_sleep, phi1*r1);
      Influence(individual, best_global_, new_individual, can_sleep, phi2*r2);
      individual = new_individual;
      Move(individual, can_sleep, acceleration);
    }

    UpdateGenerationFitness(energies, total_energy);

  }
}

void
MyPso::Move(individual_t &individual, vector<u_int> can_sleep,
          float acceleration) {

  u_int nb_changes = u_int(float(can_sleep.size())*acceleration);
  for(u_int count = 0; count < nb_changes; count++) {
    uniform_int_distribution<int> distribution(0, can_sleep.size()-1);
    u_int changed_idx = distribution(generator_);
    u_int changed_gene = can_sleep[changed_idx];

    // flips gene
    individual[changed_gene] = (individual[changed_gene] == 0) ? 1 : 0;
    can_sleep.erase(can_sleep.begin() + changed_idx);
  }

  return;
}

void
MyPso::Influence(const individual_t &original_individual,
               const individual_t &influencer,
               individual_t &new_individual,
               vector<u_int> can_sleep,
               float influence_rate) {

  uniform_real_distribution<float> distribution(0.0, 1.0);

  // different genes from influencer are copied with influence rate
  for (auto const &gene: can_sleep) {
    if (original_individual[gene] != influencer[gene]) {
      if (distribution(generator_) < influence_rate) {
        new_individual[gene] = influencer[gene];
      }
    }
  }
  return;
}

