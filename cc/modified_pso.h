// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef MODIFIED_PSO_H
#define MODIFIED_PSO_H

#include <map>
#include <utility>
#include <vector>
#include <random>
#include "optimizer.h"
#include "individual.h"

class ModifiedPso: public Optimizer {
  public:
    ModifiedPso(dict_t exclusive, regions_t overlapping,
        std::vector<u_int> ids, config_t config);
    ~ModifiedPso();

  private:
  
    // Mutate and Crossover functions modify the first argument since it is a
    // reference. This is not the best practice but it is done for performance
    // reasons.
    void Mutate(Individual &individual, std::vector<u_int> can_sleep, 
                  float mutation_rate);
    // Returns an individual that gets statistically half of its genes
    // from individual1 and half from individual2
    void Crossover(Individual &individual1, Individual &individual2);

    void Optimize(const std::vector<u_int> &can_sleep);

    fitness_t Fitness(Individual &individual);
};
#endif //MODIFIED_PSO_H
