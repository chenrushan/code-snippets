#!/usr/bin/python2
#-*- coding: utf8 -*-

# 以下给出 python 处理 utf-8 字符的标准做法，所有在程序执行过程中的字符
# 都保证是 unicode 编码，如果不是用 s = s.decode("utf-8") 来转成 unicode

import codecs
import sys

sys.stdin = codecs.getreader('utf-8')(sys.stdin)
sys.stdout = codecs.getwriter('utf-8')(sys.stdout)

for line in codecs.open(sys.argv[1], encoding='utf-8'):
  print line

