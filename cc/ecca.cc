#include "pso.h"
#include <stdio.h>

Ecca::Ecca(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config)
    :Optimizer(exclusive, overlapping, ids, config) {
}

Ecca::~Ecca() {
}

void
Ecca::InitializePopulation() {
  UpdateFitness()
}

void
Ecca::UpdateFitness() {
  Atualiza fitness para cada individuo
}

void
Ecca::Search() {
  Initialize();
  UpdateFitness()
  for iteration {
    NonDominatedSorting();
    ControlledElitism();
    UpdateFitness();
    Reproduction();
  }
}

void
Ecca::Reproduction(selected, crossover_rate) {
  children = []
  ate gerar children do tamanho da populacao {
    pega parents dos selected
    child = Crossover(parent0, parent1, crossover_rate);
    Mutate(child, mutation_rate);
    append child to children
}





void
Ecca::Optimize(float_v energies, const vector<u_int> &can_sleep,
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

  float recombination_prob, mutation_prob, reduction_rate;
  for (u_int it = 0; it < MAX_ITERATIONS_; it++) {
    learning_trace_.push_back(best_global_fitness_);
    for (u_int individual_idx = 0; individual_idx < NB_INDIVIDUALS_; individual_idx++) {
      individual_t &individual = individuals_[individual_idx];
      //acceleration   = WMAX_ - (WMAX_-WMIN_)*it/float(MAX_ITERATIONS_);
      //phi1 = 1.0 - it/float(MAX_ITERATIONS_);
      //phi2 = 1.0 - phi1;

      
      for(u_int gene_idx = 0; gene_idx < nb_nodes_; gene_idx++) {
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

      

      //float r1 = distribution(generator_);
      //float r2 = distribution(generator_);

      //auto new_individual = individuals_[individual_idx];
      //Influence(individual, best_locals_[individual_idx],
      //          new_individual, can_sleep, phi1*r1);
      //Influence(individual, best_global_, new_individual, can_sleep, phi2*r2);
      //individual = new_individual;
      //Move(individual, can_sleep, acceleration);
    }

    UpdateGenerationFitness(energies, total_energy);

  }
}

void
Ecca::Move(individual_t &individual, vector<u_int> can_sleep,
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
Ecca::Influence(const individual_t &original_individual,
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

