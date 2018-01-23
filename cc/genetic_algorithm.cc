#include "genetic_algorithm.h"
#include <stdio.h>
#include <algorithm>

GeneticAlgorithm::GeneticAlgorithm(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config)
                 :Optimizer(exclusive, overlapping, ids, config) {
}

GeneticAlgorithm::~GeneticAlgorithm() {
}

void
GeneticAlgorithm::Optimize(const vector<u_int> &can_sleep) {
  // lod stands for Local Optimum Detector
  // it counts the number of iterations where the Optimizer is stuck
  unsigned int lod = 0;
  SortFitness();

  float mutation_rate, crossover_rate;
  for (u_int it = 0; it < max_iterations_; it++) {
    float previous_best_global = best_global_.GetFitness().total;
    crossover_rate = 1.0 - it/float(max_iterations_);
    if (lod >= 3)
      mutation_rate = 1.0;
    else
      mutation_rate = wmax_ - (wmax_-wmin_)*it/float(max_iterations_);

    PushIntoLearningTraces(best_global_.GetFitness());

    // only worst fit individuals are replaced
    // best fit are cloned for the next generation
    u_int nb_unfit = 0.6*nb_individuals_; // selection_rate
    uniform_real_distribution<float> distribution(0.0, 1.0);
    for (u_int idx = nb_unfit; idx < population_.size(); idx++) {
      auto &individual = population_[idx];

      if (distribution(generator_) < crossover_rate)
        individual = Crossover(nb_unfit, can_sleep);

      Mutate(individual, can_sleep, mutation_rate);
    }

    SortFitness();

    if (best_global_.GetFitness().total == previous_best_global)
      lod++;
    else
      lod = 0;
  }
}

void
GeneticAlgorithm::SortFitness() {
  sort(population_.begin(),
       population_.end(),
       [](const Individual &left,
          const Individual &right) {
         return left.GetFitness().total > right.GetFitness().total;
       }
  );

}

void
GeneticAlgorithm::Mutate(Individual &individual,
                         vector<u_int> can_sleep,
                         float mutation_rate) {

  u_int nb_changes = u_int(float(can_sleep.size())*mutation_rate);
  auto genes = individual.GetGenes();
  for(u_int count = 0; count < nb_changes; count++) {
    uniform_int_distribution<int> distribution(0, can_sleep.size()-1);
    u_int changed_idx = distribution(generator_);
    u_int changed_gene = can_sleep[changed_idx];

    // flips gene
    genes[changed_gene] = (genes[changed_gene] == 0) ? 1 : 0;
    can_sleep.erase(can_sleep.begin() + changed_idx);
  }
  individual.SetGenes(genes);
}

Individual
GeneticAlgorithm::Crossover(u_int nb_unfit,
                            const vector<u_int> &can_sleep ) {

  uniform_real_distribution<float> distribution(0.0, 1.0);
  // choose father and mother from best fit individuals
  uniform_int_distribution<int> int_distribution(0, nb_unfit);
  // father and mother may be the same individual
  auto &father = population_[int_distribution(generator_)];
  auto &mother = population_[int_distribution(generator_)];

  // half of the genes comes from father and half from mother
  Individual child = Individual(0, this);
  auto genes = child.GetGenes();
  for (auto const &gene: can_sleep) {
    float random = distribution(generator_);
    genes[gene] = (random < 0.5) ? father.GetGenes()[gene]: mother.GetGenes()[gene];
  }
  child.SetGenes(genes);
  return child;
}

fitness_t
GeneticAlgorithm::Fitness(Individual &individual) {
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

