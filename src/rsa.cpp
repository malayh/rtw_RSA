#include<gmpxx.h>
#include<ctime>
#include<iostream>
#include<fstream>

// NTS = Note To Self
// NTS: Move this to a separate file with the tests
#include<assert.h>

// Reinvent The Wheel
namespace RTW
{


// NTS: Don't access this directly, pass it to whoever needs it
struct Configs
{
    const static int KEY_LEN = 1024;
    const static int BLOCK_SIZE = (Configs::KEY_LEN - 1)/8; 
};


/*
* Fast exponetiation.
* 
* @param a number
* @param exp exponent
* @return a ^ exp
*/
mpz_class fast_exp(const mpz_class &a, const mpz_class &exp)
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

/*
* Modular exponentiation.
*
* @param a number
* @param exp exponent
* @param mod modulus
* 
* @return (a^exp)%mod
*/
mpz_class mod_exp(const mpz_class &a, const mpz_class &exp, const mpz_class &mod)
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


/*
* Utility to generate random gmp ints
*/
class RandIntGenerator
{
    private:
        gmp_randstate_t m_rstate;
        mpz_t m_random;

    public:
    RandIntGenerator()
    {
        gmp_randinit_mt(m_rstate);
        gmp_randseed_ui(m_rstate,std::time(0));
        mpz_init(m_random);
    }

    /*
    *  Generate random int
    * 
    *  @param max_bit_len: maximum number of bits to be used for the number
    *  @return 0 < mpz_class < (2^max_bit_len) - 1
    */
    mpz_class rand_int(int max_bit_len)
    {
        mpz_urandomb(m_random,m_rstate,max_bit_len);
        mpz_class r(m_random);
        return r;
    }
    
};


/*
NTS:
    - Read/Write function need validations
        - If files doesn't exist?
        - If file is corrupted?
        - If key size mentioned is not a multiple of 8?
        - If key we are trying to read is on diffent size?
*/
class FileIO
{
    public:

    /*
    * Reads mpz_class from array of bytes
    * 
    * @param buffer: char array to read bytes from
    * @param count: how many bits to read from `buffer`
    * @return mpz_class represented by the 
    */
    mpz_class bytes_to_mpz(const char *buffer, size_t count)
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

    /*
    * Converts mpz_t to bytes
    * 
    * @param buffer: `buffer` must have space to accomodate `value`
    * @param size: `size` of `buffer`
    * @param value: `value` to be converted to bytes
    * @return int how many bytes were needed to accomodate value
    * 
    * if size of `buffer` is greater than size requeired to accomodate `value`, remaining space will be filled with 0s
    */
    int mpz_to_bytes(char *buffer, size_t size, mpz_t value)
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
    

    /*
    * Write Private key to file
    * 
    * @param path: path of private key
    * @param n: public modulas
    * @param d: private exponent
    * @param key_size: size of key in bits, has to be multiple of 8
    */
    void write_pri_key(std::string path, mpz_class &n, mpz_class &d, int key_size)
    {
        std::fstream file;
        file.open(path,std::ios::binary|std::ios::out); 

        char buffer[key_size/8];
        mpz_to_bytes(buffer,key_size/8,n.get_mpz_t());
        file.write(buffer,key_size/8);

        mpz_to_bytes(buffer,key_size/8,d.get_mpz_t());
        file.write(buffer,key_size/8);

        file.close();

    }

    /*
    * Write Public key to file
    * 
    * @param path: path of private key
    * @param n: public modulas
    * @param key_size: size of key in bits, has to be multiple of 8
    */
    void write_pub_key(std::string path, mpz_class &n, int key_size)
    {
        std::fstream file;
        file.open(path,std::ios::binary|std::ios::out); 

        char buffer[key_size/8];
        mpz_to_bytes(buffer,key_size/8,n.get_mpz_t());
        file.write(buffer,key_size/8);
        file.close();
    }

    /*
    * Reads private key from file. 
    * 
    * @param path: path to .prk file
    * @param n: public modulas
    * @param d: private exponent
    * @param key_size: size of key in bits, must be multiple of 8
    */
    void read_private_key(std::string path, mpz_class &n, mpz_class &d, int key_size)
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

    /*
    * Reads private key from file. 
    * 
    * @param path: path to .pbk file
    * @param n: public modulas
    * @param key_size: size of key in bits, must be multiple of 8
    */
    void read_public_key(std::string path, mpz_class &n, int key_size)
    {
        std::fstream file;
        file.open(path,std::ios::binary|std::ios::in);

        char buffer[key_size/8];
        file.read(buffer,key_size/8);
        n = bytes_to_mpz(buffer,key_size/8);
        file.close();
    }

};


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
    bool is_prime(mpz_class num, int check_count, int max_bit_len)
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


    public:
        
    /// Constuctor
    RSA(int key_size)  
    {
        m_key_size = key_size;
        m_block_size = key_size - 8;
    }


    /*
    * Generates public-private key pair
    * 
    * @param[in] key_size: number of in the generated key
    * @param[out] n: [out] public key 
    * @param[out] d: [out] private key
    */
    void gen_key_pair(mpz_class &n, mpz_class &d)
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


    /*
    * Encrytes message
    * @param msg: message
    * @param cipher: cipher will be writen to this
    * @param n: public mod
    * 
    * msg MUST BE less then n
    */
    void encrypt(const mpz_class &msg, mpz_class &cipher, const mpz_class &n)
    {
        cipher = mod_exp(msg,m_e,n);
    }

    /*
    * Decryptes cipher
    * @param cipher: cipher to be decryted
    * @param msg: message will be written to this
    * @param n: public mod
    * @param d: private exponent
    * 
    * msg MUST BE less then n
    */
    void decrypt(const mpz_class &cipher, mpz_class &msg, const mpz_class &n, const mpz_class &d)
    {
        msg = mod_exp(cipher,d,n);
    }
};

void generate_keys(std::string &dir, std::string &name, int size)
{
    RSA rsa(size);
    FileIO fileIO;

    mpz_class n, d;
    rsa.gen_key_pair(n,d);

    std::string prk_path = dir + "/" + name + ".prk";
    std::string pbk_path = dir + "/" + name + ".pbk";
    fileIO.write_pri_key(prk_path,n,d,size);
    fileIO.write_pub_key(pbk_path,n,size);
}


void encrypt_file(std::string &in_path, std::string &out_path)
{
    
}

} // namespace RTW


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
    RTW::FileIO fileIO;
    char buffer[128];
    fileIO.mpz_to_bytes(buffer,128,m.get_mpz_t());
    mpz_class parsed_m = fileIO.bytes_to_mpz(buffer,128);
    assert(parsed_m == m);
    std::cout<<"Passed: Byte I/0\n";

    // shot messages test
    char *test_str = "malay";
    mpz_class short_msg = fileIO.bytes_to_mpz(test_str,5);
    mpz_class e_short_msg = RTW::mod_exp(short_msg,65537,n);
    mpz_class d_short_msg = RTW::mod_exp(e_short_msg,d,n);

    char buffer_2[128];
    int x = fileIO.mpz_to_bytes(buffer_2,128,d_short_msg.get_mpz_t());

    assert(x == 5);
    std::cout<<"Passed: short message encryption."<<std::endl;

    
    // write keys to files
    std::string path = "./testing_key_write";
    fileIO.write_pri_key(path+".prk",n,d,1024);
    fileIO.write_pub_key(path+".pbk",n,1024);

    mpz_class r_n, r_d, r_n1;
    fileIO.read_private_key(path+".prk",r_n,r_d, 1024);
    fileIO.read_public_key(path+".pbk",r_n1,1024);
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

void test_functionalities()
{
    //generate keys
    std::string dir = ".";
    std::string name = "malay";
    RTW::generate_keys(dir,name,1024);

}
int main()
{
    // test_RSA();
    // test_RandIntGenerator();

    // std::string dir = ".";
    // std::string name = "malay";
    // RTW::generate_keys(dir,name,1024);

    RTW::FileIO io;
    std::string pvt = "malay.prk";
    mpz_class n,d;
    io.read_private_key(pvt,n,d,1024);
    std::cout<<n<<"\n"<<d<<"\n";
    return 0;
}



