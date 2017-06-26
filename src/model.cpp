//
//  main.cpp
//  twitterLDA
//
//  Created by qixiang on 16/8/12.
//

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

#include <model.h>
#include <c_api.h>
#include <utils.h>

using namespace std;

Model::Model(int num_topics, double alpha, double eta){
    this->num_topics_ = num_topics;
    this->alpha_ = alpha;
    this->eta_ = eta;
}

Model::~Model() {
    cleanup();
}

void Model::cleanup() {
    if (Z_ != NULL) {
        for (int u = 0; u < users_.size(); u++) {
            free(Z_[u]);
        }
        free(Z_);
        Z_ = NULL;
    }

    if (Nuk_ != NULL) {
        for (int u = 0; u < users_.size(); u++) {
            delete[] Nuk_[u];
        }
        delete[] Nuk_;
        Nuk_ = NULL;
    }

    if (Nkv_ != NULL) {
        for (int k = 0; k < num_topics_; k++) {
           delete[] Nkv_[k]; 
        }
        delete[] Nkv_;
        Nkv_ = NULL;
    }

    if (Skv_ != NULL) {
        delete[] Skv_;
        Skv_ = NULL;
    }

    if (result_ != NULL) {
        delete[] result_;
        result_ = NULL;
    }

    this->users_.clear();
}

int Model::num_topics() {
    return num_topics_;
}

int Model::num_vocabs() {
    return num_vocabs_;
}

const double *Model::result() {
    return result_;
}

void Model::init_with_corpora(vector<std::shared_ptr<User>> users_){
    cout<<"initialing model with data"<<endl;
    cleanup();

    this->users_.insert(this->users_.end(), users_.begin(), users_.end());

    Z_ = (int**) malloc(users_.size() * sizeof(int *));
    for(int u=0; u<users_.size(); u++)
    {
        Z_[u] = (int*) malloc(users_[u]->ndocs() * sizeof(int));
    }
    num_vocabs_ = 0;
    for (int u = 0; u < users_.size(); u++) {
        for (int d = 0; d < users_[u]->ndocs(); d++) {
            int z_ud = rand() % num_topics_;
            Z_[u][d] = z_ud;
            for(int n = 0; n < users_[u]->docs[d]->nwords(); n++) {
                int word = users_[u]->docs[d]->words[n];
                if(word + 1 > num_vocabs_){
                    num_vocabs_ = word + 1;
                }
            }
        }
    }

    Nuk_ = new double*[users_.size()];
    for (int u = 0; u < users_.size(); u++) {
        Nuk_[u] = new double[num_topics_];
        for(int k = 0; k < num_topics_; k++) {
            Nuk_[u][k] = alpha_;
        }
    }
    Nkv_ = new double*[num_topics_];
    for (int k =0; k < num_topics_; k++) {
        Nkv_[k] = new double[num_vocabs_];
        for (int v = 0; v < num_vocabs_; v++) {
            Nkv_[k][v] = eta_;
        }
    }
    Skv_ = new double[num_topics_];
    for (int k = 0; k < num_topics_; k++) {
        Skv_[k] = eta_ * num_vocabs_;
    }

    for (int u = 0; u < users_.size(); u++) {
        for (int d = 0; d < users_[u]->ndocs(); d++) {
            Nuk_[u][Z_[u][d]] += 1.0;
            for(int n = 0; n < users_[u]->docs[d]->nwords(); n++){
                int word = users_[u]->docs[d]->words[n];
                Nkv_[Z_[u][d]][word] += 1.0;
                Skv_[Z_[u][d]] += 1.0;
            }
        }
    }
}

void Model::inference(int maxiter) {
    this->inference(this->users_, maxiter);
}

void Model::inference(vector<std::shared_ptr<User>> users_, int maxiter){
    int iter = 0;
    double* logp = new double[num_topics_];
    while (iter < maxiter) {
        double lhood = 0.0;
        for(int u = 0; u < users_.size(); u++) {
            for(int d = 0; d < users_[u]->ndocs(); d++) {
                /*去除*/
                Nuk_[u][Z_[u][d]] -= 1.0;
                for (int n = 0; n < users_[u]->docs[d]->nwords(); n++) {
                    int word = users_[u]->docs[d]->words[n];
                    Nkv_[Z_[u][d]][word] -= 1.0;
                    Skv_[Z_[u][d]] -= 1.0;
                }

                /*采样*/
                for (int k=0; k < num_topics_; k++) {
                    double logprior = safe_log(Nuk_[u][k]);
                    double loglhood = 0.0;
                    for(int n =0; n < users_[u]->docs[d]->nwords(); n++) {
                        int word = users_[u]->docs[d]->words[n];
                        loglhood += safe_log(Nkv_[k][word]) - safe_log(Skv_[k]);
                    }
                    logp[k] = logprior + loglhood;
                }
                
                Z_[u][d] = sample(num_topics_, logp, true);
                lhood += logp[Z_[u][d]];
                
                /*添加*/
                Nuk_[u][Z_[u][d]] += 1.0;
                for(int n =0; n < users_[u]->docs[d]->nwords(); n++){
                    int word = users_[u]->docs[d]->words[n];
                    Nkv_[Z_[u][d]][word] += 1.0;
                    Skv_[Z_[u][d]] += 1.0;
                }
            }
        }
        iter += 1;
        printf("step %d\tlhood %.2f\n", iter, lhood);
    }

    result_ = new double[num_topics_ * num_vocabs_];

    for (int k = 0; k < num_topics_; ++k) {
        for (int v = 0; v < num_vocabs_; ++v) {
            result_[k * num_vocabs_ + v] = Nkv_[k][v];
        }
    }

    delete[] logp;
}

void Model::save(const char* filename){
    FILE* fileptr;
    int k, v;
    fileptr = fopen(filename, "w");
    for (k = 0; k < num_topics_; k++)
    {
        fprintf(fileptr, "%5.7f", Nkv_[k][0]);
        for (v = 1; v < num_vocabs_; v++)
        {
            fprintf(fileptr, " %5.7f", Nkv_[k][v]);
        }
        fprintf(fileptr, "\n");
    }
    fclose(fileptr);
}
