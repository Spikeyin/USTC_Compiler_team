#include "BasicBlock.h"
#include "Function.h"
#include "Pass.h"
#include "DominateTree.h"
#include "internal_types.h"

namespace SysYF {
namespace IR {

void DominateTree::execute() {
    for (auto f: module.lock()->get_functions()) {
        if (f->get_basic_blocks().empty()) {
            continue;
        }
        get_bb_idom(f);
        get_bb_dom_front(f);
    }
}

void DominateTree::get_post_order(Ptr<BasicBlock> bb, PtrSet<BasicBlock> &visited) {
    visited.insert(bb);
    auto children = bb->get_succ_basic_blocks();
    for (auto child: children) {
        auto is_visited = visited.find(child.lock());
        if (is_visited == visited.end()) {
            get_post_order(child.lock(), visited);
        }
    }
    bb2int[bb] = reverse_post_order.size();
    reverse_post_order.push_back(bb);
}

void DominateTree::get_revserse_post_order(Ptr<Function> f) {
    doms.clear();
    reverse_post_order.clear();
    bb2int.clear();
    auto entry = f->get_entry_block();
    PtrSet<BasicBlock> visited = {};
    get_post_order(entry, visited);
    reverse_post_order.reverse();
}

//ref:https://www.cs.rice.edu/~keith/EMBED/dom.pdf
void DominateTree::get_bb_idom(Ptr<Function> f) {
    get_revserse_post_order(f);

    auto root = f->get_entry_block();
    auto root_id = bb2int[root];
    for(int i = 0;i < root_id;i++){
        doms.push_back({});
    }

    doms.push_back(root);

    bool changed = true;
    while(changed){
        changed = false;
        for(auto bb:reverse_post_order){
            if(bb.lock() == root){
                continue;
            }
            auto preds = bb.lock()->get_pre_basic_blocks();
            Ptr<BasicBlock> new_idom = nullptr;
            for(auto pred_bb:preds){
                if(doms[bb2int[pred_bb.lock()]].lock() != nullptr){
                    new_idom = pred_bb.lock();
                    break;
                }
            }
            for(auto pred_bb:preds){
                if(doms[bb2int[pred_bb.lock()]].lock() != nullptr){
                    new_idom = intersect(pred_bb.lock(),new_idom);
                }
            }
            if(doms[bb2int[bb]].lock() != new_idom){
                doms[bb2int[bb]] = new_idom;
                changed = true;
            }
        }
    }

    for(auto bb:reverse_post_order){
        bb.lock()->set_idom(doms[bb2int[bb]].lock()); }
}

void DominateTree::get_bb_dom_front(Ptr<Function> f) {
    for(auto b:f->get_basic_blocks()){
        auto b_pred = b->get_pre_basic_blocks();
        if(b_pred.size() >= 2){
            for(auto pred:b_pred){
                auto runner = pred;
                while(runner.lock()!=doms[bb2int[b]].lock()){
                    runner.lock()->add_dom_frontier(b);
                    runner = doms[bb2int[runner.lock()]];
                }
            }
        }
    }
}

Ptr<BasicBlock> DominateTree::intersect(Ptr<BasicBlock> b1, Ptr<BasicBlock> b2) {
    auto finger1 = b1;
    auto finger2 = b2;
    while(finger1 != finger2){
        while(bb2int[finger1]<bb2int[finger2]){
            finger1 = doms[bb2int[finger1]].lock();
        }
        while(bb2int[finger2]<bb2int[finger1]){
            finger2 = doms[bb2int[finger2]].lock();
        }
    }
    return finger1;
}

}
}
