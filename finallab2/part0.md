本项目提供SysYF语言的中间表示(IR)优化的编译器实践框架。SysYF语言是全国编译系统设计赛要求实现的SysY语言的扩展语言，SysYF IR兼容LLVM IR。该项目由中国科学技术大学张昱老师研究组设计和开发，旨在引导更多学生和社会人员学习和实践编译器相关知识，开展语言设计与实现、程序分析、编译优化等研发，让越来越多的人有能力运用程序语言基础提供可靠、高效的软件解决方案。

# 评测说明(必看)

请在[id.txt](./id.txt)中添加组长学号，如PB00000000，否则无法记录成绩。

# 实验说明

本次实验为合作实验，任务自由分配，请在[doc/reports/contribution.md](doc/reports/contribution.md)中填写贡献详述和贡献比。**请在头歌平台的组长账号中提交测评**。

## 必做关卡

必做部分共有四关（[支配树](part1.md)、[Mem2Reg](part2.md)、[活跃变量分析](part3.md)、[优化](part4.md)）。

> 每一关的文档除了在头歌平台关卡中展示外，也在代码仓库中存在，命名为partN.md。文档中的仓库内相对链接在头歌平台关卡界面中失效。

## 编译命令的flag说明

本实验中新增了`-O2`命令行参数，使用`-O2`参数将开启全部优化（默认不开启）。例如：

```shell
./compiler -emit-ir -O2 test.sy -o test.ll
```

同时保留了`-emit-ast`参数用于从AST复原代码，`-check`参数用于静态检查。

同时新增了 `-O` 命令行参数，用于开始 Mem2Reg pass 和 DominateTree pass 。

若要开启单项优化，`-lv`代表开启活跃变量分析，`-cse`代表开启公共子表达式删除，开启这些优化的同时也会开启 Mem2Reg pass 和 DominateTree pass。例如，若你想开启活跃变量分析，就需要使用如下命令：

```shell
./compiler -emit-ir -lv test.sy -o test.ll
```

**注意：**以下两条命令产生的IR是不同的:

```shell
./compiler -emit-ir -O test.sy -o test.ll
```

```shell
./compiler -emit-ir test.sy -o test.ll
```

原因在于`-O`选项会开启Mem2Reg pass，可以消除部分栈分配指令`alloca`。

有关flag详情，可以参见`src/main.cpp`。

## 分析与优化Pass说明

- Pass

  `Pass`类是所有分析与优化Pass的基类（比如Mem2Reg、DominateTree、LiveVar等），定义在[`include/Optimize/Pass.h`](include/Optimize/Pass.h)中。该基类中有两个纯虚函数需要子类重写：一个是`virtual void execute() = 0`，该函数是pass的总控函数，该函数会被调用去执行该pass；另一个是`virtual const std::string get_name() const = 0`，该函数用于获得该pass的名字。

  该基类中有一个成员变量`Ptr<Module> module`，可以被子类访问。这里`module`表示一个编译单元，是一个源程序文件对应的中间表示。

  本实验中，你所实现的分析和优化pass均需继承自该类。你需要重写上述的`execute`函数和`get_name`函数。可以参考给出的[`src/Optimize/Mem2Reg.cpp`](src/Optimize/Mem2Reg.cpp)和[`src/Optimize/DominateTree.cpp`](src/Optimize/DominateTree.cpp)是如何继承Pass类并实现对应功能的。

  > 这里的`Pass`对应于LLVM中的[`ModulePass`](https://llvm.org/docs/WritingAnLLVMPass.html#the-modulepass-class)。LLVM还有其他类型的`Pass`，如`FunctionPass`, `LoopPass`等，针对不同规模的IR进行分析和优化，感兴趣的同学可以进一步查看[Writing an LLVM Pass (legacy PM version)](https://llvm.org/docs/WritingAnLLVMPass.html)。

- PassMgr

  `PassMgr`类负责管理Pass。定义在[`include/Optimize/Pass.h`](include/Optimize/Pass.h)中。该类有两个私有成员变量:`module`和`pass_list`。`module`含义同上,`pass_list`为当前添加的所有pass的list。该类有两个公有的函数：`addPass`和`execute`。`addPass`函数用于向`pass_list`中添加pass，每次添加采用`push_back`的方式。`execute`函数用于顺序调用`pass_list`中pass的`execute`函数(即上述需要重写的`execute`函数)。`PassMgr`的使用实例可参见[`src/main.cpp`](src/main.cpp)。

  本实验中你无需修改`Pass`类和`PassMgr`类。如果有修改，请在报告中说明。

- Mem2Reg

  `Mem2Reg`用于将IR转换成为SSA形式的IR。以LLVM IR为例，在生成IR时，局部变量被生成为alloca/load/store的形式。用 alloca 指令来“声明”变量，得到一个指向该变量的指针，用 store 指令来把值存在变量里，用 load 指令来把值读出。LLVM 在 mem2reg 这个 pass 中，会识别出上述这种模式的 alloca，把它提升为 SSA value(register)，在提升为 SSA value时会对应地消除 store 与 load，修改为 SSA 的 def-use/use-def 关系，并且在适当的位置安插 Phi 和 进行变量重命名。本次实验中，助教给出了Mem2Reg的一种实现(见[`src/Optimize/Mem2Reg.cpp`](src/Optimize/Mem2Reg.cpp))，在开启优化时会开启Mem2Reg，将IR转换为SSA形式的IR。因此本实验中的所有优化均基于SSA形式的IR。

  > 参考链接: [LLVM对mem2reg的说明](https://llvm.org/docs/Passes.html#mem2reg-promote-memory-to-register), [LLVM的mem2reg实现](https://llvm.org/doxygen/Mem2Reg_8cpp_source.html)，[静态单赋值格式构造](./doc/静态单赋值格式构造.pdf)

## 类型与API说明

为了防止`std::shared_ptr`循环引用导致的内存泄漏，助教提供了[`WeakPtr`](./include/internal_types.h#L35)，并定义了相关的容器类型，包括[`WeakPtrSet`](include/internal_types.h#L41), [`WeakPtrList`](include/internal_types.h#L57), [`WeakPtrMap`](include/internal_types.h#L90)和[`WeakPtrVec`](include/internal_types.h#L93)。

对于`WeakPtrSet`，提供了包括[`setUnion`](include/internal_types.h#L63), [`setDiff`](include/internal_types.h#L73), [`setIntersect`](include/internal_types.h#L83)等操作。

> 这是为了防止内存泄漏做的妥协。另外一种方法是侵入式智能指针(你可以参考后面这些链接了解[博客](https://zhiqiang.org/coding/boost-intrusive-ptr.html), [Boost intrusive_ptr: faster shared pointer](https://baptiste-wicht.com/posts/2011/11/boost-intrusive_ptr.html)和[Intrusive Pointers](https://jenniferchukwu.com/posts/intrusiveptr))，即让`Instruction`等类继承`intrusive_ptr_base`，通过`intrusive_ptr_base`实现引用计数管理。然而这需要添加`Instruction`的拷贝构造函数等内容。同时仅引入`intrusive_ptr_base`也无法解决循环引用的问题。总结来说在C++中没有好的办法用很少的代价解决内存泄漏问题。

## 使用Log方便调试

该部分文档见[logging.md](doc/logging.md)。



