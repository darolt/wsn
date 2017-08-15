#include "individual.h"
#include <stdio.h>

// declaration of static members
fitness_t Individual::best_global_;
std::vector<char> Individual::best_genes_;
Optimizer* Individual::optimizer_;
unsigned int Individual::fresh_run_;
std::default_random_engine Individual::generator_;

Individual::Individual(unsigned int idx, Optimizer *container_handler) {
  //printf("ind 1\n");
  optimizer_ = container_handler;
  idx_ = idx;
  auto nb_genes = optimizer_->nb_nodes_;
  genes_ = std::vector<char>(nb_genes, 0);
  days_alive_ = 0;
  SampleNewGenes();
  //printf("ind 2\n");
}

Individual::Individual(unsigned int idx, Individual &father,
                       Individual &mother, float crossover_rate,
                       Optimizer *container_handler) { 
  optimizer_ = container_handler;
  std::uniform_real_distribution<float> distribution(0.0, 1.0);
  if (distribution(generator_) > crossover_rate) {
    // copy father
    idx_ = father.idx_;
    genes_ = father.GetGenes();
    days_alive_ = 1;
    fitness_ = father.GetFitness();
  } else {
    idx_ = idx;
    auto nb_genes = optimizer_->nb_nodes_;
    genes_ = std::vector<char>(nb_genes, 0);
    days_alive_ = 0;

    // gets half of genes from father and half from mother (probability = 0.5)
    for (unsigned int idx = 0; idx < genes_.size(); idx++)
      if (distribution(generator_) > 0.5)
        genes_[idx] = father.GetGenes()[idx];
      else
        genes_[idx] = father.GetGenes()[idx];

    // mutate genes (logical flip) with probability 1/genes_.size()
    for (auto &gene: genes_)
      if (distribution(generator_) < float(1/float(genes_.size())))
        gene = (gene == 0) ? 1 : 0;
    Fitness();
  }
}

Individual::~Individual() {
}

void
Individual::SetNewRun() {
  fresh_run_ = 1;
}

std::vector<char>
Individual::GetBestGenes() {
  return best_genes_;
}


fitness_t
Individual::GetBestFitness() {
  return best_global_;
}

void
Individual::SampleNewGenes() {
  //printf("samplenewgenes 1\n");
  std::uniform_real_distribution<float> distribution(0.0, 1.0);
  for (unsigned int idx = 0; idx < genes_.size(); idx++) {
    float random = distribution(generator_);
    if ((optimizer_->ids_[idx] == optimizer_->head_id_) || 
        (optimizer_->energies_[idx] == 0.0))
      // cluster head cannot be put to sleep
      genes_[idx] = 0;
    else
      genes_[idx] = (random < 0.5) ? 1 : 0;
  }
  Fitness();
  //printf("samplenewgenes 2\n");
}

// getters & setters
fitness_t
Individual::GetFitness() {
  return fitness_;
}

std::vector<char>
Individual::GetGenes() {
  return genes_;
}

fitness_t
Individual::Fitness() {
  //printf("fitness 1\n");
  std::vector<unsigned int> sleep_nodes;
  std::vector<unsigned int> dead_nodes;
  unsigned int alive_nodes = 0;
  //printf("sleep: ");
  for (unsigned int idx = 0; idx < optimizer_->nb_nodes_; idx++) {
    // push nodes that are dead
    if (optimizer_->energies_[idx] == 0 ) {
      dead_nodes.push_back(optimizer_->ids_[idx]);
    } else {
      alive_nodes++;
      if (genes_[idx] == 1) // sleeping nodes
        sleep_nodes.push_back(optimizer_->ids_[idx]);
    }
  }
        //printf("\n");

  // calculate sum(ei-avg(e)), sum(max(ei-avg(e),0)), sum(min(ei-avg(e),0))
  float average_energy = (alive_nodes==0) ? 0.0 : optimizer_->total_energy_/alive_nodes;
  float energies_dev = 0.0, neg_energy_dev = 0.0, pos_energy_dev = 0.0;
  for (unsigned int idx = 0; idx < optimizer_->nb_nodes_; idx++) {
    //printf("ene: %f\n", optimizer_->energies_[idx]);
    if (optimizer_->energies_[idx] > 0.0) { //skip dead nodes
      float energy_dev = optimizer_->energies_[idx] - average_energy;
      if (genes_[idx] == 0) //only for awake nodes
        energies_dev += energy_dev;

      // for all alive nodes
      if (energy_dev < 0.0)
        neg_energy_dev += energy_dev;
      else
        pos_energy_dev += energy_dev;
    }
  }

  auto coverage_info = optimizer_->regions_->GetCoverage(sleep_nodes, dead_nodes);

  //printf("ene_dev %f\n", energies_dev);
  //printf("neg_dev %f\n", neg_energy_dev);
  //printf("pos_dev %f\n", pos_energy_dev);
  float total, term1 = 0.0, term2 = 0.0;
  if (pos_energy_dev-neg_energy_dev != 0.0)
    term1 = (energies_dev-neg_energy_dev)/(pos_energy_dev-neg_energy_dev);
  //printf("term1: %f\n", term1);

  if (coverage_info.total_coverage != 0)
    term2 = coverage_info.exclusive_area/coverage_info.total_coverage;

  //printf("term2: %f\n", term2);
  total = optimizer_->fitness_alpha_*term1 + optimizer_->fitness_beta_*term2;

  fitness_ = {.total = total,
              .term1 = term1,
              .term2 = term2,
              .coverage_info = coverage_info};

  if (days_alive_ == 0 || fitness_.total > best_fitness_.total)
    best_fitness_ = fitness_;

  if (fresh_run_ == 1 || fitness_.total > best_global_.total) {
    best_global_ = fitness_;
    best_genes_ = genes_;
  }

  days_alive_++;
  fresh_run_ = 0;

  //printf("fitness 2\n");
  return fitness_;
}

