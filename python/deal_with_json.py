#!/usr/bin/python2

import json

def main():
  jsn = json.load(open("../data/test.json"))
  print jsn["name"]
  print jsn["albums"]

  # dump json to string
  # @indent and @separators are for pretty print
  print json.dumps(jsn, sort_keys=True, indent=4, separators=(',', ': '))

  # use json.loads() if load directly from string
  jsn = json.loads('{ "name": "xxx", "age": "28" }')
  print jsn["name"]

if __name__ == '__main__':
  main()

