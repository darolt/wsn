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
Ecca::Run(std::vector<float> energies, unsigned int head_id) {
  ClearLearningTraces();
  InitializeSessionData(energies, head_id);

  // find all nodes that are susceptible to sleep (not dead neither ch)
  // depleted nodes and ch should cannot be taken into consideration
  std::vector<unsigned int> can_sleep;
  for(unsigned int idx = 0; idx < energies.size(); idx++)
    if (energies[idx] != 0 && ids_[idx] != head_id)
      can_sleep.push_back(idx);


  // reset best global and best locals
  Individual::SetNewRun();
  auto population = CreatePopulation1();
  //printf("population size: %d\n", population.size());
  //auto population_fitness = CalculateFitness(energies, total_energy, population);
  auto fronts = FastNonDominatedSort(population);
  //printf("fronts size: %d\n", fronts.size());
  float crossover_rate = 0.98;
  auto children = Reproduce(population, crossover_rate);
  //printf("children size: %d\n", children.size());
  //auto children_fitness = CalculateFitness(energies, total_energy, children);

  for (unsigned int it = 0; it < max_iterations_; it++) {
    PushIntoLearningTraces(Individual::GetBestFitness());
    //printf("best fitness: %f\n", Individual::GetBestFitness().total);
    population.insert(population.end(), children.begin(), children.end());
    //printf("population size: %d\n", population.size());
    for (unsigned int idx = 0; idx<population.size(); idx++)
      population[idx].idx_ = idx;
    //auto new_population_fitness = population_fitness + children_fitness;
    fronts = FastNonDominatedSort(population);
    //printf("fronts size: %d\n", fronts.size());
    auto parents = FindBestParents(fronts);
    //printf("parents size: %d\n", parents.size());
    std::random_shuffle(parents.begin(), parents.end());
    //printf("parents size: %d\n", parents.size());
    // TODO randomly select some parents
    population = children;
    children = Reproduce(parents, crossover_rate);
    //printf("children size: %d\n", children.size());
    //updateFitness(children);    
  } 

  population.insert(population.end(), children.begin(), children.end());
  for (unsigned int idx = 0; idx<population.size(); idx++)
    population[idx].idx_ = idx;
  fronts = FastNonDominatedSort(population);
  auto parents = FindBestParents(fronts);

  return Individual::GetBestGenes();
}


// private methods

std::vector<Individual>
Ecca::CreatePopulation1() {
  std::vector<Individual> population;
  for (unsigned int idx = 0; idx < nb_individuals_; idx++) {
    population.push_back(Individual(idx, this));
  }
  return population;
}

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
  //printf("front0 size %d\n", front0.size());
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
  ////printf("i11 : %f\n", individual1.GetFitness().term1);
  ////printf("i12 : %f\n", individual1.GetFitness().term2);
  ////printf("i21 : %f\n", individual2.GetFitness().term1);
  ////printf("i22 : %f\n", individual2.GetFitness().term2);
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
        //Individual swap(group[idx]);
        //group[idx] = group[idx+1];
        //group[idx] = swap;
        std::swap(group[idx], group[idx+1]);
      }
    }
  }
}

float
Ecca::GetBestOverlapping() {
  auto coverage_info = Individual::GetBestFitness().coverage_info;
  if (coverage_info.partial_overlapping == 0.0 &&
      coverage_info.total_overlapping   == 0.0)
    best_overlapping_ = 0.0;
  else
    best_overlapping_ = coverage_info.partial_overlapping/
                        coverage_info.total_overlapping;

  return best_overlapping_;
}

float
Ecca::GetBestCoverage() {
  auto coverage_info = Individual::GetBestFitness().coverage_info;
  if (coverage_info.partial_coverage == 0.0 &&
      coverage_info.total_coverage   == 0.0)
    best_coverage_ = 0.0;
  else
    best_coverage_    = coverage_info.partial_coverage/
                        coverage_info.total_coverage;
  return best_coverage_;
}

