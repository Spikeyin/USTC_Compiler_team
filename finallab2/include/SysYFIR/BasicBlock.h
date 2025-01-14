#ifndef _SYSYF_BASICBLOCK_H_
#define _SYSYF_BASICBLOCK_H_

#include "Value.h"
#include "Instruction.h"
#include "internal_macros.h"
#include "internal_types.h"

#include <algorithm>
#include <list>
#include <memory>
#include <set>
#include <string>

namespace SysYF
{
namespace IR
{
class Function;
class Instruction;
class Module;

class BasicBlock : public Value
{
public:
    static Ptr<BasicBlock> create(Ptr<Module> m, const std::string &name ,
                            Ptr<Function> parent ) {
        auto prefix = name.empty() ? "" : "label_";
        RET_AFTER_INIT(BasicBlock, m, prefix + name, parent);
    }

    // return parent, or null if none.
    Ptr<Function> get_parent() { return parent_.lock(); }
    
    Ptr<Module> get_module();

    /****************api about cfg****************/

    auto& get_pre_basic_blocks() { return pre_bbs_; }
    auto& get_succ_basic_blocks() { return succ_bbs_; }
    void add_pre_basic_block(Ptr<BasicBlock> bb) { pre_bbs_.push_back(bb); }
    void add_succ_basic_block(Ptr<BasicBlock> bb) { succ_bbs_.push_back(bb); }

    void remove_pre_basic_block(Ptr<BasicBlock> bb) {
        auto match = [bb](const WeakPtr<BasicBlock> &ptr) { return ptr.lock() == bb; };
        pre_bbs_.remove_if(match);
    }
    void remove_succ_basic_block(Ptr<BasicBlock> bb) {
        auto match = [bb](const WeakPtr<BasicBlock> &ptr) { return ptr.lock() == bb; };
        succ_bbs_.remove_if(match);
    }

    /****************api about cfg****************/

    /// Returns the terminator instruction if the block is well formed or null
    /// if the block is not well formed.
    const Ptr<Instruction> get_terminator() const;
    Ptr<Instruction> get_terminator() {
        return const_pointer_cast<Instruction>(
            static_pointer_cast<const BasicBlock>(shared_from_this())->get_terminator());
    }
    
    void add_instruction(Ptr<Instruction> instr);
    void add_instruction(PtrList<Instruction>::iterator instr_pos, Ptr<Instruction> instr);
    void add_instr_begin(Ptr<Instruction> instr);

    PtrList<Instruction>::iterator find_instruction(Ptr<Instruction> instr);

    void delete_instr(Ptr<Instruction> instr);

    bool empty() { return instr_list_.empty(); }

    int get_num_of_instr() { return instr_list_.size(); }
    PtrList<Instruction> &get_instructions() { return instr_list_; }
    
    void erase_from_parent();
    
    virtual std::string print() override;

    /****************api about dominate tree****************/
    void set_idom(Ptr<BasicBlock> bb){idom_ = bb;}
    auto get_idom(){return idom_;}
    void add_dom_frontier(Ptr<BasicBlock> bb){dom_frontier_.insert(bb);}
    void add_rdom_frontier(Ptr<BasicBlock> bb){rdom_frontier_.insert(bb);}
    void clear_rdom_frontier(){rdom_frontier_.clear();}
    auto add_rdom(Ptr<BasicBlock> bb){return rdoms_.insert(bb);}
    void clear_rdom(){rdoms_.clear();}
    auto& get_dom_frontier(){return dom_frontier_;}
    auto& get_rdom_frontier(){return rdom_frontier_;}
    auto& get_rdoms(){return rdoms_;}

    /****************api about live var****************/

    bool set_live_in(WeakPtrSet<Value> in){
        if (WeakPtrSetEq(live_in, in)) {
            return false;
        } else {
            live_in = in;
            return true;
        }
    }
    bool set_live_out(WeakPtrSet<Value> out){
        if (WeakPtrSetEq(live_out, out)) {
            return false;
        } else {
            live_out = out;
            return true;
        }
    }

    auto& get_live_in(){return live_in;}
    auto& get_live_out(){return live_out;}
    
private:
    explicit BasicBlock(Ptr<Module> m, const std::string &name ,
                        Ptr<Function> parent );
    void init(Ptr<Module> m, const std::string &name ,
                        Ptr<Function> parent );
    WeakPtrList<BasicBlock> pre_bbs_;
    WeakPtrList<BasicBlock> succ_bbs_;
    PtrList<Instruction> instr_list_;
    WeakPtrSet<BasicBlock> dom_frontier_;
    WeakPtrSet<BasicBlock> rdom_frontier_;
    WeakPtrSet<BasicBlock> rdoms_;
    WeakPtr<BasicBlock> idom_;
    WeakPtrSet<Value> live_in;
    WeakPtrSet<Value> live_out;
    WeakPtr<Function> parent_;
};

}
}

#endif // _SYSYF_BASICBLOCK_H_
