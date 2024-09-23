#include "DeadArgumentFinder.h"

void DeadArgumentFinder::mapArguments() {
    for (Argument &A : F.args()) {
        Arguments.insert(&A);
    }
    for (Instruction &I: F.getEntryBlock()) {
        if (isa<StoreInst>(&I)) {
            Value* Arg1 = I.getOperand(0); //uzimamo ono sto se upisuje, ukoliko je argument upisujemo u sta se mapira
            if (Arguments.find(Arg1) != Arguments.end()) {
                Value* Arg2 = I.getOperand(1);
                ArgumentsMap[Arg2] = Arg1;
            }
        }
    }
}
void DeadArgumentFinder::mapVariables() {
    for (BasicBlock &BB: F) {
        for (Instruction &I : BB) {
            if (isa<LoadInst>(&I)) {
                VariablesMap[&I] = I.getOperand(0);
            }
        }
    }

}
void DeadArgumentFinder::findDeadArgs() {
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            for (size_t i = 0; i < I.getNumOperands(); i++) {
                Value *Operand = I.getOperand(i);
                if (VariablesMap.find(Operand) != VariablesMap.end()) {
                    Value *LocalVariable = VariablesMap[Operand];

                    if (ArgumentsMap.find(LocalVariable) != ArgumentsMap.end()) {
                        Arguments.erase(ArgumentsMap[LocalVariable]);
                    }
                }
            }
        }
    }
}
std::unordered_set<Value*> DeadArgumentFinder::getDeadArguments() {
    mapVariables();
    mapArguments();
    findDeadArgs();
    return Arguments;

}