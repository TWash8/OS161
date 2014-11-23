#ifndef _VM_H_
#define _VM_H_

#include <machine/vm.h>

/*
 * VM system-related definitions.
 *
 * You'll probably want to add stuff here.
 */


/* Fault-type arguments to vm_fault() */
#define VM_FAULT_READ        0    /* A read was attempted */
#define VM_FAULT_WRITE       1    /* A write was attempted */
#define VM_FAULT_READONLY    2    /* A write to a readonly page was attempted*/


/* Initialization function */
void vm_bootstrap(void);

/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress);

/* Allocate/free kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(int npages);
void free_kpages(vaddr_t addr);
static void swapper(void);

/* helper functions */
int onStack (struct addrspace*, vaddr_t);
int onHeap (struct addrspace*, vaddr_t);
int increaseStack (struct addrspace* as, vaddr_t faultaddress, paddr_t* _paddr);
int increaseHeap (struct addrspace* as, vaddr_t faultaddress, paddr_t* _paddr);
int loadPage (struct addrspace* as, vaddr_t faultaddress, paddr_t* _paddr, int region);
int load (struct addrspace* as, vaddr_t faultaddress, u_int32_t base, paddr_t paddr, u_int32_t vaddr, off_t offset, int memSize, int fileSize);


/* helper functions --END-- */

#endif /* _VM_H_ */
