#include <cstdio>
#include <vector>
// V1
// 扮演的是逻辑时间
// 本例子Seq的作用
struct Sequence{
    long value=-1;

    long get()const{
        return value;
    }

    void set(long v){
        value = v;
    }

    long incrementAndGet(){
        value++;
        return value;
    }
};


template<typename T,size_t Size>
class RingBuffer{
    static_assert((Size&(Size-1))==0, "Size must be power of 2");

    public:
        void write(long sequence,const T& data){
            buffer[index(sequence)] = data;
        }

        T read(long sequence){
            return buffer[index(sequence)];
        }
    private:
        long index(long sequence)const{
            return sequence&mask;
        }
    private:
        static constexpr size_t mask=Size-1;
        T buffer[Size];
        Sequence sequence;
};

int main(){
    RingBuffer<int,8> ring;
    Sequence produceSeq;
    for(int i=0;i<12;i++){
        long seq=produceSeq.incrementAndGet();
        ring.write(seq,i);

        std::printf("seq=:%ld,index:%ld,value:%d\n",seq,seq&7,ring.read(seq));
    }
    return 0;
}