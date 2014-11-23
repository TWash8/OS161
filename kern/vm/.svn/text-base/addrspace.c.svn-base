#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <vnode.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */


struct addrspace *
as_create(void)
{
	//kprintf("does as ever print?\n");
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		return NULL;
	}
	as->as_vbase1 = 0;
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	as->as_npages2 = 0;
	as->heapstart = 0;

  /* some extra signals */
  as->v = NULL;
  as->offset1 = 0;
  as->offset2 = 0;
  as->fileSize1 = 0;
  as->fileSize2 = 0;
  as->memSize1 = 0;
  as->memSize2 = 0;
  as->stackEnd = 0;


	return as;
}

void
as_destroy(struct addrspace *as)
{ 	
  //kprintf("DESTRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRROY");
#if 1
  int spl = splhigh();
  struct lvonepagetable* lvlOnePageTable = &(as->pageTable);
  struct lvtwopagetable* lvlTwoPageTable = NULL;
  int i, j;

  for (i = 0; i < 512; i++) {
    lvlTwoPageTable = lvlOnePageTable->lvtwo[i];
    if (lvlTwoPageTable != NULL) {
      for (j = 0; j < 1024; j++) {
        if (lvlTwoPageTable->entry[j].pagePtr != NULL) {
          freePage(lvlTwoPageTable->entry[j].pagePtr);
          lvlTwoPageTable->entry[j].pageframeadd = 0;
          lvlTwoPageTable->entry[j].valid  = 0;
          lvlTwoPageTable->entry[j].pagePtr = NULL;
        }
      }
    }
  }
  if (as->v) VOP_DECREF(as->v);
  kfree(as);
  splx(spl);
#endif
}

void
as_activate(struct addrspace *as)
{
	int i, spl;

	(void)as;

	spl = splhigh();

	for (i=0; i<NUM_TLB; i++) {
		TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable, int i)
{
/*
	size_t npages; 
	
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;


	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = sz / PAGE_SIZE;

	//kprintf("as define region npages is %d\n",npages);
	(void)readable;
	(void)writeable;
	(void)executable;

	if (as->as_vbase1 == 0) {
		as->as_vbase1 = vaddr;
		as->as_npages1 = npages;
		return 0;
	}

	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
		return 0;
	}

	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;
*/

size_t newsz;
	
	newsz=(sz+(vaddr & (~(vaddr_t)PAGE_FRAME))+4096-1);
	newsz=newsz&PAGE_FRAME;  // new siz without the shift
	
	if (i==0) {
    as->as_npages1=newsz>>12;
    as->as_vbase1=(vaddr&PAGE_FRAME);
		if (as->heapstart < ((vaddr & PAGE_FRAME) + newsz)) {
      as->heapstart = (vaddr & PAGE_FRAME) + newsz;
      kprintf("This is heapstart 1: %x\n",as->heapstart);
      as->heapend = as->heapstart;
      assert((as->heapstart & PAGE_FRAME) == as->heapstart);
    }
    return 0;
	}
	
	else if (i==1) {
		as->as_npages2=newsz>>12;
		as->as_vbase2=(vaddr&PAGE_FRAME);
		if (as->heapstart < ((vaddr & PAGE_FRAME) + newsz)) {
      as->heapstart = (vaddr & PAGE_FRAME) + newsz;
      kprintf("This is heapstart 2: %x\n",as->heapstart);
      as->heapend = as->heapstart;
      assert((as->heapstart & PAGE_FRAME) == as->heapstart);
    }
    return 0;
	}
	else return EUNIMP;
}

int
as_prepare_load(struct addrspace *as)
{
	
#if 0
	assert(as->as_pbase1 == 0);
	assert(as->as_pbase2 == 0);
	assert(as->as_stackpbase == 0);
	int spl;
	spl=splhigh();
	as->as_pbase1 = alloc_kpages(as->as_npages1)-MIPS_KSEG0;
	if (as->as_pbase1 == 0) {
		return ENOMEM;
	}
	as->as_pbase2 = alloc_kpages(as->as_npages2)-MIPS_KSEG0;
	if (as->as_pbase2 == 0) {
		return ENOMEM;
	}
	as->as_stackpbase = alloc_kpages(DUMBVM_STACKPAGES)-MIPS_KSEG0;
	if (as->as_stackpbase == 0) {
		return ENOMEM;
	}		
	splx(spl);
#endif

	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	//assert(as->as_stackpbase != 0);
  as->stackEnd = USERSTACK;
	*stackptr = USERSTACK;
	return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
  //kprintf("COOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOPY\n");
#if 1
  int spl = splhigh();
	struct addrspace *new;

	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}

  new->heapstart = old->heapstart;
  new->heapend= old->heapend;
	new->as_vbase1 = old->as_vbase1;
	new->as_npages1 = old->as_npages1;
	new->as_vbase2 = old->as_vbase2;
	new->as_npages2 = old->as_npages2;
  new->progheader[0] = old->progheader[0];
  new->progheader[1] = old->progheader[1];

  new->v = old->v;
  VOP_INCREF(old->v);
  new->offset1 = old->offset1;
  new->offset2 = old->offset2;
  new->fileSize1 = old->fileSize1;
  new->fileSize2 = old->fileSize2;
  new->memSize1 = old->memSize1;
  new->memSize2 = old->memSize2;
  new->stackEnd = old->stackEnd;

  struct lvonepagetable* oldLvlOnePageTable = &(old->pageTable);
  struct lvtwopagetable* oldLvlTwoPageTable = NULL;

  struct lvonepagetable* newLvlOnePageTable = &(new->pageTable);
  struct lvtwopagetable* newLvlTwoPageTable = NULL;

  int i;
  for (i = 0; i < 512; i++) {
    oldLvlTwoPageTable = oldLvlOnePageTable->lvtwo[i];
    if (oldLvlTwoPageTable != NULL) {
      newLvlTwoPageTable = kmalloc(sizeof(struct lvtwopagetable));
      if (newLvlTwoPageTable == NULL) return ENOMEM;
      if(configure(oldLvlTwoPageTable, newLvlTwoPageTable, new)) return ENOMEM;
      newLvlOnePageTable->lvtwo[i] = newLvlTwoPageTable;
    }
  }
	*ret = new;
  splx(spl);
#endif
	return 0;
}
