#include <iostream>
#include <fstream>
#include <gmpxx.h>

#include "util.h"
#include "rsa.h"

int KEY_SIZE = 1024;

/*
* Generate public private key pairs
* @param path: where to write the key files
* @param name: name of the key. It will generate <name>.prk, <name>.puk
* @param size: size of the key in bits, has to be multiple of 8
*/
void generate_key_pair(std::string path, std::string name, int size)
{
    using namespace RTW;
    RSA rsa(size);
    mpz_class n, d;
    rsa.gen_key_pair(n,d);

    FileIO::write_pub_key(path+"/"+name+".puk",n,size);
    FileIO::write_pri_key(path+"/"+name+".prk",n,d,size);

}

/*
* Encryt file with a public key
* 
* @param path_puk: path to .puk file
* @param key_size: size of key in bits, has to be multiple of 8
* @param path_in: path to file to be encrypted
* @param path_out: path to file to write encrypted file in
* @return   0 on success, 
*          -1 on bad key, 
*          -2 bad in file, 
*          -3 bad out file
*/
int encrypt_file(std::string path_puk, int key_size, std::string path_in, std::string path_out)
{
    using namespace RTW;        

    // Try to open in/out files. Return -2,-3 on error
    std::fstream in, out;
    in.open(path_in,std::ios::binary|std::ios::in);
    if(in.fail())
    {
        in.close();
        return -2;
    }
    out.open(path_out,std::ios::binary|std::ios::out);
    if(out.fail())
    {
        out.close();
        return -3;
    }

    // Read key
    mpz_class n;
    if( FileIO::read_public_key(path_puk,n,key_size) < 0)
    {
        in.close();
        out.close();
        return -1;
    }

    RSA rsa(key_size);
    int block_size = rsa.get_block_size();
    char buffer[key_size/8];
    mpz_class msg, cipher;

    while(!in.eof())
    {
        in.read(buffer,block_size/8);
        int count = in.gcount();
        msg = FileIO::bytes_to_mpz(buffer,count);
        rsa.encrypt(msg,cipher,n);
        
        count = FileIO::mpz_to_bytes(buffer,key_size/8,cipher.get_mpz_t());
        if(count > 0)
            out.write(buffer,key_size/8);
    }

    in.close();
    out.close();    

    return 0;
}


/*
* Decrypt file with a public key
* 
* @param path_prk: path to .prk file
* @param key_size: size of key in bits, has to be multiple of 8
* @param path_in: path to file to be decrypted
* @param path_out: path to file to write decrypted file in
* @return   0 on success, 
*          -1 on bad key, 
*          -2 bad in file, 
*          -3 bad out file
*/
int decrypt_file(std::string path_prk, int key_size, std::string path_in, std::string path_out)
{
    using namespace RTW;        

    // Try to open in/out files. Return -2,-3 on error
    std::fstream in, out;
    in.open(path_in,std::ios::binary|std::ios::in);
    if(in.fail())
    {
        in.close();
        return -2;
    }
    out.open(path_out,std::ios::binary|std::ios::out);
    if(out.fail())
    {
        out.close();
        return -3;
    }

    mpz_class n,d;
    if( FileIO::read_private_key(path_prk,n,d,key_size) < 0)
    {
        in.close();
        out.close();
        return -1;
    }

    RSA rsa(key_size);
    int block_size = rsa.get_block_size();
    char buffer[key_size/8];
    mpz_class msg, cipher;

    while(!in.eof())
    {
        in.read(buffer,key_size/8);
        int count = in.gcount();

        if(!count)
            continue;

        cipher = FileIO::bytes_to_mpz(buffer,key_size/8);
        rsa.decrypt(cipher,msg,n,d);
        
        count = FileIO::mpz_to_bytes(buffer,key_size/8,msg.get_mpz_t());
        if(count > 0)
            out.write(buffer,count);
    }

    in.close();
    out.close();
    return 0;


}

int main(int argc, char const *argv[])
{
    int _return = 0;

    // generate_key_pair("../files","malay",KEY_SIZE);
    // _return = encrypt_file("../files/malay.puk",KEY_SIZE,"../files/giant_file.txt","../files/giant_file.txt.enc");
    _return = decrypt_file("../files/malay.prk",KEY_SIZE,"../files/giant_file.txt.enc","giant_file.txt");
    
    std::cout<<_return<<std::endl;
}
