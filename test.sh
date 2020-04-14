#!/bin/bash
assert(){
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected ecpected, but got $actual"
		exit 1
	fi
}

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 41 "12 + 34 - 5 "
assert 11 "1 + 2 * 5"
assert 13 "9 + 8 / 2"
assert 7 "3+2*(3+5)/(8-4)"
assert 7 "21-(3*(3-1)+1)*2"
assert 10 "-10+20"
assert 25 "(-5)*(-5)"

echo "Passed :D"

