//
//  main.cpp
//  twitterLDA
//
//  Created by qixiang on 16/8/12.
//  Copyright (c) 2016年 creditease. All rights reserved.
//

#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <map>
#include <time.h>
#include <assert.h>
#include <random>
#include <math.h>

using namespace std;

int seed = 101;
std::default_random_engine generator(seed);
std::uniform_real_distribution<double> distribution(0.0,1.0);

double safe_log(double x){
    if (x == 0) return(-1000.0);
    else return(log(x));
}

double log_sum(double log_a, double log_b){
    double v;
    
    if (log_a == -1) return(log_b);
    
    if (log_a < log_b)
    {
        v = log_b+log(1 + exp(log_a-log_b));
    }
    else
    {
        v = log_a+log(1 + exp(log_b-log_a));
    }
    return(v);
}

double rand_uniform(){
    return distribution(generator);
}

int sample(int dim, double* p, bool islog){
    if(dim==0)
        return -1;
    else if (dim==1)
        return 0;
    double* cdf =new double[dim];
    cdf[0] = p[0];
    if(!islog){
        for(int i =1; i< dim;i++){
            cdf[i] = cdf[i-1] + p[i];
        }
    }else{
        for(int i =1; i< dim;i++){
            cdf[i] = log_sum (cdf[i-1], p[i]);
        }
    }
    double randnum = rand_uniform();
    if(!islog){
        randnum = randnum*cdf[dim-1];
    }else{
        randnum = safe_log(randnum)+cdf[dim-1];
    }
    int i=0;
    while (true) {
        if(i==dim){
            delete cdf;
            return -1;
        }
        if(randnum<cdf[i]){
            delete cdf;
            return i;
        }
        i+=1;
    }
}

map<string,int> read_vocabs(string filename){
    cout<<"reading vocabs from "<<filename<<endl;
    map<string,int> vocabs;
    ifstream in(filename.data());
    string s;
    int i=0;
    while(getline(in,s)){
        vocabs[s]=i;
        i+=1;
    }
    return vocabs;
}

class Doc{
public:
    vector<int> words;
    Doc(){
    }
    int nwords(){
        return words.size();
    }
};

class User{
public:
    vector<Doc*> docs;
    User(){
    }
    int ndocs(){
        return docs.size();
    }
};

vector<User*> read_corpora(string filename,map<string, int> vocabs){
    cout<<"reading corpora from "<<filename<<endl;
    vector<User*> users;
    ifstream in(filename.data());
    string s;
    User* user = new User();
    while(getline(in,s)){
        if(s==""){
            users.push_back(user);
            user = new User();
        }else{
            Doc* doc = new Doc();
            char * p  = strtok((char*)s.c_str(), " ");
            while (p!=NULL) {
                string word = p;
                int wordid = -1;
                map<string ,int >::iterator l_it = vocabs.find(word);
                if(l_it!=vocabs.end()){
                    wordid = l_it->second;
                    doc->words.push_back(wordid);
                }
                p = strtok(NULL," ");
            }
            user->docs.push_back(doc);
        }
    }
    return users;
}

class Model{
public:
    int num_topics;
    int num_vocabs;
    double alpha;
    double eta;
    int** Z;
    double** Nuk;
    double** Nkv;
    double* Skv;
    
    Model(int num_topics, double alpha, double eta){
        this->num_topics = num_topics;
        this->alpha = alpha;
        this->eta = eta;
    }
    
    void init_with_corpora( vector<User*> users){
        cout<<"initialing model with data"<<endl;
        Z = (int**) malloc(users.size()*sizeof(int*));
        for(int u=0;u<users.size();u++)
        {
            Z[u] = (int*)malloc(users[u]->ndocs()*sizeof(int));
        }
        num_vocabs = 0;
        for(int u = 0;u< users.size();u++){
            for (int d=0;d<users[u]->ndocs();d++){
                int z_ud = rand()%num_topics;
                Z[u][d] = z_ud;
                for(int n =0;n<users[u]->docs[d]->nwords();n++){
                    int word = users[u]->docs[d]->words[n];
                    if(word+1> num_vocabs){
                        num_vocabs = word+1;
                    }
                }
            }
        }
        Nuk = new double*[users.size()];
        for(int u =0;u<users.size();u++){
            Nuk[u] = new double[num_topics];
            for(int k=0;k<num_topics;k++){
                Nuk[u][k]=alpha;
            }
        }
        Nkv = new double*[num_topics];
        for(int k =0;k<num_topics;k++){
            Nkv[k] = new double[num_vocabs];
            for(int v=0;v<num_vocabs;v++){
                Nkv[k][v]=eta;
            }
        }
        Skv = new double[num_topics];
        for(int k=0;k<num_topics;k++){
            Skv[k]=eta*num_vocabs;
        }
        for(int u = 0;u< users.size();u++){
            for (int d=0;d<users[u]->ndocs();d++){
                 Nuk[u][Z[u][d]]+=1.0;
                for(int n =0;n<users[u]->docs[d]->nwords();n++){
                    int word = users[u]->docs[d]->words[n];
                    Nkv[Z[u][d]][word]+=1.0;
                    Skv[Z[u][d]]+=1.0;
                }
            }
        }
    }
    
    void inference(vector<User*> users, int maxiter){
        int iter = 0;
        double* logp = new double[num_topics];
        while (iter< maxiter) {
            double lhood = 0.0;
            for(int u = 0;u< users.size();u++){
                for(int d=0;d<users[u]->ndocs();d++){
                    /*去除*/
                    Nuk[u][Z[u][d]]-=1.0;
                    for(int n =0;n<users[u]->docs[d]->nwords();n++){
                        int word = users[u]->docs[d]->words[n];
                        Nkv[Z[u][d]][word]-=1.0;
                        Skv[Z[u][d]]-=1.0;
                    }

                    /*采样*/
                    for(int k=0;k<num_topics;k++){
                        double logprior = safe_log(Nuk[u][k]);
                        double loglhood = 0.0;
                        for(int n =0;n<users[u]->docs[d]->nwords();n++){
                            int word = users[u]->docs[d]->words[n];
                            loglhood+=safe_log(Nkv[k][word])-safe_log(Skv[k]);
                        }
                        logp[k] = logprior + loglhood;
                    }
                    
                    Z[u][d] = sample(num_topics, logp, true);
                    lhood+=logp[Z[u][d]];
                    
                    /*添加*/
                    Nuk[u][Z[u][d]]+=1.0;
                    for(int n =0;n<users[u]->docs[d]->nwords();n++){
                        int word = users[u]->docs[d]->words[n];
                        Nkv[Z[u][d]][word]+=1.0;
                        Skv[Z[u][d]]+=1.0;
                    }
                }
            }
            iter+=1;
            printf("step %d\tlhood %.2f\n",iter,lhood);
        }
        delete logp;
    }
    
    void save(const char* filename){
        FILE* fileptr;
        int k, v;
        fileptr = fopen(filename, "w");
        for (k = 0; k < num_topics; k++)
        {
            fprintf(fileptr, "%5.7f", Nkv[k][0]);
            for (v = 1; v < num_vocabs; v++)
            {
                fprintf(fileptr, " %5.7f", Nkv[k][v]);
            }
            fprintf(fileptr, "\n");
        }
        fclose(fileptr);
    }
};

int main(int argc, const char * argv[]) {
    if (argc > 1)
    {
        //参数列表
        string corpora_filename = argv[1];
        string vocab_filename = argv[2];
        int num_topics = atoi(argv[3]);
        double alpha = atof(argv[4]);
        double eta = atof(argv[5]);
        double maxiter = atof(argv[6]);
        const char* resdir = argv[7];
        
        //读取字典
        map<string,int> vocabs = read_vocabs(vocab_filename);
        
        //读取文档
        vector<User*> users = read_corpora(corpora_filename,vocabs);
        
        //训练模型
        Model* model = new Model(num_topics,alpha,eta);
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
