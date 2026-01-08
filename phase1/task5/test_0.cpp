#include <cstdio>
#include <vector>
#include <stdexcept>
#include <atomic>
#include <thread>

//  V4没有解决多消费下生产者如何等待的问题，只解决了单消费者的并发问题
// 对于多消费者，引入gating sequences
// 当有多个 Consumer 时，Producer 应该等谁？
// 答案是 Disruptor 的核心机制之一：
// Producer 必须等待“最慢的那个 Consumer”
// 有 多个 Consumer
// 每个 Consumer 处理速度不同,Producer 必须对“最慢者”负责
// Gating Sequence = Producer 写入前必须参考的一组 Consumer Sequence
// 数学规则升级为：
// (nextSequence - bufferSize) <= min(all consumer sequences)

struct Sequence {
    std::atomic<long> value{-1};

    long get()const{
        return value.load(std::memory_order_acquire);
    }

    void set(long v){
        value.store(v,std::memory_order_release);
    }
};

struct Sequencer{
    long nextSequence=0; // 下一个可以分配的序列号，还没有发布
    size_t buffSize;

    Sequence cursor; // 已经发布的最大序列号
    std::vector<Sequence*> consumerSeqs; // 多个消费者的序列号

    Sequencer(size_t buffSize):buffSize(buffSize){}

    void addGatingSequences(Sequence* seq){
        consumerSeqs.push_back(seq);
    }

    long getMinimumGatingSequence(){
        long minseq=__LONG_MAX__;
        for(auto* seq:consumerSeqs){
            minseq=std::min(minseq,seq->get());
        }
        return minseq;
    }

    long claim(){
        long wrapPoint=nextSequence-buffSize;
        while(wrapPoint>getMinimumGatingSequence()){
        }
        return nextSequence++;
    }

    void publish(long seq){
        cursor.set(seq);
    }
};

struct Consumer{
    Sequence seq;
    int id_;
    Consumer(int id):id_(id){}

    template<typename RB>
    void consume(RB& ringBuffer,Sequence& cursor){ // 这里有问题：多个消费者读取到的是同一个数据
        long next=seq.get()+1;
        while(cursor.get()<next){}
        int value=ringBuffer.read(next);
        seq.set(next);
        std::printf("Consumer %d: consume seq=%ld, value=%d\n",id_,next,value);
    }
};

struct Producer{
    Sequencer& sequencer;

    Producer(Sequencer& seq):sequencer(seq){}

    template<typename RB>
    void publish(RB& ringBuffer,int value){
        long seq=sequencer.claim();

        ringBuffer.write(seq,value);

        sequencer.publish(seq);
        std::printf("Producer: publish seq=%ld, value=%d\n",seq,value);
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
    void write(long seq,const T& value){
        buffer[index(seq)]=value;
    }

    T read(long seq)const{
        return buffer[index(seq)];
    }
};


int main(){
    RingBuffer<int,4>ringBuffer;

    Consumer consumer1(1);
    Consumer consumer2(2);

    Sequencer sequencer(4);
    sequencer.addGatingSequences(&consumer1.seq);
    sequencer.addGatingSequences(&consumer2.seq);

    Producer producer(sequencer);

    std::thread t1([&](){
        for(int i=0;i<12;i++){
            producer.publish(ringBuffer,i);
        }
    });
    std::thread t2([&](){
        for(int i=0;i<12;i++){
            consumer1.consume(ringBuffer,sequencer.cursor);
        }
    });

    std::thread t3([&](){
        for(int i=0;i<12;i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            consumer2.consume(ringBuffer,sequencer.cursor);
        }
    });

    // 结果不对，两个消费者消费了相同的数据 TODO
    t1.join();
    t2.join();
    t3.join();
    return 0;
}