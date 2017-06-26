#include <utils.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <map>
#include <memory>
#include <time.h>
#include <assert.h>
#include <random>
#include <math.h>

using namespace std;

static int seed = 101;
static std::default_random_engine generator(seed);
static std::uniform_real_distribution<double> distribution(0.0, 1.0);

double safe_log(double x) {
    if (x == 0) return -1000.0;
    else return log(x);
}

double log_sum(double log_a, double log_b) {
    double v;
    
    if (log_a == -1) return log_b;
    
    if (log_a < log_b)
    {
        v = log_b + log(1 + exp(log_a - log_b));
    }
    else
    {
        v = log_a + log(1 + exp(log_b - log_a));
    }
    return v ;
}

double rand_uniform() {
    return distribution(generator);
}

int sample(int dim, double *p, bool islog) {
    if (dim == 0)
        return -1;
    else if (dim == 1)
        return 0;
    double *cdf = new double[dim];
    cdf[0] = p[0];
    if (!islog) {
        for (int i =1; i< dim; i++) {
            cdf[i] = cdf[i-1] + p[i];
        }
    } else {
        for (int i =1; i< dim; i++) {
            cdf[i] = log_sum(cdf[i-1], p[i]);
        }
    }
    double randnum = rand_uniform();
    if (!islog) {
        randnum = randnum * cdf[dim - 1];
    } else {
        randnum = safe_log(randnum) + cdf[dim - 1];
    }
    int i = 0;
    while (true) {
        if(i == dim) {
            delete[] cdf;
            return -1;
        }
        if (randnum < cdf[i]) {
            delete[] cdf;
            return i;
        }
        i += 1;
    }
}

map<string, int> read_vocabs(const char *filename) {
    cout << "reading vocabs from " << filename << endl;
    map<string, int> vocabs;
    ifstream in(filename);
    string s;
    int i = 0;
    while (getline(in, s)) {
        vocabs[s] = i;
        i += 1;
    }
    return vocabs;
}

vector<std::shared_ptr<User>> read_corpora(const char *filename, map<string, int> vocabs) {
    cout << "reading corpora from " << filename << endl;
    vector<std::shared_ptr<User>> users;
    ifstream in(filename);
    string s;
    std::shared_ptr<User> user = std::make_shared<User>();
    while (getline(in, s)) {
        if (s == "") {
            users.push_back(user);
            user = std::make_shared<User>();
        } else {
            std::shared_ptr<Doc> doc = std::make_shared<Doc>();
            char *p = strtok((char*) s.c_str(), " ");
            while (p!=NULL) {
                string word = p;
                int wordid = -1;
                map<string ,int >::iterator l_it = vocabs.find(word);
                if(l_it!=vocabs.end()){
                    wordid = l_it->second;
                    doc->words.push_back(wordid);
                }
                p = strtok(NULL, " ");
            }
            user->docs.push_back(doc);
        }
    }
    return users;
}

