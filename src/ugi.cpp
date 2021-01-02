#include<iostream>
#include<vector>
#include<cmath>

#define Print(x) std::cout<<x<<std::endl
// #define _UBI_DEBUG
class u_giant_int
{
    /*
        Little endien: Least index will have least significant digits
    */
    private:
        std::vector<unsigned char> *m_digits;

        u_giant_int(std::vector<unsigned char> *ptr)
        {
            // This constuctor is kept private to ensure no user of this class can pass a pointer that 
            // the user later calls a delete upon. No dangling pointer shit
            m_digits = ptr;
            strip();
        }

        u_giant_int karatsuba(const u_giant_int &a, const u_giant_int &b )
        {
            // Shit ton of copying going on in this one.
            // TODO: Remove those shit tone of copying somehow

            if(a.length() == 1 && b.length() == 1)
                return u_giant_int(a.at(0)*b.at(0));

            
            int m = a.length() > b.length() ? std::ceil((float)a.length()/2) : std::ceil((float)b.length()/2);


            // TODO: Copies can be avoided when m > a.lenght()
            // x0
            int size_x0 = m > a.length() ? a.length() : m ;
            
            std::vector<unsigned char> *ptr_x0 = new std::vector<unsigned char>(size_x0);
            for(int i = 0; i < size_x0; i++)
                (*ptr_x0)[i] = a.at(i);

            u_giant_int x0(ptr_x0);

            // x1
            int size_x1 = a.length() - m > 0 ? a.length() - m : 1; 
            std::vector<unsigned char> *ptr_x1 = new std::vector<unsigned char>(size_x1);
            if(size_x1 < a.length())
            {
                for(int j = 0, i = size_x0; i < a.length();i++, j++)
                    (*ptr_x1)[j] = a.at(i);
            }
            else
                (*ptr_x1)[0] = 0;
            u_giant_int x1(ptr_x1);


            // y0
            int size_y0 = m > b.length() ? b.length() : m;
            std::vector<unsigned char> *ptr_y0 = new std::vector<unsigned char>(size_y0);
            for(int i = 0; i < size_y0; i++)
                (*ptr_y0)[i] = b.at(i);

            u_giant_int y0(ptr_y0);

            // y1
            int size_y1 = b.length() - m > 0 ? b.length() - m : 1;
            std::vector<unsigned char> *ptr_y1 = new std::vector<unsigned char>(size_y1);
            if(size_y1 < b.length() )
            {
                for(int j=0, i = size_y0; i < b.length();i++, j++)
                    (*ptr_y1)[j] = b.at(i);
            }
            else
                (*ptr_y1)[0] =  0;

            u_giant_int y1(ptr_y1);

            u_giant_int z0 = karatsuba(x1,y1);
            u_giant_int z2 = karatsuba(x0,y0);

            u_giant_int z1 = karatsuba( x0.add(x1), y0.add(y1) ).sub( z0.add(z2) );

            return z0.mult_pow_10(2*m).add(z1.mult_pow_10(m)).add(z2);            
        }

        int compare(const std::vector<unsigned char> *a, const std::vector<unsigned char> *b) const
        {
            if( a -> size() < b -> size() )
                return -1;
            if( a -> size() > b -> size() )
                return 1;

            int result = 0;
            for( int i = a -> size()-1 ; i >= 0 ; i--)
            {
                if( (*a)[i] == (*b)[i] )
                    continue;

                if( (*a)[i] > (*b)[i] )
                    result = 1;
                if( (*a)[i] < (*b)[i] )
                    result = -1;

                break;
            }

            return result;
        }
    
    public:
        // TODO: Add validation to check is all the digits are numbers
        u_giant_int(const std::string &number)
        {
            
            #ifdef _UBI_DEBUG 
            std::cout<<"Constructor called."<<std::endl;
            #endif

            m_digits = new std::vector<unsigned char>(number.length());
            for(int i = number.length()-1 ,j = 0; i >= 0 ; i--, j++)
                (*m_digits)[j] = number[i] - '0';


            strip();
        }


        u_giant_int(int number)
        {
        
            m_digits = new std::vector<unsigned char>;
            
            do
            {
                m_digits->push_back(number%10);
                number /= 10;    
            } while(number!=0);
        }
        
        // copy constuctor
        u_giant_int(const u_giant_int &gi)
        {   
            #ifdef _UBI_DEBUG
            std::cout<<"Copy constructor called."<<std::endl;
            #endif
            
            m_digits = new std::vector<unsigned char>(gi.length());
            for(int i=0; i< gi.length(); i++)
                (*m_digits)[i] = gi.at(i);

        }

        ~u_giant_int()
        {
            delete m_digits;
        }

        // Lenght of the number
        int length() const { return m_digits->size(); }

        // Return digit at index
        unsigned char at(int index) const { return (*m_digits)[index]; }

        // Print the shit
        void print() const
        {
            for(int i = m_digits->size()-1; i >= 0 ; i--)
                std::cout<<(int)(*m_digits)[i];

            std::cout<<std::endl;
        }

        // Removes zeros after Most significant digit
        void strip()
        {
            for(int i = m_digits->size()-1; (*m_digits)[i] == 0 && m_digits->size() > 1; m_digits->pop_back(), i--);
        }

        u_giant_int add(const u_giant_int &b)
        {
            // This entire method is O(size) time

            int size = (length() > b.length()? length(): b.length());
            std::vector<unsigned char> *sum = new std::vector<unsigned char>(size);

            int i, j, k;
            unsigned char carry = 0;
            for(i = 0, j = 0, k = 0; i<length() && j < b.length(); i++, j++, k++)
            {
                unsigned char s = at(i) + b.at(j) + carry;
                (*sum)[k] = s % 10;
                carry = s / 10;                
            }

            const u_giant_int &remaining = (length() > b.length()? *this: b);
            for(;k < remaining.length(); k++)
            {
                unsigned char s = carry + remaining.at(k);
                (*sum)[k] = s % 10;
                carry = s / 10;
            }
            if(carry > 0)
                sum->push_back(carry);

            return u_giant_int(sum);
        }

        // Unknown behaviour if this < b
        u_giant_int sub(const u_giant_int &b)
        {
            int size = length() > b.length() ? length() : b.length();
            std::vector<unsigned char> *result = new std::vector<unsigned char>(size);

            unsigned char carry = 0;
            int k = 0;
            int i = 0;
            for(int j = 0; i < length() && j < b.length(); i++, j++, k++)
            {
                unsigned char r = at(i); 
                if(r < b.at(j)+carry)
                {
                    r += 10;
                    r -= b.at(j)+carry;
                    carry = 1;
                }
                else
                {
                    r -= b.at(j)+carry;
                    carry = 0;
                }
                (*result)[k] = r;
            }

            for(;k < size && i < length(); k++, i++)
            {
                unsigned char r = at(i); 
                if( r < carry)
                {
                    r += 10;
                    r -= carry;
                    carry = 1;
                }
                else
                {
                    r -= carry;
                    carry = 0;
                }
                (*result)[k] = r;
            }

            if(carry > 0)
                (*result).push_back(carry);    

            return u_giant_int(result);        
        }
        
        void substract_ptr(std::vector<unsigned char> *a, const std::vector<unsigned char> *b) const
        {
            // Assuming a >= b
            // Will leave 0s after MSD


            unsigned char carry = 0;
            int i;
            for( i=0 ; i < (*b).size() ; i++)
            {
                unsigned char r = (*a)[i];
                if( r < (*b)[i] + carry )
                {
                    r = r + 10 - (*b)[i] - carry;
                    carry = 1;
                }
                else
                {
                    r -= (*b)[i] + carry;
                    carry = 0;
                }
                (*a)[i] = r;
            }

            for(; i < a->size(); i++)
            {
                if((*a)[i] < carry)
                {
                    (*a)[i] = (*a)[i] + 10 - carry;
                    carry = 1;
                }
                else
                {
                    (*a)[i] = (*a)[i] - carry;
                    carry = 0;
                }
            }

            for(int i = a->size()-1; (*a)[i] == 0 && a->size() > 1; a->pop_back(), i--);
        }

        void operator ++()
        {

            unsigned char s = at(0) + 1;
            (*m_digits)[0] = s % 10;
            unsigned char carry = s/10;

    
            for(int i = 1 ; i < length(); i++)
            {
                unsigned char s = at(i) + carry;
                (*m_digits)[i] = s % 10;
                carry = s / 10;
            }
            if(carry > 0)
                m_digits->push_back(carry);
        }
        
        // Multiply by powers of 10:
        // eg: if x = 13, p = 3, returns 13 * 10^3 = 13000
        u_giant_int mult_pow_10(int p)
        {
            std::vector<unsigned char> *result = new std::vector<unsigned char>(length()+p);
            int i = 0;
            for(;i < p; i++)
                (*result)[i] = 0;

            for(int j = 0; j < length() ; i++, j++)
                (*result)[i] = at(j);

            return u_giant_int(result);
        }
        
        u_giant_int mult(const u_giant_int &b)
        {
            return karatsuba(*this, b);
        }

        u_giant_int div(const u_giant_int &b)
        {
            // Repeated substractions

            if(b.compare(0) == 0 )
                throw;

            if(this->compare(b) < 0)
                return 0;

            if(b.compare(1) == 0)
                return *this;

            // copying
            std::vector<unsigned char> *ptr_remaining = new std::vector<unsigned char>(length());
            for(int i = 0; i < length(); i++ )
                (*ptr_remaining)[i] = this->at(i);

            u_giant_int quotient(0);
            for( ; compare(ptr_remaining,b.m_digits) >= 0; ++quotient)
            {
                substract_ptr(ptr_remaining,b.m_digits);
            }
            
            u_giant_int remainder(ptr_remaining);
            remainder.print();
            quotient.print();

            return quotient;


            



        }

        // Returns: 0 if this == b, 1 if this > b, -1 if this < b
        int compare(const u_giant_int &b) const
        {
            return compare(this->m_digits,b.m_digits);
        }


};

int main(int argc, char const *argv[])
{
    std::string num_1;
    std::string num_2;


    std::cin>>num_1>>num_2;

    u_giant_int gi_1(num_1);
    u_giant_int gi_2(num_2);

    gi_1.div(gi_2);

    
    
    // std::cout<<gi_1.compare(gi_2)<<std::endl;
     
    return 0;
}
