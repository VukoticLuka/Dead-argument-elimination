#ifndef DEADARGUMENTFINDER_H
#define DEADARGUMENTFINDER_H
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include <unordered_set>

using namespace llvm;

class DeadArgumentFinder {
private:
    std::unordered_map<Value*, Value*> VariablesMap;
    std::unordered_map<Value*, Value*> ArgumentsMap;
    std::unordered_set<Value*> Arguments;
    Function &F;
    void mapArguments();
    void mapVariables();
    void findDeadArgs();
public:
    explicit DeadArgumentFinder(Function &f) : F(f) {}
    std::unordered_set<Value*> getDeadArguments();
};



#endif //DEADARGUMENTFINDER_H
