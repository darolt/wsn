// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef PSO_H
#define PSO_H

#include <map>
#include <utility>
#include <vector>
#include "regions.h"

using namespace std;

typedef pair<vector<int>, float> region_t;
typedef vector<region_t> regions_t;
typedef map<int, float> dict_t;
// using char for the smallest addressable unit
typedef vector<char> individual_t;
typedef unsigned int u_int;

class PSO {
  public:
    PSO(dict_t exclusive, regions_t overlapping, vector<int> ids, u_int nb_particles);
    ~PSO();

    // returns a vector with the configuration for the best individual,
    // indicating, for each node, if it should sleep or not.
    individual_t run(vector<float> energies, vector<float> sleep_probs);

  private:
    // attributes
    Regions *_regions;
    vector<int> _ids;
    vector<individual_t> _particles;
    vector<individual_t> _best_locals;
    individual_t _best_global;
    vector<float> _best_local_fitness;
    float _best_global_fitness;
    u_int _nb_particles;
    u_int _nb_nodes;

    // methods
    float fitness(individual_t individual, vector<float> energies,
                  float total_energy);
    individual_t mutation(individual_t individual, vector<float> sleep_probs);
    individual_t crossover(individual_t individual1, individual_t individual2);
    float should_update(individual_t particle1, individual_t particle2);
};
#endif //PSO_H
