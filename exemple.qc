// ::= defines an operator.
// #...# => output ca direct en C
+ ::= {|int x, int y| #return x + y;# }

add = {|int x, int y| x + y}

run = {
  int r = add 10 2
  print r
}

main run
