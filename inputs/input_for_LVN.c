#include <stdio.h>
int add(int a, int b) {
  int x = (a + b) * (a + b);
  int y = 30;
  int r = 20 * y + x;
  return r;
}
int main() {
  printf("%d", add(10, 20));
  return 0;
}
