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
    sorted_fitness.push_back(make_pair(idx, population_[idx].GetFitness().total));
  }
  SortFitness(sorted_fitness);

  float mutation_rate, crossover_rate;
  for (u_int it = 0; it < max_iterations_; it++) {
    mutation_rate   = wmax_ - (wmax_-wmin_)*it/float(max_iterations_);
    crossover_rate = 1.0 - it/float(max_iterations_);

    PushIntoLearningTraces(best_global_.GetFitness());

    // only worst fit individuals are replaced
    // best fit are cloned for the next generation
    u_int nb_unfit = 0.6*nb_individuals_; // selection_rate
    for (u_int sorted_idx = 0; sorted_idx < nb_unfit; sorted_idx++) {
      auto individual_idx = sorted_fitness[sorted_idx].first;
      auto &individual = population_[individual_idx];

      Crossover(individual, sorted_fitness, nb_unfit, can_sleep, crossover_rate);

      Mutate(individual, can_sleep, mutation_rate);
    }

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

void
GeneticAlgorithm::Crossover(Individual &child,
                            const vector<pair<u_int, float>> &sorted_fitness,
                            u_int nb_unfit,
                            const vector<u_int> &can_sleep,
                            float crossover_rate) {

  uniform_real_distribution<float> distribution(0.0, 1.0);
  if (distribution(generator_) < crossover_rate) {
    // choose father and mother from best fit individuals
    uniform_int_distribution<int> int_distribution(0, nb_unfit);
    // father and mother may be the same individual
    auto father_idx = sorted_fitness[int_distribution(generator_)].first;
    auto &father = population_[father_idx];
    auto mother_idx = sorted_fitness[int_distribution(generator_)].first;
    auto &mother = population_[mother_idx];

    // half of the genes comes from father and half from mother
    auto genes = child.GetGenes();
    for (auto const &gene: can_sleep) {
      float random = distribution(generator_);
      genes[gene] = (random < 0.5) ? father.GetGenes()[gene]: mother.GetGenes()[gene];
    }
    child.SetGenes(genes);
  } 
}

fitness_t
GeneticAlgorithm::Fitness(Individual &individual) {
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

  fitness_t  fitness_ret = {.total = fitness_val,
                            .term1 = term1,
                            .term2 = term2,
                            .coverage_info = coverage_info};

  if (fitness_ret.total > best_global_.GetFitness().total)
    best_global_ = individual;

  individual.SetFitness(fitness_ret);

  return fitness_ret;

}

