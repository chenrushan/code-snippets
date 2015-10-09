#!/usr/bin/python2

import BaseHTTPServer
from urlparse import urlparse, parse_qs

class WebRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):

  """Main logic of this server""" 

  def sendResponce(self, code, msg):
    self.send_response(code)
    self.end_headers()
    self.wfile.write(msg)

  def do_GET(self):

    # break down url into units
    # example:
    #  self.path = "http://example.com/test/?name=jayzhou&gender=mail"
    #  units = ParseResult(scheme='http', netloc='example.com', path='/test/',
    #                      params='', query='name=jayzhou&gender=mail', fragment='')
    units = urlparse(self.path)

    # break down parameters
    # example:
    #  paras = {'gender': ['mail'], 'name': ['jayzhou']}
    paras = parse_qs(units.query)

    # ... ...
    # Some Logic
    # ... ...

    # send back response
    self.sendResponce(200, "message")

def main():
  # create server on localhost:8080
  server = BaseHTTPServer.HTTPServer(('0.0.0.0',8080), WebRequestHandler)
  server.serve_forever()

if __name__ == '__main__':
  main()
