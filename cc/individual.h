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

    // used by NSGA-II to:
    // indicate front membership
    unsigned int rank_;
    unsigned int idx_;
    fitness_t fitness_;
    // value of the crowding distance
    float crowd_dist_;

  private:
    std::vector<char> genes_;
    fitness_t best_fitness_;

    unsigned int days_alive_;
    static unsigned int fresh_run_;

    static fitness_t best_global_;
    static std::vector<char> best_genes_;

    // Individuals must be
    static Optimizer *optimizer_;

    //static unsigned int nb_genes_;
    //static std::vector<float> energies_;
    //static unsigned int head_id_;
    //static float total_energy_;
    //static std::vector<u_int> ids_;
    //static float fitness_alpha_;
    //static float fitness_beta_;
    std::default_random_engine generator_;

    fitness_t Fitness();
    void SampleNewGenes();
};
#endif //INDIVIDUAL_H
