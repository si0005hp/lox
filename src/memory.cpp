#include "memory.h"

#include "vm.h"

namespace lox {

  void Memory::collectGarbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
#endif

    vm_->gcMarkRoots();

    vm_->gcBlackenObjects();

    vm_->gcSweep();

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
#endif
  }

} // namespace lox
