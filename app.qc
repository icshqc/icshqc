// WORKING

int #::
float #::
double #::
char #::
Vector3 #:: (int x) (int y) (int z)

doubler = {|int x| add x x }

+ = {|int x, int y| add x y }
- = {|int x, int y| sus x y }
* = {|int x, int y| mult x y }
/ = {|int x, int y| divide x y }

//include "src/lib.h"

p1 = (#IntPair 1 1)
p2 = (#IntPair 2 2)

add5 = {|int x| x + 5}

// NOT WORKING YET

add10 = {|int x| (add5 (add5 x))}

foo = {|int x, int y| (x + y) * 4}

// For the parser to be created

cmd = "10"
cmd2 = "p1"
cmd3 = "2 + 2"
cmd4 = "add 10 2"

parse = {|char* cmd| cmd}
