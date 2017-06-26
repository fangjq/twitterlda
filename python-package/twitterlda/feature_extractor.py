#-*-coding:utf-8-*-

import sys
import os
import numpy as np
import json
import jieba
import datetime
import langs
reload(sys)
sys.setdefaultencoding('utf-8')

taobaolda_train_file = os.getcwd()+'/taobao/data/taobaolda.dat'
taobaolda_model_file = os.getcwd()+'/taobao/data/taobaolda.model'
vocab_file = os.getcwd() +'/taobao/data/vocabs.txt'
taobaolda_recompile_shell = os.getcwd()+'taobao/recompile.sh'

class Purchase(object):
	def __init__(self,js, vocabs):
		self.title = js.get('item_title','')
		self.titleids = [vocabs[w] for w in list(jieba.cut(self.title)) if w in vocabs]
		self.price = float(js.get('price','0.0'))
		self.time = js.get('purchase_time','201611')
		self.time = datetime.datetime.strptime('%s-%s-01'%(self.time[0:4],self.time[4:6]),"%Y-%m-%d")
		self.brand = js.get('item_brand',None)
		self.sellerLocation = js.get('seller_location',None)

class WolongShoppingFeatureExtractor(object):
	def __init__(self):
		self.load_taobaolda()

	def load_taobaolda(self):
		if not os.path.exists(vocab_file) or not os.path.exists(taobaolda_model_file):
			self.train_taobaolda()
		self.vocabs = {w:i for i,w in enumerate(open(vocab_file).read().strip().decode('utf-8').split('\n'))}
		self.topic_beta = []
		for line in open(taobaolda_model_file):
			betak = np.array(map(float,line.strip().split()))
			betak = np.log(betak/np.sum(betak))
			self.topic_beta.append(betak)
		self.topic_beta = np.array(self.topic_beta)
		self.num_topics = self.topic_beta.shape[0]
		self.num_vocabs = self.topic_beta.shape[1]

	def train_taobaolda(self, num_vocab=4000,num_topics=20, alpha = 0.05, eta = 0.01,maxiter = 400):
		if not os.path.exists(taobaolda_train_file):
			print '%s not exist!'%taobaolda_model_file
			sys.exit(1)
		print 'build vocabulary %d from %s'%(num_vocab, taobaolda_train_file)
		vocabs ={}
		for line in open(taobaolda_train_file):
			line = line.decode('utf-8').strip()
			if line=='':
				continue
			for word in line.split():
				if langs.is_punctuation(word) or langs.is_num(word):
					continue
				if not word in vocabs:
					vocabs[word]=0
				vocabs[word]+=1
		vocabs = [x for x, y in sorted(vocabs.items(), key=lambda d:d[1],reverse=True)][0:num_vocab]
		with open(vocab_file,'w') as fw:
			fw.write('\n'.join(vocabs))
		print 'recompile taobaolda exe'
		os.system('sh %s'%(taobaolda_recompile_shell))
		print 'train taobaolda model'
		cmd = './taobaolda %s %s %d %f %f %d %s'%(taobaolda_train_file, vocab_file, num_topics, alpha, eta, maxiter, taobaolda_model_file)
		os.system(cmd)

	def estimate_topic(self, wordlist):
		pz = np.zeros(self.num_topics)
		for k in xrange(self.num_topics):
			for word in wordlist:
				pz[k]+=self.topic_beta[k,word]
		argmaxpz = np.zeros(self.num_topics)
		argmaxpz[np.argmax(pz)]=1
		return argmaxpz

	def extract(self, rsp_message, prefix='wolong_shopping'):
		f={}
		if rsp_message is None or rsp_message in set(['None','null','']):
			return None
		js = json.loads(rsp_message)
		code = js.get(u'ErrorCode',None)
		if code is None or code not in ['1','0']:
			return None
		if code =='1':
			f['has_data']=0
		Data = js.get('Data')
		data = Data.get('data',None)
		if data is None:
			f['has_data']=0
		else:
			f['has_data']=1
		if f['has_data']==1:
			data=json.loads(data)
			purchaseList = [Purchase(data[i], self.vocabs) for i in xrange(len(data))]
			#总体购买情况
			f['total_purchase_cnt'] = len(purchaseList)
			f['total_spent_money'] = sum([x.price for x in purchaseList])
			if len(purchaseList)>0:
				f['average_price'] = 1.0*sum([x.price for x in purchaseList])/len(purchaseList)
				f['max_price'] = max([x.price for x in purchaseList])
				f['min_price'] = min([x.price for x in purchaseList])
			purchase_times = [x.time for x in purchaseList]
			if len(purchaseList)==0:
				span=1
			else:
				span = max(np.ceil((max(purchase_times) - min(purchase_times)).days/30),1)		
			f['average_purchase_cnt_per_month'] = 1.0 * len(purchaseList) / span
			#单类购买情况（物品类别预估，并和件数、价格组合）
			if len(purchaseList)>0:
				topic_cnt = np.zeros(self.num_topics)
				topic_price = np.zeros(self.num_topics)
				for x in purchaseList:
					topic_post = self.estimate_topic(x.titleids)
					topic_cnt += topic_post
					topic_price += topic_post*x.price
				topic_cnt_proportion = topic_cnt/np.sum(topic_cnt)
				topic_price_proportion = topic_price/np.sum(topic_price)
				for k in xrange(self.num_topics):
					f['topic_%d_cnt'%(k)]=topic_cnt[k]
					f['topic_%d_spent'%(k)]=topic_price[k]
					f['topic_%d_cnt_proportion'%(k)]=topic_cnt_proportion[k]
					f['topic_%d_spent_proportion'%(k)]=topic_price_proportion[k]
			prefix_f ={}
			for k,v in f.iteritems():
				prefix_f[prefix+'_'+k]=v
			return prefix_f

if __name__ == '__main__':
	m = WolongShoppingFeatureExtractor()
	m.train_taobaolda()
