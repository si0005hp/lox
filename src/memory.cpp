#include "memory.h"

#include "vm.h"

namespace lox {

  void Memory::collectGarbage() {
#ifdef DEBUG_LOG_GC
    printf("==> gc begin\n");
#endif

#ifdef DEBUG_LOG_GC
    printf("--> mark roots begin\n");
#endif
    vm_->gcMarkRoots();
#ifdef DEBUG_LOG_GC
    printf("<-- mark roots end\n");
#endif

#ifdef DEBUG_LOG_GC
    printf("--> blacken objects begin\n");
#endif
    vm_->gcBlackenObjects();
#ifdef DEBUG_LOG_GC
    printf("<-- blacken objects end\n");
#endif

#ifdef DEBUG_LOG_GC
    printf("--> remove weak references begin\n");
#endif
    vm_->gcRemoveWeakReferences();
#ifdef DEBUG_LOG_GC
    printf("<-- remove weak references end\n");
#endif

#ifdef DEBUG_LOG_GC
    printf("--> sweep begin\n");
#endif
    vm_->gcSweep();
#ifdef DEBUG_LOG_GC
    printf("<-- sweep end\n");
#endif

#ifdef DEBUG_LOG_GC
    printf("<== gc end\n");
#endif
  }

} // namespace lox
