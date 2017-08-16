// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <random>
#include "regions.h"

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

typedef struct {
  float total;
  float term1;
  float term2;
  coverage_info_t coverage_info;
} fitness_t;
typedef std::vector<fitness_t> population_fitness_t;

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
    individual_t Run(float_v energies, u_int head_id);

    // setters & getters
    void SetAlpha(float value);
    void SetBeta(float value);
    std::vector<float> GetLearningTrace();
    std::vector<float> GetTerm1Trace();
    std::vector<float> GetTerm2Trace();
    float GetBestCoverage();
    float GetBestOverlapping();

  private:
    Regions *regions_;

    // Returns a float indicating how fit a individual/particle is,
    // and the coverage and overlapping areas for that particle.
    fitness_t Fitness(const individual_t &individual, char do_print);

  protected:
    // attributes
    std::vector<u_int> ids_;
    u_int nb_nodes_;
    u_int nb_individuals_;
    u_int max_iterations_;
    float wmax_;
    float wmin_;

    float fitness_alpha_;
    float fitness_beta_;

    // std::vector with all individuals
    std::vector<individual_t> population_;
    // each index contains the best individuals found in that index
    // (it supposes that the number of individuals is constant)
    std::vector<individual_t> best_locals_;
    // best individual found
    individual_t best_global_;
    // std::vector with the fitness of the best individual found
    // (used for optimization)
    std::vector<float> best_local_fitness_;
    // fitness of the best individual in the history
    fitness_t best_global_fitness_;
    // coverage and overlapping for the best configuration found in the
    // last run. These are not necessarily the best coverages found.
    float best_coverage_;
    float best_overlapping_;
    // learning trace for the last run
    std::vector<float> learning_trace_;
    std::vector<float> term1_trace_;
    std::vector<float> term2_trace_;

    // random related
    std::default_random_engine generator_;

    // session attributes (stored here for convenience)
    float_v energies_;
    float total_energy_;
    u_int head_id_;

    // methods
    void PrintIndividual(individual_t individual);

    virtual void CreatePopulation();
    virtual void Optimize(const std::vector<u_int> &can_sleep);

    population_fitness_t CalculateFitness(std::vector<individual_t> &group);

    void UpdateFitness();

    void ClearLearningTraces();
    void InitializeSessionData(const float_v &energies, const u_int &head_id);

    void PushIntoLearningTraces(const fitness_t &fitness);
};
#endif //OPTIMIZER_H
