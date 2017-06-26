#!/usr/bin/env python
#-*-coding:utf-8-*-
import sys
import re
import string
reload(sys)
sys.setdefaultencoding('utf-8')

'''【检测中文】'''
min_chinese_unicode = u'\u4e00'
max_chinese_unicode = u'\u9fa5'


def is_chinese(uText):
    if len(uText) == 0:
        return False
    if not isinstance(uText, unicode):
        uText = unicode(uText)
    for ch in uText:
        if ch < min_chinese_unicode or ch > max_chinese_unicode:
            return False
    return True


def has_chinese(uText):
    if not isinstance(uText, unicode):
        uText = unicode(uText)
    if len(uText) == 0:
        return False
    for s in uText:
        if is_chinese(s):
            return True
    return False

'''【检测标点】'''
EngP = string.punctuation
ChP = ',，.。?？<《>》/／\、~~!！@@#＃$¥%％^……&&*＊(（)）-——+＋_－=＝[【]】［］'


def is_punctuation(uText):
    if not isinstance(uText, unicode):
        uText = unicode(uText)
    if len(uText) == 0:
        return False
    if uText in EngP or uText in ChP:
        return True
    else:
        return False


def has_punctuation(uText):
    if not isinstance(uText, unicode):
        uText = unicode(uText)
    if len(uText) == 0:
        return False
    for s in uText:
        if is_punctuation(s):
            return True
    return False

'''【检测数字】'''
re_num = re.compile('[0-9]+(?:.[0-9]+)?')


def is_num(uText):
    if len(uText) == 0:
        return False
    if not isinstance(uText, unicode):
        uText = unicode(uText)
    m = re_num.findall(uText)
    if len(m) == 1 and m[0] == uText:
        return True
    else:
        return False


def has_num(uText):
    if len(uText) == 0:
        return False
    if not isinstance(uText, unicode):
        uText = unicode(uText)
    for w in uText:
        if str.isdigit(str(w)):
            return True
    return False

'''【检测英文】'''
re_eng = re.compile('[a-zA-Z]+')


def is_eng(uText):
    if len(uText) == 0:
        return False
    if not isinstance(uText, unicode):
        uText = unicode(uText)
    for w in uText:
        if len(re_eng.findall(w)) == 0:
            return False
    return True


def has_eng(uText):
    if len(uText) == 0:
        return False
    if not isinstance(uText, unicode):
        uText = unicode(uText)
    for w in uText:
        if is_eng(w):
            return True
    return False

'''【Trim】'''


def ltrim(input, trim):
    input = unicode(input)
    trim = unicode(trim)
    if input.startswith(trim):
        input = input[len(trim):len(input)]
    return input


def rtrim(input, trim):
    input = unicode(input)
    trim = unicode(trim)
    if input.endswith(trim):
        input = input[0:len(input) - len(trim)]
    return input

'''【最大匹配分词器】'''


def lmmseg(line, dic, window_size=5):
    chars = line.decode("utf8")
    words = []
    idx = 0
    while idx < len(chars):
        matched = False
        lastmatched = False
        for i in xrange(window_size, 0, -1):
            cand = chars[idx:idx + i].encode("utf8")
            if cand in dic:
                words.append(cand)
                matched = True
                break
        if not matched:
            i = 1
            words.append(chars[idx].encode("utf8"))
        idx += i
    return words


def lmmseg(line, dic, window_size=5):
    chars = line.decode("utf8")
    words = []
    idx = 0
    while idx < len(chars):
        matched = False
        for i in xrange(window_size, 0, -1):
            cand = chars[idx:idx + i]
            if cand in dic:
                words.append(cand)
                matched = True
                break
        if not matched:
            i = 1
            words.append(chars[idx].encode("utf8"))
        idx += i
    return words


def rmmseg(line, dic, window_size=5):
    chars = line.decode("utf-8")
    words = []
    idx = len(chars)
    while idx > 0:
        matched = False
        for i in xrange(window_size, 0, -1):
            if idx - i < 0:
                continue
            cand = chars[idx - i:idx]
            if len(cand) == 0:
                continue
            if cand in dic:
                words.append(cand)
                matched = True
                break
        if not matched:
            i = 1
            words.append(chars[idx - 1].encode("utf8"))
        idx -= i
    words.reverse()
    return words
