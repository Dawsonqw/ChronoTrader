#include <cstdio>
#include <vector>
#include <stdexcept>
#include <atomic>
#include <thread>

// 本轮唯一目标
// 允许多个 Producer 并发发布事件，同时保证：
// sequence 不重复
// 不覆盖未消费数据
// Consumer 看到的数据始终是完整的
// 从这一轮开始，Disruptor 正式分裂出两个概念：
// nextSequence	已“认领”（claim），但未发布
// cursor	已发布、对 Consumer 可见
// 多生产者的核心工具：CAS
// 唯一要做的事：
// “从一个全局序号池中，安全地抢一个唯一的 sequence”

struct Sequence{
    std::atomic<long> value{-1};

    long get()const{
        return value.load(std::memory_order_acquire);
    }

    void set(long v){
        value.store(v, std::memory_order_release);
    }
};

struct Sequencer{
    std::atomic<long> nextSequence{0}; // 下一个可以分配的序列号，还没有发布
    size_t buffSize;

    std::vector<Sequence*> consumerSeqs; // 多个消费者的序列号
    Sequence cursor; // 已经发布的最大序列号

    Sequencer(size_t size):buffSize(size){}

    void addGatingSequences(Sequence* seq){
        consumerSeqs.push_back(seq);
    }

    long getMinSequence(){
        long seq=__LONG_MAX__;
        for(auto* s:consumerSeqs){
            seq=std::min(seq,s->get());
        }
        return seq;
    }

    long claim(){
        long current;
        long next;

        while(true){
            current=nextSequence.load(std::memory_order_relaxed);
            next=current+1;

            long wrapPoint=next-buffSize;
            while(wrapPoint>getMinSequence()){}

            if(nextSequence.compare_exchange_weak(current,next,std::memory_order_acquire,std::memory_order_relaxed)){
                return current;
            }
        }
    }

    void publish(long seq){
        while(cursor.get()!=(seq-1)){} // 等待，后面的p可能先于前面的写完，如果不加限制C会认为前面的p也可以消费了
        //需要保证cursor被顺序推进
        cursor.set(seq);
    }
};

struct Producer{
    Sequencer& sequencer;
    int id_;
    Producer(Sequencer& s,int id_):sequencer(s),id_(id_){}

    template<typename RB>
    void publish(RB& ringbuffer,int value){
        long seq=sequencer.claim();

        ringbuffer.write(seq,value);

        sequencer.publish(seq);
        std::printf("Producer %d: publish seq=%ld, value=%d\n",id_,seq,value);
    }
};

struct Consumer{
    Sequence seq;
    int id_;

    Consumer(int id_):id_(id_){}

    template<typename RB>
    void consume(RB& ringBuffer,Sequence& cursor){
        long next=seq.get()+1;
        while(cursor.get()<next){}
        int value=ringBuffer.read(next);
        seq.set(next);
        std::printf("Consumer %d: consume seq=%ld, value=%d\n",id_,next,value);
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
    // 这个程序也有问题，TODO
    RingBuffer<int,4> ring;
    Consumer c1(1);
    Consumer c2(2);

    Sequencer seq(4);
    seq.addGatingSequences(&c1.seq);
    seq.addGatingSequences(&c2.seq);

    Producer p1(seq,1);
    Producer p2(seq,2);

    std::thread t1([&](){
        for(int i=0;i<12;i++){
            p1.publish(ring,i);
        }
    });

    std::thread t2([&](){
        for(int i=0;i<12;i++){
            p1.publish(ring,i);
        }
    });

    std::thread t3([&](){
        for(int i=0;i<12;i++){
            c1.consume(ring,seq.cursor);
        }
    });

    std::thread t4([&](){
        for(int i=0;i<12;i++){
            c2.consume(ring,seq.cursor);
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    return 0;
}