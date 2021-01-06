<p align="center">
  <img height="100" src="https://imgur.com/download/OTNcfwW/" alt="rsa"/>
</p>


## Why  `Reinvent the wheel`
Because all those who invented something refused to use whatever already existed. 

#### The goal of `Reinvent the wheel` is to do the following
- Learn and implement cool/important algorithms from scratch.
- Create something usable with it.
- Document every detail of the algorithm and its implementation.

#### What will that achieve?
- You will have a deeper understanding of the algorithm as you are trying to use it beyond just a toy implementation.
- Apart from the target algorithm you'll learn a lot of other stuff that goes into getting a software working.
- You can use one such implementation as a gateway to dive deeper into the set of problem such algorithms try to solve.


## RSA
This is a implemention of RSA cryptosystem from scratch. I have documented how the underlying mathematics and the implementation in the following blog post. I have tried my best to make it pedagogically complete. I hope it's useful.

* To do: Add url to blog post



## Setup
This implementation depends on GMP.

### Install GMP

```
wget https://gmplib.org/download/gmp/gmp-6.2.1.tar.lz
```
```
tar --lzip -x gmp-6.2.1.tar.lz
```
```
sudo apt-get install m4
```
```
./configure --enable-cxx
```
```
make && make check
```
```
make install
```


In your `.bashrc` add this line
```
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### Setup build
- Create a dir named `build` in the main project dir
- CD into `build` and run 
  ```
  cmake . -s ../
  ```
- Run `make`

### Usage

You can encrypt or decrypt text files using key pairs.

- Generate keypair
```
./rtwrsa genkey -p . -n malay 
```
This will create a pair of key files named `malay.puk` and `malay.prk` in the current working directory. `.puk` is public key `.prk` is private key

- Encrypt text file
```
./rtwrsa encrypt -k malay.puk -i sample.txt -o sample.txt.enc
```
This will encrypt `sample.txt` using public key `malay.puk` and write the encrpted file to `sample.txt.enc`

- Decrypt a file
```
./rtwrsa decrypt -k malay.prk -i sample.txt.enc -o sample_d.txt
```
This will decrypt `sample.txt.enc` using private key `malay.prk` and write the decrypted text file to `sample_d.txt`
