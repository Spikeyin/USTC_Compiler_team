#include "CodeSizeOptimizer.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

namespace SysYF {
namespace IR {

void CodeSizeOptimizer::execute() {
    for (auto f : module.lock()->get_functions()) {
        if (f->get_basic_blocks().empty()) {
            continue;
        }
        
        for (auto bb : f->get_basic_blocks()) {
            if (is_loop_block(bb)) {
                optimize_loop_block(bb);
            }
        }
    }

    merge_similar_functions();
}

bool CodeSizeOptimizer::is_loop_block(Ptr<BasicBlock> bb) {
    for (auto instr : bb->get_instructions()) {
        if (auto br = std::dynamic_pointer_cast<BranchInst>(instr)) {
            for (auto op : br->get_operands()) {
                auto op_shared = op.lock();
                if (auto bb_op = std::dynamic_pointer_cast<BasicBlock>(op_shared)) {
                    if (bb_op == bb) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void CodeSizeOptimizer::optimize_loop_block(Ptr<BasicBlock> bb) {
    std::vector<Ptr<Instruction>> instructions(bb->get_instructions().begin(), 
                                             bb->get_instructions().end());
    std::vector<Ptr<Instruction>> invariant_instrs;
    std::unordered_map<std::string, Ptr<Instruction>> computed_exprs;
    
    for (auto instr : instructions) {
        if (auto binop = std::dynamic_pointer_cast<BinaryInst>(instr)) {
            if (is_loop_invariant(binop)) {
                std::string key = get_expression_key(binop);
                if (computed_exprs.find(key) == computed_exprs.end()) {
                    computed_exprs[key] = binop;
                    invariant_instrs.push_back(binop);
                } else {
                    replace_all_uses_with(binop, computed_exprs[key]);
                }
            }
        }
    }
    
    if (!invariant_instrs.empty()) {
        auto preheader = create_loop_preheader(bb, invariant_instrs);
        move_invariant_instructions(invariant_instrs, preheader);
    }
}

bool CodeSizeOptimizer::is_loop_invariant(Ptr<Instruction> instr) {
    for (auto op : instr->get_operands()) {
        auto op_shared = op.lock();
        if (auto inst_op = std::dynamic_pointer_cast<Instruction>(op_shared)) {
            if (inst_op->get_parent() == instr->get_parent()) {
                return false;
            }
        }
    }
    return true;
}

Ptr<BasicBlock> CodeSizeOptimizer::create_loop_preheader(Ptr<BasicBlock> loop_header, 
                                                        const std::vector<Ptr<Instruction>>& invariant_instrs) {
    auto func = loop_header->get_parent();
    auto preheader = BasicBlock::create(module.lock(), "loop_preheader", func);
    
    auto it = std::find(func->get_basic_blocks().begin(), 
                       func->get_basic_blocks().end(), 
                       loop_header);
    if (it != func->get_basic_blocks().end()) {
        func->get_basic_blocks().insert(it, preheader);
    }
    
    BranchInst::create_br(loop_header, preheader);
    
    return preheader;
}

void CodeSizeOptimizer::move_invariant_instructions(const std::vector<Ptr<Instruction>>& invariant_instrs,
                                                  Ptr<BasicBlock> preheader) {
    for (auto instr : invariant_instrs) {
        instr->get_parent()->delete_instr(instr);
        preheader->add_instruction(instr);
    }
}

std::string CodeSizeOptimizer::get_expression_key(Ptr<BinaryInst> binop) {
    std::string key = "";
    key += std::to_string(binop->get_instr_type()) + "_";
    key += get_operand_string(binop->get_operand(0).get()) + "_";
    key += get_operand_string(binop->get_operand(1).get());
    return key;
}

std::string CodeSizeOptimizer::get_operand_string(Value* op) {
    std::string result;
    if (auto constant_float = dynamic_cast<ConstantFloat*>(op)) {
        result = std::to_string(constant_float->get_value());
    } else if (auto constant_int = dynamic_cast<ConstantInt*>(op)) {
        result = std::to_string(constant_int->get_value());
    } else {
        result = op->get_name();
    }
    return result;
}

void CodeSizeOptimizer::replace_all_uses_with(Ptr<Value> old_val, Ptr<Value> new_val) {
    auto uses = old_val->get_use_list();
    for (auto &use : uses) {
        use.val_ = new_val;
    }
}

void CodeSizeOptimizer::merge_similar_functions() {
    auto m = module.lock();
    std::vector<Ptr<Function>> functions(m->get_functions().begin(), m->get_functions().end());
    
    for (size_t i = 0; i < functions.size(); i++) {
        for (size_t j = i + 1; j < functions.size(); j++) {
            if (are_functions_similar(functions[i], functions[j])) {
                auto merged_func = create_merged_function(functions[i], functions[j]);
                replace_function_calls(functions[i], merged_func);
                replace_function_calls(functions[j], merged_func);
                
                functions.erase(functions.begin() + j);
                functions.erase(functions.begin() + i);
                functions.push_back(merged_func);
                i--;
                break;
            }
        }
    }
}

bool CodeSizeOptimizer::are_functions_similar(Ptr<Function> f1, Ptr<Function> f2) {
    if (f1->get_return_type() != f2->get_return_type() ||
        f1->get_args().size() != f2->get_args().size()) {
        return false;
    }
    
    float similarity = calculate_similarity(f1, f2);
    return similarity > 0.8f;
}

float CodeSizeOptimizer::calculate_similarity(Ptr<Function> f1, Ptr<Function> f2) {
    int matching_instructions = 0;
    int total_instructions = 0;
    
    auto bb1_list = f1->get_basic_blocks();
    auto bb2_list = f2->get_basic_blocks();
    
    for (auto &bb1 : bb1_list) {
        for (auto &instr1 : bb1->get_instructions()) {
            total_instructions++;
            
            for (auto &bb2 : bb2_list) {
                for (auto &instr2 : bb2->get_instructions()) {
                    if (instr1->get_instr_type() == instr2->get_instr_type()) {
                        matching_instructions++;
                        break;
                    }
                }
            }
        }
    }
    
    return static_cast<float>(matching_instructions) / total_instructions;
}

Ptr<Function> CodeSizeOptimizer::create_merged_function(Ptr<Function> f1, Ptr<Function> f2) {
    auto m = module.lock();
    std::string merged_name = get_merged_name(f1, f2);
    
    PtrVec<Type> param_types;
    for (auto &arg : f1->get_args()) {
        param_types.push_back(arg->get_type());
    }
    param_types.push_back(Type::get_int32_type(m));
    
    auto func_type = FunctionType::create(f1->get_return_type(), param_types, m);
    auto merged_func = Function::create(func_type, merged_name, m);
    
    auto entry_block = BasicBlock::create(m, "entry", merged_func);
    auto f1_block = BasicBlock::create(m, "f1_body", merged_func);
    auto f2_block = BasicBlock::create(m, "f2_body", merged_func);
    
    for (auto &bb : f1->get_basic_blocks()) {
        for (auto &instr : bb->get_instructions()) {
            f1_block->add_instruction(instr);
        }
    }
    
    for (auto &bb : f2->get_basic_blocks()) {
        for (auto &instr : bb->get_instructions()) {
            f2_block->add_instruction(instr);
        }
    }
    
    auto zero = ConstantInt::create(0, m);
    auto discriminator = merged_func->get_args().back();
    
    auto cond = CmpInst::create_cmp(CmpInst::EQ, discriminator, zero, entry_block, m);
    BranchInst::create_cond_br(cond, f1_block, f2_block, entry_block);
    
    return merged_func;
}

std::string CodeSizeOptimizer::get_merged_name(Ptr<Function> f1, Ptr<Function> f2) {
    return "merged_" + f1->get_name() + "_" + f2->get_name();
}

void CodeSizeOptimizer::replace_function_calls(Ptr<Function> old_func, Ptr<Function> new_func) {
    auto m = module.lock();
    
    for (auto &f : m->get_functions()) {
        for (auto &bb : f->get_basic_blocks()) {
            for (auto &instr : bb->get_instructions()) {
                if (auto call = std::dynamic_pointer_cast<CallInst>(instr)) {
                    if (call->get_function() == old_func) {
                        PtrVec<Value> args;
                        for (auto &op : call->get_operands()) {
                            args.push_back(op.lock());
                        }
                        
                        auto discriminator = ConstantInt::create(old_func == new_func ? 0 : 1, m);
                        args.push_back(discriminator);
                        
                        auto new_call = CallInst::create(new_func, args, bb);
                        replace_all_uses_with(call, new_call);
                        bb->delete_instr(call);
                    }
                }
            }
        }
    }
}

}
}