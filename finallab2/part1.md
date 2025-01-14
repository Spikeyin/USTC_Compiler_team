# B1 支配树和反向支配树

在教材 9.6.1 节中，我们学习了支配关系的概念。

为了更高效地计算支配关系，我们可以构建支配树。支配树上的每个结点的父结点都是直接支配该结点的结点，即在所有支配它的结点中距离它最近的那个（它本身除外）。

支配树主要用于计算支配边界，从而在合适的地方插入 Phi 指令，得到 SSA（详见 [`src/Optimize/Mem2Reg.cpp`](src/Optimize/Mem2Reg.cpp)）。

在死代码删除优化遍中，可能会用到反向支配树，即从流图的 EXIT 结点出发，逆着控制流的方向生成的支配树。

- **任务1-1 构建支配树的算法**

  为构建支配树，采用一种基于数据流迭代计算的方法，该方法类似教材算法 9.6。

  请仔细阅读 [`doc/dom.pdf`](doc/dom.pdf)，回答以下问题：

  - **B1-1**. 证明：若 $x$ 和 $y$ 支配 $b$，则要么 $x$ 支配 $y$，要么 $y$ 支配 $x$。 

  - **B1-2**. 在 [`doc/dom.pdf`](doc/dom.pdf) 中，`Figure 1: The Iterative Dominator Algorithm` 是用于计算支配关系的迭代算法，该算法的内层 `for` 循环是否一定要以后序遍历的逆序进行，为什么？

  - **B1-3**. `Figure 3: The Engineered Algorithm` 为计算支配树的算法。在其上半部分的迭代计算中，内层的 `for` 循环是否一定要以后序遍历的逆序进行，为什么？

  - **B1-4**. 在 [`doc/dom.pdf`](doc/dom.pdf) 中，`Figure 3: The Engineered Algorithm` 为计算支配树的算法。其中下半部分 `intersect` 的作用是什么？内层的两个 `while` 循环中的小于号能否改成大于号？为什么？

  - **B1-5**. 这种通过构建支配树从而得到支配关系的算法相比教材中算法 9.6 在时间和空间上的优点是什么？

- **任务1-2 理解支配树和反向支配树的实现**

  [`src/Optimize/DominateTree.cpp`](src/Optimize/DominateTree.cpp) 和 [`src/Optimize/RDominateTree.cpp`](src/Optimize/RDominateTree.cpp) 是本实验框架提供的支配树和反向支配树的一种实现，请阅读代码并回答以下问题：

  - **B1-6**. 在反向支配树的构建过程中，是怎么确定 EXIT 结点的？为什么不能以流图的最后一个基本块作为 EXIT 结点？

请将问题答案写在[doc/reports/B1.md](doc/reports/B1.md)中。

