#!/bin/bash

cc -xc -c -o tmp2.o func_call_test.h

assert(){
	expected="$1"
	input="$2"

	./c38 "$input" > tmp.s
	cc -o tmp tmp.s tmp2.o
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 0 "main() { return 0; }" 
assert 42 "main() { return 42; }"

assert 21 "main() { return 5+20-4; }"
assert 41 "main() { return 12 + 34 - 5; }"
assert 11 "main() { return 1 + 2 * 5; }"
assert 13 "main() { return 9 + 8 / 2; }"
assert 7 "main() { return 3+2*(3+5)/(8-4); }"
assert 7 "main() { return 21-(3*(3-1)+1)*2; }"
assert 10 "main() { return -10+20; }"
assert 25 "main() { return (-5)*(-5); }"

assert 0 "main() { return 0==1; }"
assert 1 "main() { return 38==38; }"
assert 1 "main() { return 0!=1; }"
assert 0 "main() { return 38!=38; }"
assert 1 "main() { return 0<1; }"
assert 0 "main() { return 1<1; }"
assert 0 "main() { return 2<1; }"
assert 1 "main() { return 0<=1; }"
assert 1 "main() { return 1<=1; }"
assert 0 "main() { return 2<=1; }"
assert 0 "main() { return 0>1; }"
assert 0 "main() { return 1>1; }"
assert 1 "main() { return 2>1; }"
assert 0 "main() { return 0>=1; }"
assert 1 "main() { return 1>=1; }"
assert 1 "main() { return 2>=1; }"

assert 18 "main() { a = 3; b = 5 * 6; return a + b / 2; }"
assert 13 "main() { foo = 3; bar = 5 * 6; return foo + bar / 3; }"
assert 38 "main() { s_1 = 30; s_2 = 8; return s_1 + s_2; }"
assert 38 "main() { s_1 = 30; s_2 = 8; return s_1 + s_2; return 11; }"

assert 38 "main() { if(1 == 1) return 38; else return 3; }"
assert 3 "main() { if(1 == 0) return 38; else return 3; }"
assert 38 "main() { a = 30; while(a < 38) a = a + 1; return a; }"
assert 30 "main() { a = 30; while(a > 38) a = a + 1; return a; }"
assert 38 "main() { a = 0; i = 0; for(i = 0; i < 38; i = i + 1) a = a + 1; return a; }"
assert 38 "main() { for(;;) return 38; return 0; }"
assert 38 "main() { {return 38;} }"
assert 38 "main() { a = 0; while(a < 38) {b = 1; a = a + b;} return a; }"
assert 55 "main() { i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j; }"

assert 1 "main() { return funcTest1(); }"
assert 2 "main() { return funcTest2(); }"
assert 3 "main() { a = funcTest1(); b = funcTest2(); return a + b; }"
assert 38 "main() { a = 30; b = 8; return funcTestAdd(a, b); }"
assert 21 "main() { a = 1; b = 2; return funcTestAdd6(a, b, 3, 4, 5, 6); }"

assert 38 "main() { return func38();} func38() {return 38;}"

echo ""
echo "Passed :D"

