#include <stdio.h>
#include <limits>
#include <algorithm>
#include "ecca.h"
#include "individual.h"

Ecca::Ecca(dict_t exclusive, regions_t overlapping,
           std::vector<unsigned int> ids, config_t config)
    :Optimizer(exclusive, overlapping, ids, config) {
}

Ecca::~Ecca() {
}

// public methods

individual_t
Ecca::Run(std::vector<float> energies) {
  ClearLearningTraces();
  InitializeSessionData(energies);

  CreatePopulation();
  auto fronts = FastNonDominatedSort(population_);
  float crossover_rate = 0.98;
  auto children = Reproduce(population_, crossover_rate);

  for (unsigned int it = 0; it < max_iterations_; it++) {
    PushIntoLearningTraces(best_global_.GetFitness());
    population_.insert(population_.end(), children.begin(), children.end());
    for (unsigned int idx = 0; idx<population_.size(); idx++)
      population_[idx].idx_ = idx;
    //auto new_population_fitness = population_fitness + children_fitness;
    fronts = FastNonDominatedSort(population_);
    auto parents = FindBestParents(fronts);
    std::random_shuffle(parents.begin(), parents.end());
    // TODO randomly select some parents
    population_ = children;
    children = Reproduce(parents, crossover_rate);
  } 

  population_.insert(population_.end(), children.begin(), children.end());
  for (unsigned int idx = 0; idx<population_.size(); idx++)
    population_[idx].idx_ = idx;
  fronts = FastNonDominatedSort(population_);
  auto parents = FindBestParents(fronts);

  return best_global_.GetGenes();
}

// private methods

std::vector<std::vector<Individual>>
Ecca::FastNonDominatedSort(std::vector<Individual> &population) {
  // each index in fronts represent a single rank, that contain all
  // individuals that have that rank
  std::vector<std::vector<Individual>> fronts;

  std::vector<unsigned int> dom_count =  
    std::vector<unsigned int>(population.size(), 0);
  std::vector<std::vector<unsigned int>> dom_set = 
    std::vector<std::vector<unsigned int>>(population.size(),
                                           std::vector<unsigned int>());

  // find front0 and calculate rank 0 (store it in every individual)
  std::vector<Individual> front0;
  for (unsigned int idx1 = 0; idx1<population.size(); idx1++) {
    auto &individual1 = population[idx1];
    for (unsigned int idx2 = 0; idx2<population.size(); idx2++) {
      if (idx1 == idx2)
        continue;
      auto &individual2 = population[idx2];
      if (Dominates(individual1, individual2))
        dom_set[idx1].push_back(idx2);
      else if (Dominates(individual2, individual1))
        dom_count[idx1]++;
    }
    if (dom_count[idx1] == 0) {
      population[idx1].rank_ = 0;
      front0.push_back(population[idx1]);
    }
  }
  fronts.push_back(front0);

  // calculate higher-ranked fronts
  unsigned int curr = 0;
  do {
    std::vector<Individual> next_front;
    for (auto &individual1: fronts[curr]) {
      for (auto &idx2: dom_set[individual1.idx_]) {
        auto individual2 = population[idx2];
        dom_count[individual2.idx_]--;
        if (dom_count[individual2.idx_] == 0) {
          individual2.rank_ = curr+1;
          next_front.push_back(individual2);
        }
      }
    }
    curr++;
    if (next_front.size() != 0) {
      fronts.push_back(next_front);
    }
  } while (curr < fronts.size());
  return fronts;
}

bool
Ecca::Dominates(Individual &individual1, Individual &individual2) {
  if (fitness_alpha_ != 0.0 && fitness_beta_ != 0.0) {
    if ((individual1.GetFitness().term1 > individual2.GetFitness().term1) &&
        (individual1.GetFitness().term2 > individual2.GetFitness().term2))
      return true;
  } else if (fitness_alpha_ != 0.0 && fitness_beta_ == 0.0) {
    if (individual1.GetFitness().term1 > individual2.GetFitness().term1)
      return true;
  } else if (fitness_alpha_ == 0.0 && fitness_beta_ != 0.0) {
    if (individual1.GetFitness().term2 > individual2.GetFitness().term2)
      return true;
  }
  return false;
}

std::vector<Individual>
Ecca::Reproduce(std::vector<Individual> &population, float crossover_rate) {
  std::vector<Individual> children;
  
  for (unsigned int idx1=0; idx1<population.size(); idx1++) {
    auto individual1 = population[idx1];
    unsigned int idx2 = (idx1%2==0) ? idx1+1 : idx1-1;
    auto individual2 = (idx2 == population.size()) ? population[0] : population[idx2];

    Individual child = Individual(idx1, individual1, individual2, crossover_rate, this);
    children.push_back(child);
    if (children.size() >= population.size()) 
      break;
  }

  return children;
}

std::vector<Individual>
Ecca::FindBestParents(std::vector<std::vector<Individual>> &fronts) {
  std::vector<Individual> offspring;
  unsigned int last_front = 0;

  for (auto &front: fronts) {
    CalculateCrowdingDistance(front);
    if (offspring.size()+front.size() > nb_individuals_)
      break;
    for (auto &individual: front)
      offspring.push_back(individual);
    last_front++;
  }

  unsigned int remaining = nb_individuals_-offspring.size();
  if (remaining > 0) {
    // sort according to crowd_dist_ (ascending)
    std::sort(fronts[last_front].begin(), fronts[last_front].end(),
              [](const Individual &lhs, const Individual &rhs) {
                return lhs.crowd_dist_ < rhs.crowd_dist_;
              });

    offspring.insert(offspring.end(), fronts[last_front].begin(),
                                      fronts[last_front].begin()+remaining);
  }

  return offspring;
}

void
Ecca::CalculateCrowdingDistance(std::vector<Individual> &group) {
  float max1 = 0.0, min1 = std::numeric_limits<float>::max();
  float max2 = 0.0, min2 = std::numeric_limits<float>::max();
  for (auto &individual: group) {
    individual.crowd_dist_ = 0.0;
    // find max and min fitness values in group (for each objective)
    if (individual.fitness_.term1 > max1)
      max1 = individual.fitness_.term1;
    if (individual.fitness_.term2 > max2)
      max2 = individual.fitness_.term2;
    if (individual.fitness_.term1 < min1)
      min1 = individual.fitness_.term1;
    if (individual.fitness_.term1 < min1)
      min1 = individual.fitness_.term1;
  }
  // calculates the ranges
  float rge1 = max1 - min1;
  float rge2 = max2 - min2;

  // first and last value are set to infinity
  group.front().crowd_dist_ = std::numeric_limits<float>::max();
  group.back().crowd_dist_  = std::numeric_limits<float>::max();

  if (rge1 != 0.0) {
    for (unsigned int idx=1; idx<group.size()-1; idx++) {
      group[idx].crowd_dist_ += (group[idx+1].fitness_.term1 - group[idx-1].fitness_.term1)/rge1;
    }  
  }
  if (rge2 != 0.0) {
    for (unsigned int idx=1; idx<group.size()-1; idx++) {
      group[idx].crowd_dist_ += (group[idx+1].fitness_.term2 - group[idx-1].fitness_.term2)/rge2;
    }  
  }
}

void
Ecca::CrowdedSorting(std::vector<Individual> &group) {
  for (unsigned int idx=0; idx< group.size()-1; idx++) {
    for (unsigned int idx=0; idx< group.size()-1; idx++) {
      if (group[idx].crowd_dist_ < group[idx+1].crowd_dist_) {
        // swap
        std::swap(group[idx], group[idx+1]);
      }
    }
  }
}


fitness_t
Ecca::Fitness(Individual &individual) {
  u_int nb_active_nodes = 0;
  auto genes = individual.GetGenes();
  for (unsigned int idx = 0; idx < nb_nodes_; idx++)
    if (genes[idx] == 0 && energies_[idx] != 0.0) // active nodes
      nb_active_nodes++;

  auto coverage_info = regions_->GetCoverage(genes, energies_);

  float total, term1 = 0.0, term2 = 0.0;
  if (nb_alive_nodes_ != 0)
    term1 = 1 - nb_active_nodes/float(nb_alive_nodes_);

  if (coverage_info.total_coverage != 0.0)
    term2 = coverage_info.partial_coverage/coverage_info.total_coverage;

  total = fitness_alpha_*term1 + fitness_beta_*term2;

  fitness_t fitness_ret = {.total = total,
                           .term1 = term1,
                           .term2 = term2,
                           .coverage_info = coverage_info};

  individual.SetFitness(fitness_ret);

  if (fitness_ret.total > best_global_.GetFitness().total)
    best_global_ = individual;

  return fitness_ret;
}

