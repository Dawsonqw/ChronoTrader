#include <cstdio>
#include <vector>

struct ClassInfo{
    int a;
    float b;

    ClassInfo(){
        printf("Constructor,addr:%p\n",(void*)this);
    }

    ~ClassInfo(){
        printf("Destructor:addr:%p\n",(void*)this);
    }

    ClassInfo(ClassInfo&& other){
        // 这也是构造函数，因此不会再调用上面的构造函数，this才是当前对象的地址。
        printf("Move Constructor,this:%p,other:%p\n",(void*)this,(void*)&other);
        a=other.a;
        b=other.b;
    }

    ClassInfo(const ClassInfo& other){
        printf("Copy Constructor,this:%p,other:%p\n",(void*)this,(void*)&other);
        a=other.a;
        b=other.b;
    }
};
int main(){
    printf("----------------\n");
    std::vector<ClassInfo> vec;
    printf("-------push_back---------\n");
    // 临时对象构造，有移动构造调用，临时对象destructor
    vec.push_back(ClassInfo{});// O0:constructor -> m?move:copy -> destructor 
    printf("-------reserve--------\n");
    vec.reserve(10); //O0: copy -> destructor
    printf("-------push_back---------\n");
    vec.push_back(ClassInfo{}); // O0:constructor -> move -> destructor
    printf("-------resize--------\n");
    vec.resize(5); // O0:constructor,constructor
    printf("----------------\n");
    vec.emplace_back();
    printf("----------------\n");
    return 0;
}