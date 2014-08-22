int #::
float #::
double #::
char #::
Vector3 #:: (int x) (int y) (int z)

doubler :: {|int x| add x x }

+ ::: {|int x, int y| add x y }
- ::: {|int x, int y| sus x y }
* ::: {|int x, int y| mult x y }
/ ::: {|int x, int y| divide x y }

include "src/lib.h"

foo :: {|int x, int y| (x + y) * 4}

add5 = {|int x| x + 5}
