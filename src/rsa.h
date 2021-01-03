#include<gmpxx.h>
#include "util.h"

namespace RTW
{

/*
NTS:
    - in encrypt have to verify that msg < n
*/
class RSA
{
    private:
        RandIntGenerator m_rg;
        const mpz_class m_e = 65537;
        int m_key_size;
        int m_block_size;

        /*
        * Performs millar-rabin primality test
        * 
        * @param num: nuber to be tested for primality
        * @param check_count: `check_count` many number of rounds will be performed
        * @param max_bit_len: `num` is a `max_bit_len` bit int
        */
        bool is_prime(mpz_class num, int check_count, int max_bit_len);

    public:
        /// Constuctor
        RSA(int key_size);

            

        /*
        * Generates public-private key pair
        * 
        * @param[in] key_size: number of in the generated key
        * @param[out] n: [out] public key 
        * @param[out] d: [out] private key
        */
        void gen_key_pair(mpz_class &n, mpz_class &d);


        /*
        * Encrytes message
        * @param msg: message
        * @param cipher: cipher will be writen to this
        * @param n: public mod
        * 
        * msg MUST BE less then n
        */
        void encrypt(const mpz_class &msg, mpz_class &cipher, const mpz_class &n);

        /*
        * Decryptes cipher
        * @param cipher: cipher to be decryted
        * @param msg: message will be written to this
        * @param n: public mod
        * @param d: private exponent
        * 
        * msg MUST BE less then n
        */
        void decrypt(const mpz_class &cipher, mpz_class &msg, const mpz_class &n, const mpz_class &d);

};


}// namsespace RTW