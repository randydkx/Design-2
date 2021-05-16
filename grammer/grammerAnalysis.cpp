#include<iostream>
#include<cstdio>
#include<string.h>
#include<vector>
#include<fstream>
#include<map>
#include<ctime>
#include"grammerAnalysis.h"
using namespace std;

#define EPSILON  '@'//字符形式的epsilon
#define EPSILON_STRING "@"//字符串形式的epsilon
#define SCIENTIFIC_FORMAT "[scientific-format]"
#define DELIMITER "[delimiter]"
#define IDENTIFIER "[identifier]"
#define KEYWORDS  "[keywords]"
#define OPERATOR "[operator]"
const char OPENING_BRACKET = '{';
const char CLOSING_BRACKET = '}';
#define FRONT_SERARCH "#"//初始前向搜索符
// 定义非终结符和终结符输入时左右包围的符号
#define left_of_terminal '\''
#define right_of_terminal '\''
#define left_of_nonterminal '\"'
#define right_of_nontermial '\"'

#ifndef DEBUG
    #define DEBUG
#endif

// 定义文法规则的文件名称
const string grammer_filename = "grammer-rule2.txt";
// 词例表
string TOKEN_FILE = "../lexical/token.txt";
ifstream tokenStream(TOKEN_FILE);
// 从TYPETYPE转化成string
map<int,string> tokenType2String;
map<string,int> string2TokenType;

void production::print(){
    cout<<this->index<<" : "<<left.str<<" -> ";
    for(int i=0;i<right.size();i++){
        cout<<right[i].str<<" ";    
    }
    cout<<endl;
}
// 产生式集合数据结构
allProduction pros;
Processor processor;

// 从给定字符串得到一个产生式
production getProduction(string source){
    production pro;
    // 产生式的编号
    pro.index = pros.productions.size();
    // 读取产生式左边的非终结符
    int posl = source.find(left_of_nonterminal,0);
    int posr,length = source.length();
    if(posl != string::npos){
        posr = source.find(right_of_nontermial,posl+1);
        pro.left = symbol(source.substr(posl+1,posr-posl-1),TYPE::nonterminal);
    }
    // 读取产生式右边的所有符号，分解装入vector中
    string right_side = source.substr(source.find(">")+1);
    length = right_side.length();
    posl = posr = 0;
    while(posr < length){
        if(right_side[posl]==left_of_nonterminal){
            posr =  right_side.find(right_of_nontermial,posl+1);
            symbol sym = symbol(right_side.substr(posl+1,posr-posl-1),TYPE::nonterminal);
            pro.right.push_back(sym); 
        }else if (right_side[posl]==left_of_terminal){
            posr =  right_side.find(right_of_terminal,posl+1);
            string terminal_identified = right_side.substr(posl+1,posr-posl-1);
            symbol sym;
            if(terminal_identified==EPSILON_STRING){//右端遇到epsilon，不能将其当做非终结符处理
                sym = symbol(terminal_identified,TYPE::epsilon);
            }else{
                sym = symbol(terminal_identified,TYPE::terminal);
            }
            // 设置每个终结符的tokenType
            // 如果该非终结符是关键字，则标记为关键字
            if(find(keywordsList.begin(),keywordsList.end(),terminal_identified) != keywordsList.end()){
                sym.tokenType = KEYWORDS;
            }else if(terminal_identified==OPERATOR){
                sym.tokenType = OPERATOR;
            }else if(terminal_identified==SCIENTIFIC_FORMAT){
                sym.tokenType = SCIENTIFIC_FORMAT;
            }else if(terminal_identified[0] != '['){
                sym.tokenType = DELIMITER;
            }else sym.tokenType = IDENTIFIER;
            pro.right.push_back(sym); 
        }
        posl = posr = posr + 1;
    }
    return pro;
}

// 打印所有产生式
void allProduction::printAllProduction(){
    for(int i=0;i<pros.productions.size();i++){
        pros.productions[i].print();
    }
}

// 检测vector中是否包含元素value
bool vectorContain(vector<symbol> v,string value){
    for(auto x:v){if(x.str==value)return true;}
    return false;
}

// 获取所有的文法的产生式
void get_grammer(){
    string filename = grammer_filename;
    string line;
    ifstream is(filename);
    if(is.is_open()){
        while(is.peek() != '$'){
            getline(is,line);
            pros.productions.push_back(getProduction(line));
        }
    }

    pros.printAllProduction();

    is.close();
}

// 打印epsilon推出表
void printTable(vector<symbol> &v){
    for(int i=0;i<pros.nonterminal.size();i++){
        cout<<pros.nonterminal[i].str<<"   ";
    }
    cout<<endl;
    for(int i=0;i<v.size();i++){
        switch (v[i].can_generate_epsilon)
        {
        case -1:
            cout<<"未定"<<"   ";
            break;
        case 0:
            cout<<"否"<<"   ";
            break;
        case 1:
            cout<<"是"<<"   ";
        break;
        
        default:
            break;
        }
    }
    cout<<endl;
}

// 检查是否所有的非终结符都能够确定是否能推出epsilon
bool allProduction::check_is_end(){
    for(symbol sym: this->nonterminal){
        if(sym.can_generate_epsilon == -1)return false;
    }
    return true;
}

//初始化production数据结构中的其他属性，包括终结符list和非终结符list以及非终结符的映射表
void allProduction::initial_production_set(){
    set<symbol> terminal,nonterminal;
    for(auto production: pros.productions){
        nonterminal.insert(symbol(production.left.str,TYPE::nonterminal));
        // 扫描右边的符号，分别放入非终结符和终结符集合中
        for(auto rightSymbol: production.right){
            if(rightSymbol.type == TYPE::nonterminal){
                nonterminal.insert(symbol(rightSymbol.str,TYPE::nonterminal,rightSymbol.tokenType));
            }
            else if(rightSymbol.type == TYPE::terminal){
                terminal.insert(symbol(rightSymbol.str,TYPE::terminal,rightSymbol.tokenType));
            }
        }
    }
    // 获取产生式集合的终结符列表和非终结符列表
    pros.terminal = vector<symbol>(terminal.begin(),terminal.end());
    pros.nonterminal = vector<symbol>(nonterminal.begin(),nonterminal.end());
    // cout<<"终结符:"<<endl;
    // for(symbol sym:pros.terminal) cout<<sym.str<<" "<<sym.tokenType<<" "<<sym.type<<endl;
    // cout<<"非终结符:"<<endl;
    // for(symbol sym:pros.nonterminal) cout<<sym.str<<" "<<sym.tokenType<<" "<<sym.type<<endl;
    // 设置终结符都无法推出epsilon
    for(symbol sym:pros.terminal)sym.can_generate_epsilon = 0;
    // 设置非终结符的索引映射表
    pros.indexTable = map<string,int>();
    for(int i=0;i<pros.nonterminal.size();i++){
        symbol& sym = pros.nonterminal[i];
        pros.indexTable.insert(make_pair(sym.str,i));
    }
}

// 标记可以推出epsilon的产生式
void allProduction::markEpsilonProduction(){

    #ifdef DEBUG
    cout<<endl<<"非终结符是否能推出epsilon判断："<<endl;
    #endif

    vector<bool> isdeleted = vector<bool>();
    vector<production> &proList = pros.productions;
    vector<symbol> &nonterminal = pros.nonterminal;
    // 非终结符的索引表
    map<symbol,int> index;
    int length = proList.size();
    // 标记没有任何产生式被删除
    // 最初能否推出epsilon都是未定
    for(int i=0;i<proList.size();i++){
        isdeleted.push_back(false);
    }
    // (1)检查所有产生式，删除所有右边含有终结符的产生式
    for(int i=0;i<length;i++){
        bool has_terminal=false;
        production &pro = proList[i];
        for(int j=0;j<pro.right.size();j++){
            if(pro.right[j].type == TYPE::terminal){
                has_terminal = true;
                break;
            }
        }
        if(has_terminal)isdeleted[i]=true;
    }
    // 记录此时还存在的非终结符
    set<symbol> exist;
    for(int i=0;i<length;i++){
        production &pro = proList[i];
        if(!isdeleted[i])exist.insert(pro.left);
    }
    // 将不能推出epsilon的非终结符标记；
    for(auto& sym: nonterminal){
        if(!exist.count(sym))sym.can_generate_epsilon = 0;
    }
    #ifdef DEBUG
    printTable(nonterminal);
    #endif
    // for(int i=0;i<isdeleted.size();i++)cout<<isdeleted[i]<<" ";
    // cout<<endl;
    
    map<string,int> &indexMap = pros.indexTable;
    // (2)检查所有产生式，将能够推出epsilon的非终结符设置 1
    for(int i=0;i<length;i++){
        if(isdeleted[i])continue;
        production &pro = proList[i];
        // 右端推出唯一的字符epsilon
        if(pro.right.size()==1 && pro.right[0].str==EPSILON_STRING){
            for(auto& sym: nonterminal){
                if(sym.str == pro.left.str)sym.can_generate_epsilon = 1;
            }
            // 将以该非终结符为起始的产生式标记为已经删除
            for(int j=0;j<length;j++){
                if(!isdeleted[j] && proList[j].left.str==pro.left.str)isdeleted[j]=true;
            }
        }
    }
    #ifdef DEBUG
    printTable(nonterminal);
    #endif

    //(3)开始第三阶段扫描,没有全部确定则继续循环
    while(!(this->check_is_end())){
        // vector<production> newProList = proList;
        // 重新扫描产生式，将右端全是非终结符且每个非终结符都能推出epsilon的产生式删除
        for(int i=0;i<length;i++){
            if(isdeleted[i])continue;
            // 第i条产生式
            production &pro = proList[i];
            bool exit = false;
            // 需要考虑该非终结符对应的产生式
            string current_nonterminal = pro.left.str;
            // 此时存在的产生式右边都是非终结符
            for(int j=0;j<pro.right.size();j++){
                // 右端存在一个不能推出epsilon的产生式，将其删除
                // cout<<pro.right[j].str<<" "<<pro.right[j].can_generate_epsilon<<endl;
                string non = pro.right[j].str;
                // 只有nonterminal集合能够判断是否能推出epsilon，需要根据索引做转换
                if(nonterminal[indexMap[non]].can_generate_epsilon == 0){
                    // cout<<"delete "<<i<<" "<<non<<endl;
                    isdeleted[i]=true;
                    // 如果这使得该非终结符对应的产生式被删完，则标记无法推出epsilon
                    bool  still = false;//标志当前考虑的非终结符相关的产生式是否还存在
                    for(int k=0;k<length;k++){
                        if(isdeleted[k])continue;
                        if(proList[k].left.str == current_nonterminal)still=true;
                    }
                    if(!still){
                        nonterminal[indexMap[current_nonterminal]].can_generate_epsilon=0;
                        // cout<<"当前已经删除非终结符"<<current_nonterminal<<"所有的产生式"<<endl;
                    }
                    exit=true;
                    break;
                }
            }
            if(exit)break;

            //①查看是否有未定的产生式，如果存在则标记all_can_generate = -1 
            int all_can_generate = -2;//不确定是否所有的都能推出能推出epsilon
            for(int j=0;j<pro.right.size();j++){
                string non = pro.right[j].str;
                int can = nonterminal[indexMap[non]].can_generate_epsilon;
                if(can == -1){
                    //只要有一个非终结符无法确定，则设定本轮循环没法确定左边非终结符是否能推出epsilon
                    // 无法确定是否能推出的情况下不修改左边非终结符的can_generate_epsilon
                    all_can_generate = -1;
                    // cout<<"存在没有确定是否能推出epsilon的产生式"<<endl;
                    break;
                }
            }
            // ②经过上一轮扫描，不存在未定符号，查看是否有符号不能推出epsilon
            // 一旦有符号不能推出epsilon，则标记不能左边非终结符推出epsilon
            if(all_can_generate == -2){
                for(int j=0;j<pro.right.size();j++){
                    if(nonterminal[indexMap[pro.right[j].str]].can_generate_epsilon==0){
                        all_can_generate=0;
                        isdeleted[i]=true;
                        // 如果这使得该非终结符对应的产生式被删完，则标记无法推出epsilon
                        bool  still = false;//标志当前考虑的非终结符相关的产生式是否还存在
                        for(int k=0;k<length;k++){
                            if(isdeleted[k])continue;
                            if(proList[k].left.str == current_nonterminal)still=true;
                        }
                        if(!still){
                            nonterminal[indexMap[current_nonterminal]].can_generate_epsilon=0;
                            // cout<<"当前已经删除非终结符"<<current_nonterminal<<"所有的产生式"<<endl;
                        }
                    }
                }
            }  
            // ③经过上面两轮扫描，右端所有符号都能推出epsilon
            // 设定左边非终结符能够推出epsilon，并且删除所有该非终结符的产生式
            if(all_can_generate == -2){
                nonterminal[indexMap[current_nonterminal]].can_generate_epsilon=1;
                // 删除该非终结符对应的所有产生式
                for(int j=0;j<length;j++){
                    if(isdeleted[j])continue;
                    if(proList[j].left.str == current_nonterminal)isdeleted[j]=true;
                }
            }
        }
        #ifdef DEBUG
        printTable(nonterminal);
        #endif
        // for(int i=0;i<isdeleted.size();i++)cout<<isdeleted[i]<<" ";
        // cout<<endl;
    }
    cout<<endl;
}

template <typename T>
void printVector(vector<T> v){
    cout << OPENING_BRACKET<<" ";
    for(int i=0;i<v.size();i++){
        if(i==v.size()-1) cout<<v[i]<<" ";
        else cout<<v[i]<<" , ";
    }
    cout << CLOSING_BRACKET;
}

void allProduction::print_symbol_first(){
    for(int i=0;i<this->nonterminal.size();i++){
        cout<<"FIRST( "<<this->nonterminal[i].str<<" ) = ";
        set<string> &first = this->nonterminal[i].first;
        printVector(vector<string>(first.begin(),first.end()));
        cout<<endl;
    }
    for(auto sym: this->terminal){
        cout<<"FIRST( "<<sym.str<<" ) = ";
        set<string> &first = sym.first;
        printVector(vector<string>(first.begin(),first.end()));
        cout<<endl;
    }
    cout<<endl;
}

// 获取全局数据结构pros的终结符和非终结符的first集合
void allProduction::getFirstSet(){
    #ifdef DEBUG
    cout<<"计算非终结符和终结符的first集合:"<<endl;
    #endif
    map<string,int> &indexMap = this->indexTable;

    // 书算法流程(1) 设置非终结符的first集合为其本身
    for(int i=0;i<this->terminal.size();i++){
        this->terminal[i].first.insert(this->terminal[i].str);
    }

    // 对于nonterminal->terminal(others)类型的产生式，将terminal插入nonterminal的first集合中
    vector<production> proList = this->productions;
    bool changed = true;
    #ifdef DEBUG
    this->print_symbol_first();//打印初始的first表
    #endif
    // 一旦first集合发生变化就重新迭代计算，直到first集合都收敛
    while(changed){
        changed = false;
        for(production pro:proList){
            // 右端第一个是非终结符或者只有一个epsilon，则将右端第一个符号加入first集合中
            // 书73算法流程(2)(3)
            if((pro.right.size()>0 && pro.right[0].type == TYPE::terminal) || 
            (pro.right.size()==1 && pro.right[0].str == EPSILON_STRING)){
                // 对非终结符的first集合中插入当前产生式右端的第一个终结符
                // 需要对该值得first集合进行更新，所以使用引用
                symbol &current_nonterminal = nonterminal[indexMap[pro.left.str]];
                // first集合发生变化，已经包含则 changed保持
                if(!current_nonterminal.first.count(pro.right[0].str)){
                    current_nonterminal.first.insert(pro.right[0].str);
                    #ifdef DEBUG
                    cout<<"(2)(3)"<<endl;
                    this->print_symbol_first();
                    #endif
                    changed = true;
                }
            }
            // 书算法流程(3)，将能直接推出epsilon的产生式对应的非终结符的first集合中加入epsilon
            else{//此时右端开头一个一定是非终结符
                assert(pro.right[0].type == TYPE::nonterminal);
                vector<symbol> &right = pro.right;
                // 产生式左侧的符号
                symbol &X = nonterminal[indexMap[pro.left.str]];
                set<string> &firstX = X.first;
                // 检索第一个不能推出epsilon的位置
                int firstPostion = -1;
                for(int i=0;i<right.size();i++){
                    symbol sym;
                    // 注意：对于can_generate_epsilon的改变都必须在nonterminal或者terminal上，
                    // 如果右边推出的字符中有一个是非终结符，则firstPosition最多停留到该位置上
                    if(right[i].type == TYPE::terminal)sym.can_generate_epsilon=0;
                    else if(right[i].type == TYPE::nonterminal)sym=nonterminal[indexMap[right[i].str]];
                    if(!sym.can_generate_epsilon){
                        firstPostion = i;
                        break;
                    }
                }
                // 右端所有符号都能推出epsilon,算法流程(5)
                // 需要将右侧所有符号的first集合都并入X的first集合中
                if(firstPostion == -1){
                    for(int i=0;i<right.size();i++){
                        symbol current_symbol = right[i];
                        vector<string> firstY;
                        if(current_symbol.type == TYPE::terminal){
                            firstY.push_back(current_symbol.str);
                        }
                        // 取出右端符号的first集合
                        else if(current_symbol.type == TYPE::nonterminal){
                            set<string> temp = nonterminal[indexMap[current_symbol.str]].first;
                            firstY = vector<string>(temp.begin(),temp.end());
                        }
                        //  FIRST(X) <- FIRST(Y1) & FIRST(Y2)... & {epsilon}
                        for(auto ele: firstY){
                            if(!firstX.count(ele)){
                                firstX.insert(ele);
                                changed = true;
                                #ifdef DEBUG
                                cout<<"(5)"<<endl;
                                this->print_symbol_first();
                                #endif
                            }
                        }
                        // cout<<"数量："<<firstX.size()<<endl;
                        // 将epsilon也并入集合中
                        if(!firstX.count(EPSILON_STRING)){
                            firstX.insert(EPSILON_STRING);
                            changed=true;
                            #ifdef DEBUG
                            cout<<"(5)epsilon进入"<<endl;
                                this->print_symbol_first();
                            #endif
                        }
                    }
                }else{//有端符号存在不能推出epsilon的
                    assert(firstPostion >=0 && firstPostion < right.size());
                    for(int i=0;i<right.size();i++){    
                        symbol current_symbol = right[i];
                        set<string> firstY;
                        if(current_symbol.type == TYPE::terminal){
                            firstY.insert(current_symbol.str);
                        }
                        // 取出右端符号的first集合
                        else if(current_symbol.type == TYPE::nonterminal){
                            firstY = nonterminal[indexMap[current_symbol.str]].first;
                            // firstY = vector<string>(temp.begin(),temp.end());
                        }
                        // cout<<nonterminal[indexMap[current_symbol.str]].str<<endl;
                        if(i<firstPostion){//将集合去除epsilon后加入到firstX中
                            assert(firstY.count(EPSILON_STRING) > 0);
                            firstY.erase(EPSILON_STRING);
                            // 扫描set，将其中firstX中不存在的加入firstX中
                            for(set<string>::iterator it=firstY.begin();it != firstY.end();it++){
                                if(!firstX.count(*it)){
                                    firstX.insert(*it);
                                    #ifdef DEBUG
                                    cout<<"(4)前面内容加入"<<endl;
                                    this->print_symbol_first();
                                    #endif
                                    changed = true;
                                }
                            }
                        }else if(i>firstPostion){//直到考虑到第一个不能推出epsilon的符号
                            break;
                        }else{//将firstYi加入firstX中
                        // 扫描set，将其中不存在的加入firstX中
                            for(set<string>::iterator it=firstY.begin();it!=firstY.end();it++){
                                if(!firstX.count(*it)){
                                    firstX.insert(*it);
                                    #ifdef DEBUG
                                    cout<<"(4)firstY中元素 "<<*it<<" 加入"<<endl;
                                    this->print_symbol_first();
                                    #endif
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// 从任意符号串得到FIRST集
set<string> allProduction::First(vector<symbol> symList){
    // 检查symList的合法性，即所有的符号是否都在nonterminal或者terminal中出现
    for(auto element: symList){
        if( (element.type == TYPE::terminal && !vectorContain(terminal,element.str)  && element.str != "#" ) || 
        (element.type == TYPE::nonterminal && !vectorContain(nonterminal,element.str))){
            cout<<"无法识别的符号: "<<element.str<<endl;
            return set<string>();
        }
    }
    set<string> first = set<string>();
    // 第一个能推出epsilon的符号的位置
    int firstPosition = -1;
    for(int i=0;i<symList.size();i++){
        // symbol sym = symList[i];
        symbol sym;
        int can = -2;//能够推出epsilon
        // 非终结符，则检测nonterinal，查看是否能推出epsilon
        if(symList[i].type == TYPE::nonterminal){
            sym = nonterminal[indexTable[symList[i].str]];
            can = (sym.can_generate_epsilon ? 1 : 0);
        }else can = 0;
        if(!can){
            firstPosition = i;
            break;
        }
    }  
    // 所有符号都能推出epsilon
    if(firstPosition == -1){
        for(int i=0;i<symList.size();i++){
            // first = first1 & first2 & ...
            for(auto str: this->nonterminal[indexTable[symList[i].str]].first)first.insert(str);
        }
    }else{
        for(int i=0;i<symList.size();i++){
            // 将firstPosition之前的 (first集合-epsilon)加入first集合中
            if(i<firstPosition){
                assert(nonterminal[indexTable[symList[i].str]].first.count(EPSILON_STRING)>0);
                // 不可使用引用，删除sym中的epsilon元素
                symbol sym = nonterminal[indexTable[symList[i].str]];
                sym.first.erase(EPSILON_STRING);
                assert(sym.first.count(EPSILON_STRING)==0);
                for(auto element: sym.first)first.insert(element);
            }else{//将当前不能推出epsilon的符号first集合并入first变量中
                // 分成符号是非终结符和符号是终结符两种情况来处理
                symbol sym = symbol();
                if(symList[i].type==TYPE::terminal)sym.type=TYPE::terminal,sym.first.insert(symList[i].str);
                else if(symList[i].type==TYPE::nonterminal)sym=nonterminal[indexTable[symList[i].str]];

                for(auto element: sym.first)first.insert(element);
                break;
            }
        }
    }

    return first;
}

// 项目点 向右移动一格
Item Item::moveOneStep(Item item){
    Item retItem = item;
    retItem.pointer++;
    // pointer最多指在right.size()位置上
    assert(retItem.pointer <= retItem.right.size());
    return retItem;
}

// 测试是否是规约项目
bool Item::checkCanReduction(){
    // 当pointer指的位置在产生式的最右端时为规约项目
    return this->pointer == this->right.size();
}

// 测试是否可以移进
bool Item::checkCanShiftIn(){
    // 项目点在最后，无法读字符和移进
    if(this->pointer >= right.size())return false;
    // 项目点右边第一个符号是非终结符
    return (right[this->pointer].type == TYPE::terminal);
}

void Item::printItem(){
    cout<<this->left.str<<" -> ";
    if(this->pointer == 0)cout<<"·";
    for(int i=0;i<right.size();i++){
        cout<<right[i].str;
        if(i == this->pointer - 1)cout<<"·";
    }
    cout<<" , "<<OPENING_BRACKET<<" ";
    vector<string> v(frontSet.begin(),frontSet.end());
    for(int i=0;i<v.size();i++){
        if(i!=v.size()-1){
            cout<<v[i]<<",";
        }else cout<<v[i];
    }
    cout<<" "<<CLOSING_BRACKET<<endl;
}

void ItemSet::printItemSet(){
    cout<<"状态： "<<this->state<<endl;
    for(int i=0;i<this->itemList.size();i++){
        itemList[i].printItem();
    }
    cout<<endl;
}

// 比较两个DFA状态是否相等
bool ItemSet::operator==(const ItemSet &other) const {
    // 两个状态包含的item数量不一样，则false
    if(other.itemList.size() != this->itemList.size())return false;
    //两个状态中相同的item数量,初始时相同的item数量为1
    int total=0;
    // 对于每个i，在j中都存在itemj==itemi，则两个item相等
    for(int i=0;i<this->itemList.size();i++){
        for(int j=0;j<other.itemList.size();j++){
            // item的比较包括对前向搜索符号集合的比较
            assert(this->itemList[i].frontSet.size()==1);
            assert(other.itemList[j].frontSet.size()==1);
            if(Item::isEqual(itemList[i],other.itemList[j],true)){
                total++;
                break;
            }
        }
    }

    return total == itemList.size();
}

// 找到包含S’ -> S· , { # }的状态，为acc状态
vector<int>  Processor::findAccpetState(){
    vector<int> ret;
    production pro;
    // 扩充的产生式是第一条，构造一个终止状态，设定firstSet为#
    Item final = Item(this->pros.productions[0]);
    final.frontSet.clear();
    final.frontSet.insert(FRONT_SERARCH);
    final.pointer = final.right.size();
    // cout<<"final:";
    // final.printItem();
    assert(final.pointer == 1);
    for(int i=0;i<this->DFA.size();i++){
        ItemSet &state = this->DFA[i];
        for(auto item: state.itemList){
            if(Item::isEqual(item,final,true))ret.push_back(i);
        }
    }
    // cout<<"acc状态:";
    // for(auto x: ret)cout<<x<<" ";
    // cout<<endl;
    return ret;
}

// 根据DFA状态图计算ACTION和GOTO表
void Processor::generateActionAndGoto(){
    // 书146页算法
    this->Goto.clear();
    this->Action.clear();
    // 初始化，将表的所有转移状态都设置成非法状态
    for(int i=0;i<this->DFA.size();i++){
        for(auto ele: this->pros.terminal){
            map<string,Element> toinsert;
            toinsert.insert(make_pair(ele.str,Element(TYPE_OF_ACTION_GOTO::ILLEGAL,-1)));
            Action.insert(make_pair(i,toinsert));
        }
        map<string,Element> toinsert;
        toinsert.insert(make_pair(FRONT_SERARCH,Element(TYPE_OF_ACTION_GOTO::ILLEGAL,-1)));
        Action.insert(make_pair(i,toinsert));
        for(auto ele: this->pros.nonterminal){
            map<string,Element> toinsert;
            toinsert.insert(make_pair(ele.str,Element(TYPE_OF_ACTION_GOTO::ILLEGAL,-1)));
            Goto.insert(make_pair(i,toinsert));
        }
    }
    // 扫描每一个项目集
    for(int stateIndex = 0;stateIndex < this->DFA.size();stateIndex ++ ){
        // 扫描每一个item
        for(int i=0;i<DFA[stateIndex].itemList.size();i++){
            Item &item = DFA[stateIndex].itemList[i];
            // 移进项目，ACTION[i,a]=toState,i:当前状态，a:移进符号
            // 一定可以通过移进达到新的状态
            if(item.checkCanShiftIn()){
                string string_trans = item.right[item.pointer].str;
                int toState = this->transferTable[stateIndex][string_trans];
                this->Action[stateIndex][string_trans] = Element(TYPE_OF_ACTION_GOTO::STATE,toState);
            }
            // 规约项目，设置ACTION[i,a]=Rj,其中a 属于item的前向搜索符号集合，Rj为产生式的编号
            else if(item.checkCanReduction()){
                // 扫描该item的前向搜索符号,
                for(set<string>::iterator it = item.frontSet.begin();it!=item.frontSet.end();it++){
                    //将产生式编号赋值，item继承自production
                    this->Action[stateIndex][*it]=Element(TYPE_OF_ACTION_GOTO::PRODUCTION,item.index);
                }
            }
            //不是移进项目也不是规约项目，则可以通过非终结符进行转移
            // 设置GOTO表 GOTO[i,"STR"]=toState;
            else{
                string string_trans = item.right[item.pointer].str;
                assert(item.right[item.pointer].type == TYPE::nonterminal);
                // 0为默认值，若存在转移边则>=1,在构造DFA时一定有转移弧
                int toState = this->transferTable[stateIndex][string_trans];
                this->Goto[stateIndex][string_trans] = Element(TYPE_OF_ACTION_GOTO::STATE,toState);
            }
        }
    }
    // 设置ACTIONGOTO表中的接受状态
    vector<int> acceptedStates = this->findAccpetState();
    for(auto state: acceptedStates){
        this->Action[state][FRONT_SERARCH]=Element(TYPE_OF_ACTION_GOTO::ACCEPT,-1);
        // cout<<"设置accept状态"<<endl;
    }

    
}

Item::Item(production pro){
    
    this->left = pro.left;
    // 如果是一条能推出epsilon的产生式，则构造一个只含点的item
    if(pro.right.size()==1 && pro.right[0].str == EPSILON_STRING){
        this->right.clear();
    }else this->right = pro.right;
    this->index = pro.index;
    this->pointer = 0;
    this->frontSet.clear();
}

// 判断两个item是否相等，in表示是否将前向搜索符号集合一起比较
bool Item::isEqual(Item item1,Item item2,bool in){
    // ① 比较产生式左边字符
    if((item1.left.str != item2.left.str))return false;
    // ② 左边字符相等，比较产生式右边字符
    vector<symbol> right1 = item1.right;
    vector<symbol> right2 = item2.right;
    if(right1.size()!=right2.size())return false;
    for(int i=0;i<right1.size();i++){
        // 右端不全相等，返回false
        if((right1[i].str != right2[i].str) || (right1[i].str == right2[i].str && right1[i].type != right2[i].type))
            return false;
    }
    // ③ 产生式和点位置都一致，
    if(item1.pointer == item2.pointer){
        if(in){
            if(item1.frontSet == item2.frontSet)return true;
            else return false;
        }else return true;
    }
    return false;
}

// 判断是否是一条推出空的item
bool Item::isEpsilonItem(){
    return (this->right.size()==0 && this->pointer == 0);
}

// 检查一条item是否可以插入当前itemSet中
bool ItemSet::checkCanInsert(Item item){
    for(int i=0;i<this->itemList.size();i++){
        assert(itemList[i].frontSet.size() == 1);
        // item存在则返回false，无法进行插入
        if(Item::isEqual(itemList[i],item,true)){
            return false;
        }
    }
    return true;
}

// 计算项目集的闭包
// 传入参数：itemSet的核产生式集合
ItemSet Processor::stateClosure(ItemSet kernel){
    // 根据kenerl构造一个DFA项目集
    ItemSet retItemSet = kernel;
    retItemSet.state = 0;
    // 下一个需要进行转移的产生式
    int next_to_trans = 0;
    // 当前闭包中的item索引
    int index = 0;
    // 存在可以进行转移的条目则取出进行转移
    // 没有新的条目插入时，扫描完整个retItemSet之后结束
    while(next_to_trans < retItemSet.itemList.size()){
        Item item = retItemSet.itemList[next_to_trans ++ ];
        // 如果是一个规约项目，则不需要计算闭包
        if(item.checkCanReduction())continue;
        // cout<<"下一个需要闭包运算的式子："<<next_to_trans<<endl;
        // 如果是一条形式如 A->·的item，不考虑闭包运算
        if(item.isEpsilonItem())continue;
        string toMatch = item.right[item.pointer].str;
        for(production &pro: this->pros.productions){
            // 对所有该符号对应的产生式，将其加入项目集中
            if(pro.left.str == toMatch){
                // cout<<pro.left.str<<endl;
                //从该产生式构造item
                Item itemToInsert = Item(pro);
                assert(itemToInsert.frontSet.size()==0);
                // 对需要考虑添加的项目，对其前向搜索符号集合进行扫描
                assert(item.frontSet.size()==1);
                vector<symbol> toFirst;//用于进入first集合的符号串
                // 从pointer开始，将之后的所有 
                for(int i=item.pointer+1;i<item.right.size();i++){
                    toFirst.push_back(item.right[i]); 
                    // cout<<item.right[i].str<<endl;
                }
                    // 将item中唯一的前向搜索符号放到list尾部
                symbol sym = symbol(*(item.frontSet.begin()),TYPE::terminal);
                toFirst.push_back(sym);
                // 设置前向搜索符号集合,前向搜索符号集合可能有多个值，对每个值以及当前产生式都判断能否插入
                set<string> FIRST = pros.First(toFirst);
                assert(FIRST.size()>0);
                for(set<string>::iterator it=FIRST.begin();it!=FIRST.end();it++){
                    set<string> frontToInsert = set<string>();
                    frontToInsert.insert(*it);
                    itemToInsert.frontSet = frontToInsert;
                    itemToInsert.pointer = 0;
                    // 如果检查之后发现能够插入retItemSet中则插
                    if(retItemSet.checkCanInsert(itemToInsert)){
                        retItemSet.itemList.push_back(itemToInsert);
                    }
                }
            }
        }
    }

    return retItemSet;
}

// 紧缩操作，将对应同一个产生式的item的first集合合并
ItemSet Processor::merge(ItemSet itemSet){
    ItemSet ansItemSet;
    ansItemSet.state = itemSet.state;
    vector<Item> &itemList = itemSet.itemList;
    // 标记产生式是否处理过
    vector<bool> checked = vector<bool>();
    int length = itemList.size();
    for(int i=0;i<length;i++)checked.push_back(false);
    for(int i=0;i<length;i++){
        // 线性扫描item集合，如果已经考虑过则继续搜索
        if(checked[i])continue;
        // 将没考虑过且有相同的item（除前向搜索符号集合）合并first集合
        for(int j=i+1;j<length;j++){
            if(checked[j])continue;
            // 不比较frontSet部分，如果相等，则j的frontset中必然只有一个元素，并且不同
            // i的first集合中包括更多的元素，因为在不断添加元素
            if(Item::isEqual(itemList[j],itemList[i],false)){
                assert(itemList[j].frontSet.size() == 1);
                assert(*(itemList[j].frontSet.begin()) != *(itemList[i].frontSet.begin()));
                itemList[i].frontSet.insert(*(itemList[j].frontSet.begin()));
                checked[j]=true;
            }
        }
        checked[i]=true;
        ansItemSet.itemList.push_back(itemList[i]);
    }

    return ansItemSet;
}

bool Processor::checkCanTrans(Item item){
    // 形式如 A->@的item不应该出现
    assert(!(item.right.size()==1 && item.right[0].str==EPSILON_STRING));
    if(item.right.size()==0)return false;//像A->·这样的产生式,无法转移
    if(item.checkCanReduction())return false;//可以规约的项目，不能进行转移
    return true;
}

// 查看DFA中是否包含状态itemset，包含则返回索引
// 不存在则返回-1
int Processor::isExist(ItemSet itemSet){
    for(int i=0;i<this->DFA.size();i++){
        if(DFA[i]==itemSet)return i;
    }
    return -1;
}

// 构造项目集规范族
void Processor::generateDFA(allProduction prodctions){
    // 设置Processor的产生式数据结构
    this->pros = prodctions;
    production pro_1 = this->pros.productions[0];
    Item initialItem = Item(pro_1);
    // 闭包内每个item的前向搜索符号集合都只有一个符号
    initialItem.frontSet.insert(FRONT_SERARCH);
    ItemSet initialkernel;
    //初始项目集合中只有一个item
    initialkernel.itemList.push_back(initialItem);
    initialkernel.state = 0;
    ItemSet initialItemSet = processor.stateClosure(initialkernel);
    #ifdef DEBUG
    merge(initialItemSet).printItemSet();
    #endif
    // 使用队列存放当前需要进行转移的项目集
    queue<ItemSet> queue;
    queue.push(initialItemSet);
    this->DFA.push_back(initialItemSet);
    int state_index = 0;
    // 广度优先搜索
    while(!queue.empty()){
        ItemSet toTrans = queue.front();
        queue.pop();
        vector<Item> &itemList = toTrans.itemList;
        // cout<<"考虑的状态编号："<<toTrans.state<<endl;
        // cout<<itemList.size()<<endl;
        // 标志item是否已经经过转移
        vector<bool> checked = vector<bool>();
        for(int i=0;i<itemList.size();i++)checked.push_back(false);
        // 线性扫描，将已经转移的设置标记
        for(int i=0;i<itemList.size();i++){
            if(checked[i])continue;
            Item &itemi = itemList[i];
            // 无法进行转移，则continue
            if(!checkCanTrans(itemi))continue;
            ItemSet kernel;
            // 转移边对应的字符串
            string transform_string = itemi.right[itemi.pointer].str;
            // cout<<"考虑的："<<i<<" i转移字符="<<itemi.right[itemi.pointer].str<<endl;
            // 最初kernel中只有转移只有的 itemi
            kernel.itemList.push_back(Item::moveOneStep(itemi));
            // kernel.printItemSet();
            // 将相同的转移字符对应的item放入核中
            for(int j=i+1;j<itemList.size();j++){
                Item &itemj = itemList[j];
                // 能够进行转移,并且转移字符相同
                if( checkCanTrans(itemj) && (itemj.right[itemj.pointer].str == itemi.right[itemi.pointer].str)){
                    // cout<<"j="<<j<<" 转移字符是： "<<itemj.right[itemj.pointer].str<<" pointer="<<itemj.pointer<<endl;
                    kernel.itemList.push_back(Item::moveOneStep(itemj));
                    checked[j]=true;
                }
            }

            ItemSet newState = this->stateClosure(kernel);
            int toState = isExist(newState);
            // 设置ACTION和GOTO表
            // newState.printItemSet();
            if(toState == -1){
                (this->DFA).push_back(newState);
                newState.state = ++ state_index;
                #ifdef DEBUG
                merge(newState).printItemSet();
                #endif
                queue.push(newState);
                // 将所有的状态都+1，则状态都从1开始，无法转移的状态保持0
                transferTable[toTrans.state].insert(make_pair(transform_string,state_index));
            }else{
                transferTable[toTrans.state].insert(make_pair(transform_string,toState));
            }
        }
    }
}

void Processor::printTransTable(){
    cout<<endl<<"转移映射表:"<<endl;
    for(auto it=this->transferTable.begin();it != this->transferTable.end();it++){
        cout<<it->first<<" "<<endl;
        for(auto ele:it->second){
            cout<<ele.first<<" "<<ele.second - 1<<endl;
        }
        cout<<endl;
    }
}

// 转变ACTION GOTO表的输出形式
string Element::transElement(){
    if(this->type==TYPE_OF_ACTION_GOTO::STATE){
        return "S"+to_string(this->index);
    }else if(this->type==TYPE_OF_ACTION_GOTO::PRODUCTION){
        return "r"+to_string(this->index);
    }else if(this->type == TYPE_OF_ACTION_GOTO::ACCEPT){
        return "acc";
    }else{
        return to_string(this->index);
    }
}

// 打印action 和 goto表
void Processor::printActionGoto(){
    cout<<"ACTION+GOTO表："<<endl;
    // 列：非终结符在前，终结符在后
    vector<string> col;
    for(auto x: this->pros.terminal)col.push_back(x.str);
    col.push_back(FRONT_SERARCH);
    for(auto x: this->pros.nonterminal)col.push_back(x.str);
    int count = this->pros.terminal.size();
    for(int i=0;i<col.size();i++){
        // 如果是扩充的文法开始符号，则在ACTIONGOTO表中不考虑这一项
        if(col[i] == this->pros.productions[0].left.str)continue;
        if(i==0)cout<<"   ";
        cout<<col[i]<<"  ";
    }
    cout<<endl;
    for(int i=0;i<this->Action.size();i++){
        cout<<i<<"  ";
        vector<symbol> syms = this->pros.terminal;
        syms.push_back(symbol(FRONT_SERARCH,TYPE::epsilon));
        for(auto ele : syms){
            Element current = Action[i][ele.str];
            cout<<current.transElement()<<"   ";
        }
        for(auto ele:this->pros.nonterminal){
            if(ele.str == this->pros.productions[0].left.str)continue;
            // if(Goto[i][ele.str].type == TYPE_OF_ACTION_GOTO::STATE)
                cout<<Goto[i][ele.str].index<<"   ";
        }
        cout<<endl;
    }
    cout<<endl;
}

// 判断一个字符序列是否是LR(1)自动机能够识别的字符序列
bool Processor::isIllegalList(vector<symbol> symList,vector<Token> tokenList){
    if(symList.size()==0)return true;
    for(int i=0;i<tokenList.size();i++)cout<<tokenList[i].value<<" ";
    cout<<"的LR(1)识别过程"<<endl;
    cout<<"步骤   状态栈   符号栈   输入字符串   ACTION  GOTO"<<endl;
    int step = 0;//步骤
    vector<int> stateStack;//状态栈
    vector<symbol> symbolStack;//符号栈
    stateStack.push_back(0);//初始状态0
    symbolStack.push_back(symbol(FRONT_SERARCH,TYPE::front_search));//初始符号栈中有一个#
    int current_state = stateStack.back();//当前的状态
    int scanner = 0;//当前字符序列扫描到的位置
    Element action = this->Action[current_state][symList[0].str];

    cout<<++step<<" : ";
    printVector(stateStack);
    cout<<"        ";
    for(int i=0;i<symbolStack.size();i++)cout<<symbolStack[i].str<<" ";
    cout<<"        ";
    for(int i=scanner;i<tokenList.size();i++)cout<<tokenList[i].value<<" ";
    cout<<" #       ";
    cout<<action.transElement()<<"(action)        ";
    cout<<endl;
    while(true){
        if(action.index == -1){
            cout<<"error"<<endl;
            this->errorPos = scanner;//当前正在扫描的字符是错误标志
            return false;
        }
        if(action.type==TYPE_OF_ACTION_GOTO::PRODUCTION){
            production pro = this->pros.productions[action.index];
            int number;
            if(pro.right.size()>0 && pro.right[0].str ==EPSILON_STRING){
                number = 0;
            }else{
                number = pro.right.size();
            }
            // 状态栈和符号栈推出n个符号或者状态，n:产生式右端的长度
            // 修改bug:如果产生式右边只有一个空，则该产生式规约事状态栈和符号栈不弹栈
            while(number -- ){
                stateStack.pop_back();
                symbolStack.pop_back();
            }
            // 符号栈进入规约之后的符号
            symbolStack.push_back(pro.left);
            // 状态栈之后进入GOTO之后的状态
            int GotoState = this->Goto[stateStack.back()][pro.left.str].index;
            stateStack.push_back(GotoState);
            Element new_action = this->Action[stateStack.back()][symList[scanner].str];
            // 打印一行识别情况
            cout<<++step<<" : ";
            printVector(stateStack);
            cout<<"     ";
            for(int i=0;i<symbolStack.size();i++)cout<<symbolStack[i].str<<" ";
            cout<<"        ";
            for(int i=scanner;i<tokenList.size();i++)cout<<tokenList[i].value<<" ";
            cout<<" #       ";
            cout<<new_action.transElement()<<"(action)        ";
            if(new_action.type == TYPE_OF_ACTION_GOTO::PRODUCTION){
                production pro = this->pros.productions[new_action.index];
                cout<<this->Goto[stateStack[stateStack.size()-pro.right.size()-1]][pro.left.str].index<<"(goto)      ";
            }
            cout<<endl;
            action = new_action;
        }else if(action.type == TYPE_OF_ACTION_GOTO::STATE){
            int newState = action.index;
            stateStack.push_back(newState);
            symbolStack.push_back(symList[scanner]);
            scanner++;
            Element new_action = this->Action[stateStack.back()][symList[scanner].str];
            // 打印一行结果
            cout<<++step<<" : ";
            printVector(stateStack);
            cout<<"     ";
            for(int i=0;i<symbolStack.size();i++)cout<<symbolStack[i].str<<" ";
            cout<<"        ";
            for(int i=scanner;i<tokenList.size();i++)cout<<tokenList[i].value<<" ";
            cout<<" #       ";
            cout<<new_action.transElement()<<"(action)        ";
            if(new_action.type == TYPE_OF_ACTION_GOTO::PRODUCTION){
                production pro = this->pros.productions[new_action.index];
                cout<<this->Goto[stateStack[stateStack.size()-pro.right.size()-1]][pro.left.str].index<<"(goto)        ";
            }
            cout<<endl;
            // 下一步的aciton，进行迭代
            action = new_action;
        }
        if(action.type == TYPE_OF_ACTION_GOTO::ACCEPT)return true;
    }
    cout<<"error at the end"<<endl;
    return false;
}

void test_LR_1(){
    vector<symbol> symList;//list中都是非终结符号
    vector<Token> tokenList;
    tokenList.push_back(Token(0,-1,FRONT_SERARCH));
    // symList.push_back(symbol("a",TYPE::terminal));
    // symList.push_back(symbol("b",TYPE::terminal));
    // symList.push_back(symbol("a",TYPE::terminal));
    // symList.push_back(symbol("b",TYPE::terminal));
    // symList.push_back(symbol("#",TYPE::front_search));
    processor.isIllegalList(symList,tokenList);
}

// 测试函数：测试itemset相等函数是否正确
void test_itemset_equal(){
    ItemSet itemset1;
    production pro;
    pro.left = symbol("A",TYPE::terminal);
    pro.right.push_back(symbol("b",TYPE::nonterminal));
    pro.right.push_back(symbol("B",TYPE::terminal));
    Item item1=Item(pro);
    item1.pointer=0;
    item1.frontSet.insert("*"); 
    itemset1.itemList.push_back(item1);
    production pro2;
    pro.left = symbol("B",TYPE::terminal);
    pro.right.push_back(symbol("c",TYPE::nonterminal));
    pro.right.push_back(symbol("C",TYPE::terminal));
    Item item2=Item(pro);
    item2.pointer=1;
    item2.frontSet.insert("&"); 
    itemset1.itemList.push_back(item2);

    ItemSet itemset2;
    itemset2.itemList.push_back(item2);
    itemset2.itemList.push_back(item1);

    itemset1.printItemSet();
    itemset2.printItemSet();

    cout<<(itemset1==itemset2)<<endl;
}

// 测试任意求FIRST的函数的正确性
void testFirstFunction(){
    vector<symbol> test;
    symbol sym1,sym2;
    sym1.str = "#";
    sym1.type = TYPE::terminal;
    // sym2.str = "#";
    // sym2.type = TYPE::terminal;
    test.push_back(sym1);
    // test.push_back(sym2);
    set<string> first = pros.First(test);

    // vector<symbol> test;
    // symbol sym1,sym2;
    // sym1.str = "[identifier]";
    // sym1.type = TYPE::terminal;
    // // sym2.str = "D";
    // // sym2.type = TYPE::nonterminal;
    // test.push_back(sym1);
    // // test.push_back(sym2);
    // set<string> first = pros.First(test);
    // cout<<"first:";
    // for(set<string>::iterator it = first.begin();it!=first.end();it++){
    //     cout<<*it<<" ";
    // }
}

void Token::printToken(){
    cout<<"( "<<this->line<<" , "<<tokenType2String[this->type]<<" , '"<<this->value<<"' )"<<endl;
}
// 使用文档流读取一个token项
bool readToken(Token &token){
    if(tokenStream.is_open()){
        string line;    
        getline(tokenStream,line);
        // 删除无用的空格
        while(find(line.begin(),line.end(),' ') != line.end())
            line.erase(find(line.begin(),line.end(),' '));
        int pos_1 = line.find_first_of(',');
        int pos_2 = line.find_first_of(',',pos_1 + 1);
        string line_number = line.substr(1,pos_1-1);
        if(line=="$")return false;//到tokenlist的结尾
        token.line = atoi(line_number.c_str());//string 转化成 int
        string second_part = line.substr(pos_1+1,pos_2-pos_1-1);
        token.type = string2TokenType[second_part];
        int pos_3 = line.find_first_of('\'');
        int pos_4 = line.find_last_of('\'');
        token.value = line.substr(pos_3+1,pos_4-pos_3-1);
        return true;
    }
    return false;
}

string Processor::errorMsg(Token token){
    if(token.type == TokenType::constant){
        return "第 "+to_string(token.line)+" 行 : 非法的常量： "+token.value;
    }else if(token.type == TokenType::delimiter){
        return "第 "+to_string(token.line)+" 行 : 非法的限定符： "+token.value;
    }else if(token.type == TokenType::keyword){
        return "第 "+to_string(token.line)+" 行 : 非法的关键词： "+token.value;
    }else if(token.type == TokenType::identifier){
        return "第 "+to_string(token.line)+" 行 : 非法使用标识符： "+token.value;
    }else{
        return "第 "+to_string(token.line)+" 行 : 非法使用运算符： "+token.value;
    }
}   

// 检查输入的tokenList是否符合产生式规范
bool isIllegalTokenList(){
    vector<symbol> symList;
    vector<Token> tokenList;
    while(true){
        Token token;
        bool successRead = readToken(token);
        if(!successRead)break;//直到读到最后一个文档终止字符
        token.printToken();
        symbol newSymbol;
        if(token.type == TokenType::identifier){
            newSymbol.tokenType = token.type;
            newSymbol.str = IDENTIFIER;
        }else if(token.type == TokenType::keyword){
            newSymbol.tokenType = TokenType::identifier;
            newSymbol.str = token.value;
        }else if(token.type == TokenType::delimiter){
            newSymbol.tokenType = TokenType::delimiter;
            newSymbol.str = token.value;
        }else if(token.type == TokenType::operand){
            newSymbol.tokenType = token.type;
            newSymbol.str = token.value;
        }else if(token.type == TokenType::constant){
            newSymbol.tokenType = token.type;
            newSymbol.str = SCIENTIFIC_FORMAT;
        }
        symList.push_back(newSymbol);
        tokenList.push_back(token);
    }
    symList.push_back(symbol(FRONT_SERARCH,TYPE::front_search));
    bool success = processor.isIllegalList(symList,tokenList);
    string error_msg;
    if(!success){
        if(processor.errorPos == tokenList.size()){
            error_msg = "程序存在闭合性错误(缺少'{'或者'}'等原因)";
            cout<<error_msg<<endl;
        }else{
            Token token = tokenList[processor.errorPos];
            error_msg = processor.errorMsg(token);
            cout<<error_msg<<endl;
        }
        ofstream output("error_file.txt");
        if(output.is_open()){
            output.clear();
            output<<error_msg;
            output.close();
        }
    }
    return success;
}

void initialParam(){
    string2TokenType.insert(make_pair("标识符",TokenType::identifier));
    string2TokenType.insert(make_pair("限定符",TokenType::delimiter));
    string2TokenType.insert(make_pair("运算符",TokenType::operand));
    string2TokenType.insert(make_pair("关键字",TokenType::keyword));
    string2TokenType.insert(make_pair("常量",TokenType::constant));

    tokenType2String.insert(make_pair(TokenType::identifier,"标识符"));
    tokenType2String.insert(make_pair(TokenType::delimiter,"限定符"));
    tokenType2String.insert(make_pair(TokenType::operand,"运算符"));
    tokenType2String.insert(make_pair(TokenType::keyword,"关键字"));
    tokenType2String.insert(make_pair(TokenType::constant,"常量"));
}
int main(){
    // 记录程序的执行时间
    clock_t startTime,endTime;
    startTime = clock();
    // 获取文法的产生式
    get_grammer();
    // 获取终结符和非终结符集合
    pros.initial_production_set();
    // 获取非终结符推出epsilon的表
    pros.markEpsilonProduction();
    // 获取所有文法符号的first集合
    pros.getFirstSet();
    // 测试first函数计算
    // testFirstFunction(); 

    processor.generateDFA(pros);
    #ifdef DEBUG
    processor.printTransTable();
    #endif
    processor.findAccpetState();
    processor.generateActionAndGoto();
    processor.printActionGoto();    

    // test_LR_1();

    initialParam();
    if(isIllegalTokenList())
        cout<<"成功识别"<<endl;
    
    #ifdef DEBUG
        // test_itemset_equal();
    #endif
    endTime = clock();
    cout << "程序的运行时间（含I/O)：" <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    return 0;
}