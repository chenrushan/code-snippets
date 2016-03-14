#!/usr/bin/python2
#-*- coding: utf8 -*-

### {{{ 以下 python 处理 utf-8 输入的标准开头，所有在程序执行过程中的字符
### 都保证是 unicode 编码，如果不是用 s = s.decode("utf-8") 来转成 unicode，
### 同时打开文件都用 codecs.open(sys.argv[1], encoding='utf-8')
import codecs
sys.stdin = codecs.getreader('utf-8')(sys.stdin)
sys.stdout = codecs.getwriter('utf-8')(sys.stdout)
### }}}

import sys
for line in codecs.open(sys.argv[1], encoding='utf-8'):
  print line

