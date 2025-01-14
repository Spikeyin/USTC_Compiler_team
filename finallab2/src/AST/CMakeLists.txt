add_library(
    ASTLib STATIC
    SyntaxTree.cpp
)

add_library(
    ASTPrinter
    SyntaxTreePrinter.cpp
)

add_library(
    Checker STATIC
    SyntaxTreeChecker.cpp
)