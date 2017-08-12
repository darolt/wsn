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
  // create sorted_fitness and keep it sorted
  // used for sorting least fit individuals faster
  // this implements GA selection implicitly
  vector<pair<u_int, float>> sorted_fitness;
  for (u_int idx = 0; idx < nb_individuals_; idx++) {
    sorted_fitness.push_back(make_pair(idx, best_local_fitness_[idx]));
  }
  SortFitness(sorted_fitness);

  float mutation_rate, crossover_rate;
  for (u_int it = 0; it < max_iterations_; it++) {
    mutation_rate   = wmax_ - (wmax_-wmin_)*it/float(max_iterations_);
    crossover_rate = 1.0 - it/float(max_iterations_);

    learning_trace_.push_back(best_global_fitness_);

    // only worst fit individuals are replaced
    // best fit are cloned for the next generation
    u_int nb_unfit = 0.6*can_sleep.size(); // selection_rate
    for (u_int sorted_idx = 0; sorted_idx < nb_unfit; sorted_idx++) {
      auto individual_idx = sorted_fitness[sorted_idx].first;
      auto &individual = population_[individual_idx];

      Crossover(individual, sorted_fitness, nb_unfit, can_sleep, crossover_rate);

      Mutate(individual, can_sleep, mutation_rate);
    }

    UpdateFitness();
    SortFitness(sorted_fitness);
  }
}

void
GeneticAlgorithm::SortFitness(vector<pair<u_int, float>> &sorted_fitness) {
  sort(sorted_fitness.begin(),
       sorted_fitness.end(),
       [](const std::pair<u_int,float> &left,
          const std::pair<u_int,float> &right) {
         return left.second > right.second;
       }
  );
}

void
GeneticAlgorithm::Mutate(individual_t &individual,
                         vector<u_int> can_sleep,
                         float mutation_rate) {

  u_int nb_changes = u_int(float(can_sleep.size())*mutation_rate);
  for(u_int count = 0; count < nb_changes; count++) {
    uniform_int_distribution<int> distribution(0, can_sleep.size()-1);
    u_int changed_idx = distribution(generator_);
    u_int changed_gene = can_sleep[changed_idx];

    // flips gene
    individual[changed_gene] = (individual[changed_gene] == 0) ? 1 : 0;
    can_sleep.erase(can_sleep.begin() + changed_idx);
  }
}

void
GeneticAlgorithm::Crossover(individual_t &child,
                            const vector<pair<u_int, float>> &sorted_fitness,
                            u_int nb_unfit,
                            const vector<u_int> &can_sleep,
                            float crossover_rate) {

  uniform_real_distribution<float> distribution(0.0, 1.0);
  if (distribution(generator_) < crossover_rate) {
    // choose father and mother from best fit individuals
    uniform_int_distribution<int> int_distribution(nb_unfit, nb_individuals_);
    // father and mother may be the same individual
    auto father_idx = sorted_fitness[int_distribution(generator_)].first;
    auto &father = population_[father_idx];
    auto mother_idx = sorted_fitness[int_distribution(generator_)].first;
    auto &mother = population_[mother_idx];

    // half of the genes comes from father and half from mother
    for (auto const &gene: can_sleep) {
      float random = distribution(generator_);
      child[gene] = (random < 0.5) ? father[gene]: mother[gene];
    }
  } /*else {
    // TODO if not crossing over, clone individual
    // maybe it is better to create new individual
  }*/
}

