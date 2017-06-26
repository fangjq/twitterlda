/*!
 * Copyright 2017
 * \file main.cpp
 * \brief Command line tool
 * \author Jiaquan Fang
 */

#include <map>
#include <string>
#include <memory>
#include <cstdlib>

#include <model.h>
#include <utils.h>


using namespace std;

int main(int argc, const char * argv[]) {
    if (argc > 1)
    {
        //参数列表
        const char *corpora_filename = argv[1];
        const char *vocab_filename = argv[2];
        int num_topics = atoi(argv[3]);
        double alpha = atof(argv[4]);
        double eta = atof(argv[5]);
        double maxiter = atof(argv[6]);
        const char* resdir = argv[7];
        
        //读取字典
        map<string,int> vocabs = read_vocabs(vocab_filename);
        
        //读取文档
        vector<std::shared_ptr<User>> users = read_corpora(corpora_filename,vocabs);
        
        //训练模型
        std::shared_ptr<Model> model = std::make_shared<Model>(num_topics, alpha, eta);

        model->init_with_corpora(users);
        model->inference(users,maxiter);
        
        //保存模型
        model->save(resdir);
        //printf_gslmatrix(resdir, model->Nkv);
    
        return(0);
    }
    printf("usage : <dataset> <vocab> <num_topics> <alpha> <eta> <maxiter> <resfile>\n");
    return 0;
}
