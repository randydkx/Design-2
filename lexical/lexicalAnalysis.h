#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <map> 
#include <stack> 
#include <algorithm> 
#include<set>
#include<queue>
using namespace std;

// 导入和输出时状态的左右括号
const char OPENING_BRACKET = '{';
const char CLOSING_BRACKET = '}';
#define EPSILON  '@'
#define SCIENTIFIC_FORMAT "[scientific-format]"
#define DELIMITER "[delimiter]"
#define IDENTIFIER "[identifier]"
#define OPERATOR "[operator]"

// 关键字列表
// vector<string> keywordsList = vector<string>({"auto", "break", "case", "char", "const", "continue",
//     "default", "do", "double", "else", "enum", "extern",
//     "float", "for", "goto", "if", "int", "long","string",
//     "register", "return", "short", "signed", "sizeof", "static",
//     "struct", "switch", "typedef", "union", "unsigned", "void",
//     "volatile", "while"});
vector<string> keywordsList = vector<string>({"Void","Double","Int","Int32","Int8","Int64","String","for","while","Static","continue",
    "break","switch","case","Class","func","if","let","var","return"});

struct DFAState {
    // 当前状态是否在集合中是否存在
    // true:当前状态已经考虑过弧转换
    // false:当前结果未转换，有待考虑
    bool marked;
    vector<int> states;
    //   弧转换的结果，map分别存放的是(弧，DFA中转接后的节点编号)
    map<char,int> moves;
};
// DFA状态：每一个状态可以经过转换到一个另一个状态节点
typedef map<int, DFAState> DFA;
// NFA表的结构：一个状态+弧=转移节点集合
typedef map<int, map<char, vector<int>>> NFA;

//token表中元素的类型
enum TokenType{constant,identifier,keyword,delimiter,operand};
map<int,string> type2String;


// 令牌表数据结构
struct token{
    int line;//所在行号
    int type;//类型
    string value;//取值
    token(int line,int type,string value):line(line),type(type),value(value){}
    token(){}
};

vector<token> tokenList;

// 自动机处理器
class Processor{
    public:
        // 词法分析的产生式规则
        vector<string> rules;
        // NFA的初始状态
        int nfa_init_state;
        // NFA的所有的状态数量
        int nfa_state_count;
        // DFA状态的数量
        int dfa_state_count;
        // NFA终止状态列表
        vector<int> nfa_final_states;
        // DFA终止状态集合
        set<int> dfa_final_states;
        // 符号表`
        vector<char> alphabet;
        // NFA状态表
        NFA nfa;
        // DFA状态表
        DFA dfa;
        // 下一个需要进行epsilon(move(T))的状态标号
        int to_mark;
        // 非终结符的索引表
        map<char,int> indexTable;

    public:
    // 判断vecotr中包含某一个元素
        bool vectorContain(vector<int> vec, int key);
        // 判断是否已经是已有的DFA状态
        int is_a_state_of_DFA(vector<int> state);
        // 计算epsilon闭包
        vector<int> eclosure(vector<int> T);
        // 计算move集合
        vector<int> move(vector<int> T, char move);
        // 构建一个新的DFA状态
        DFAState newDFAState(bool mark, vector<int> s);
        // 查找DFA的结果集合
        vector<int> findFinalDFAStates();
        // 子集法将NFA转化成DFA
        void subsetMethod();
        // 打印DFA状态转移表
        void printDFA();
        // 打印NFA表
        void printNFA();
};

