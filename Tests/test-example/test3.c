int foo(unsigned a) {
    int temp[20];
    for (int i = 0; i < 20; i ++)
    if ((a & (1 << i )) > 0) {
        temp[i] = 1;
    } else temp[i] = 0;
    int sum  = 0;
    for (int i = 0; i < 20; i ++) {
      if (temp[i] > 0)
        sum += (temp[i] > 0) ? 1:  0;
    }
    switch (sum) {
        case 0 : sum = 2; break;
        case 1 : sum = 4; break;
        case 2 : sum = 5; break;
        default: break;
    }
    return sum;
}
