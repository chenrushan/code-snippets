#!/usr/bin/python2

import httplib

def sendRequest():
  conn = httplib.HTTPConnection("http://example.com:8080")
  conn.request("GET", "/test/?name=jayzhou&gender=mail", headers={"Accept": "application/json"}) 
  response = conn.getresponse()
  res = response.read()
  return res

