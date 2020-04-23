#!/bin/bash

cc -xc -c -o tmp2.o func_call_test.h

echo ""
echo "input.."
read input

./c38 "$input" > tmp.s
cc -o tmp tmp.s tmp2.o
./tmp
actual="$?"

echo ""
echo "=> $actual"

