#include "pso.h"
#include <stdio.h>

Ecca::Ecca(dict_t exclusive, regions_t overlapping,
         vector<u_int> ids, config_t config) {

  regions_ = new Regions(exclusive, overlapping);

  NB_INDIVIDUALS_ = config.first["NB_INDIVIDUALS"];
  MAX_ITERATIONS_ = config.first["MAX_ITERATIONS"];
  FITNESS_ALPHA_  = config.second["FITNESS_ALPHA"];
  FITNESS_BETA_   = config.second["FITNESS_BETA"];
  WMAX_           = config.second["WMAX"];
  WMIN_           = config.second["WMIN"];
  
  nb_nodes_       = ids.size();

  population_    = vector<individual_t>(NB_INDIVIDUALS_,
                                         individual_t(nb_nodes_, 0));
  best_locals_    = vector<individual_t>(NB_INDIVIDUALS_,
                                         individual_t(nb_nodes_, 0));
  best_global_    = individual_t(nb_nodes_, 0);
  best_local_fitness_ = vector<float>(NB_INDIVIDUALS_, 0.0);

}

Ecca::~Ecca() {
  delete regions_;
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


individual_t
Optimizer::Run(vector<float> energies, u_int head_id) {
}

void
Optimizer::InitializePopulation(float_v energies, u_int head_id, float total_energy) {
  uniform_real_distribution<float> distribution(0.0, 1.0);
  for (auto &individual: population_) {
    for (u_int node_idx = 0; node_idx < nb_nodes_; node_idx++) {
      float random = distribution(generator_);
      if ((ids_[node_idx] == head_id) || (energies[node_idx] == 0)) {
        // cluster head cannot be put to sleep
        individual[node_idx] = 0;
      } else {
        individual[node_idx] = (random < 0.5) ? 1 : 0;
      }
    } 
  }
  UpdateGenerationFitness(energies, total_energy);
}

void
Optimizer::UpdateGenerationFitness(float_v energies, float total_energy) {
  for (u_int idx = 0; idx < NB_INDIVIDUALS_; idx++) {
    auto const &individual = population_[idx];
    auto fitness_ret = Fitness(individual, energies, total_energy, 0);
    float individual_fitness = fitness_ret.fitness_value;
    if (individual_fitness > best_local_fitness_[idx]) {
      best_locals_[idx] = individual;
      best_local_fitness_[idx] = individual_fitness;
    }
    if (individual_fitness > best_global_fitness_) {
      best_global_ = individual;
      best_global_fitness_ = individual_fitness;
      best_coverage_ = fitness_ret.coverage_info.partial_coverage/
                       fitness_ret.coverage_info.total_coverage;
      best_overlapping_ = fitness_ret.coverage_info.partial_overlapping/
                          fitness_ret.coverage_info.total_overlapping;
    }
  }
}

fitness_ret_t
Optimizer::Fitness(const individual_t &individual, vector<float> energies,
                   float total_energy, char do_print) {
  vector<u_int> sleep_nodes;
  float partial_energy = 0.0;
  vector<u_int> dead_nodes;
  //float threshold = 0.005;
  u_int alive_nodes = 0;
  for (u_int gene_idx = 0; gene_idx < nb_nodes_; gene_idx++) {
    // push nodes that are dead
    if (energies[gene_idx] == 0 ) {
      dead_nodes.push_back(ids_[gene_idx]);
    } else if (individual[gene_idx] == 1) { // sleeping nodes
      sleep_nodes.push_back(ids_[gene_idx]);
      alive_nodes++;
    } else {
      partial_energy += energies[gene_idx];
      alive_nodes++;
    }
  }

  // calculate sum(ei-avg(e)), sum(max(ei-avg(e),0)), sum(min(ei-avg(e),0))
  float average_energy = (alive_nodes==0) ? 0.0 : total_energy/alive_nodes;
  float energies_dev = 0.0, neg_energy_dev = 0.0, pos_energy_dev = 0.0;
  for (u_int gene_idx = 0; gene_idx < nb_nodes_; gene_idx++) {
    float energy_dev = energies[gene_idx] - average_energy;
    if (energies[gene_idx] > 0.0) { //skip dead nodes
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

  auto coverage_info = regions_->GetAll(sleep_nodes, dead_nodes);

  // TODO add try catch here
  float term1;
  if (pos_energy_dev-neg_energy_dev == 0.0) {
    term1 = 0.0;
  } else {
    //term1 = FITNESS_ALPHA_*(1.0-partial_energy/total_energy);
    //printf("%f %f %f\n", energies_dev, neg_energy_dev, pos_energy_dev);
    term1 = (energies_dev-neg_energy_dev)/(pos_energy_dev-neg_energy_dev);
  }
  float term2;
  if (coverage_info.total_coverage == 0) {
    term2 = 0.0;
  } else { // gamma*(1- partial_overlapping/total_overlapping)
    term2 = coverage_info.exclusive_area/coverage_info.total_coverage;
  }

  float fitness_val = FITNESS_ALPHA_*term1 + FITNESS_BETA_*term2;
  if (do_print == 1) {
    printf("fitness %f, 1: %f, 2: %f\n", fitness_val, term1, term2);
  }

  fitness_ret_t  fitness_ret = {.fitness_value = fitness_val,
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

