#!/bin/sh 

if [ ! -d ./out ]; then
  mkdir out
fi

javac -d out -sourcepath src -classpath burpsuite.jar src/burp/*

jar -cf pip3line_plugin.jar -C out/ burp
