#include "individual.h"
#include <stdio.h>
#include "optimizer.h"

// declaration of static members
//fitness_t Individual::best_global_;
//std::vector<char> Individual::best_genes_;
Optimizer* Individual::optimizer_;
//unsigned int Individual::fresh_run_;
std::default_random_engine Individual::generator_;


Individual::Individual() {
  idx_ = 0;
  optimizer_ = NULL;
  fitness_.total = 0.0;
}

Individual::Individual(unsigned int idx, Optimizer *container_handler) {
  optimizer_ = container_handler;
  idx_ = idx;
  auto nb_genes = optimizer_->nb_nodes_;
  genes_ = std::vector<char>(nb_genes, 0);
  //days_alive_ = 0;
  SampleNewGenes();
  container_handler->Fitness(*this);
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
    //days_alive_ = 1;
    fitness_ = father.GetFitness();
  } else {
    idx_ = idx;
    auto nb_genes = optimizer_->nb_nodes_;
    genes_ = std::vector<char>(nb_genes, 0);
    //ndays_alive_ = 0;

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
    //UpdateFitness();
    container_handler->Fitness(*this);
  }
}

Individual::~Individual() {
}

//void
//Individual::SetNewRun() {
//  best_global_.total = 0.0;
//}

//std::vector<char>
//Individual::GetBestGenes() {
//  return best_genes_;
//}
//
//fitness_t
//Individual::GetBestFitness() {
//  return best_global_;
//}

void
Individual::SampleNewGenes() {
  std::uniform_real_distribution<float> distribution(0.0, 1.0);
  for (unsigned int idx = 0; idx < genes_.size(); idx++) {
    float random = distribution(generator_);
    if (optimizer_->energies_[idx] == 0.0)
      genes_[idx] = 0;
    else
      genes_[idx] = (random < 0.5) ? 1 : 0;
  }
  //UpdateFitness();
}

// getters & setters
fitness_t
Individual::GetFitness() {
  return fitness_;
}

void
Individual::SetFitness(fitness_t value) {
  fitness_ = value;
}

std::vector<char>
Individual::GetGenes() {
  return genes_;
}

void
Individual::SetGenes(std::vector<char> value) {
  genes_ = value;
  optimizer_->Fitness(*this);
}

//fitness_t
//Individual::UpdateFitness() {
//  u_int nb_active_nodes = 0;
//  for (unsigned int idx = 0; idx < optimizer_->nb_nodes_; idx++)
//    if (genes_[idx] == 0 && optimizer_->energies_[idx] != 0.0) // active nodes
//      nb_active_nodes++;
//
//  auto coverage_info = optimizer_->regions_->GetCoverage(genes_, optimizer_->energies_);
//
//  float total, term1 = 0.0, term2 = 0.0;
//  if (nb_alive_nodes != 0)
//    term1 = 1 - nb_active_nodes/float(optimizer_->nb_alive_nodes_);
//
//  if (coverage_info.total_coverage != 0.0)
//    term2 = coverage_info.partial_coverage/coverage_info.total_coverage;
//
//  total = optimizer_->fitness_alpha_*term1 + optimizer_->fitness_beta_*term2;
//
//  fitness_ = {.total = total,
//              .term1 = term1,
//              .term2 = term2,
//              .coverage_info = coverage_info};
//
//  // update if best fitness was improved
//  if (days_alive_ == 0 || fitness_.total > best_fitness_.total)
//    best_fitness_ = fitness_;
//
//  if (fresh_run_ == 1 || fitness_.total > best_global_.total) {
//    best_global_ = fitness_;
//    best_genes_ = genes_;
//  }
//
//  days_alive_++;
//  fresh_run_ = 0;
//
//  return fitness_;
//}

