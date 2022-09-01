# 文件的目录结构如下所示，包括词法分析、LR(1)分析、基于表达式的语义分析  
博客链接：<a href="https://www.cnblogs.com/randy-lo/p/14773609.html" target="_blank">详解博客</a>  
``` 
.
├── README.md
├── dir.txt
├── grammer
│   ├── error_file.txt
│   ├── grammer-rule.txt
│   ├── grammer-rule2.txt
│   ├── grammerAnalysis
│   ├── grammerAnalysis.cpp
│   ├── grammerAnalysis.h
│   ├── test-rule.txt
│   ├── test-rule2.txt
│   ├── test-rule3.txt
│   ├── test-rule4.txt
│   ├── test-rule5.txt
│   └── test-rule6.txt
├── lexical
│   ├── code.txt
│   ├── code2.txt
│   ├── code3.txt
│   ├── lexical-rule.txt
│   ├── lexical-rule2.txt
│   ├── lexical-rule3.txt
│   ├── lexicalAnalysis
│   ├── lexicalAnalysis.cpp
│   ├── lexicalAnalysis.h
│   ├── problem3_code.txt
│   ├── test.cpp
│   └── token.txt
├── semantic
│   ├── error_file.txt
│   ├── rule.txt
│   ├── semanticAnalysis
│   ├── semanticAnalysis.cpp
│   ├── semanticAnalysis.h
│   ├── test1
│   └── test1.cpp
├── 使用实例.txt
├── 代码说明文档.docx
└── 课程设计报告.docx

3 directories, 36 files

```

# 解析语言格式（类似Python + Swift的结合 详细见产生式表）：
``` Swift
func main(args:Void,args2:Double, args3 : String) -> Void{
    function((1+342.321),current);
    let number: Int64 = 312321.3123123E10+31321;
    function(.1,31231,321);
    function2();
    let ans:Int32 = (43242+32423);/*这是我的一条测试注释*/
    var a:Int32 = 10;
    var a:String;
    {
        let _fasd:String = 100;
        let a : Int32=10;//这是我的的另一条注释
        while a<10 {
            i=(10+10);
            a=(1312E10);
        }
        while( b<100 )i=(i+10);
    }
        {
            4232;
            if a<b{
                a = 1;
            }
        }
    return ;
}
func test()->String{}
```
# 表达式的语义分析 -- 解析并计算如下形式的表达式（含错误提示）
``` C++
78+(423*423)-423
```
