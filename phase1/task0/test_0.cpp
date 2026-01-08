#include <cstdio>
#include <vector>
// V0
// RingBuffer不需要保证本身的线程安全性
template<typename T,size_t Size>
class RingBuffer{
    static_assert((Size&(Size-1))==0,"Size must be a power of 2");

    public:
        // sequence 单调递增不会回头
        void write(long sequence,const T& value){
            buffer[index(sequence)] = value;
        }

        T read(long sequence)const{
            return buffer[index(sequence)];
        }

    private:
        static constexpr size_t mask = Size-1;

        size_t index(long sequence)const{
            // mask是2的幂，所以&操作相当于取模
            // 证明：假设Size=2^n，则mask=2^n-1，其二进制表示为n个1
            // 对于任意整数x，x & (2^n - 1) 等价于x % 2^n，因为高于第n位的所有位都会被掩码的0位清零
            // 假设Size=4，则mask=3,二进制为11
            // 例如：sequence=6(110)，6&3=2(10)，只会保留低两位，高位被清零，相当于6%4=2
            return sequence&mask;
        }

        T buffer[Size];
};
int main(){
    return 0;
}