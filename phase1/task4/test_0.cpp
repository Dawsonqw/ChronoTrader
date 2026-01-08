#include <cstdio>
#include <vector>
#include <stdexcept>
#include <atomic>
#include <thread>

// 解决V3存在的并发问题
// 并发问题：CPU/编译器指令重排（ARM更严重）； 可见行破坏
// Disruptor 的解决方式
// Sequence 的更新是“发布点”
// 数据先写
// sequence 后发布
// Consumer 以 sequence 为“是否可读”的唯一信号

struct Sequence{
    std::atomic<long> value{-1};

    long get()const {
        return value.load(std::memory_order_acquire);
    }

    void set(long v){
        value.store(v,std::memory_order_release);
    }
};



// 引入cursor
struct Sequencer{
    long nextSequence=0; // 下一个可以分配的序列号,尚未发布
    size_t buffersize;

    Sequence cursor; // 已经最大发布
    Sequence* consumerSeq;

    Sequencer(Sequence* consumer,size_t size):
        buffersize(size),
        consumerSeq(consumer)
    {}

    // 理解：producer在consumer前面，但是不能领先一个buffersize。
    long claim(){
        long wrapPoint=nextSequence-buffersize; 
        while(wrapPoint>consumerSeq->get()){
        }
        return nextSequence++;
    }

    void publish(long sequence){
        cursor.set(sequence); //发布点
    }
};

struct Producer{
    Sequencer& sequencer;

    Producer(Sequencer& seq):
        sequencer(seq){}

    template<typename RB>
    void publish(RB& ringbuffer,int value){
        long sequence=sequencer.claim(); // 这里确保不会覆盖未消费数据

        ringbuffer.write(sequence,value); //  普通写数据

        sequencer.publish(sequence); // 发布点，保证数据写入后才能被消费

        std::printf("publish: seq:%ld  value:%d\n",sequence,value);
    }
};

struct Consumer{
    Sequence seq;

    template<typename RB>
    void consume(RB& ringbuffer,Sequence& cursor){// 只相信cursor数据
        long next=seq.get()+1; // 下一个可读的序列号

        // 等待发布
        while(cursor.get()<next){}

        int value=ringbuffer.read(next);
        seq.set(next);

        std::printf("consume: seq:%ld  value:%d\n",next,value);
    }
};

template<typename T,size_t Size>
class RingBuffer{
    static_assert((Size&(Size-1))==0,"Size must be a power of 2");
    T buffer[Size];
    static constexpr size_t mask=Size-1;

    private:
    size_t index(long sequence)const{
        return sequence&mask;
    }

    public:
    T read(long sequence)const{
        return buffer[index(sequence)];
    }

    void write(long sequence,const T& value){
        buffer[index(sequence)]=value;
    }
};


int main(){
    RingBuffer<int,4>ring;
    Consumer consumer;

    Sequencer sequencer(&consumer.seq,4);
    Producer producer(sequencer);

    std::thread t1([&](){
        for(int i=0;i<12;i++){
            producer.publish(ring,i);
        }
    });

    std::thread t2([&](){
        for(int i=0;i<12;i++){
            consumer.consume(ring,sequencer.cursor);
        }
    });

    t1.join();
    t2.join();
    return 0;
}