#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <vector>
#include <random>
#include "types.h"

class Optimizer;

class Individual {
  public:
    Individual();
    Individual(unsigned int idx, Optimizer *container_handler);
    // copy constructor
    //Individual(const Individual &individual);
    Individual(unsigned int idx, Individual &father,
               Individual &mother, float crossover_rate,
               Optimizer *container_handler);
    ~Individual();

    fitness_t GetFitness() const;
    void SetFitness(fitness_t value);
    std::vector<char> GetGenes();
    void SetGenes(std::vector<char> value);
    //static void SetNewRun();
    //static std::vector<char> GetBestGenes();
    //static fitness_t GetBestFitness();

    // used by NSGA-II to:
    // indicate front membership
    unsigned int rank_;
    unsigned int idx_;
    // value of the crowding distance
    float crowd_dist_;

    fitness_t fitness_;

    // best fitness and genes in family lineage
    //fitness_t best_fitness_;
    //std::vector<char> best_genes;

    // best fitness and genes in population's history
    //static fitness_t best_global_fitness_;
    //static std::vector<char> best_global_genes_;

  private:
    std::vector<char> genes_;

    // All individuals share a handler to optimizer
    static Optimizer *optimizer_;

    static std::default_random_engine generator_;

    //fitness_t UpdateFitness();
    void SampleNewGenes();
};
#endif //INDIVIDUAL_H

