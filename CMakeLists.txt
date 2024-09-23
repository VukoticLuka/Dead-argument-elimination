add_llvm_library( LLVMSimpleDeadArgumentElimination MODULE BUILDTREE_ONLY
  DeadArgumentFinder.cpp
  DeadArgumentElimination.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
