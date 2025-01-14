# B4 优化

## 公共子表达式删除

在这一部分，你需要实现一个公共子表达式删除算法。

- **任务4-1**

  你需要补充[ComSubExprEli.h](include/Optimize/ComSubExprEli.h)和[ComSubExprEli.cpp](src/Optimize/ComSubExprEli.cpp)，来实现一个完整的过程内全局公共子表达式删除算法。该算法基于SSA形式的控制流图（CFG）。`ComSubExprEli`中的`execute`函数会被调用来执行你实现的公共子表达式删除算法，你需要将你的实现流程体现在该函数中。在本关任务中，公共子表达式删除的对象**不包括**: void类指令, phi, alloca, load, call, cmp, fcmp指令。指令是否在考虑范围内可以通过`static bool ComSubExprEli::is_valid_expr`进行判断。

  ```cpp
  bool ComSubExprEli::is_valid_expr(Ptr<Instruction> inst) {
      return !(
          inst->is_void() // ret, br, store, void call
          || inst->is_phi()
          || inst->is_alloca()
          || inst->is_load()
          || inst->is_call()
          || inst->is_cmp()
          || inst->is_fcmp()
      );
  }
  ```

  **B4-1** 请在报告[doc/reports/B4.md](doc/reports/B4.md)中说明你实现的公共子表达式删除算法的设计思路。

  **B4-2** `void类指令, phi, alloca, load, call, cmp, fcmp指令`中是否存在一些指令可以在公共子表达式删除优化中分析？如果要分析，需要考虑哪些因素？

  > 参考材料: [UCSD CSE](https://cseweb.ucsd.edu/classes/sp02/cse231/lec8seq.pdf)

  在本地环境下，你可以在[test](test/)目录中使用`python test.py -O2`开启全部优化选项，测试优化的正确性。

## 检查Pass

由于对框架代码不熟悉、对优化算法理解不深或者在编码实现时的疏忽，我们很容易在实现了某个优化Pass之后在不经意间就对原本编译器中数据结构的一些约定进行了破坏（包括如LLVM IR中不同基本块之间的前驱后继关系，指令的操作数必须有定值才能使用，基本块必须以br、ret等指令结尾，def-use链等）。虽然可以通过打印LLVM IR并执行来确定某个Pass的正确性，并且这在很大程度上可以保证经过了该Pass优化生成的IR执行是正确的，但是这只能说明当前打印得到的IR是没有问题的，不能说明在编译器中仍然保留的IR Module是符合约定的。经过一个Pass之后的坏的IR Module可能会由于不满足约定而毒害下一个原本正确的优化遍导致出错。因此必要时我们可以在代码变换优化Pass之后添加一个**检查Pass**，来验证经过一个Pass变换之后的IR Module仍然满足一些良好的基本性质。

- **任务4-2**

  - **B4-3**. 基于你对LLVM IR Module的理解，在[`include/Optimize/Check.h`](include/Optimize/Check.h)和[`src/Optimize/Check.cpp`](src/Optimize/Check.cpp)中编写相应的约定验证代码，并在[`src/main.cpp`](src/main.cpp)或其他合适的位置插入相应的代码来利用你所写的检查器检查优化后的IR是否满足约定。将你检查的内容记录在报告[doc/reports/B4.md](doc/reports/B4.md)中，并说明检查理由。将检查器插入的代码位置记录在报告中，说明理由。

  > 检查Pass合理即可，主要是为优化Pass服务。

