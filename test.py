import random

def prove_identity_1():
    """
    (m*n)mod p = ((m mod p) * (n mod p)) mod p
    """
    m = random.randint(1,100)
    n = random.randint(1,100)
    p = random.randint(1,100)

    print(f"m={m}\nn={n}\np={p}")

    rhs = (m*n) % p
    lhs = ((m % p) * (n % p))%p    
    print(rhs==lhs)


def mod_exp(a,b,m):
    if b == 0:
        return 1
    if b == 1:
        return a % m
    r = mod_exp(a,b//2,m)
    if b % 2 == 0:
        return r*r % m    
    return (r * r * a) % m

a = 12
d = 5
n = 81
print(mod_exp(a,d,n))


