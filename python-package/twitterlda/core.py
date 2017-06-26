# -*- coding: utf-8 -*- #
# Author: Jiaquan Fang

import sys
import ctypes
import numpy as np

from .libpath import find_lib_path

class TLDAError(Exception):
    pass


def c_str(string):
    return ctypes.c_char_p(string.encode("utf-8"))


def _load_lib():
    lib_path = find_lib_path()
    if len(lib_path) == 0:
        return None
    lib = ctypes.cdll.LoadLibrary(lib_path[0])
    return lib

_LIB = _load_lib()

def _check_call(ret):
    if ret != 0:
        raise TLDAError("Something went wrong!")

class TLDA(object):


    def __init__(self, num_topics, alpha, eta):
        self.num_topics = num_topics
        self.alpha = alpha
        self.eta = eta

        self.handle = None
        self.topic_beta = None
        self.vocabs = None

    def cleanup(self):
        if self.handle:
            _check_call(_LIB.TLDAFree(self.handle))
            self.handle = None

    def load_data_from_file(self, corpora_filename, vocab_filename):
        self.cleanup()
        self.handle = ctypes.c_void_p()

        self.vocabs = open(vocab_filename).read().rstrip('\n').decode('utf-8').split('\n')

        _LIB.TLDACreateFromFile.argtypes = [ctypes.c_char_p, ctypes.c_char_p,
                                            ctypes.c_int, ctypes.c_float, ctypes.c_float,
                                            ctypes.c_void_p]

        _check_call(_LIB.TLDACreateFromFile(c_str(corpora_filename),
                                            c_str(vocab_filename),
                                            self.num_topics,
                                            self.alpha,
                                            self.eta,
                                            ctypes.byref(self.handle)))

    def inference(self, maxiter):
        n_rows = ctypes.c_ulong()
        n_cols = ctypes.c_ulong()
        data = ctypes.POINTER(ctypes.c_double)()

        _check_call(_LIB.TLDAInference(self.handle, maxiter))
        _check_call(_LIB.TLDAGetResult(self.handle,
                                       ctypes.byref(n_rows),
                                       ctypes.byref(n_cols),
                                       ctypes.byref(data)))

        self.topic_beta = np.zeros((n_rows.value, n_cols.value), dtype=np.float64, order='C')
        if not ctypes.memmove(self.topic_beta.ctypes.data, data, n_rows.value * self.topic_beta.strides[0]):
            raise RuntimeError('memmove failed')
        print self.topic_beta

    def show_topics(self, num_words=25):
        if self.topic_beta is None:
            return

        for i in xrange(self.topic_beta.shape[0]):
            print 'topic', i
            weights = self.topic_beta[i, :]

            for i, idx in enumerate(np.argsort(weights)[::-1]):
                print self.vocabs[idx], weights[idx]
                if i == num_words:
                    break
            print ''

    def estimate_topic(self, wordlist):
        pz = np.zeros(self.num_topics)
        for k in xrange(self.num_topics):
            for word in wordlist:
                pz[k]+=self.topic_beta[k,word]
        argmaxpz = np.zeros(self.num_topics)
        argmaxpz[np.argmax(pz)]=1
        return argmaxpz

    def save(self, filename):
        _check_call(_LIB.TLDASave(self.handle, c_str(filename)));

    def get_author_name(self):
        _LIB.TLDAGetAuthorName.restype = ctypes.c_char_p
        return _LIB.TLDAGetAuthorName()

    def __del__(self):
        self.cleanup()
