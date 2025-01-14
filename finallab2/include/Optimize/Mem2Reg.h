#ifndef SYSYF_MEM2REG_H
#define SYSYF_MEM2REG_H

#include "BasicBlock.h"
#include "Function.h"
#include "GlobalVariable.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "Pass.h"
#include "internal_types.h"
#include <memory>

namespace SysYF {
namespace IR {

class Mem2Reg : public Pass{
private:
    WeakPtr<Function> func_;
    WeakPtr<IRBuilder> builder;
    WeakPtrMap<BasicBlock, WeakPtrVec<Value>> define_var;
    const std::string name = "Mem2Reg";
    WeakPtrMap<Value, WeakPtr<Value>> lvalue_connection;
    WeakPtrSet<Value> no_union_set;

public:
	explicit Mem2Reg(WeakPtr<Module> m) : Pass(m) {}
	~Mem2Reg(){};
	void execute() final;
	void genPhi();
	void insideBlockForwarding();
	void valueDefineCounting();
	void valueForwarding(Ptr<BasicBlock> bb);
	void removeAlloc();
    const std::string get_name() const override {return name;}

	bool isLocalVarOp(Ptr<Instruction> inst){
		if (inst->get_instr_type() == Instruction::OpID::store){
			auto sinst = static_pointer_cast<StoreInst>(inst);
			auto lvalue = sinst->get_lval();
			auto glob = dynamic_pointer_cast<GlobalVariable>(lvalue);
			auto array_element_ptr = dynamic_pointer_cast<GetElementPtrInst>(lvalue);
			return !glob && !array_element_ptr;
		}
		else if (inst->get_instr_type() == Instruction::OpID::load){
			auto linst = static_pointer_cast<LoadInst>(inst);
			auto lvalue = linst->get_lval();
			auto glob = dynamic_pointer_cast<GlobalVariable>(lvalue);
			auto array_element_ptr = dynamic_pointer_cast<GetElementPtrInst>(lvalue);
			return !glob && !array_element_ptr;
		}
		else
			return false;
	}
};

}
}

#endif // SYSYF_MEM2REG_H
