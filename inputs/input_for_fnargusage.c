int demo(int a, int *b) {
    return a * 2 + *b;
}

int callee(char a, int b, long c) {
    return demo(b, (long *)&a); // ptr to char -> ptr to long -> ptr to int
}

int main() {
    callee('a', 10, 100l); // full correct call
    int i;
    callee(i, 10, 100L); // int instead of char
    int *ip = &i;
    int **ipp = &ip;
    demo(100, ip); // full correct call
    demo(100, ipp); // ptr instead of ptr to ptr
    demo(100L, ip); // convert int to long - ignored by LLVM
    return 0;
}
