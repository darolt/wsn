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
Optimizer::Run(vector<float> energies, u_int head_id) {
  // for tracing the learning of pso
  learning_trace_.clear();

  // initialize session data
  energies_ = energies;
  head_id_ = head_id;
  total_energy_ = 0.0;
  best_global_fitness_ = 0.0;
  for (auto const &energy: energies_)
    total_energy_ += energy;

  // find all nodes that are susceptible to sleep (not dead neither ch)
  // depleted nodes and ch should cannot be taken into consideration
  vector<u_int> can_sleep;
  for(u_int idx = 0; idx < energies_.size(); idx++)
    if (energies_[idx] != 0 && ids_[idx] != head_id_)
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
      if ((ids_[node_idx] == head_id_) || (energies_[node_idx] == 0))
        // cluster head cannot be put to sleep
        individual[node_idx] = 0;
      else
        individual[node_idx] = (random < 0.5) ? 1 : 0;
    } 
}

population_fitness_t
Optimizer::CalculateFitness(vector<individual_t> &group) {
  vector<fitness_ret_t> group_fitness;
  for (u_int idx = 0; idx < group.size(); idx++) {
    auto const &individual = group[idx];
    auto fitness_ret = Fitness(individual, 0);
    group_fitness.push_back(fitness_ret);
  }
  return group_fitness;
}

void
Optimizer::UpdateFitness() {
  for (u_int idx = 0; idx < nb_individuals_; idx++) {
    auto const &individual = population_[idx];
    auto fitness_ret = Fitness(individual, 0);
    float individual_fitness = fitness_ret.total;
    if (individual_fitness > best_local_fitness_[idx]) {
      best_locals_[idx] = individual;
      best_local_fitness_[idx] = individual_fitness;
    }
    if (individual_fitness > best_global_fitness_) {
      best_global_ = individual;
      best_global_fitness_ = individual_fitness;
      auto coverage_info = fitness_ret.coverage_info;
      if (coverage_info.partial_coverage == 0.0 &&
          coverage_info.total_coverage   == 0.0)
        best_coverage_ = 0.0;
      else
        best_coverage_    = coverage_info.partial_coverage/
                            coverage_info.total_coverage;
      
      if (coverage_info.partial_overlapping == 0.0 &&
          coverage_info.total_overlapping   == 0.0)
        best_overlapping_ = 0.0;
      else
        best_overlapping_ = coverage_info.partial_overlapping/
                            coverage_info.total_overlapping;
    }
  }
}

// default empty implementation
void
Optimizer::Optimize(const vector<u_int> &can_sleep) {
  printf("Calling Optimizer::Optimize!");
}

fitness_ret_t
Optimizer::Fitness(const individual_t &individual, char do_print) {
  vector<u_int> sleep_nodes;
  float partial_energy = 0.0;
  vector<u_int> dead_nodes;
  u_int alive_nodes = 0;
  for (u_int gene_idx = 0; gene_idx < nb_nodes_; gene_idx++) {
    // push nodes that are dead
    if (energies_[gene_idx] == 0 ) {
      dead_nodes.push_back(ids_[gene_idx]);
    } else if (individual[gene_idx] == 1) { // sleeping nodes
      sleep_nodes.push_back(ids_[gene_idx]);
      alive_nodes++;
    } else {
      partial_energy += energies_[gene_idx];
      alive_nodes++;
    }
  }

  // calculate sum(ei-avg(e)), sum(max(ei-avg(e),0)), sum(min(ei-avg(e),0))
  float average_energy = (alive_nodes==0) ? 0.0 : total_energy_/alive_nodes;
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

  auto coverage_info = regions_->GetCoverage(sleep_nodes, dead_nodes);

  // TODO add try catch here
  float term1;
  if (pos_energy_dev-neg_energy_dev == 0.0) {
    term1 = 0.0;
  } else {
    //term1 = FITNESS_ALPHA_*(1.0-partial_energy/total_energy_);
    //printf("%f %f %f\n", energies__dev, neg_energy_dev, pos_energy_dev);
    term1 = (energies_dev-neg_energy_dev)/(pos_energy_dev-neg_energy_dev);
  }
  float term2;
  if (coverage_info.total_coverage == 0) {
    term2 = 0.0;
  } else { // gamma*(1- partial_overlapping/total_overlapping)
    term2 = coverage_info.exclusive_area/coverage_info.total_coverage;
  }

  float fitness_val = fitness_alpha_*term1 + fitness_beta_*term2;
  if (do_print == 1) {
    printf("fitness %f, 1: %f, 2: %f\n", fitness_val, term1, term2);
  }

  fitness_ret_t  fitness_ret = {.total = fitness_val,
                                .term1 = term1,
                                .term2 = term2,
                                .coverage_info = coverage_info};
  return fitness_ret;
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

vector<float>
Optimizer::GetLearningTrace() {
  return learning_trace_;
}

float
Optimizer::GetBestCoverage() {
  return best_coverage_;
}

float
Optimizer::GetBestOverlapping() {
  return best_overlapping_;
}

