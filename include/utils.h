#ifndef UTILS_H_
#define UTILS_H_

#include <string.h>
#include <vector>
#include <map>
#include <model.h>

using namespace std;

double safe_log(double x);

double log_sum(double log_a, double log_b);

double rand_uniform();

int sample(int dim, double* p, bool islog);

map<string,int> read_vocabs(const char *filename);

vector<std::shared_ptr<User>> read_corpora(const char *filename, map<string, int> vocabs);

#endif

