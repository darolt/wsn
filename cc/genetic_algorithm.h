// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include <map>
#include <utility>
#include <vector>
#include <random>
#include "optimizer.h"
#include "individual.h"

using namespace std;

class GeneticAlgorithm: public Optimizer {
  public:
    GeneticAlgorithm(dict_t exclusive, regions_t overlapping,
        vector<u_int> ids, config_t config);
    ~GeneticAlgorithm();

  private:
    // change individual's position randomly (random walk). The number of
    // altered genes is proportional to mutation_rate
    void Mutate(Individual &individual, vector<u_int> can_sleep, 
                float mutation_rate);

    // individual1 copy parts of individual2 position depending on influence
    // rate and how far it is from individual2 (the farer the more copies) 
    Individual Crossover(u_int nb_unfit,
                         const vector<u_int> &can_sleep);

    void Optimize(const vector<u_int> &can_sleep);

    // sort population according to fitness value
    void SortFitness();

    fitness_t Fitness(Individual &individual);
};
#endif //GENETIC_ALGORITHM_H
