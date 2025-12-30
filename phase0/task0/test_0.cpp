#include <cstdio>
#include <vector>

struct Info{
    int constrcutor=0;
    int destructor=0;
    int move_constructor=0;
    int copy_constructor=0;
    void clear(){
        constrcutor=0;
        destructor=0;
        move_constructor=0;
        copy_constructor=0;
    }
};

static Info info;

struct ClassInfo{
    int a;
    float b;

    ClassInfo(){
        // printf("Constructor,addr:%p\n",(void*)this);
        info.constrcutor++;
    }

    ~ClassInfo(){
        // printf("Destructor:addr:%p\n",(void*)this);
        info.destructor++;
    }

    ClassInfo(ClassInfo&& other){
        // 这也是构造函数，因此不会再调用上面的构造函数，this才是当前对象的地址。
        // printf("Move Constructor,this:%p,other:%p\n",(void*)this,(void*)&other);
        a=other.a;
        b=other.b;
        info.move_constructor++;
    }

    ClassInfo(const ClassInfo& other){
        // printf("Copy Constructor,this:%p,other:%p\n",(void*)this,(void*)&other);
        a=other.a;
        b=other.b;
        info.copy_constructor++;
    }
};
int main(){
    printf("----------------\n");
    std::vector<ClassInfo> vec;
    printf("-------push_back---------\n");
    // 临时对象构造，有移动构造调用，临时对象destructor
    info.clear();
    vec.push_back(ClassInfo{});// O0:constructor -> m?move:copy -> destructor 
    printf("constructor:%d,move_constructor:%d,copy_constructor:%d,destructor:%d\n",
            info.constrcutor,info.move_constructor,info.copy_constructor,info.destructor);

    printf("-------reserve--------\n");
    info.clear();
    vec.reserve(10); //O0: copy -> destructor
    printf("constructor:%d,move_constructor:%d,copy_constructor:%d,destructor:%d\n",
            info.constrcutor,info.move_constructor,info.copy_constructor,info.destructor);

    printf("-------push_back---------\n");
    info.clear();
    vec.push_back(ClassInfo{}); // O0:constructor -> move -> destructor
    printf("constructor:%d,move_constructor:%d,copy_constructor:%d,destructor:%d\n",
            info.constrcutor,info.move_constructor,info.copy_constructor,info.destructor);

    printf("-------resize--------\n");
    info.clear();
    vec.resize(5); // O0:constructor,constructor
    printf("constructor:%d,move_constructor:%d,copy_constructor:%d,destructor:%d\n",
            info.constrcutor,info.move_constructor,info.copy_constructor,info.destructor);


    printf("----------------\n");
    // emplace_back的设计就是原地构造，在vector的堆上,比较和push_back差异，少了move和destructor
    info.clear();
    vec.emplace_back(); //O0: constructor 
    printf("constructor:%d,move_constructor:%d,copy_constructor:%d,destructor:%d\n",
            info.constrcutor,info.move_constructor,info.copy_constructor,info.destructor);

    printf("----------------\n");
    // 这里会退化为push_back，正确用法：如果无参数：emplace_back()，如果有参数：emplace_back(args...)
    info.clear();
    vec.emplace_back(ClassInfo{}); // 
    printf("constructor:%d,move_constructor:%d,copy_constructor:%d,destructor:%d\n",
            info.constrcutor,info.move_constructor,info.copy_constructor,info.destructor);
    printf("----------------\n");
    return 0;
}


// time bin_file
// O0:
// real    0m0.003s
// user    0m0.002s
// sys     0m0.000s

// O2:
// real    0m0.002s
// user    0m0.001s
// sys     0m0.001s

// O0
//  Performance counter stats for './task0/test_0':

//               0.60 msec task-clock                       #    0.662 CPUs utilized             
//                  0      context-switches                 #    0.000 /sec                      
//                  0      cpu-migrations                   #    0.000 /sec                      
//                118      page-faults                      #  197.162 K/sec                     
//      <not counted>      cpu_atom/cycles/                                                        (0.00%)
//          2,853,702      cpu_core/cycles/                 #    4.768 GHz                       
//      <not counted>      cpu_atom/instructions/                                                  (0.00%)
//          3,949,748      cpu_core/instructions/           #    1.38  insn per cycle            
//      <not counted>      cpu_atom/branches/                                                      (0.00%)
//            661,378      cpu_core/branches/               #    1.105 G/sec                     
//      <not counted>      cpu_atom/branch-misses/                                                 (0.00%)
//             16,010      cpu_core/branch-misses/          #    2.42% of all branches           
//              TopdownL1 (cpu_core)                 #     32.8 %  tma_backend_bound      
//                                                   #     13.5 %  tma_bad_speculation    
//                                                   #     30.3 %  tma_frontend_bound     
//                                                   #     23.4 %  tma_retiring           

//        0.000904296 seconds time elapsed

//        0.000000000 seconds user
//        0.000993000 seconds sys

// O2:
// Performance counter stats for './task0/test_0':

//               0.95 msec task-clock                       #    0.723 CPUs utilized             
//                  0      context-switches                 #    0.000 /sec                      
//                  0      cpu-migrations                   #    0.000 /sec                      
//                117      page-faults                      #  122.976 K/sec                     
//          3,463,970      cpu_atom/cycles/                 #    3.641 GHz                         (64.84%)
//      <not counted>      cpu_core/cycles/                                                        (0.00%)
//          3,943,858      cpu_atom/instructions/           #    1.14  insn per cycle            
//      <not counted>      cpu_core/instructions/                                                  (0.00%)
//            660,399      cpu_atom/branches/               #  694.128 M/sec                     
//      <not counted>      cpu_core/branches/                                                      (0.00%)
//             18,102      cpu_atom/branch-misses/          #    2.74% of all branches           
//      <not counted>      cpu_core/branch-misses/                                                 (0.00%)
//              TopdownL1 (cpu_atom)                 #     20.5 %  tma_bad_speculation    
//                                                   #     25.9 %  tma_retiring           
//                                                   #     34.0 %  tma_backend_bound      
//                                                   #     34.0 %  tma_backend_bound_aux  
//                                                   #     19.6 %  tma_frontend_bound     

//        0.001316302 seconds time elapsed

//        0.001364000 seconds user
//        0.000000000 seconds sys