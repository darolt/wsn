#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <vector>
#include <random>
#include "optimizer.h"

typedef struct {
  float total;
  float term1;
  float term2;
  coverage_info_t coverage_info;
} fitness_t;


class Individual {
  public:
    //Individual();
    Individual(unsigned int idx, Optimizer *container_handler);
    // copy constructor
    //Individual(const Individual &individual);
    Individual(unsigned int idx, Individual &father,
               Individual &mother, float crossover_rate,
               Optimizer *container_handler);
    ~Individual();

    fitness_t GetFitness();
    std::vector<char> GetGenes();
    static void SetNewRun();
    static std::vector<char> GetBestGenes();
    static fitness_t GetBestFitness();

    // used by NSGA-II to:
    // indicate front membership
    unsigned int rank_;
    unsigned int idx_;
    fitness_t fitness_;
    // value of the crowding distance
    float crowd_dist_;

  private:
    std::vector<char> genes_;
    // best fitness in individual's history
    fitness_t best_fitness_;

    unsigned int days_alive_;
    static unsigned int fresh_run_;

    // best fitness and genes in population's history
    static fitness_t best_global_;
    static std::vector<char> best_genes_;

    // Individuals must be
    static Optimizer *optimizer_;

    static std::default_random_engine generator_;

    fitness_t Fitness();
    void SampleNewGenes();
};
#endif //INDIVIDUAL_H
