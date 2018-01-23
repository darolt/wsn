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
typedef pair<vector<u_int>, vector<float>> config_t;

class PSO {
  public:
    PSO(dict_t exclusive, regions_t overlapping,
        vector<u_int> ids, config_t config);
    ~PSO();

    // returns a vector with the configuration for the best individual,
    // indicating, for each node, if it should sleep or not.
    individual_t run(vector<float> energies, vector<float> sleep_probs);

  private:
    // attributes
    Regions *_regions;
    vector<u_int> _ids;
    vector<individual_t> _particles;
    vector<individual_t> _best_locals;
    individual_t _best_global;
    vector<float> _best_local_fitness;
    float _best_global_fitness;
    u_int _nb_nodes;

    // from config.py
    u_int _NB_PARTICLES;
    u_int _MAX_ITERATIONS;
    float _FITNESS_ALPHA;
    float _FITNESS_BETA;
    float _FITNESS_GAMMA;
    float _WMAX;
    float _WMIN;

    // methods
    float fitness(individual_t individual, vector<float> energies,
                  float total_energy);
    individual_t mutation(individual_t individual, vector<float> sleep_probs);
    individual_t crossover(individual_t individual1, individual_t individual2);
    float should_update(individual_t particle1, individual_t particle2);
};
#endif //PSO_H
