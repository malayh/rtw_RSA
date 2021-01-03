#pragma once
#include<gmpxx.h>


namespace RTW
{
    /*
    * Fast exponetiation.
    * 
    * @param a number
    * @param exp exponent
    * @return a ^ exp
    */
    mpz_class fast_exp(const mpz_class &a, const mpz_class &exp);

    /*
    * Modular exponentiation.
    *
    * @param a number
    * @param exp exponent
    * @param mod modulus
    * 
    * @return (a^exp)%mod
    */
    mpz_class mod_exp(const mpz_class &a, const mpz_class &exp, const mpz_class &mod);

    /*
    * Utility to generate random gmp ints
    */
    class RandIntGenerator
    {
        private:
            gmp_randstate_t m_rstate;
            mpz_t m_random;

        public:
        RandIntGenerator();

        /*
        *  Generate random int
        * 
        *  @param max_bit_len: maximum number of bits to be used for the number
        *  @return 0 < mpz_class < (2^max_bit_len) - 1
        */
        mpz_class rand_int(int max_bit_len);
    };

    namespace FileIO
    {
        /*
        * Reads mpz_class from array of bytes
        * 
        * @param buffer: char array to read bytes from
        * @param count: how many bits to read from `buffer`
        * @return mpz_class represented by the 
        */
        mpz_class bytes_to_mpz(const char *buffer, size_t count);

        /*
        * Converts mpz_t to bytes. 
        * If size of `buffer` is greater than size requeired to accomodate `value`, remaining space will be filled with 0s
        * 
        * @param buffer: `buffer` must have space to accomodate `value`
        * @param size: `size` of `buffer`
        * @param value: `value` to be converted to bytes
        * @return int how many bytes were needed to accomodate value
        */
        int mpz_to_bytes(char *buffer, size_t size, mpz_t value);

        /*
        * Write Private key to file
        * 
        * @param path: path of private key
        * @param n: public modulas
        * @param d: private exponent
        * @param key_size: size of key in bits, has to be multiple of 8
        */
        void write_pri_key(std::string path, mpz_class &n, mpz_class &d, int key_size);

        /*
        * Write Public key to file
        * 
        * @param path: path of private key
        * @param n: public modulas
        * @param key_size: size of key in bits, has to be multiple of 8
        */
        void write_pub_key(std::string path, mpz_class &n, int key_size);

        /*
        * Reads private key from file. 
        * 
        * @param path: path to .prk file
        * @param n: public modulas
        * @param d: private exponent
        * @param key_size: size of key in bits, must be multiple of 8
        */
        int read_private_key(std::string path, mpz_class &n, mpz_class &d, int key_size);

        /*
        * Reads private key from file. 
        * 
        * @param path: path to .pbk file
        * @param n: public modulas
        * @param key_size: size of key in bits, must be multiple of 8
        * @return 0 on success, -1 on failure
        */
        int read_public_key(std::string path, mpz_class &n, int key_size);

    } // Namespace fileIO
} //namespace RTW