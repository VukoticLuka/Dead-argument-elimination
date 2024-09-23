#include "DeadArgumentFinder.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <queue>

using namespace llvm;

namespace {
struct DeadArgumentElimination : public ModulePass {
    static char ID;
    DeadArgumentElimination() : ModulePass(ID) {}

    std::vector<Function*> FunctionsToRemove;
    std::vector<Instruction*> InstructionsToRemove;
    std::unordered_map<Value*, std::pair<Function*, std::queue<unsigned>>> FunctionMapping;

    void cloneAndRemapFunction(Function &F, std::unordered_set<Value*> &DeadArguments) {

        // get types of each new argument
        std::vector<Type*> NewArgTypes;
        std::queue<unsigned> DeadIndexes;
        for (Argument &A : F.args()) {
            if (DeadArguments.find(&A) == DeadArguments.end()) {
                NewArgTypes.push_back(A.getType());
            } else {
                DeadIndexes.push(A.getArgNo());
            }
        }

        // Create new function
        FunctionType *NewFunctionType = FunctionType::get(F.getReturnType(), NewArgTypes, F.isVarArg());
        Function *NewFunction = Function::Create(NewFunctionType, F.getLinkage(), F.getAddressSpace(), F.getName() + "_new", F.getParent());

        // Map the old function to the replacement and keep track of arguments to remove
        FunctionMapping[&F] = std::make_pair(NewFunction, DeadIndexes);
        FunctionsToRemove.push_back(&F);

        std::unordered_map<const Value*, Value*> CloneMapping;

        // Map new arguments to the old ones, skipping unused ones
        Function::arg_iterator NewArgIterator = NewFunction->arg_begin();
        for (Argument &OldArg : F.args()) {
            if (DeadArguments.find(&OldArg) == DeadArguments.end()) {
                CloneMapping[&OldArg] = &(*NewArgIterator);
                NewArgIterator->setName(OldArg.getName());
                ++NewArgIterator;
            }
        }

        // Clone the basic blocks of the original to new function
        for (BasicBlock &BB : F) {
            BasicBlock *NewBB = BasicBlock::Create(NewFunction->getContext(), BB.getName(), NewFunction);
            CloneMapping[&BB] = NewBB;

            for (Instruction &I : BB) {
                // Skip the instruction storing the dead argument
                if (isa<StoreInst>(I) && DeadArguments.find(I.getOperand(0)) != DeadArguments.end())
                    continue;

                // Clone each instruction
                Instruction *Cloned = I.clone();
                Cloned->insertInto(NewBB,NewBB->end());
                CloneMapping[&I] = Cloned;
            }
        }

        // Update references in each cloned instruction to use appropriate values
        for (BasicBlock &BB : *NewFunction) {
            for (Instruction &I : BB) {
                for (size_t i = 0; i < I.getNumOperands(); i++) {
                    if (CloneMapping.find(I.getOperand(i)) != CloneMapping.end()) {
                        I.setOperand(i, CloneMapping[I.getOperand(i)]);
                    }
                }

            }
        }
    }

    void replaceFunctionCalls(Module &M) {
    // Go through all instructions in the module
        for (Function &F : M) {
            for (BasicBlock &BB : F) {
                for (Instruction &I : BB) {
                    if (CallInst* OldCall = dyn_cast<CallInst>(&I)) {
                        Function* CalledFunction = OldCall->getCalledFunction();
                        if (CalledFunction->isVarArg()) continue;
                        // If it is a call, and we have a replacement for the called function, substitute them
                        if (FunctionMapping.find(CalledFunction) != FunctionMapping.end()) {
                            Function* NewFunction = FunctionMapping[CalledFunction].first;
                            std::queue<unsigned> DeadIndexes = FunctionMapping[CalledFunction].second;
                            std::vector<Value *> NewArgs;
                            // Take only the arguments the replacement needs
                            for (size_t i = 0; i < CalledFunction->arg_size(); i++) {
                                if (i == DeadIndexes.front()) {
                                    DeadIndexes.pop();
                                    continue;
                                }
                                NewArgs.push_back(OldCall->getArgOperand(i));
                            }

                            // Create a new call for the replacement function
                            IRBuilder Builder(BB.getContext());
                            CallInst* newCall = Builder.CreateCall(NewFunction, NewArgs);
                            newCall->insertBefore(OldCall);
                            OldCall->replaceAllUsesWith(newCall);
                            InstructionsToRemove.push_back(OldCall);
                        }
                    }
                }
            }
        }
    }


    bool runOnModule(Module &M) override {

        for (Function &F : M) {
            // Declaration - no body, usually external functions
            // main - skip to avoid problems when running the program
            if (F.isDeclaration() || F.getName().equals("main")) continue;

            // Find dead arguments
            DeadArgumentFinder Finder(F);
            std::unordered_set<Value*> DeadArguments = Finder.getDeadArguments();
            if (DeadArguments.empty()) continue;

            // Create replacement function
            cloneAndRemapFunction(F, DeadArguments);
        }
        // Replace all calls of old functions with replacements
        replaceFunctionCalls(M);

        // Remove unused instructions and functions
        for (auto I : InstructionsToRemove) {
            I->eraseFromParent();
        }
        for (auto F : FunctionsToRemove) {
            F->eraseFromParent();
        }

        return true;
    }
};
}

char DeadArgumentElimination::ID = 0;
static RegisterPass<DeadArgumentElimination> X("our-dae", "Our dead argument elimination",
                             false,false );
