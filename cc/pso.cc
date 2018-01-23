#include "pso.h"
#include <stdio.h>

using namespace std;

Pso::Pso(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config)
    :Optimizer(exclusive, overlapping, ids, config) {
}

Pso::~Pso() {
}

void
Pso::Optimize(const vector<u_int> &can_sleep) {
  uniform_real_distribution<float> distribution(0.0, 1.0);
  uniform_real_distribution<float> distribution2(-0.5, 0.5);

  // initialize velocity_
  velocity_ = vector<float_v>(nb_individuals_, float_v(nb_nodes_, 0.0));

  for (u_int individual_idx = 0; individual_idx < nb_individuals_; individual_idx++)
    for(u_int idx = 0; idx < nb_nodes_; idx++)
      velocity_[individual_idx][idx] = distribution2(generator_);

  float acceleration = 1.0, phi1 = 2.0, phi2 = 2.0;
  for (u_int it = 0; it < max_iterations_; it++) {
    PushIntoLearningTraces(best_global_.GetFitness());
    for (u_int individual_idx = 0; individual_idx < nb_individuals_; individual_idx++) {
      Individual &individual = population_[individual_idx];
      Individual &best_local = best_locals_[individual_idx];
      auto genes = individual.GetGenes();
      
      for(auto const &idx: can_sleep) {
        float r1 = distribution(generator_);
        float r2 = distribution(generator_);
        float r3 = distribution(generator_);

        int diff_to_global = best_global_.GetGenes()[idx] - genes[idx];
        int diff_to_local  = best_local.GetGenes()[idx] - genes[idx];
        velocity_[individual_idx][idx] = acceleration*velocity_[individual_idx][idx] +
                                         phi1*r1*diff_to_global + 
                                         phi2*r2*diff_to_local;

        float velocity_norm = 1 / (1 + exp(-velocity_[individual_idx][idx]));
        
        genes[idx] = (r3 < velocity_norm) ? 1 : 0;
      }
      individual.SetGenes(genes);
      if (individual.GetFitness().total > best_local.GetFitness().total)
        best_local = individual;
    }
  }
}

fitness_t
Pso::Fitness(Individual &individual) {
  float partial_energy = 0.0;
  u_int nb_active_nodes = 0;
  auto genes = individual.GetGenes();
  for (u_int idx = 0; idx < nb_nodes_; idx++)
    if (genes[idx] == 0 && energies_[idx] != 0.0) { // active nodes
      partial_energy += energies_[idx];
      nb_active_nodes++;
    }

  auto coverage_info = regions_->GetCoverage(genes, energies_);

  float term1 = 0.0, term2 = 0.0, term3 = 0.0;
  if (total_energy_ != 0.0)
    term1 = partial_energy/total_energy_;

  if (coverage_info.total_coverage != 0.0)
    term2 = coverage_info.partial_coverage/coverage_info.total_coverage;

  if (nb_alive_nodes_ != 0)
    term3 = 1 - nb_active_nodes/float(nb_alive_nodes_);

  float fitness_val = fitness_alpha_*term1 +
                      fitness_beta_*term2  +
                      fitness_gamma_*term3;

  fitness_t fitness_ret = {.total = fitness_val,
                           .term1 = term1,
                           .term2 = term2,
                           .coverage_info = coverage_info};

  individual.SetFitness(fitness_ret);

  if (fitness_ret.total > best_global_.GetFitness().total)
    best_global_ = individual;

  return fitness_ret;
}

