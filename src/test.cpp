#include "util.h"
#include "rsa.h"
#include <assert.h>
#include <iostream>

#include <gmpxx.h>
#include <fstream>
#include <vector>
#include <thread>

#include <CLI/CLI.hpp>

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


struct char_wrapper
{
    char *buffer;
};


void concurrent_read(char_wrapper w, int buffer_size, int block_size, int offset)
{
    // offset + block_size must be less than buffer_size

    using namespace RTW;

    int count_bytes = block_size;
    if ( (offset*block_size) + block_size > buffer_size )
        count_bytes = buffer_size - (offset*block_size);


    char _b[block_size];
    memcpy(_b, &w.buffer[offset*block_size],count_bytes);

    mpz_class msg = FileIO::bytes_to_mpz(_b,count_bytes);
    mpz_class cipher = mod_exp(msg,655371234567891,1234567891234567);

    // std::cout<<" Offset: "<<offset<<" Start Index: "<<(offset*block_size)<<" Bytes: "<<count_bytes<<std::endl;

    
}

void test_thread_stuff()
{
    int thread_count = 100;
    int block_size = 127;
    int buffer_size = block_size * thread_count;

    std::fstream file;
    file.open("../files/giant_file.txt",std::ios::in|std::ios::binary);

    char buffer[buffer_size];

    std::vector<std::thread*> threads;
    char_wrapper cw { buffer };
    while(!file.eof())
    {
        file.read(buffer,buffer_size);

        int bytes_read = file.gcount();
        int block_count = file.gcount() / block_size;
        if( bytes_read > 0 && bytes_read % block_size != 0 )
            block_count++;

        for(int i = 0; i < block_count; i++)
        {
            std::thread *_t = new std::thread(concurrent_read, cw,bytes_read,block_size,i);
            threads.push_back(_t);
        }

        for(int i = 0 ; i<threads.size(); i++)
            threads[i]->join();

        threads.clear();

    }




}

int test_argpaser(int argc, char const *argv[])
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
    {
        std::cout<<"Key: "<<key_file<<" In:"<<in_file<<" Out:"<<out_file<<std::endl;
        // Do encryption shit and return
    }
    if(app.got_subcommand("decrypt"))
    {
        std::cout<<"Key: "<<key_file<<" In:"<<in_file<<" Out:"<<out_file<<std::endl;
    }

    if(app.got_subcommand("genkey"))
    {
        std::cout<<"Path: "<<out_path<<" Name:"<<name<<std::endl;
    }

}

void test_encrypt_decrypt()
{
    using namespace RTW;
    std::fstream file;
    file.open("../files/file_1.test",std::ios::binary|std::ios::out);
    char buffer[256];
    int i;

    for(i = 0; i < 10; i++)
        buffer[i] = 0;

    for(;i < 125; i++)
        buffer[i] = 'b';

    for(; i < 250; i++)
        buffer[i] = 'a';

    for(; i < 256; i++)
        buffer[i] = 0;

    file.write(buffer,256);
    file.close();

    file.open("../files/file_1.test",std::ios::binary|std::ios::in);

    file.read(buffer,127);
    mpz_class n = FileIO::bytes_to_mpz(buffer,file.gcount());
    std::cout<<n<<"\n\n";

    file.read(buffer,127);
    n = FileIO::bytes_to_mpz(buffer,file.gcount());
    std::cout<<n<<"\n\n";

    file.read(buffer,127);
    n = FileIO::bytes_to_mpz(buffer,file.gcount());
    std::cout<<n<<"\n\n";


}
void test_IO()
{
    using namespace RTW;
    mpz_class m = 0;
    char buffer[128];
    int count = FileIO::mpz_to_bytes(buffer,128,m.get_mpz_t());
    std::cout<<count<<std::endl;
}

int main(int argc, char const *argv[])
{
    // test_RandIntGenerator();   
    // test_RSA();

    // test_thread_stuff();
    // test_argpaser(argc,argv);
    // test_IO();

    test_encrypt_decrypt();



    return 0;
}
