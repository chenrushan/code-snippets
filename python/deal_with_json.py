#!/usr/bin/python2

import json

def main():
  jsn = json.load(open("../data/test.json"))
  print jsn["name"]
  print jsn["albums"]

if __name__ == '__main__':
  main()

