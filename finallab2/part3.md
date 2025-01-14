# B3 活跃变量分析

在第三部分，你需要完成活跃变量分析。

- **任务3-1**

  你需要补充[`include/Optimize/LiveVar.h`](include/Optimize/LiveVar.h)和[`src/Optimize/LiveVar.cpp`](src/Optimize/LiveVar.cpp)中的`LiveVar`类，来实现一个完整的活跃变量分析算法，分析所有`BasicBlock`块的入口和出口的活跃变量，该算法基于**SSA**形式的CFG。  
  在本次实验的框架中,`BasicBlock`类实现了4个和活跃变量分析相关的函数，你需要调用它们，设置所有`BasicBlock`的入口与出口处的活跃变量，以完成实验：  

  ```cpp
  // 设置该BasicBlock入口处的活跃变量集合。若活跃变量集合发生改变，返回true, 否则返回false。
  bool set_live_in(PtrSet<Value> in)
  // 设置该BasicBlock出口处的活跃变量集合。若活跃变量集合发生改变，返回true, 否则返回false。
  bool set_live_out(PtrSet<Value> out)
  // 获取该BasicBlock入口处的活跃变量集合
  PtrSet<Value>& get_live_in()
  // 获取该BasicBlock出口处的活跃变量集合
  PtrSet<Value>& get_live_out()
  ```

  `LiveVar`类中的`execute`函数会被调用来执行你实现的活跃变量分析算法，你需要将你的实现流程体现在该函数中。

  测试会将你的分析得到的每个基本块的`IN`和`OUT`和正确答案匹配。在本地环境下，你可以在[test](test/)目录中使用`python test_live_var.py`来测试活跃变量分析算法的正确性。脚本生成的`live_var.out`和`test.ll`文件是临时文件，你可以忽略它。

- **任务3-2**

  在实验报告[doc/reports/B3.md](doc/reports/B3.md)中回答下列问题

  - **B3-1**. 请说明你实现的活跃变量分析算法的设计思路。

- **Tips**

  和上课时所讲的活跃变量分析不同，本次实验的活跃变量分析需要考虑phi指令，而数据流方程：$\rm OUT[B] =\cup_{S是B的后继}IN[S]$ 的定义蕴含着基本块S入口处活跃的变量在基本块S所有前驱的出口处都是活跃的。  
  由于`phi`指令的特殊性(`phi`指令文档见[doc/SysYFIR.md Phi小节](doc/SysYFIR.md#phi))，例如`%0 = phi [%op1, %bb1], [%op2, %bb2]`如果使用如上数据流方程，则默认此`phi`指令同时产生了`op1`与`op2`的活跃性，而事实上只有控制流从`%bb1`传过来才有`%op1`的活跃性，从`%bb2`传过来才有`%op2`的活跃性。因此你需要对此数据流方程做一些修改。

  参考[--dot-cfg选项](https://www.llvm.org/docs/Passes.html#dot-cfg-print-cfg-of-function-to-dot-file)，你可以使用`opt --dot-cfg <xxx.ll>`生成包含控制流图CFG的`.dot`文件(`opt`一般会和`clang`一起安装)，然后使用`dot -Tpng -o <xxx.png> <xxx.dot>`生成特定函数的CFG对应的png图片。`dot`工具可以通过`sudo apt-get install graphviz`安装。

