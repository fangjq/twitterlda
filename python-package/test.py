# -*- coding: utf-8 -*- #
# Author: Jiaquan Fang

import sys
import twitterlda

reload(sys)
sys.setdefaultencoding('utf-8')

if __name__ == "__main__":
    model = twitterlda.TLDA(20, 0.05, 0.1)
    model.load_data_from_file("../data/taobaolda.dat", "../data/vocabs.txt")
    model.inference(maxiter=20)
    model.show_topics(25)
    model.save("test.model")
