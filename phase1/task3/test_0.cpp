#include <cstdio>
#include <vector>
#include <stdexcept>

// v3：解决覆盖问题，增加单生产者版的Sequencer协调
// producer写第N个事件时，必须取保consumer至少消费到N-bufferSize的位置

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

// 当前为简化版，后续演进
// 由Producer持有，获取下一个可用的序列号
// 持有消费者的Sequence引用，用于判断是否会覆盖未消费的数据
struct Sequencer{
    long nextSequence=0; //下一个可以分配的序列号，生产者的序号
    Sequence* consumerSeqs_; // 指向 consumerSeqs的指针
    size_t bufferSize_;

    Sequencer(Sequence* consumerSeqs,size_t bufferSize)
        :consumerSeqs_(consumerSeqs),bufferSize_(bufferSize){};

    long claim(){
        long wrapPoint=nextSequence-bufferSize_; // 判断是否会覆盖
        if(wrapPoint>consumerSeqs_->get()){
            throw std::runtime_error("buffer full");
        }
        return nextSequence++;
    }
};

struct Producer{
    Sequencer& sequencer_;

    Producer(Sequencer& sequencer):sequencer_(sequencer){}

    template<typename RB>
    void publish(RB& ringbuffer,int value){
        long seq=sequencer_.claim();
        ringbuffer.write(seq,value);
        std::printf("Produced: seq=%ld, value=%d\n",seq,value);
    }
};

struct Consumer{
    Sequence seq;

    template<typename RB>
    void consume(RB& ringbuffer){
        long next=seq.get()+1;
        int value=ringbuffer.read(next);
        seq.set(next);
        std::printf("--- Consumed: seq=%ld, value=%d\n",next,value);
    }
};

template<typename T,size_t Size>
class RingBuffer{
    static_assert((Size&(Size-1))==0,"Size must be a power of 2");
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
    RingBuffer<int,4> ring;
    Consumer consumer;

    Sequencer sequencer(&consumer.seq,4);
    Producer producer(sequencer);

    for(int i=0;i<12;i++){
        try{
            producer.publish(ring,i);
        }catch(std::runtime_error& e){
            std::printf("Caught: %s\n",e.what());
        }

        if(i%2==0){
            consumer.consume(ring);
        }
    }

    return 0;
}