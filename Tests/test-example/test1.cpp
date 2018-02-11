int foo(volatile unsigned a) {
  volatile unsigned x = 10;
  
  if (a > 5)
    x = x + 5;
  if(a > 10)
    x = x + 50;
  if(a > 20)
    x = x + 1;
  else
    return x;
  
  return x+a;
}
