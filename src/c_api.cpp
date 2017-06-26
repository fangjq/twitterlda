/*!
 * Copyright 2017
 * \file c_api.cpp
 * \brief C API for Python
 * \author Jiaquan Fang
 */

#include <c_api.h>
#include <model.h>
#include <utils.h>
#include <memory>
#include <string>
#include <vector>

using namespace std;

int TLDACreateFromFile(const char *corpora_filename,
                       const char *vocab_filename,
                       int num_topics,
                       double alpha,
                       double eta,
                       TLDAHandle *out) {
        //读取字典
        map<string,int> vocabs = read_vocabs(vocab_filename);
        
        //读取文档
        vector<std::shared_ptr<User>> users = read_corpora(corpora_filename,vocabs);
        
        //训练模型
        Model* model = new Model(num_topics,alpha,eta);
        model->init_with_corpora(users);
        *out = new std::shared_ptr<Model>(model);

        return 0;
}

int TLDAFree(TLDAHandle handle) {
    delete static_cast<std::shared_ptr<Model> *>(handle);
    return 0;
}

int TLDAInference(TLDAHandle handle, int maxiter) {
    std::shared_ptr<Model> *model = 
        static_cast<std::shared_ptr<Model> *>(handle);

    (*model)->inference(maxiter);

    return 0;
}

int TLDAGetResult(TLDAHandle handle,
                  uint64_t *n_rows,
                  uint64_t *n_cols,
                  const double **out_result) {
    
    std::shared_ptr<Model> *model = 
        static_cast<std::shared_ptr<Model> *>(handle);

    *n_rows = (*model)->num_topics();
    *n_cols = (*model)->num_vocabs();
    *out_result = (*model)->result();

    return 0;
}

int TLDASave(TLDAHandle handle, const char *resdir) { 
    std::shared_ptr<Model> *model = 
        static_cast<std::shared_ptr<Model> *>(handle);
    (*model)->save(resdir);

    return 0;
}

const char *TLDAGetAuthorName() {
    return "Jiaquan Fang";
}


