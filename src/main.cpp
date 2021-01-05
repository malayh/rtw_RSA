#include <iostream>
#include <fstream>
#include <gmpxx.h>

#include "util.h"
#include "rsa.h"
#include <CLI/CLI.hpp>

int KEY_SIZE = 1024;
int THREAD_COUNT = 100;

/*
* Todo
*   - need to add error handling in key read/write functions
*   - make encryption and decryption parallel
*/


/*
* Generate public private key pairs
* @param path: where to write the key files
* @param name: name of the key. It will generate <name>.prk, <name>.puk
* @param size: size of the key in bits, has to be multiple of 8
* @return 0 of success, -1 on failure
*/
int generate_key_pair(std::string path, std::string name, int size)
{
    using namespace RTW;
    RSA rsa(size);
    mpz_class n, d;
    rsa.gen_key_pair(n,d);

    if( FileIO::write_pub_key(path+"/"+name+".puk",n,size) < 0 )
        return -1;

    if ( FileIO::write_pri_key(path+"/"+name+".prk",n,d,size) < 0 )
        return -1;

    return 0;

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
    // This shit is slow, because of the giant values of d.
    // Todo: Put multiple thread reading the file
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

/*
* Dispatch action according to command.
* @param argc argc
* @param argv argv
* @return status code of the task dispatched
*/
int dispatch_action(int argc, char const *argv[])
{
    CLI::App app{"Encrypt/Decrypt Files using RSA."};

    app.require_subcommand(1);
    

    std::string key_file = "";
    std::string in_file = "";
    std::string out_file = "";

    std::string out_path = "";
    std::string name = "";


    CLI::App *enc = app.add_subcommand("encrypt","Encrypt file with a public key.");
    enc->add_option("-k",key_file,"Path to .puk file")->required();
    enc->add_option("-i",in_file,"Path to file")->required();
    enc->add_option("-o",out_file,"Path to write encrypted file")->required();

    CLI::App *dec = app.add_subcommand("decrypt", "Decrypt file with a private key.");
    dec->add_option("-k",key_file,"Path to .prk file")->required();
    dec->add_option("-i",in_file,"Path to encryted file")->required();
    dec->add_option("-o",out_file,"Path to write encrypted file")->required();

    CLI::App *keygen = app.add_subcommand("genkey","Generate a public-private key pair.");
    keygen->add_option("-p",out_path,"Path to dir to write the files")->required();
    keygen->add_option("-n",name,"Name of the key.")->required();

    CLI11_PARSE(app,argc,argv);

    if(app.got_subcommand("encrypt"))
        return encrypt_file(key_file,KEY_SIZE,in_file,out_file);

    if(app.got_subcommand("decrypt"))
        return decrypt_file(key_file,KEY_SIZE,in_file,out_file);

    if(app.got_subcommand("genkey"))
        return generate_key_pair(out_path,name,KEY_SIZE);

    return -1;
}

int main(int argc, char const *argv[])
{
    return dispatch_action(argc,argv);
}
