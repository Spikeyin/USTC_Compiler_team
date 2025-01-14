#ifndef SYSYF_LIVEVAR_H
#define SYSYF_LIVEVAR_H

#include "Pass.h"
#include "Module.h"

namespace SysYF{
namespace IR{

class LiveVar : public Pass
{
public:
    LiveVar(WeakPtr<Module> m) : Pass(m) {}
    void execute() final;
    const std::string get_name() const override {return name;}
    void dump();
private:
    Ptr<Function> func_;
    const std::string name = "LiveVar";
};

bool ValueCmp(WeakPtr<Value> a, WeakPtr<Value> b);
WeakPtrVec<Value> sort_by_name(WeakPtrSet<Value> &val_set);
const std::string lvdump = "live_var.out";

}
}

#endif  // SYSYF_LIVEVAR_H
