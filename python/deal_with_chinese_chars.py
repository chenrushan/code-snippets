#!/usr/bin/python2
#-*- coding: utf8 -*-

def main():

  # encode('utf8') turns utf8 chars into bytes, 'encode' means encryption
  # decode('utf8') turns bytes into utf8 chars, 'decode' means decryption

  name = "周杰伦"
  print len(name) # 9
  print len(name.decode('utf8')) # 3

  # print each chinese char
  # 周, 杰, 伦
  for c in name.decode('utf8'):
    print c

if __name__ == '__main__':
  main()

