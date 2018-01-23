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
    PushIntoLearningTraces(best_global_.GetFitness());
    for (u_int idx = 0; idx < nb_individuals_; idx++) {
      Individual &particle = population_[idx];
      mutation_rate   = wmax_ - (wmax_-wmin_)*it/float(max_iterations_);
      crossover_rate1 = 1.0 - it/float(max_iterations_);
      crossover_rate2 = 1.0 - crossover_rate1;

      Mutate(particle, can_sleep, mutation_rate);
      if (distribution(generator_) < crossover_rate1) {
        Crossover(particle, best_locals_[idx]);
      }
      if (distribution(generator_) < crossover_rate2) {
        Crossover(particle, best_global_);
      }
      // TODO make mutation/crossover generate genes and pass it forward
      
      if (particle.GetFitness().total > best_locals_[idx].GetFitness().total)
        best_locals_[idx] = particle;
    }

  }
}

void
ModifiedPso::Mutate(Individual &individual, vector<u_int> can_sleep,
              float mutation_rate) {
  // mutate mutation_rate percent of particles' genes
  u_int nb_mutations = u_int(float(can_sleep.size())*mutation_rate);
  auto genes = individual.GetGenes();
  for(u_int count = 0; count < nb_mutations; count++) {
    uniform_int_distribution<int> distribution(0, can_sleep.size()-1);
    u_int mutated_idx = distribution(generator_);
    u_int mutated_gene = can_sleep[mutated_idx];
    //printf("%d %d\n", mutated_idx, mutated_gene);

    // flips idx
    genes[mutated_gene] = (genes[mutated_gene] == 0) ? 1 : 0;
    can_sleep.erase(can_sleep.begin() + mutated_idx);
  }
  individual.SetGenes(genes);

  return;
}

void
ModifiedPso::Crossover(Individual &individual1, Individual &individual2) {
  uniform_real_distribution<float> distribution(0.0, 1.0);

  auto genes1 = individual1.GetGenes();
  auto genes2 = individual2.GetGenes();
  for (u_int idx = 0; idx < nb_nodes_; idx++) {
    u_int origin = (distribution(generator_) < 0.5) ? 0 : 1;
    genes1[idx] = (origin) ? genes1[idx] : genes2[idx];
  }
  individual1.SetGenes(genes1);
  return;
}

fitness_t
ModifiedPso::Fitness(Individual &individual) {
  vector<u_int> sleep_nodes;
  float partial_energy = 0.0;
  auto genes = individual.GetGenes();
  for (u_int idx = 0; idx < nb_nodes_; idx++) {
    // push nodes that are dead
    if (energies_[idx] != 0.0) {
      if (genes[idx] == 1) { // sleeping nodes
        sleep_nodes.push_back(ids_[idx]);
      } else {
        partial_energy += energies_[idx];
      }
    }
  }

  // calculate sum(ei-avg(e)), sum(max(ei-avg(e),0)), sum(min(ei-avg(e),0))
  float average_energy = (nb_alive_nodes_==0) ? 0.0 : total_energy_/nb_alive_nodes_;
  float energies_dev = 0.0, neg_energy_dev = 0.0, pos_energy_dev = 0.0;
  for (u_int idx = 0; idx < nb_nodes_; idx++) {
    float energy_dev = energies_[idx] - average_energy;
    if (energies_[idx] > 0.0) { //skip dead nodes
      if (genes[idx] == 0) { //only for awake nodes
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

  auto coverage_info = regions_->GetCoverage(genes, energies_);

  // new term1
  float total_weighted_energy = 0.0, partial_weighted_energy = 0.0;
  for (unsigned int idx = 0; idx < nb_nodes_; idx++) {
    if (energies_[idx] > 0.0) { //skip dead nodes
      float energy_dev = energies_[idx] - average_energy;
      if (genes[idx] == 0) { //only for awake nodes
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

  //printf("fitness_val: %f", fitness_val);
  fitness_t  fitness_ret = {.total = fitness_val,
                            .term1 = term1,
                            .term2 = term2,
                            .coverage_info = coverage_info};

  individual.SetFitness(fitness_ret);

  if (fitness_ret.total > best_global_.GetFitness().total)
    best_global_ = individual;

  return fitness_ret;

}

