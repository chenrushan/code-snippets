#!/bin/sh

sed '/^#/d' CMakeLists.txt | sed '/^$/N;/^\n$/D'

