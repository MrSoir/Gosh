#ifndef DYNAMICFUNCTIONCALLER_H
#define DYNAMICFUNCTIONCALLER_H

#include <map>
#include <functional>

template <class Key, class Value>
class DynamicFunctionCaller
{
public:
    DynamicFunctionCaller(){}

    ~DynamicFunctionCaller(){
        m_functions.clear();
    }

    bool containsFunction(const Key &funcName){
        return (m_functions.find(funcName) != m_functions.end());
    }

    void setFunction(const Key &funcName, Value func){ m_functions[funcName] = func; }

    Value removeFunction(const Key &funcName){ m_functions.erase(funcName); }

    Value getFunction(const Key &funcName)
    {
        if(containsFunction(funcName)){
            return m_functions[funcName];
        }
        return nullptr;
    }

    int formulaCount(){
        return m_functions.size();
    }
private:
    std::map<Key, Value> m_functions;
};

#endif // DYNAMICFUNCTIONCALLER_H
