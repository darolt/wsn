#ifndef ECCA_H
#define ECCA_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include "optimizer.h"
#include "individual.h"

class Ecca: public Optimizer {
  public:
    Ecca(dict_t exclusive, regions_t overlapping,
         std::vector<unsigned int> ids, config_t config);
    virtual ~Ecca();

    // returns a std::vector with the best configuration found (best particle),
    // indicating, for each node, if it should sleep or not;
    // the learning trace (trace of the best fitness value at each iteration);
    // and a std::vector with the coverage and overlapping areas for the best
    // configuration
    individual_t Run(std::vector<float> energies);

  private:
    
    //std::vector<Individual> CreatePopulation1();

    std::vector<std::vector<Individual>>  
    FastNonDominatedSort(std::vector<Individual> &population);

    bool Dominates(Individual &individual1,
                   Individual &individual2);

    std::vector<Individual>
    Reproduce(std::vector<Individual> &population, float crossover_rate);

    std::vector<Individual>
    FindBestParents(std::vector<std::vector<Individual>> &fronts);

    void CalculateCrowdingDistance(std::vector<Individual> &group);

    void CrowdedSorting(std::vector<Individual> &group);

    fitness_t Fitness(Individual &individual);
};
#endif //ECCA_H
