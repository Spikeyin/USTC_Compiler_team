#ifndef CODESIZEOPTIMIZER_H
#define CODESIZEOPTIMIZER_H

#include "Module.h"
#include "BasicBlock.h"
#include "Instruction.h"
#include "Value.h"
#include "User.h"
#include "Pass.h"
#include "Function.h"
#include "Type.h"
#include "Instruction.h"
#include "IRBuilder.h"
#include "Constant.h"
#include <string>
#include <vector>

namespace SysYF {
namespace IR {

class CodeSizeOptimizer : public Pass {
public:
    explicit CodeSizeOptimizer(WeakPtr<Module> m) : Pass(m) {}
    virtual ~CodeSizeOptimizer() = default;
    
    const std::string get_name() const override {
        return "CodeSizeOptimizer";
    }
    
    void execute() override;

private:
    bool is_loop_block(Ptr<BasicBlock> bb);
    void optimize_loop_block(Ptr<BasicBlock> bb);
    bool is_loop_invariant(Ptr<Instruction> instr);
    Ptr<BasicBlock> create_loop_preheader(Ptr<BasicBlock> loop_header, 
                                        const std::vector<Ptr<Instruction>>& invariant_instrs);
    void move_invariant_instructions(const std::vector<Ptr<Instruction>>& invariant_instrs,
                                   Ptr<BasicBlock> preheader);
    std::string get_expression_key(Ptr<BinaryInst> binop);
    std::string get_operand_string(Value* op);
    void replace_all_uses_with(Ptr<Value> old_val, Ptr<Value> new_val);
    void merge_similar_functions();
    bool are_functions_similar(Ptr<Function> f1, Ptr<Function> f2);
    float calculate_similarity(Ptr<Function> f1, Ptr<Function> f2);
    Ptr<Function> create_merged_function(Ptr<Function> f1, Ptr<Function> f2);
    void replace_function_calls(Ptr<Function> old_func, Ptr<Function> new_func);
    std::string get_merged_name(Ptr<Function> f1, Ptr<Function> f2);
};

}
}

#endif // CODESIZEOPTIMIZER_H