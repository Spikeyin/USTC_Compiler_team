add_library(
        SysYFPass STATIC
        Pass.cpp
        ComSubExprEli.cpp
        DominateTree.cpp
        RDominateTree.cpp
        Mem2Reg.cpp
        LiveVar.cpp
        Check.cpp
        CodeSizeOptimizer.cpp
)
