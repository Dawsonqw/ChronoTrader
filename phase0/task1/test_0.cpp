void fun_0(){
}

inline void fun_1(){
}

void test(){
    fun_0();
    fun_1();
}

inline int value=0;
int main(){
    // O0:上面所有函数都没有内联
    // O2:上面所有函数都有内联
    // 所以inline实际上已经失去了建议编译器优化的作用，现在更多的作用是ODR规则，即多个源文件中定义的函数不认为是重复定义，
    // 比如一个头文件定义了一个函数，如果多个源文件饮用，不inline的情况下会被认为重复定义，而inline则不会，同理对于变量也可以有同样作用
    test();
    return 0;
}