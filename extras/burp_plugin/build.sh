#!/bin/sh 

if [ ! -f burpsuite.jar ]; then
  echo "BurpSuite jar file not present (burpsuite.jar) . Need it for linkage"
  exit -1
fi

if [ ! -d ./out ]; then
  mkdir out
fi

javac -d out -sourcepath src -classpath burpsuite.jar src/burp/*

jar -cf pip3line_plugin.jar -C out/ burp
