#include<gmpxx.h>

struct Configs
{
    const static int KEY_LEN = 1024;
    static int get_block_size()
    {
        
        return (Configs::KEY_LEN - 1)/8; 
    }
};



void testing()
{

}

int main(int argc, char const *argv[])
{
    testing();
    return 0;
}



