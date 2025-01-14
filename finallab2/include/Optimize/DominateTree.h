#ifndef SYSYF_DOMINATETREE_H
#define SYSYF_DOMINATETREE_H

#include "BasicBlock.h"
#include "Module.h"
#include "Pass.h"
#include "internal_types.h"
#include <memory>

namespace SysYF {
namespace IR {

class DominateTree: public Pass {
public:
    explicit DominateTree(WeakPtr<Module> m): Pass(m) {}
    void execute() final;
    void get_revserse_post_order(Ptr<Function> f);
    void get_post_order(Ptr<BasicBlock> bb, PtrSet<BasicBlock>& visited);
    void get_bb_idom(Ptr<Function> f);
    void get_bb_dom_front(Ptr<Function> f);
    Ptr<BasicBlock> intersect(Ptr<BasicBlock> b1, Ptr<BasicBlock> b2);
    const std::string get_name() const override {return name;}
private:
    WeakPtrList<BasicBlock> reverse_post_order;
    WeakPtrMap<BasicBlock, int> bb2int;
    WeakPtrVec<BasicBlock> doms;
    const std::string name = "DominateTree";
};

}
}

#endif // SYSYF_DOMINATETREE_H
