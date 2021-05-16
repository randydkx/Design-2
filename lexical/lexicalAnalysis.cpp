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
// 头文件，包含使用的数据结构
#include"lexicalAnalysis.h"
using namespace std;

#ifndef DEBUG  
    // #define DEBUG
#endif

#ifndef TEST
    #define TEST
#endif

const string problem3 = "problem3_code.txt";
const string initial_code = "code.txt";
const string code_file = initial_code;


// 四个种类单词的文法自动机
Processor scientifics,delimiters,identifiers,operators,keywords;

bool Processor::vectorContain(vector<int> v, int element){
  for(auto ele: v){
      if(ele==element)return true;
  }
  return false;
}

void printVector(vector<int> v){
    cout << OPENING_BRACKET;
    for(int i=0;i<v.size();i++){
        if(i==v.size()-1) cout<<v[i];
        else cout<<v[i]<<",";
    }
    cout << CLOSING_BRACKET;
}

int Processor::is_a_state_of_DFA(vector<int> state){
    int size = this->dfa.size();
    for(int i = 0; i < size; i++){
        DFAState current = this->dfa[i];
        if(current.states == state)
            return i;
    }
    //   该状态不是DFA表中的一个状态
    return -1;
}

vector<int> Processor::eclosure(vector<int> T){
    // epsilon闭包集合
    set<int> eclosure;
    // 广搜队列
    queue<int> queue;
    // 初始的转换元素是集合本身
    for(auto x:T){queue.push(x);}
    //初始化epsilon闭包
    for(auto x:T){eclosure.insert(x);}
    // 广度优先搜索，将T集合中所有能通过epsilon转化到的边都放到闭包中
    while(!queue.empty()){
        int cur = queue.front();
        queue.pop();
        // 取NFA当前cur状态的epsilon弧转换集合
        vector<int> moveset = this->nfa[cur][EPSILON];

        for(auto element: moveset){
            // 集合中不包含新的epsilon弧转移状态，加入集合
            // 将其放入队列，进行下一轮转换
            if(!eclosure.count(element)){
                eclosure.insert(element);
                queue.push(element);
            }
        }
    }
    return vector<int>(eclosure.begin(),eclosure.end());
}

vector<int> Processor::move(vector<int> states, char ch){
    set<int> can_reach;
    // 对于states中的给个state，将能够通过ch弧转换得到的状态都放到set中
    for(auto state:states){
        vector<int> reach = this->nfa[state][ch];
        for(auto move_to_state: reach)
            can_reach.insert(move_to_state);
    }
    // 使用set容器构建vector容器，由于set是红黑树，重构之后vector有序
    return vector<int>(can_reach.begin(),can_reach.end());
}

DFAState Processor::newDFAState(bool mark, vector<int> s){
    DFAState newState;
    newState.marked = mark;
    newState.states = s;
    newState.moves = map<char,int>();
    return newState;
}

vector<int> Processor::findFinalDFAStates(){
    vector<int> finals;
    set<int> nfa_final_set(this->nfa_final_states.begin(),this->nfa_final_states.end());
    for(int i = 0; i < dfa.size(); i++){
        //   对每一个NFA的终点，检查DFA的状态集合中是否包含，包含则为DFA的终态
        for(auto one_final:this->nfa_final_states){
            if(vectorContain(dfa[i].states,one_final)){
                finals.push_back(i);
            }
        }
    }

    return finals;    
}

void Processor::subsetMethod(){

    // 初始化DFA状态标号
    this->dfa_state_count = 0;
    //   初始状态集合，也就是DFA的入口集合
    vector<int> initialStateVector;
    initialStateVector.push_back(nfa_init_state);

    // 初始状态的闭包
    vector<int> eclos = eclosure(initialStateVector);
    #ifdef DEBUG
    cout << "epsilon-closure{"<<this->nfa_init_state<<"} = ";
    printVector(eclos);
    cout << " = " << this->dfa_state_count << "(DFA)\n\n";
    #endif
    // 添加一个DFA状态，将其加入状态集合，设置为未标记
    DFAState initState = newDFAState(false, eclos);

    this->dfa[this->dfa_state_count++] = initState;
    this->to_mark = 0;

    while(this->to_mark < this->dfa_state_count){
        int k = this->to_mark;
        this->dfa[k].marked = true;

        #ifdef DEBUG
        cout << "\n标记状态 " << k << endl;
        #endif

        for(auto ch:alphabet){
            vector<int> moveset = move(this->dfa[k].states,ch);
            vector<int> eclosure_move = eclosure(moveset);
            // move集合的结果非空，即存在当前状态通过ch的弧转换
            if(!moveset.empty()){
                #ifdef DEBUG
                printVector(this->dfa[k].states);
                cout << "-- " << ch << " --> ";
                printVector(moveset);
                cout << "\n";
                cout << "epsilon-closure";
                printVector(moveset);
                cout << " = ";
                printVector(eclosure_move);
                cout << " = ";
                #endif
            }
            // 判断是否是DFA的状态之一
            int j = is_a_state_of_DFA(eclosure_move);

        // 如果转移之后的某个DFA状态已经存在就直接设置状态标号
            if(j >= 0){
                #ifdef DEBUG
                cout << j << "\n";
                #endif
                // DFA: k-- ch --> j
                this->dfa[k].moves[ch] = j;
            }
            else{
                //   状态不存在的情况下将该集合对应的状态设置为新的DFA状态
                if(!eclosure_move.empty()){
                    // 新的DFA状态标号
                    #ifdef DEBUG
                    cout << dfa_state_count << "\n";
                    #endif
                    DFAState newState = newDFAState(false, eclosure_move);
                    this->dfa[dfa_state_count] = newState;
                    //   设置当前状态转换之后的状态
                    this->dfa[k].moves[ch] = dfa_state_count;
                    dfa_state_count++;
                }
                else{
                    //move状态转换的结果为空集
                    this->dfa[k].moves[ch] = -1;
                }
            }
        }

        // 标记完一个状态之后，进入下一个需要标记的状态
        this->to_mark ++ ;
    }
    #ifdef DEBUG
    cout << "\n";
    #endif
}

void Processor::printDFA(){
    // 打印字符表
    cout<<"DFA：";
    for(auto element: alphabet){
        if(element!=EPSILON)
            cout<<fixed<<setw(12)<<element;
    }
    cout << endl;
    for(int i = 0; i < dfa.size(); i++){
        cout << i;
        for(auto ch: alphabet){
            if(ch == EPSILON)continue;
            if(dfa[i].moves[ch]!=-1){
                cout<<fixed<<setw(12)<<OPENING_BRACKET<<dfa[i].moves[ch]<<CLOSING_BRACKET;
            }else{
                cout<<fixed<<setw(12)<<OPENING_BRACKET<<" "<<CLOSING_BRACKET;
            }
        }
        cout << endl;
    }
}

void Processor::printNFA(){
    // 打印表头信息
    cout<<"NFA:";
    for(auto ch: alphabet)cout<<fixed<<setw(5)<<ch;
    cout<<endl;
    // 打印NFA转移表信息
    for(int i=1;i<=this->nfa_state_count;i++){
        cout<<i<<"    ";
        for(auto ch: alphabet){
            cout<<OPENING_BRACKET<<"";
            for(int j=0;j<nfa[i][ch].size();j++){
                if(j==0)cout<<nfa[i][ch][j];
                else cout<<","<<nfa[i][ch][j];
            }
            cout<<CLOSING_BRACKET<<"   ";
        }
        cout<<endl;
    }
}

// 读取规则文件
vector<string> read_rule_file(string filename,string label){
    ifstream ifstream(filename);
    vector<string> rules;
    if(ifstream.is_open()){
        string line;
        getline(ifstream,line);
        // 直到读到label，开始产生式的读取
        while(line != label)
            getline(ifstream,line);
        while(ifstream.peek()!=EOF){
            getline(ifstream,line);
            rules.push_back(line);
            // 下面的规则对应的产生式
            if(ifstream.peek() == '[')break;
        }
    }
    return rules;
}

// 判断是否符合自动机的规则
bool judge(string content,Processor& pro){
    // 转换后的DFA的起始状态
    int dfa_index = 0;
    set<char> char_set = set<char>(pro.alphabet.begin(),pro.alphabet.end());
    for(auto ch : content){
        // 出现了不是字符表中的字符，false
        if(!char_set.count(ch))return false;
        // 如果当前状态存在转移边，则转移
        if(pro.dfa[dfa_index].moves[ch]!=-1){
            dfa_index = pro.dfa[dfa_index].moves[ch];
        }else{
            return false;
        }
    }
    // 转移完成之后检查是否是终态
    if(pro.dfa_final_states.count(dfa_index))return true;
    else return false;
}

// 从currentState转移到自动机的另一个状态  
// 返回值： -1 ： 当前弧转换之后无法到达合法状态 
int transform(char next,Processor& pro,int currentState){
    assert(currentState >= 0);
    set<char> terminal_set = set<char>(pro.alphabet.begin(),pro.alphabet.end());
    // 不在该自动机对应的非终结符表中
    if(!terminal_set.count(next))return -1;
    if(pro.dfa[currentState].moves[next]!=-1){
        return pro.dfa[currentState].moves[next];
    }else{
        return -1;//无法从当前状态通过next弧进行转换
    }
}

// 测试科学计数法的功能
void test_science_format(){
    vector<string> test;
    test.push_back("4234");
    test.push_back("44323e10");
    test.push_back(".1E10");
    test.push_back(".1e-10");
    test.push_back("103213.3213e+10");
    test.push_back(".3123");
    // 测试虚数的识别
    test.push_back("4234i");
    test.push_back("44323e10i");
    test.push_back(".231E10i");
    test.push_back(".312e-10i");
    test.push_back(".3213e+10i");
    // 测试错误数据
    test.push_back("e10");
    test.push_back("e++10");
    test.push_back(".");
    test.push_back("e");
    test.push_back(".43f3e10");
    test.push_back(".e10");

    for(auto content: test){
        cout<<content<<" : "<<(judge(content,scientifics) ? "true":"false")<<endl;
    }
}

//使用样例测试标识符自动机
void testIdentifier(){
    vector<string> v;
    // 测试正确的标识符
    v.push_back("_");
    v.push_back("fadfas");
    v.push_back("fa312312");
    v.push_back("___");
    v.push_back("421");
    v.push_back("$fadf");
    for(auto to_judge: v){
        cout<<to_judge<<" : "<<(judge(to_judge,identifiers) ? "true":"false")<<endl;
    }
}

// 使用样例测试界符自动机
void test_delimiter(){
    vector<string> v;
    v.push_back(";");
    v.push_back(",");
    v.push_back("[");
    v.push_back("$");
    for(auto content:v){
        cout<<content<<" :"<<(judge(content,delimiters) ? "true":"false")<<endl;
    }
}

// 获取包含[a-z]和[A-Z]的集合
vector<char> get_A_Z(){
    vector<char> v=vector<char>();
    for(char i='a';i<='z';i++)v.push_back(i);
    for(char i='A';i<='Z';i++)v.push_back(i);
    return v;
}

// 获取文件中不同类型串的文法以及生成自动机
// 参数label：构建的自自动机类型
// 参数processor：返回构建的自动机
void get_rule(string label,Processor& processor){
    string filename = "lexical-rule2.txt";
    cout<<"=============================================="<<label<<"======================================"<<endl;
    processor.rules = read_rule_file(filename,label);
    // 打印产生式
    cout<<"文法的产生式:"<<endl;
    for(auto rule:processor.rules)cout<<rule<<endl;

    set<char> nonterminal;
    set<char> terminals;

// 提取所有的终结符和非终结符
    for(int i=0;i<processor.rules.size();i++){
        string rule = processor.rules[i];
        nonterminal.insert(rule[0]);
        if(rule.length()==4){
            if(rule[3]==EPSILON);
            else if(rule[3]!=EPSILON){
                // 如果是科学计数法的形式，将d字符转化成[0-9]终结符
                // 如果是标识符的自动机，将d字符转化成[0-9]终结符
                if((label == SCIENTIFIC_FORMAT && rule[3]=='d') || (label == IDENTIFIER && rule[3]=='d')){
                    for(char i = '0';i<='9';i++){
                        terminals.insert(i);
                    }
                }
                // 如果是标识符的自动机，将产生式中的a换成[a-z]和[A-Z]
                else if(label == IDENTIFIER && rule[3]=='a'){
                    vector<char> all = get_A_Z();
                    for(auto ch:all)terminals.insert(ch);
                }else{
                    terminals.insert(rule[3]);
                }
            }
        }else{
            // 科学计数法，将其中的d转化成自动机中的[0-9]非终结符弧
            if((label == SCIENTIFIC_FORMAT && rule[3]=='d') || (label == IDENTIFIER && rule[3]=='d')){
                for(char i='0';i<='9';i++){
                    terminals.insert(i);
                }
            }
            // 标识符的自动机中，将a转化成[a-z]+[A-Z]
            else if(label == IDENTIFIER && rule[3]=='a'){
                vector<char> all = get_A_Z();
                for(auto ch:all){terminals.insert(ch);}
            }
            else{
                terminals.insert(rule[3]);
            }
            //非终结符的插入不变
            nonterminal.insert(rule[4]);
        }
    }
    processor.alphabet = vector<char>(terminals.begin(),terminals.end());
    vector<char> nonTerminal_vector = vector<char>(nonterminal.begin(),nonterminal.end());
    // 添加epsilon弧
    processor.alphabet.push_back(EPSILON);
    // 输出所有的终结符
    cout<<"字母表：";
    for(auto alpha: processor.alphabet){
        cout<<alpha<<" ";
    }
    cout<<endl;
    
    // 通过终结符集合构建NFA状态映射索引
    processor.indexTable = map<char,int>();
    for(int i=1;i<=nonterminal.size();i++){
        processor.indexTable.insert(make_pair(nonTerminal_vector[i-1],i));
    }
    // 非终结符映射表：
    cout<<"非终结符映射表:"<<endl;
    for(auto x:processor.indexTable){
        cout<<x.first<<" "<<x.second<<endl;
    }
    processor.nfa_state_count = nonterminal.size() + 1; 
    processor.nfa_init_state = processor.indexTable[processor.rules[0][0]];
    // 终态默认是最后一个编号的状态
    processor.nfa_final_states.push_back(processor.nfa_state_count);
    // 唯一的终止状态
    int final = processor.nfa_final_states[0];

    cout<<"状态总数："<<processor.nfa_state_count<<endl;
    cout<<"开始状态："<<processor.nfa_init_state<<" 终止状态："<<final<<endl;
    
    // 构建NFA转移状态表
    for(char ch: nonterminal){
        map<char,vector<int> > movemap;
        // cout<<"状态标号："<<processor.indexTable[ch]<<endl;
        // 指示可以通过某些边转换，将不能通过转换的非终结符对应的转换集合设置成空集
        set<char> has_edge;
        for(int i=0;i<processor.rules.size();i++){
            string rule = processor.rules[i];
            // 找到对应该非终结符的产生式
            if(rule[0]==ch){
                // 右端只有一个非终结符，直接指向最终状态
                if(rule.size()==4){
                    char c = rule[3];
                    // 如果是科学计数法，将d转换成弧[0-9]，转移状态都是终态
                    if((label == SCIENTIFIC_FORMAT && rule[3]=='d') || (label == IDENTIFIER && rule[3]=='d')){
                        for(char i='0';i<='9';i++){
                            has_edge.insert(i);
                            movemap[i].push_back(final);
                        }
                    }
                    else if(label == IDENTIFIER && rule[3]=='a'){
                        vector<char> all=get_A_Z();
                        for(auto ch : all){
                            has_edge.insert(ch);
                            movemap[ch].push_back(final);
                        }
                    }
                    else{
                        has_edge.insert(c);
                        movemap[c].push_back(final);
                    }
                }else{
                    char c = rule[3];
                    int to_nonterminal = processor.indexTable[rule[4]];
                    if((label == SCIENTIFIC_FORMAT && rule[3]=='d') || (label == IDENTIFIER && rule[3]=='d')){
                        for(char i='0';i<='9';i++){
                            has_edge.insert(i);
                            movemap[i].push_back(to_nonterminal);
                        }
                    }
                    // 对于标识符，遇到a时将其转化成[a-z]和[A-Z]
                    else if(label == IDENTIFIER && rule[3]=='a'){
                        vector<char> all = get_A_Z();
                        for(auto ch : all){
                            has_edge.insert(ch);
                            movemap[ch].push_back(to_nonterminal);
                        }
                    }
                    else{
                        has_edge.insert(c);
                        movemap[c].push_back(to_nonterminal);
                    }
                }
            }
        }
        // 不能通过某些非终结符转移，转移状态设置成空
        for(auto alpha : processor.alphabet){
            if(!has_edge.count(alpha)){
                movemap[alpha]=vector<int>();
            }
        }

        // 对NFA转移表排序去重
        for(auto ch : processor.alphabet){
            vector<int> v = movemap[ch];
            sort(v.begin(),v.end());
            v.erase(unique(v.begin(),v.end()),v.end()); 
            movemap[ch] = v;
        }
        processor.nfa[processor.indexTable[ch]]=movemap;
    }
    // 对于终止状态，将其所有转移集合都设置成空集
    {
        map<char,vector<int> > final_movemap;
        for(auto ch: processor.alphabet){
            final_movemap[ch]=vector<int>();
        }
        processor.nfa[final]=final_movemap;
    }
    
    processor.subsetMethod();
    // 打印NFA表
    processor.printNFA();
    // dfa初始状态恒为0
    cout << "初始状态集合: {0}\n";
    cout << "终止状态集合: ";
    vector<int> final_states = processor.findFinalDFAStates();
    // 设置processor的dfa final集合
    processor.dfa_final_states = set<int>(final_states.begin(),final_states.end());
    printVector(final_states);
    cout << "\n";
    // 打印自动机的DFA表
    processor.printDFA();
}

// 预处理，将引用的头文件和注释去除
string preprocessing(string filename){
    string code;
    // 当前扫描到的行数
    int line_number = 1;
    ifstream inputfile(filename);
    if(inputfile.is_open()){
        while(inputfile.peek()!=EOF){
            code.push_back(inputfile.peek());
            inputfile.ignore();
        }
    }
    #ifdef DEBUG
    cout<<"程序文档"<<endl<<code<<endl;
    #endif
    code.push_back('$');
    string ret;
    int i=0;
    while(i<code.length()-1){
        // 将一行中的注释全部去除
        if(code[i]=='/' && code[i+1]=='/'){
            while(code[i]!='\n')i++;
        }else if(code[i]=='/' && code[i+1]=='*'){
            i+=2;
            bool check=false;
            while(!(code[i]=='*' && code[i+1]=='/')){
                if(code[i]=='$'){
                    cout<<"error: 第"<<line_number<<"行，错误的的代码注释"<<endl;
                    break;
                }
                i++;
            }
            if(code[i]=='*' && code[i+1]=='/'){
                i+=2;
            }
        }else{
            ret.push_back(code[i]);
            if(code[i]=='\n')line_number++;
            i++;
        }
    }

    inputfile.close();
    // 删除源程序中所有不考虑的字符，留下空格用来说明行数
    while(ret.find('\t') != string::npos)
        ret.erase(ret.find('\t'),1);

    return ret;
}

//测试str是否是非终结符
bool isKeywords(string str){
    return  (find(keywordsList.begin(),keywordsList.end(),str) != keywordsList.end());
}

// 打印令牌表
void printTokenList(){
    cout<<"token表："<<endl;
    // 将token信息输入token.txt文件中,并且打印到控制台
    ofstream output("token.txt");
    if(output.is_open()){
        string print;
        for(int i=0;i<tokenList.size();i++){
            token token = tokenList[i];
            print = "( " + to_string(token.line) + " , " + type2String[token.type] + " , '" + token.value + "' )";
            cout<<print<<endl;
            output<<print<<endl;
        }
    }
    output<<"$";
}

// 设置token类型到中文解释的映射表
void initial(){
    type2String.insert(make_pair(TokenType::constant,"常量"));
    type2String.insert(make_pair(TokenType::delimiter,"限定符"));
    type2String.insert(make_pair(TokenType::operand,"运算符"));
    type2String.insert(make_pair(TokenType::keyword,"关键字"));
    type2String.insert(make_pair(TokenType::identifier,"标识符"));
}

int main() {
    // 初始化程序中的某些变量
    initial();
    // 获取分类好的所有的产生式规则
    get_rule(SCIENTIFIC_FORMAT,scientifics);
    #ifdef TEST
    test_science_format();
    #endif
    get_rule(DELIMITER,delimiters);
    #ifdef TEST
    test_delimiter();
    #endif
    get_rule(IDENTIFIER,identifiers);
    #ifdef TEST
    testIdentifier();
    #endif
    get_rule(OPERATOR,operators);


    string code = preprocessing(code_file);
    #ifdef DEBUG
    cout<<code<<endl;
    #endif

    // 当前的行数
    int current_line = 1;
    // token表中的条目数量
    int token_size = 0;
    // 从字符串构架字符串输入刘
    istringstream is(code);
    // 扫描整个串，每次从中取出子串进行统计算
    bool error = false;
    string errormsg;
    // 扫描code文件
    while(is.peek() != EOF){
        if(error)break;
        if(is.peek() == '\n' && is.peek() != EOF){
            current_line++;
        }
        // 对字符串去除空格，对每个短串进行处理
        string sub;
        is>>sub;
        int position = 0,length = 1;
        while(position < sub.length()){
            // 对每个字符进行扫描
            char ch = sub[position + length - 1];
            // 查看能否接受第一个char，能够接受则继续转移，否则进行其他自动机的判断
            int state = transform(ch,identifiers,0);
            int last_state = -1;//无法转移时，对应的状态
            if(state != -1){
                while(state != -1){
                    // 转移到下一个状态
                    length ++ ;
                    ch = sub[position + length - 1];
                    last_state = state;
                    state = transform(ch,identifiers,state); 
                }
                // 判断当前识别出来的是否是关键字，不是关键字就是标识符
                string current_identified = sub.substr(position,length - 1);
                if(!identifiers.dfa_final_states.count(last_state)){
                    errormsg = "行号： "+to_string(current_line)+" 非法的标识符： "+current_identified;
                    error = true;
                    break;
                }
                if(isKeywords(current_identified)){
                    // cout<<"识别关键字:"<<current_identified<<endl;
                    tokenList.push_back(token(current_line,TokenType::keyword,current_identified));
                }else{
                    // cout<<"识别标识符："<<current_identified<<endl;
                    tokenList.push_back(token(current_line,TokenType::identifier,current_identified));
                }
                position = position + length - 1;//下一个开始识别的位置
                length = 1;
                continue;
            }
            // 无法识别为标识符
            else{
                // 查看是否能按照界符的规则进行转移
                state = transform(ch,delimiters,0);
                if(state != -1){
                    while(state != -1){
                        length ++ ;
                        ch = sub[position + length - 1];
                        last_state = state;
                        state = transform(ch,delimiters,state);
                    }
                    // 长度为length处的字符是失配字符，从position 开始长度为length-1的子串是识别出来的字符串
                    // cout<<"识别界符："<<sub.substr(position,length - 1)<<endl;
                    string current_identified = sub.substr(position,length - 1);
                    if(!delimiters.dfa_final_states.count(last_state)){
                        cout<<"行号： "<<current_line<<" 非法的界符："<<current_identified<<endl;
                        error = true;
                        break;
                    }
                    tokenList.push_back(token(current_line,TokenType::delimiter,current_identified));
                    position = position + length - 1;
                    length = 1;
                    continue;
                }
                // 无法识别为界符，继续识别是否是科学计数法
                else {
                    state = transform(ch,scientifics,0);
                    if(state != -1){
                        while(state != -1){
                            length ++ ;
                            ch = sub[position + length - 1];
                            last_state = state;
                            state = transform(ch,scientifics,state);
                        }
                        // cout<<"识别常量："<<sub.substr(position,length - 1)<<endl;
                        string current_identified = sub.substr(position,length - 1);
                        if(!scientifics.dfa_final_states.count(last_state)){
                            errormsg = "行号： "+to_string(current_line)+" 非法的常量： "+current_identified;
                            error = true;
                            break;
                        }
                        tokenList.push_back(token(current_line,TokenType::constant,current_identified));
                        position = position + length - 1;
                        length = 1;
                        continue;
                    }
                    else{
                        state = transform(ch,operators,0);
                        if(state != -1){
                            // state == 1时，从positon 开始的长度为length处的字符不属于该种类
                            while(state != -1){
                                length ++ ;
                                ch = sub[position + length - 1];
                                last_state = state;
                                state = transform(ch,operators,state);
                            }
                            string current_identified = sub.substr(position,length - 1);
                            if(!operators.dfa_final_states.count(last_state)){
                                errormsg = "行号： "+to_string(current_line)+" 非法的运算符： "+current_identified;
                                error = true;
                                break;
                            }
                            // cout<<"识别运算符："<<sub.substr(position,length - 1)<<endl;
                            tokenList.push_back(token(current_line,TokenType::operand,current_identified));
                            position = position + length - 1;
                            length = 1;
                            continue;
                        }else{
                            error = true;
                            errormsg = "无法识别的符号："+to_string(sub[position + length - 1]);
                            break;
                        }
                    }
                }
            }
        }
      
    }
    
    if(error)cout<<errormsg<<endl;
    else printTokenList();

    return 0;
}
