#ifndef LEAK_PARAM_ITEM
#define LEAK_PARAM_ITEM
#include <string>
#include "SVF-FE/LLVMUtil.h"
#include "SVF-FE/SVFIRBuilder.h"
using namespace SVF;

class LeakParamItem
{
private:
    SVF::NodeID id;
    std::string func_name;
    SVF::NodeID param_index;
    std::string leak_type;

public:
    LeakParamItem() {}
    std::string getFuncName()
    {
        return func_name;
    }
    SVF::NodeID getParamIndex()
    {
        return param_index;
    }
    std::string getLeakType()
    {
        return leak_type;
    }
    SVF::NodeID getId()
    {
        return id;
    }

    friend std::istream &operator>>(std::istream &in, LeakParamItem &obj)
    {
        in >> obj.id >> obj.func_name >> obj.param_index >> obj.leak_type;
        return in;
    }

    friend std::ostream &operator<<(std::ostream &out,LeakParamItem &obj)
    {
        out << obj.id << "," << obj.func_name << "," << obj.param_index << "," << obj.leak_type;
        return out;
    }
};
#endif