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
    PushIntoLearningTraces(best_global_fitness_);
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

fitness_t
ModifiedPso::Fitness(const individual_t &individual) {
  vector<u_int> sleep_nodes;
  float partial_energy = 0.0;
  for (u_int gene_idx = 0; gene_idx < nb_nodes_; gene_idx++) {
    // push nodes that are dead
    if (energies_[gene_idx] != 0.0) {
      if (individual[gene_idx] == 1) { // sleeping nodes
        sleep_nodes.push_back(ids_[gene_idx]);
      } else {
        partial_energy += energies_[gene_idx];
      }
    }
  }

  // calculate sum(ei-avg(e)), sum(max(ei-avg(e),0)), sum(min(ei-avg(e),0))
  float average_energy = (nb_alive_nodes_==0) ? 0.0 : total_energy_/nb_alive_nodes_;
  float energies_dev = 0.0, neg_energy_dev = 0.0, pos_energy_dev = 0.0;
  for (u_int gene_idx = 0; gene_idx < nb_nodes_; gene_idx++) {
    float energy_dev = energies_[gene_idx] - average_energy;
    if (energies_[gene_idx] > 0.0) { //skip dead nodes
      if (individual[gene_idx] == 0) { //only for awake nodes
        energies_dev += energy_dev;
      }
      // for all alive nodes
      if (energy_dev < 0.0) {
        neg_energy_dev += energy_dev;
      } else {
        pos_energy_dev += energy_dev;
      }
    }
  }

  auto coverage_info = regions_->GetCoverage(individual, energies_);

  // new term1
  float total_weighted_energy = 0.0, partial_weighted_energy = 0.0;
  for (unsigned int idx = 0; idx < nb_nodes_; idx++) {
    if (energies_[idx] > 0.0) { //skip dead nodes
      float energy_dev = energies_[idx] - average_energy;
      if (individual[idx] == 0) { //only for awake nodes
        if (energy_dev < 0.0)
          partial_weighted_energy += energies_[idx];
        else
          partial_weighted_energy += energies_[idx];
      }
      if (energy_dev < 0.0)
        total_weighted_energy += energies_[idx];
      else
        total_weighted_energy += energies_[idx];
    }
  }

  float term1 = 0.0, term2 = 0.0;
  //if (pos_energy_dev-neg_energy_dev != 0.0)
  //  term1 = (energies_dev-neg_energy_dev)/(pos_energy_dev-neg_energy_dev);
  if (total_weighted_energy != 0.0)
    term1 = 1 - partial_weighted_energy/total_weighted_energy;

  if (coverage_info.total_coverage != 0.0)
    term2 = coverage_info.partial_coverage/coverage_info.total_coverage;
    //term2 = coverage_info.exclusive_area/coverage_info.total_coverage;


  float fitness_val = fitness_alpha_*term1 + fitness_beta_*term2;

  fitness_t  fitness_ret = {.total = fitness_val,
                            .term1 = term1,
                            .term2 = term2,
                            .coverage_info = coverage_info};
  return fitness_ret;

}

