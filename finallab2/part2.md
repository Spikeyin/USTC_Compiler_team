# B2 Mem2Reg

在第二部分，你需要理解Mem2Reg优化遍的具体含义，结合例子说明Mem2Reg是如何对LLVM IR代码进行变换的。

- **任务2-1**

  你需要仔细阅读[`include/Optimize/Mem2Reg.h`](include/Optimize/Mem2Reg.h)和[`src/Optimize/Mem2Reg.cpp`](src/Optimize/Mem2Reg.cpp)，参考[静态单赋值构造](doc/%E9%9D%99%E6%80%81%E5%8D%95%E8%B5%8B%E5%80%BC%E6%A0%BC%E5%BC%8F%E6%9E%84%E9%80%A0.pdf)中的算法，将`Mem2Reg`优化遍的***流程***记录在报告中，并结合例子说明在`Mem2Reg`的***哪些阶段***分别***存储了什么***，***对IR做了什么处理***。
  - 为了体现`Mem2Reg`的效果，例子需要包含至少两层由条件分支、循环组成的嵌套结构，并且对同一个变量有在不同分支或者迭代中的定值和引用。

  两层嵌套结构例如

  ```
  while {
    if {}
    else {}
  }
  ```

  > `Mem2Reg`可能会产生`phi`指令，关于`phi`指令的含义可以参考[SysYFIR.md Phi小节](doc/SysYFIR.md#phi)的说明。

  - **B2-1**. 请说明`Mem2Reg`优化遍的流程。
  - **B2-2**. 结合例子说明在`Mem2Reg`的***哪些阶段***分别***存储了什么***，***对IR做了什么处理***。需要说明
    - `Mem2Reg`可能会删除的指令类型是哪些？对哪些分配(alloca)指令会有影响？
    - 在基本块内前进`insideBlockForwarding`时，对`store`指令处理时为什么`rvalue`在`forward_list`中存在时，就需要将`rvalue`替换成`forward_list`映射中的`->second`值？
    - 在基本块内前进时，`defined_list`代表什么含义？
    - 生成phi指令`genPhi`的第一步两层for循环在收集什么信息，这些信息在后面的循环中如何被利用生成Phi指令？
    - `valueDefineCounting`为`defined_var`记录了什么信息
    - `valueForwarding`在遍历基本块时采用的什么方式
    - `valueForwarding`中为什么`value_status`需要对phi指令做信息收集
    - `valueForwarding`中第二个循环对`load`指令的替换是什么含义
    - `valueForwarding`中出现的`defined_var`和`value_status`插入条目之间有什么联系
  - **B2-3**. 请说明上述例子的要求为什么能体现`Mem2Reg`的效果？

请将问题答案写在[doc/reports/B2.md](doc/reports/B2.md)中。
