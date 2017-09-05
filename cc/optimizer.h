// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <random>
#include "types.h"
#include "regions.h"
#include "individual.h"

// most of these definitions are used to improve readability
typedef unsigned int u_int;
typedef std::vector<float> float_v;
typedef std::vector<u_int> u_int_v;

typedef std::pair<std::vector<u_int>,
                  float> region_t;
typedef std::vector<region_t> regions_t;
typedef std::map<u_int, float> dict_t;
// using char for the smallest addressable unit
typedef std::vector<char> individual_t;
typedef std::pair<std::map<std::string, u_int>, 
                  std::map<std::string, float>> config_t;


class Optimizer {

  friend class Individual;

  public:
    Optimizer(dict_t exclusive, regions_t overlapping,
              std::vector<u_int> ids, config_t config);
    virtual ~Optimizer();

    // returns a std::vector with the best configuration found (best particle),
    // indicating, for each node, if it should sleep or not;
    // the learning trace (trace of the best fitness value at each iteration);
    // and a std::vector with the coverage and overlapping areas for the best
    // configuration
    individual_t Run(float_v energies);

    // setters & getters
    void SetAlpha(float value);
    void SetBeta(float value);
    void SetGamma(float value);
    std::vector<float> GetLearningTrace();
    std::vector<float> GetTerm1Trace();
    std::vector<float> GetTerm2Trace();
    float GetBestCoverage();
    float GetBestOverlapping();

  protected:
    Regions *regions_;

    // attributes
    std::vector<u_int> ids_;
    u_int nb_nodes_;
    u_int nb_individuals_;
    u_int max_iterations_;
    float wmax_;
    float wmin_;

    float fitness_alpha_;
    float fitness_beta_;
    float fitness_gamma_;

    // session attributes (stored here for convenience)

    // std::vector with all individuals
    std::vector<Individual> population_;
    Individual best_global_;
    std::vector<Individual> best_locals_;

    // random related
    std::default_random_engine generator_;

    float_v energies_;
    float total_energy_;
    std::vector<unsigned int> dead_nodes_;
    unsigned int nb_alive_nodes_;

    // learning traces for the last run
    std::vector<float> learning_trace_;
    std::vector<float> term1_trace_;
    std::vector<float> term2_trace_;

    // methods
    void PrintIndividual(individual_t individual);

    void CreatePopulation();
    virtual void Optimize(const std::vector<u_int> &can_sleep);

    void ClearLearningTraces();
    void InitializeSessionData(const float_v &energies);

    void PushIntoLearningTraces(const fitness_t &fitness);

    // Returns a float indicating how fit a individual/particle is,
    // and the coverage and overlapping areas for that particle.
    virtual fitness_t Fitness(Individual &individual) = 0;
};
#endif //OPTIMIZER_H
