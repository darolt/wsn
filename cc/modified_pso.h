// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef MODIFIED_PSO_H
#define MODIFIED_PSO_H

#include <map>
#include <utility>
#include <vector>
#include <random>
#include "optimizer.h"

using namespace std;

class ModifiedPso: public Optimizer {
  public:
    ModifiedPso(dict_t exclusive, regions_t overlapping,
        vector<u_int> ids, config_t config);
    ~ModifiedPso();

  private:
  
    // Mutate and Crossover functions modify the first argument since it is a
    // reference. This is not the best practice but it is done for performance
    // reasons.
    void Mutate(individual_t &individual, vector<u_int> can_sleep, 
                  float mutation_rate);
    // Returns an individual that gets statistically half of its genes
    // from individual1 and half from individual2
    void Crossover(individual_t &individual1, individual_t &individual2);

    void Optimize(float_v energies, const vector<u_int> &can_sleep,
                  float total_energy);
};
#endif //MODIFIED_PSO_H
