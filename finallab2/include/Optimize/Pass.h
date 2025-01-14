#ifndef SYSYF_PASS_H
#define SYSYF_PASS_H


#include <memory>
#include <string>
#include <list>
#include "Module.h"

namespace SysYF{
namespace IR{

class Pass{
public:
    explicit Pass(WeakPtr<Module> m){module = m;}
    virtual void execute() = 0;
    virtual const std::string get_name() const = 0;
protected:
    WeakPtr<Module> module;
};

template<typename T>
using PassList = PtrList<T>;

class PassMgr{
public:
    explicit PassMgr(WeakPtr<Module> m){module = m;pass_list = PassList<Pass>();}
    template <typename PassTy> void addPass(){pass_list.emplace_back(new PassTy(module));}
    void execute() {
        for (auto pass : pass_list) {
            pass->execute();
        }
    }
private:
    WeakPtr<Module> module;
    PassList<Pass> pass_list;
};

}
}

#endif // SYSYF_PASS_H
