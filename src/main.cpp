#include<gmpxx.h>
#include "util.h"

int main(int argc, char const *argv[])
{
    using namespace RTW;
    mpz_class a = 5, b = 100;
    mpz_class c = mod_exp(a,b,5);
    return 0;
}
