#include "util.h"
#include "rsa.h"
#include <assert.h>
#include <iostream>

void test_RandIntGenerator()
{
    RTW::RandIntGenerator rg;

    int bit_len = 32;
    mpz_class max = RTW::fast_exp(2,bit_len);
    mpz_class a = rg.rand_int(bit_len);
    std::cout<<"32 bit int: "<<a<<std::endl;
    assert(a<max);
}

void test_RSA()
{
    // Test key gen
    RTW::RSA rsa(1024);
    mpz_class n,d;
    rsa.gen_key_pair(n,d);    
    mpz_class c = RTW::mod_exp(12348,65537,n);
    mpz_class m = RTW::mod_exp(c,d,n);
    assert(m == 12348);
    std::cout<<"Passed: Key gen. \n";

    // test buffer i/o
    
    char buffer[128];
    RTW::FileIO::mpz_to_bytes(buffer,128,m.get_mpz_t());
    mpz_class parsed_m = RTW::FileIO::bytes_to_mpz(buffer,128);
    assert(parsed_m == m);
    std::cout<<"Passed: Byte I/0\n";

    // shot messages test
    char *test_str = "malay";
    mpz_class short_msg = RTW::FileIO::bytes_to_mpz(test_str,5);
    mpz_class e_short_msg = RTW::mod_exp(short_msg,65537,n);
    mpz_class d_short_msg = RTW::mod_exp(e_short_msg,d,n);

    char buffer_2[128];
    int x = RTW::FileIO::mpz_to_bytes(buffer_2,128,d_short_msg.get_mpz_t());

    assert(x == 5);
    std::cout<<"Passed: short message encryption."<<std::endl;

    
    // write keys to files
    std::string path = "./testing_key_write";
    RTW::FileIO::write_pri_key(path+".prk",n,d,1024);
    RTW::FileIO::write_pub_key(path+".pbk",n,1024);

    mpz_class r_n, r_d, r_n1;
    RTW::FileIO::read_private_key(path+".prk",r_n,r_d, 1024);
    RTW::FileIO::read_public_key(path+".pbk",r_n1,1024);
    assert(n == r_n);
    assert(d = r_d);
    assert(n == r_n1);
    std::cout<<"Passed: IO keys\n";

    // test encrypt decryt
    mpz_class msg, cipher, decrypted;
    msg = 900;
    rsa.encrypt(msg,cipher,n);
    rsa.decrypt(cipher,decrypted,n,d);
    assert(decrypted == msg);
    std::cout<<"Passed: Encryption/Decryption.\n";

}

int main(int argc, char const *argv[])
{
    test_RandIntGenerator();   
    test_RSA();

    return 0;
}
