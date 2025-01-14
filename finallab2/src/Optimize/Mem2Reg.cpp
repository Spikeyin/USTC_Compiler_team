#include "Mem2Reg.h"
#include "IRBuilder.h"
#include <cstddef>

namespace SysYF {
namespace IR {

void Mem2Reg::execute(){
    for(auto fun: module.lock()->get_functions()){
        if(fun->get_basic_blocks().empty())continue;
        func_ = fun;
        lvalue_connection.clear();
        no_union_set.clear();
        insideBlockForwarding();
        genPhi();
        module.lock()->set_print_name();
        valueDefineCounting();
        valueForwarding(func_.lock()->get_entry_block());
        removeAlloc();
    }
}

void Mem2Reg::insideBlockForwarding(){
    for(auto bb: func_.lock()->get_basic_blocks()){
        Map<Ptr<Value>, Ptr<Instruction>> defined_list;
        Map<Ptr<Instruction>, Ptr<Value>> forward_list;
        Map<Ptr<Value>, Ptr<Value>> new_value;
        PtrSet<Instruction> delete_list;
        for(auto inst: bb->get_instructions()){
            if(!isLocalVarOp(inst))continue;
            if(inst->get_instr_type() == Instruction::OpID::store){
                Ptr<Value> lvalue = static_pointer_cast<StoreInst>(inst)->get_lval();
                Ptr<Value> rvalue = static_pointer_cast<StoreInst>(inst)->get_rval();
                auto load_inst = dynamic_pointer_cast<Instruction>(rvalue);
                if(load_inst && forward_list.find(load_inst) != forward_list.end()){
                    rvalue = forward_list.find(load_inst)->second;
                }
                if(defined_list.find(lvalue) != defined_list.end()){
                    auto pair = defined_list.find(lvalue);
                    delete_list.insert(pair->second);
                    pair->second = inst;
                }
                else{
                    defined_list.insert({lvalue, inst});
                }
                if(new_value.find(lvalue) != new_value.end()){
                    new_value.find(lvalue)->second = rvalue;
                }
                else{
                    new_value.insert({lvalue, rvalue});
                }
            }
            else if(inst->get_instr_type() == Instruction::OpID::load){
                Ptr<Value> lvalue = static_pointer_cast<LoadInst>(inst)->get_lval();
                if(defined_list.find(lvalue) == defined_list.end())continue;
                Ptr<Value> value = new_value.find(lvalue)->second;
                forward_list.insert({inst, value});
            }
        }

        for(auto submap: forward_list){
            auto inst = submap.first; 
            auto value = submap.second;
            for(auto use: inst->get_use_list()){
                auto use_inst = dynamic_pointer_cast<Instruction>(use.val_.lock());
                use_inst->set_operand(use.arg_no_, value);
            }
            bb->delete_instr(inst);
        } 
        for(auto inst:delete_list){
            bb->delete_instr(inst);
        }       
    }
}

void Mem2Reg::genPhi(){
    PtrSet<Value> globals;
    Map<Ptr<Value>, PtrSet<BasicBlock>> defined_in_block;
    for(auto bb: func_.lock()->get_basic_blocks()){
        for(auto inst: bb->get_instructions()){
            if(!isLocalVarOp(inst))continue;
            if(inst->get_instr_type() == Instruction::OpID::load){
                Ptr<Value> lvalue = static_pointer_cast<LoadInst>(inst)->get_lval();
                globals.insert(lvalue);
            }
            else if(inst->get_instr_type() == Instruction::OpID::store){
                Ptr<Value> lvalue = static_pointer_cast<StoreInst>(inst)->get_lval();
                if(defined_in_block.find(lvalue) != defined_in_block.end()){
                    defined_in_block.find(lvalue)->second.insert(bb);
                }
                else{
                    defined_in_block.insert({lvalue, {bb}});
                }
            }
        }
    }

    Map<Ptr<BasicBlock>, PtrSet<Value>> bb_phi_list;

    for(auto var: globals){
        auto define_bbs = defined_in_block.find(var)->second;
        PtrVec<BasicBlock> queue;
        queue.assign(define_bbs.begin(), define_bbs.end());
        size_t iter_pointer = 0;
        for(; iter_pointer < queue.size(); iter_pointer++){
            for(auto bb_domfront: queue[iter_pointer]->get_dom_frontier()){
                if(bb_phi_list.find(bb_domfront.lock()) != bb_phi_list.end()){
                    auto phis = bb_phi_list.find(bb_domfront.lock());
                    if(phis->second.find(var) == phis->second.end()){
                        phis->second.insert(var);
                        auto newphi = PhiInst::create_phi(var->get_type()->get_pointer_element_type(), 
                            bb_domfront.lock());
                        newphi->set_lval(var);
                        bb_domfront.lock()->add_instr_begin(newphi);
                        queue.push_back(bb_domfront.lock());
                    }
                }
                else{
                    auto newphi = PhiInst::create_phi(var->get_type()->get_pointer_element_type(), 
                            bb_domfront.lock());
                    newphi->set_lval(var);
                    bb_domfront.lock()->add_instr_begin(newphi);                  
                    queue.push_back(bb_domfront.lock());
                    bb_phi_list.insert({bb_domfront.lock(), {var}});
                }
            }
        }
    }
}

void Mem2Reg::valueDefineCounting(){
    define_var = {};
    for(auto bb: func_.lock()->get_basic_blocks()){
        define_var.insert({bb, {}});
        for(auto inst: bb->get_instructions()){
            if(inst->get_instr_type() == Instruction::OpID::phi){
                auto lvalue = dynamic_pointer_cast<PhiInst>(inst)->get_lval();
                define_var.find(bb)->second.push_back(lvalue);
            }
            else if(inst->get_instr_type() == Instruction::OpID::store){
                if(!isLocalVarOp(inst))continue;
                auto lvalue = dynamic_pointer_cast<StoreInst>(inst)->get_lval();
                define_var.find(bb)->second.push_back(lvalue);
            }
        }
    }
}

Map<Ptr<Value>, PtrVec<Value>> value_status;
PtrSet<BasicBlock> visited;

void Mem2Reg::valueForwarding(Ptr<BasicBlock> bb){
    PtrSet<Instruction> delete_list;
    visited.insert(bb);
    for(auto inst: bb->get_instructions()){
        if(inst->get_instr_type() != Instruction::OpID::phi)break;
        auto lvalue = dynamic_pointer_cast<PhiInst>(inst)->get_lval();
        auto value_list = value_status.find(lvalue);
        if(value_list != value_status.end()){
            value_list->second.push_back(inst);
        }
        else{
            value_status.insert({lvalue, {inst}});
        }
    }

    for(auto inst: bb->get_instructions()){
        if(inst->get_instr_type() == Instruction::OpID::phi)continue;
        if(!isLocalVarOp(inst))continue;
        if(inst->get_instr_type() == Instruction::OpID::load){
            auto lvalue = static_pointer_cast<LoadInst>(inst)->get_lval();
            auto new_value = *(value_status.find(lvalue)->second.end() - 1);
            inst->replace_all_use_with(new_value);
        }
        else if(inst->get_instr_type() == Instruction::OpID::store){
            auto lvalue = static_pointer_cast<StoreInst>(inst)->get_lval();
            auto rvalue = static_pointer_cast<StoreInst>(inst)->get_rval();
            if(value_status.find(lvalue) != value_status.end()){
                value_status.find(lvalue)->second.push_back(rvalue);
            }
            else{
                value_status.insert({lvalue, {rvalue}});
            }
        }
        delete_list.insert(inst);
    }

    for(auto succbb: bb->get_succ_basic_blocks()){
        for(auto inst: succbb.lock()->get_instructions()){
            if(inst->get_instr_type() == Instruction::OpID::phi){
                auto phi = dynamic_pointer_cast<PhiInst>(inst);
                auto lvalue = phi->get_lval();
                if(value_status.find(lvalue) != value_status.end()){
                    if(value_status.find(lvalue)->second.size() > 0){
                        auto new_value = *(value_status.find(lvalue)->second.end() - 1);
                        phi->add_phi_pair_operand(new_value, bb);
                    }
                    else{
                        //std::cout << "undefined value used: " << lvalue->get_name() << "\n";
                        // exit(-1);
                    }
                }
                else{
                    //std::cout << "undefined value used: " << lvalue->get_name() << "\n";
                    // exit(-1);
                }
            }
        }
    }

    for(auto succbb: bb->get_succ_basic_blocks()){
        if(visited.find(succbb.lock())!=visited.end())continue;
        valueForwarding(succbb.lock());
    }

    // for(auto inst: bb->get_instructions()){
        auto var_set = define_var.find(bb)->second;
        for(auto var: var_set){
            if(value_status.find(var.lock()) == value_status.end())continue;
            if(value_status.find(var.lock())->second.size() == 0)continue;
            value_status.find(var.lock())->second.pop_back();
        }
    // }

    for(auto inst: delete_list){
        bb->delete_instr(inst);
    }
} 

void Mem2Reg::removeAlloc(){
    for(auto bb: func_.lock()->get_basic_blocks()){
        PtrSet<Instruction> delete_list;
        for(auto inst: bb->get_instructions()){
            if(inst->get_instr_type() != Instruction::OpID::alloca)continue;
            auto alloc_inst = dynamic_pointer_cast<AllocaInst>(inst);
            if(alloc_inst->get_alloca_type()->is_integer_type() || alloc_inst->get_alloca_type()->is_float_type() ||
               alloc_inst->get_alloca_type()->is_pointer_type())delete_list.insert(inst);
        }
        for(auto inst: delete_list){
            bb->delete_instr(inst);
        }
    }
}


}
}
