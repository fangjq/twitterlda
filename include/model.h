/*!
 * Copyright 2017
 * \file model.h
 * \brief Model structure for Twitter LDA
 * \author Jiaquan Fang
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <vector>
#include <memory>

class Doc{
public:
    std::vector<int> words;
    Doc(){
    }
    int nwords(){
        return words.size();
    }
};

class User{
public:
    std::vector<std::shared_ptr<Doc>> docs;
    User(){
    }
    int ndocs(){
        return docs.size();
    }
};

class Model{
private:
    int num_topics_;
    int num_vocabs_;
    double alpha_;
    double eta_;
    int** Z_ {NULL};
    double** Nuk_ {NULL};
    double** Nkv_ {NULL};
    double* Skv_ {NULL};
    double* result_ {NULL};
    std::vector<std::shared_ptr<User>> users_;
    
public:
    Model(int num_topics, double alpha, double eta);

    ~Model();

    void cleanup(); 

    int num_topics();  
    int num_vocabs();

    const double *result();

    void init_with_corpora(std::vector<std::shared_ptr<User>> users_);

    void inference(int maxiter);    

    void inference(std::vector<std::shared_ptr<User>> users_, int maxiter);    

    void save(const char* filename);
};

#endif
