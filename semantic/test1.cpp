#include<iostream>
#include<omp.h>
using namespace std;

// 测试函数模板
template<typename T>
void print(T &t){
    cout<<t<<endl;
}

int main(){
    int a = 10;
    string b = "123212";
    // print(a);
    // print(b);
    // cout<<(2+3)*(3-(5*10+100)+100*10+312323213)<<endl;
    // cout<<(10 + (10*(321-213+321)*321)*(32+321))<<endl;
    cout<<(432+489-4*2+(3+12-2))<<endl;
    cout<<321+213*(31-12)+3<<endl;
    cout<<78+(423*423)-423<<endl;
    return 0;
}
    