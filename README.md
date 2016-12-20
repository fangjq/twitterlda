# 淘宝购物模型

该模型包括两部分，第一部分是用淘宝购物记录的用户-标题pair数据训练taobaoLDA模型，用以估计物品的类别；第二部分是根据购物记录的时间、金额以及taobaoLDA估计的类别等进行特征工程。最后的分类模型并不包含，推荐使用LR或者RF训练。

## taobaoLDA

该模型是对LDA的改进，认为一个用户有多篇文档（淘宝购物商品的标题），每个文档具有一个主题，文档中所有的字都由该主题产生；一个用户则具有一个主题分布，按该分布产生每个文档的主题。

taobaolda.cpp实现了该模型的Gibbs采样算法，其使用方式为

	./taobaolda <dataset> <vocab> <num_topics> <alpha> <eta> <maxiter> <resfile>

其中dataset是数据集，每一行是一个标题，空行表示另一个用户,每行用空格表示分词。vocab是词典，每行是一个词。num_topics是主题数K。alpha是主题分布的Dirichlet先验，越小则越稀疏，推荐设置1/K。eta是主题词典的Dirichlet先验，越小则越稀疏，推荐设置0.01。maxiter是Gibbs采样的轮数,推荐500或以上。resfile是保存模型的位置，保存的模型很简单，就是num_topics * num_vocabs的主题-词概率矩阵（使用时需做行归一化）。

## 特征工程
feature_extractor.py实现了基于购买记录和预训练的taobaoLDA模型的特征工程。其extract函数作用于wolong/shopping三方数据查询接口的rsp_messgae，输出一个json表示的特征。

## 其他

topics.py可用来查看taobaolda模型学习到的主题，使用方式为
	
	python topics.py data/taobaolda.model data/vocabs.txt 20

langs.py包含了一些文字预处理的函数。

data目录中包含taobaolda.dat 一个从jhjj数据中制作的taobaoLDA训练集；vocabs.txt该训练集的词典；taobaolda.model，训练的结果。

想实现好的分类效果，需要根据用户客群制作相应的taobaoLDA训练数据。
