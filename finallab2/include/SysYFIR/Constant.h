#ifndef _SYSYF_CONSTANT_H_
#define _SYSYF_CONSTANT_H_
#include "User.h"
#include "Value.h"
#include "Type.h"
#include <memory>

namespace SysYF
{
namespace IR
{
class Constant : public User
{
protected:
    explicit Constant(Ptr<Type> ty, const std::string &name = "", unsigned num_ops = 0, Ptr<Module> m=nullptr)
    : User(ty, name, num_ops), parent_(m) {}
    void init(Ptr<Type> ty, const std::string &name = "", unsigned num_ops = 0);
    // int value;
public:
    WeakPtr<Module> parent_;
    ~Constant() = default;
};

class ConstantInt : public Constant
{
private:
    int value_;
    explicit ConstantInt(Ptr<Type> ty, int val, Ptr<Module> m) 
        : Constant(ty,"",0,m),value_(val) {}
    void init(Ptr<Type> ty, int val, Ptr<Module> m);

public:
    
    static int get_value(Ptr<ConstantInt> const_val) { return const_val->value_; }
    int get_value() { return value_; }
    static Ptr<ConstantInt> create(int val, Ptr<Module> m);
    static Ptr<ConstantInt> create(bool val, Ptr<Module> m);
    virtual std::string print() override;
};

class ConstantFloat : public Constant
{
private:
    float value_;
    explicit ConstantFloat(Ptr<Type>  ty,float val, Ptr<Module> m) 
        : Constant(ty,"",0,m),value_(val) {}
    void init(Ptr<Type> ty, float val, Ptr<Module> m);

public:
    
    static float get_value(Ptr<ConstantFloat> const_val) { return const_val->value_; }
    float get_value() { return value_; }
    static Ptr<ConstantFloat> create(float val, Ptr<Module> m);
    virtual std::string print() override;
};

class ConstantArray : public Constant
{
private:
    PtrVec<Constant> const_array;
    explicit ConstantArray(Ptr<ArrayType> ty, const PtrVec<Constant> &val, Ptr<Module> m);
    void init(Ptr<ArrayType> ty, const PtrVec<Constant> &val, Ptr<Module> m);

public:
    
    ~ConstantArray() = default;

    Ptr<Constant> get_element_value(int index);

    unsigned get_size_of_array() { return const_array.size(); } 

    static Ptr<ConstantArray> create(Ptr<ArrayType> ty, const PtrVec<Constant> &val, Ptr<Module> m);

    virtual std::string print() override;
};

class ConstantZero : public Constant 
{
private:
    explicit ConstantZero(Ptr<Type> ty, Ptr<Module> m)
        : Constant(ty,"",0,m) {}
    void init(Ptr<Type> ty, Ptr<Module> m);

public:
    static Ptr<ConstantZero> create(Ptr<Type> ty, Ptr<Module> m);
    virtual std::string print() override;
};

}
}

#endif //_SYSYF_CONSTANT_H_
