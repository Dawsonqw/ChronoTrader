#include <cstdio>
#include <vector>

// v2:暴露v0-1问题,seq没有协调的情况下存在覆盖，不解决
struct Sequence{
    long value=-1;

    long get()const{
        return value;
    }

    void set(long value){
        this->value=value;
    }

    long incrementAndGet(){
        return ++value;
    }
};

struct Producer{
    Sequence sequence;
    template<typename RB>
    void publish(RB& ringbuffer,int value){
        long seq=sequence.incrementAndGet();
        ringbuffer.write(seq,value);
        std::printf("Produced: seq=%ld, value=%d\n",seq,value);
    }
};

struct Consumer{
    Sequence sequence;
    template<typename RB>
    void consume(RB& ringbuffer){
        long next=sequence.get()+1;
        int value=ringbuffer.read(next);
        sequence.set(next);
        std::printf("--- Consumed: seq=%ld, value=%d\n",next,value);
    }
};

template<typename T,size_t Size>
class RingBuffer{
    static_assert((Size&(Size-1))==0,"Size must be a power of 2");
    private:
    static constexpr size_t mask=Size-1;
    T buffer[Size];

    private:
    size_t index(long sequence)const{
        return sequence&mask;
    }

    public:
    void write(long sequence,const T& data){
        buffer[index(sequence)]=data;
    }

    T read(long sequence)const{
        return buffer[index(sequence)];
    }
};

int main(){
    RingBuffer<int,4>ring;
    Producer producer;
    Consumer consumer;

    for(int i=0;i<12;i++){
        producer.publish(ring,i);

        if(i%2==0){
            consumer.consume(ring);
        }
    }

    while(consumer.sequence.get()<producer.sequence.get()){
        consumer.consume(ring);
    }

    return 0;
}

