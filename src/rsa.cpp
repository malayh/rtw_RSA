#pragma once
#include <gmpxx.h>
#include "util.h"
#include "rsa.h"


// ------------- RSA ------------------------------------
/*
NTS:
    - in encrypt have to verify that msg < n
*/

RTW::RSA::RSA(int key_size)  
{
    m_key_size = key_size;
    m_block_size = key_size - 8;
}

int RTW::RSA::get_block_size() { return m_block_size; }

bool RTW::RSA::is_prime(mpz_class num, int check_count, int max_bit_len)
{
    if(num % 2 == 0)
        return false;

    mpz_class r, d, n0 = num - 1, tmp, a;
    
    // Does r has to start from 1?
    r = 0;
    while(true)
    {
        r++;
        tmp = fast_exp(2,r);
        if( n0 % tmp != 0)
            continue;

        d = n0 / tmp;
        if(d % 2 == 0)
            continue;
        
        break;
    }

    while(--check_count)
    {            
        a = ( m_rg.rand_int(max_bit_len) % n0-1 ) + 2;
        tmp = mod_exp(a,d,num);

        // Means, the test passed for current a. So move to the next test
        if(tmp == 1 || tmp == n0)
            continue;

        //If control reacheas here, the test it not conclusive yet
        bool passed = false;

        for(mpz_class i = 0 ; i < r; i++)
        {
            tmp = mod_exp(tmp,2,num);
            // Test pass
            if(tmp == n0)
            {
                passed = true;
                break;
            }
            
            //if 1, definitly composit
            if(tmp == 1)
                return false;

        }

        // if control reaches here without tests being conclusive, the number is composit
        if(!passed)
            return false;
        
    }

    return true;
}

void RTW::RSA::gen_key_pair(mpz_class &n, mpz_class &d)
{
    
    // Since, size of m (message) will be at most 2^m_block_size
    // and for m%n to yeild a remainder, n has to be greater than m.
    // Hense n has to be greater than 2^m_block_size
    mpz_class min_n = fast_exp(2,m_block_size);

    // Since n has to be a `key_size` bit int, and n = p*q
    // Thus, p, q can be at most m_key_size/2 bit long
    int prime_number_len = m_key_size/2;

    mpz_class p,q,phi;

    do
    {
        for(p = m_rg.rand_int(prime_number_len); !is_prime(p,120,prime_number_len); p = m_rg.rand_int(prime_number_len));
        for(q = m_rg.rand_int(prime_number_len); !is_prime(q,120,prime_number_len); q = m_rg.rand_int(prime_number_len));
        n = p*q;
    } while ( n < min_n );

    phi = (p - 1) * (q - 1);

    //calculate d
    mpz_class k;
    for(k = 1; ((k*phi)+1) % m_e != 0; k++);
    d = ((k*phi)+1)/m_e;

}

void RTW::RSA::encrypt(const mpz_class &msg, mpz_class &cipher, const mpz_class &n)
{
    cipher = mod_exp(msg,m_e,n);
}

void RTW::RSA::decrypt(const mpz_class &cipher, mpz_class &msg, const mpz_class &n, const mpz_class &d)
{
    msg = mod_exp(cipher,d,n);
}
