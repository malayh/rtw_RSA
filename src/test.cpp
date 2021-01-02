#include<iostream>
#include<fstream>
#include<gmpxx.h>
#include<ctime>


struct Configs
{
    const static int KEY_LEN = 1024;
    const static int BLOCK_SIZE = (Configs::KEY_LEN - 1)/8; 
};


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


class RandGen
{
    private:
        gmp_randstate_t m_rstate;
        mpz_t m_random;

    public:
    RandGen()
    {
        gmp_randinit_mt(m_rstate);
        gmp_randseed_ui(m_rstate,std::time(0));
        mpz_init(m_random);
    }
    
    // Returns a < pow(2,max_bit_len) - 1
    mpz_class randmpz(int max_bit_len)
    {
        mpz_urandomb(m_random,m_rstate,max_bit_len);
        mpz_class r(m_random);
        return r;
    }


};


bool miller_rabin(mpz_class num, int check_count, int bit_len, RandGen &rg)
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
        
        a = ( rg.randmpz(bit_len) % n0-1 ) + 2;
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

        if(!passed)
            return false;
        
    }

    return true;
    


}

bool is_prime(mpz_class num)
{
    for(mpz_class i = 3; i<num; i++ )
        if (num % i == 0 )
            return true;

    return false;
}


void generate_keys(std::string name)
{

    // Write bytes to file and digits.

    mpz_class min = fast_exp(2,Configs::BLOCK_SIZE*8);
    mpz_class p,q,n,d,e,phi;
    RandGen rg;

    do
    {
        for(p = rg.randmpz(Configs::KEY_LEN/2); ! miller_rabin(p,128,Configs::KEY_LEN/2, rg); p = rg.randmpz(Configs::KEY_LEN/2));
        for(q = rg.randmpz(Configs::KEY_LEN/2); ! miller_rabin(q,128,Configs::KEY_LEN/2, rg); q = rg.randmpz(Configs::KEY_LEN/2));
        n = p * q;
    } while (n < min);


    // Have to make sure gcd(e,phi) = 1
    e = 65537;
    phi = (p - 1) * (q - 1);

    // find d
    mpz_class k;
    for(k = 1; ((k*phi)+1) % e != 0; k++);
    d = ((k*phi)+1)/e;


    char buffer[Configs::KEY_LEN/8];
    size_t count_bytes_written;

    // Lest significant digit first. $3 = -1
    // size of each word = sizeof(buffer[0])
    // each word is written in least significant first maner, $5 = -1
    // no nails
    mpz_export(buffer, &count_bytes_written, -1 , sizeof(buffer[0]), -1, 0, n.get_mpz_t());
    std::cout<<"Bytes n: "<<count_bytes_written<<std::endl;
    // Since LSB is written first, if count_bytes_written < buffer size, I can populate the end of the buffer with 0 s and write it to file
    for(int i = count_bytes_written; i < Configs::KEY_LEN/8; i++)
        buffer[i] = 0;

    std::ofstream pub_key;
    pub_key.open( name+".pubkey",std::ios::out | std::ios::binary);
    pub_key.write(buffer,Configs::KEY_LEN/8);
    pub_key.close();

    // private key
    mpz_export(buffer, &count_bytes_written, -1 , sizeof(buffer[0]), -1, 0, d.get_mpz_t());
    std::cout<<"Bytes d: "<<count_bytes_written<<std::endl;
    for(int i = count_bytes_written; i < Configs::KEY_LEN/8; i++)
        buffer[i] = 0;

    std::ofstream pri_key;
    pri_key.open( name+".prikey",std::ios::out | std::ios::binary);
    pri_key.write(buffer,Configs::KEY_LEN/8);
    pri_key.close();


    std::cout<<n<<std::endl<<d<<std::endl;


    
}


void read_keys(mpz_class &pb_k, mpz_class &pr_k )
{
    std::ifstream pub_key, pri_key;
    char buffer[128];

    pub_key.open("testKeygen.pubkey",std::ios::binary|std::ios::in);
    pub_key.read(buffer,128);
    pub_key.close();

    mpz_t _t;
    mpz_init(_t);
    mpz_import(
        _t ,                // where to load to
        128,                // How many words to read,
        -1,                 // LSW first
        sizeof(buffer[0]),  // size of each words
        -1,                 // Each word is little endien
        0,                  // no nails
        buffer              // read from
    );

    pb_k = mpz_class(_t);

    pri_key.open("testKeygen.prikey",std::ios::binary|std::ios::in);
    pri_key.read(buffer,128);
    pri_key.close();
    mpz_init(_t);
    mpz_import(
        _t ,                // where to load to
        128,                // How many words to read,
        -1,                 // LSW first
        sizeof(buffer[0]),  // size of each words
        -1,                 // Each word is little endien
        0,                  // no nails
        buffer              // read from
    );

    pr_k = mpz_class(_t);
}

void test_encrypt()
{
    std::fstream f_in,f_out,f_pubkey;
    f_in.open("sample.txt",std::ios::binary|std::ios::in);
    f_out.open("sample.encrypted",std::ios::binary|std::ios::out);

    mpz_class n,d;
    read_keys(n,d);

    mpz_t _t;
    mpz_init(_t);

    char buffer[128];
    f_in.read(buffer,127);
    buffer[127] = 0;
    f_in.close();

    mpz_import(
        _t ,                // where to load to
        128,                // How many words to read,
        -1,                 // LSW first
        sizeof(buffer[0]),  // size of each words
        -1,                 // Each word is little endien
        0,                  // no nails
        buffer              // read from
    );

    mpz_class m(_t);
    mpz_class e = 65537;

    mpz_class c = mod_exp(m,e,n);
    size_t count_bytes_written;
    mpz_export(buffer, &count_bytes_written, -1 , sizeof(buffer[0]), -1, 0, c.get_mpz_t());
    for(int i = count_bytes_written; i < Configs::KEY_LEN/8; i++)
        buffer[i] = 0;

    f_out.write(buffer,128);
    f_out.close();

}


void test_decrypt()
{
    std::fstream f_in,f_out;
    f_in.open("sample.encrypted",std::ios::binary|std::ios::in);
    f_out.open("sample_decrypted.txt",std::ios::binary|std::ios::out);

    mpz_class n,d;
    read_keys(n,d);

    char buffer[128];
    mpz_t _t;
    mpz_init(_t);

    f_in.read(buffer,128);
    f_in.close();

    mpz_import(
        _t ,                // where to load to
        128,                // How many words to read,
        -1,                 // LSW first
        sizeof(buffer[0]),  // size of each words
        -1,                 // Each word is little endien
        0,                  // no nails
        buffer              // read from
    );

    mpz_class c(_t);
    mpz_class m = mod_exp(c,d,n);

    size_t count_bytes_written;
    mpz_export(buffer, &count_bytes_written, -1 , sizeof(buffer[0]), -1, 0, m.get_mpz_t());
    f_out.write(buffer,count_bytes_written);
    f_out.close();

}

int main(int argc, char const *argv[])
{
    // test_encrypt();
    test_decrypt();
    
    
    return 0;
}
