#include "util.h"
#include <ctime>
#include <fstream>


mpz_class RTW::fast_exp(const mpz_class &a, const mpz_class &exp)
{
    if(exp == 0)
        return 1;
    if(exp == 1)
        return a;

    mpz_class r = fast_exp(a,exp/2);
    if(exp % 2 == 0)
        return r*r;
    else
        return r*a*r;
}

mpz_class RTW::mod_exp(const mpz_class &a, const mpz_class &exp, const mpz_class &mod)
{
    if( exp == 0 )
        return 1;
    
    if( exp == 1)
        return a % mod;

    mpz_class r = mod_exp(a,exp/2,mod);
    if( exp%2 == 0)
        return (r*r)%mod;

    return (r*r*a)%mod;
}


// ------------------ Random int generator ----------------------

RTW::RandIntGenerator::RandIntGenerator()
{
    gmp_randinit_mt(m_rstate);
    gmp_randseed_ui(m_rstate,std::time(0));
    mpz_init(m_random);
}

mpz_class RTW::RandIntGenerator::rand_int(int max_bit_len)
{
    mpz_urandomb(m_random,m_rstate,max_bit_len);
    mpz_class r(m_random);
    return r;
}


// ------------------ File IO -----------------------------------

mpz_class RTW::FileIO::bytes_to_mpz(const char *buffer, size_t count)
{
    mpz_t _t;
    mpz_init(_t);

    mpz_import(
        _t ,                // where to load to
        count,              // How many words to read,
        -1,                 // Least significant word first
        sizeof(buffer[0]),  // size of each words
        -1,                 // Each word is little endien
        0,                  // no nails
        buffer              // read from
    );

    mpz_class r(_t);
    return r;
}

int RTW::FileIO::mpz_to_bytes(char *buffer, size_t size, mpz_t value)
{
    size_t bytes_written;
    mpz_export(
        buffer,             // where to write. Buffer must have enough space to accomodate the values
        &bytes_written,     // Will update how many bytes are written into the buffer from the number
        -1 ,                // Least significant words first
        sizeof(buffer[0]),  // Size of each word
        -1,                 // Each word are little endien
        0,                  // no nails
        value               // where to read from
    );

    // If bytes_written < size, I can fill the remaining of the buffer as 0s, beacase numbers are little endian
    // and 0s after most significant word are ignored
    for(int i = bytes_written; i < size ; i++)
        buffer[i] = 0;

    return bytes_written;

}

void RTW::FileIO::write_pub_key(std::string path, mpz_class &n, int key_size)
{
    std::fstream file;
    file.open(path,std::ios::binary|std::ios::out); 

    char buffer[key_size/8];
    mpz_to_bytes(buffer,key_size/8,n.get_mpz_t());
    file.write(buffer,key_size/8);
    file.close();
}

void RTW::FileIO::read_private_key(std::string path, mpz_class &n, mpz_class &d, int key_size)
{
    std::fstream file;
    file.open(path,std::ios::binary|std::ios::in);

    char buffer[key_size/8];
    file.read(buffer,key_size/8);
    n = bytes_to_mpz(buffer,key_size/8);
    
    file.read(buffer,key_size/8);
    d = bytes_to_mpz(buffer,key_size/8);
    file.close();
}

void RTW::FileIO::read_public_key(std::string path, mpz_class &n, int key_size)
{
    std::fstream file;
    file.open(path,std::ios::binary|std::ios::in);

    char buffer[key_size/8];
    file.read(buffer,key_size/8);
    n = bytes_to_mpz(buffer,key_size/8);
    file.close();
}