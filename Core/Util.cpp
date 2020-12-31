#include "Util.h"

int mod( int x, int m ) { return x >= 0 ? x % m : (x+1) % m + m-1; }
