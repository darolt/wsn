#include "optimizer.h"
#include <stdio.h>

using namespace std;

// public methods

Optimizer::Optimizer(dict_t exclusive, regions_t overlapping,
                     vector<u_int> ids, config_t config)
          :ids_(ids) {

  regions_ = new Regions(exclusive, overlapping);

  // config.first are all integers
  nb_individuals_ = config.first["NB_INDIVIDUALS"];
  max_iterations_ = config.first["MAX_ITERATIONS"];
  // config.second are all floats
  fitness_alpha_  = config.second["FITNESS_ALPHA"];
  fitness_beta_   = config.second["FITNESS_BETA"];
  fitness_gamma_  = config.second["FITNESS_GAMMA"];
  wmax_           = config.second["WMAX"];
  wmin_           = config.second["WMIN"];
  
  nb_nodes_       = ids.size();

  population_     = vector<individual_t>(nb_individuals_,
                                         individual_t(nb_nodes_, 0));
  best_locals_    = vector<individual_t>(nb_individuals_,
                                         individual_t(nb_nodes_, 0));
  best_global_    = individual_t(nb_nodes_, 0);
  best_local_fitness_ = vector<float>(nb_individuals_, 0.0);
}

Optimizer::~Optimizer() {
  delete regions_;
}

individual_t
Optimizer::Run(vector<float> energies) {
  ClearLearningTraces();
  InitializeSessionData(energies);

  // find all nodes that are susceptible to sleep (not dead neither ch)
  // depleted nodes and ch should cannot be taken into consideration
  vector<u_int> can_sleep;
  for(u_int idx = 0; idx < energies_.size(); idx++)
    if (energies_[idx] != 0.0)
      can_sleep.push_back(idx);

  CreatePopulation();
  UpdateFitness();
  Optimize(can_sleep);
  return best_global_;
}

void
Optimizer::CreatePopulation() {
  uniform_real_distribution<float> distribution(0.0, 1.0);
  for (auto &individual: population_)
    for (u_int node_idx = 0; node_idx < nb_nodes_; node_idx++) {
      float random = distribution(generator_);
      if ((energies_[node_idx] == 0))
        // cluster head cannot be put to sleep
        individual[node_idx] = 0;
      else
        individual[node_idx] = (random < 0.5) ? 1 : 0;
    } 
}

void
Optimizer::UpdateFitness() {
  for (u_int idx = 0; idx < nb_individuals_; idx++) {
    auto const &individual = population_[idx];
    auto fitness_ret = Fitness(individual);
    float individual_fitness = fitness_ret.total;
    if (individual_fitness > best_local_fitness_[idx]) {
      best_locals_[idx] = individual;
      best_local_fitness_[idx] = individual_fitness;
    }
    if (individual_fitness > best_global_fitness_.total) {
      best_global_ = individual;
      best_global_fitness_ = fitness_ret;
    }
  }
}

// default empty implementation
void
Optimizer::Optimize(const vector<u_int> &can_sleep) {
}

fitness_t
Optimizer::Fitness(const individual_t &individual) {
}

void
Optimizer::PrintIndividual(individual_t individual) {
  for (auto const &gene: individual) {
    printf("%d", gene);
  }
  printf("\n");
}

// setters & getters
void
Optimizer::SetAlpha(float value) {
  fitness_alpha_ = value;
}

void
Optimizer::SetBeta(float value) {
  fitness_beta_ = value;
}

void
Optimizer::SetGamma(float value) {
  fitness_gamma_ = value;
}

vector<float>
Optimizer::GetLearningTrace() {
  return learning_trace_;
}

vector<float>
Optimizer::GetTerm1Trace() {
  return term1_trace_;
}

vector<float>
Optimizer::GetTerm2Trace() {
  return term2_trace_;
}

float
Optimizer::GetBestCoverage() {
  auto coverage_info = best_global_fitness_.coverage_info;
  if (coverage_info.total_coverage   != 0.0)
    return coverage_info.partial_coverage/coverage_info.total_coverage;
  return 0.0;
}

float
Optimizer::GetBestOverlapping() {
  auto coverage_info = best_global_fitness_.coverage_info;
  if (coverage_info.total_overlapping != 0.0)
    return coverage_info.partial_overlapping/
           coverage_info.total_overlapping;
  return 0.0;
}

void
Optimizer::ClearLearningTraces() {
  learning_trace_.clear();
  term1_trace_.clear();
  term2_trace_.clear();
}

void 
Optimizer::InitializeSessionData(const float_v &energies) {
  energies_ = energies;
  total_energy_ = 0.0;
  best_global_fitness_.total = 0.0;
  nb_alive_nodes_ = 0;
  for (u_int idx = 0; idx < nb_nodes_; idx++) {
    total_energy_ += energies_[idx];
    if (energies_[idx] == 0.0)
      dead_nodes_.push_back(ids_[idx]);
    else
      nb_alive_nodes_++;
  }
  regions_->InitSession(energies_);
}

void
Optimizer::PushIntoLearningTraces(const fitness_t &fitness) {
  learning_trace_.push_back(fitness.total);
  term1_trace_.push_back(fitness.term1);
  term2_trace_.push_back(fitness.term2);
}
