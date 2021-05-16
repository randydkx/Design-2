#include<iostream>
#include<cstdio>
#include<string.h>
#include<algorithm>
#include<vector>
#include<map>
#include<set>
#include<queue>
using namespace std;

// 类型，非终结符，终结符和epsilon,前向搜索符号#
enum TYPE{terminal=0,nonterminal=1,epsilon=2,front_search=3};

// 关键字列表
vector<string> keywordsList = vector<string>({"auto", "break", "case", "char", "const", "continue",
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "int", "long","string",
    "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while"});

// 扩展String，加上类型
struct symbol{
    string str;
    int type;
    // 如果是终结符，则tokenType表示该终结符的token表中的类型，一共有五种类型
    string tokenType;
    // 针对非终结符而言，能否产生epsilon,-1：未定，0：不能，1：能
    int can_generate_epsilon = -1; 
    // first集合
    set<string> first;
    symbol(string str,int type):str(str),type(type){}
    symbol(string str,int type,string tokenType):str(str),type(type),tokenType(tokenType){}
    symbol(){}
    // 设定symbol的比较规则，为了将其插入set中进行判重
    bool operator < (const symbol other) const{
        return this->str < other.str;
    }
};

// 语义分析动作的类型
enum SEMANTIC_ACTION_TYPE{
    constant_assign=0,//常量初始化类型的赋值 比如 t1 := 10
    parameter_assign=1,//变量之间的赋值 比如 t2 := t1
    computing_assign_l_r=2,//计算形式的赋值，二元运算 比如 t3 := t1 * t2 ,并且左操作数在左
    computing_assign_r_l=3,//计算形式的赋值，二元运算 比如 t3 := t2 * t1,并且右操作数在左
    print=4//输出语义值，也就是表达式的计算结果
};

// 产生式数据结构
class production{
public:
    // 产生式的索引
    int index;
    // 产生式左边的非终结符
    symbol left;
    // 产生式右边的
    vector<symbol> right = vector<symbol>();
    // 产生式的语义动作
    string semanticAction;
    // 产生式语义动作的类型
    int actionType = -1;

public:
    // 打印产生式：
    void print();
};


class allProduction{
public:
    // 产生式集合
    vector<production> productions;
    // 终结符集合
    vector<symbol> terminal;
    // 非终结符集合
    vector<symbol> nonterminal;
    // 非终结符到int的映射表
    map<string,int> indexTable;
public:
    void initial_production_set();
    // 检查是否每一个非终结符都确定是否能推出epsilon，辅助函数
    bool check_is_end();
    // 标记产生式是否能推出epsilon
    void markEpsilonProduction();
    // 获取每个终结符和非终结符的first集合
    void getFirstSet();
    // 打印first集合
    void print_symbol_first();
    // 给定任意符号串，计算其first值
    set<string> First(vector<symbol> symList);
    // 打印所有产生式
    void printAllProduction();
};

// 项目,继承自产生式，加入了项目点和前向搜索符号集合
class Item : public production{
public:
    // 项目集点的位置，[pointer,end)之间为项目点右边的符号串
    // 同时，pointer为点左边的符号数量
    int pointer = 0;
    // 前向搜索符集合
    set<string> frontSet = set<string>(); 
    // 可归项目
    bool canReduction = false;
    // 移进项目
    bool canShiftIn = false;
public:
    // 从产生式构造一个item
    Item(production pro);
    // 项目点 向右移动一格
    static Item moveOneStep(Item item);
    // 检查是否是可规约项目
    bool checkCanReduction();
    // 检查是否是可移进项目
    bool checkCanShiftIn();
    // 检查是否是一条能推出epsilon的item
    bool isEpsilonItem();
    // 打印项目
    void printItem();
     // 判断两个项目集是否相等，in=true:算进first集合进行比较
    //  in=false:不算first集合进行比较
    static bool isEqual(Item item1,Item item2,bool in);
};

// 项目集:包含多个项目的项目集合，也是一个DFA状态
class ItemSet{
public:
    // 状态编号
    int state = -1;
    // 项目集合
    vector<Item> itemList = vector<Item>();
public:
    // 打印项目集
    void printItemSet();
    // 检查一条Item是否可以插入
    bool checkCanInsert(Item item); 
    // 重新定义==，用于测试DFA状态是否已经存在
    bool operator == (const ItemSet &other)const;
};

enum TYPE_OF_ACTION_GOTO{
    PRODUCTION=0,//表项中是产生式
    STATE=1,//表项中是状态编号
    ACCEPT=2,//表项中是接受状态
    ILLEGAL=3//表项为非法
};

// ACTION GOTO表中的条目，扩种设置其中的内容类型
class Element{
public:
    // 条目的类型，来自于TYPE_OF_ACTION_GOTO
    int type = -1;
    // 索引
    // 如果是产生式，则为产生式编号
    // 如果是移进动作，则为下一个状态的索引
    // 如果是接受状态或者是非法状态，则index任意
    int index = -1;
public:
    Element(){};
    Element(int type,int index):type(type),index(index){}
    string transElement();
};

// token词例表数据结构
class Token{
public:
    int line;//所在行号
    int type;//类型
    string value;//取值
public:
    Token(int line,int type,string value):line(line),type(type),value(value){}
    Token(){}
    void printToken();
};
// token的类型
enum TokenType{constant,identifier,keyword,delimiter,operand};

// 项目集族：用于识别活前缀的确定性有限自动机
class Processor{
public:
    // 所有产生式以及产生式的处理函数集合
    allProduction pros;
    // 项目集构成的DFA
    vector<ItemSet> DFA;
    // ACTION表,二维表
    map<int, map<string,Element> > Action = map<int, map<string,Element> >();
    // GOTO表
    map<int, map<string,Element> > Goto;
    // 转移状态表
    // 对于每一个int输入以及转移弧都有一个状态对应，若无，则为-1
    map<int, map<string,int> > transferTable;
    // 错误位
    int errorPos = -1;
public:
    // 项目集构造算法
    void generateDFA(allProduction prodctions);
    // 构造action和goto表
    void generateActionAndGoto();
    // 查找句子识别态
    vector<int> findAccpetState();
    // 得到一个状态集
    ItemSet stateClosure(ItemSet kernel);
    // 得到指定非终结符对应的所有项目
    vector<Item> getItemsByNonterminal(symbol nonter);
    // 将项目集中对应同一个产生式的first集合合并成一个
    static ItemSet merge(ItemSet item);
    // 查看一条item是否可以转移
    static bool checkCanTrans(Item item);
    // 检查一个状态是否存在
    int isExist(ItemSet itemSet);
    // 打印转移状态表
    void printTransTable();
    // 打印ACTION+GOTO表
    void printActionGoto();
    // 检查符号串是否符合该文法的规则
    bool isIllegalList(vector<symbol> symList,vector<Token> tokenList,vector<string> &list);
    // 错误信息构造函数
    string errorMsg(Token token);
};

