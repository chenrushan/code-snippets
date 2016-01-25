#!/usr/bin/python

import sys
import os

for root, subFolders, files in os.walk(sys.argv[1]):
    print root, subFolders, files

