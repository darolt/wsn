
#ifndef TYPES_H
#define TYPES_H
typedef struct {
  float partial_coverage;
  float total_coverage;
  float partial_overlapping;
  float total_overlapping;
  float exclusive_area;
} coverage_info_t;

typedef struct {
  float total;
  float term1;
  float term2;
  coverage_info_t coverage_info;
} fitness_t;

typedef std::vector<fitness_t> population_fitness_t;

#endif // TYPES_H
