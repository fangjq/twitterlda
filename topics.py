#!/usr/bin/env python
#-*-coding:utf-8-*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

def show_topics(betafile,vocabfile,num_words=25):
	vocabs=open(vocabfile).read().rstrip('\n').decode('utf-8').split('\n')
	k=0
	for line in open(betafile):
		print 'topic', k
		betak = map(float,line.rstrip('\n').split())
		weights = {}
		for v, betakv in enumerate(betak):
			weights[v]=betakv
		i=0
		for x, y in sorted(weights.items(), key=lambda d:d[1],reverse=True):
			print vocabs[x], y
			i+=1
			if i==num_words:break
		k+=1
		print ''


if __name__ == '__main__':
	show_topics(sys.argv[1], sys.argv[2], int(sys.argv[3]))