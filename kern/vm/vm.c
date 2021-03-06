#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <elf.h>
#include <uio.h>
#include <kern/stat.h>
#include <vfs.h>
#include <kern/unistd.h>
#include <synch.h>
#include <vnode.h>
/*
 * Dumb MIPS-only "VM system" that is intended to only be just barely
 * enough to struggle off the ground. You should replace all of this
 * code while doing the VM assignment. In fact, starting in that
 * assignment, this file is not included in your kernel!
 */
static struct {
    u_int32_t  numofpage;
    u_int32_t  FirstPageAdd;
    u_int32_t  numofpageleft;
    struct page* firstPage;
    }coremap;

//swap definition start
char* bitmap;
static struct bitmap *bit_map = NULL;
struct vnode* swap; 
u_int32_t swap_end;
u_int32_t swap_start;
u_int32_t swap_size;
u_int32_t swap_free;
struct cv* cv_needmore;
struct cv* cv_needevict;
struct lock* swap_lock;
//swap definition end

/* under dumbvm, always have 48k of user stack */
//#define DUMBVM_STACKPAGES    1

void
vm_bootstrap(void)
{
  //************coremap start
  u_int32_t firstadd , lastadd;
  u_int32_t firstpage, lastpage , totalpage , mapsize;
  u_int32_t mappage,bitmappage,bitmap_size;
  ram_getsize(&firstadd, &lastadd); 

  firstpage=(firstadd)&0xfffff000; //mask out addr
  lastpage=(lastadd)&0xfffff000;
  totalpage=(lastpage>>12)-(firstpage>>12);
  mapsize=totalpage*sizeof(struct page); // how big is the map?
  mappage=mapsize/4096+1; //how many pages are needed to store coremap

  coremap.FirstPageAdd=firstpage;
  coremap.numofpage=totalpage;


  coremap.firstPage= (struct page*)PADDR_TO_KVADDR(firstadd); 
  bzero(coremap.firstPage, coremap.numofpage * sizeof(struct page)); //initialize everything to zero.. nessisary????
  coremap.FirstPageAdd=coremap.FirstPageAdd+(mappage<<12); //update content of coremap
  coremap.numofpage=coremap.numofpage-mappage;
  coremap.firstPage = coremap.firstPage+mappage;
  //*************coremap end

  //*************swap
  struct stat disk;
  char swapdevice[]= {"lhd0raw:"};
  int err = vfs_open(swapdevice, O_RDWR, &swap);
	if (err) {
    		panic("can't open swap device\n");
	}

  VOP_STAT(swap, &disk);
  swap_size=disk.st_size/4096;
  bitmap_size = (((disk.st_size)/4096)*sizeof(char));
  bitmappage=bitmap_size/4096+1;

  bitmap=(char*)PADDR_TO_KVADDR(coremap.FirstPageAdd);
  int i;
  for(i=0;i<bitmap_size;i++)
    bitmap[i]=0;

  swap_start=totalpage/5;
  swap_end=totalpage/2;

  coremap.FirstPageAdd=coremap.FirstPageAdd+(bitmappage<<12); //update content of coremap
  coremap.numofpage=coremap.numofpage-bitmappage;
  coremap.firstPage = coremap.firstPage+bitmappage;
  coremap.numofpageleft=totalpage;

  //kprintf(" i have %d pages\n",coremap.numofpage);
  //*****************swap
  //bit_map = bitmap_create(swap_size);
  //lock cv create
  swap_lock=lock_create("swaplock");
  cv_needmore=cv_create("needmore");
  cv_needevict=cv_create("needevict");

  err = thread_fork("swapper", NULL, 0, swapper, NULL);
    if (err) {
        panic("Can't create thread for swap.");
    }
}

static
paddr_t
getppages(unsigned long npages)
{
	paddr_t addr;
	
	
	if(coremap.firstPage==NULL)
	{
	addr = ram_stealmem(npages);
	return addr;
	}
	else 
	{
		//kprintf("trying to get %d page\n",npages);
		struct page* start=coremap.firstPage;
		struct page* freepagestart=coremap.firstPage + coremap.numofpage; //track where the first free page start, i set it to random value at first		
		//int found =0;
		
		int count=0;
		while (start<(coremap.firstPage + coremap.numofpage))
		{
				int i=0;
			while(start<(coremap.firstPage + coremap.numofpage) && !(start->allocated==0))
				{
				start++;   //skip all allocated pages
				i++;
				//kprintf("not free %d\n",i);
				}
				freepagestart=start; //save the begining of free page
			while (start<(coremap.firstPage + coremap.numofpage) && (start->allocated==0)) //check if there are enough consecutive avaliable pages
				{
				count++;
				start++;
				}
			if (count>=npages)
				{	
				addr=(paddr_t)(coremap.FirstPageAdd + ((freepagestart) - coremap.firstPage) * 4096);
				freepagestart->consec = npages;
				
				for(count=0; count<npages;count++)
					{
        				freepagestart[count].allocated = 1;
        				freepagestart[count].as = NULL;
					freepagestart[count].isKernelPage = 1;
					}	
				coremap.numofpageleft=coremap.numofpageleft-npages;
				return addr;
				} 
		
		}

	}
	
	//kprintf(" i failed to allocated %d nigga\n",npages);
	return 0;// resturn 0 if no page found
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t 
alloc_kpages(int npages)
{
	//npages=1;
	int spl;
	paddr_t pa,pa2;
	spl = splhigh();
	pa = getppages(npages);
	if (pa==0) {
		return (paddr_t)0;
	}

	
	splx(spl);

	return PADDR_TO_KVADDR(pa);
}

void 
free_kpages(vaddr_t addr)
{
  //kprintf("in free_kpages\n");
	int spl;
	spl = splhigh();
	paddr_t paddr;
	paddr=addr-MIPS_KSEG0;	
  assert(paddr == (paddr&0xfffff000));
	
	struct page* currentpage;
	currentpage =(coremap.firstPage + (((u_int32_t)(paddr) - coremap.FirstPageAdd) >> 12));	
	int i = 0; 
	int npage;

  assert(currentpage != NULL);

  //kprintf("%d\n",currentpage->consec);

  assert(currentpage +  currentpage->consec <= coremap.firstPage + coremap.numofpage);
	
    	for (i = 0; i < currentpage->consec; i++) 
	{
    assert(currentpage != NULL);
    assert(currentpage[i].isKernelPage == 1);
    assert(currentpage[i].allocated == 1);
		currentpage[i].allocated=0; 
		currentpage[i].isKernelPage =0; 
		//kprintf("i freed %d pages\n",i+1);
	}	
	coremap.numofpageleft=coremap.numofpageleft+currentpage->consec;
  currentpage->consec = 0;
	splx(spl);
}

#if 1
int onStack (struct addrspace* as, vaddr_t faultaddress) {
  if ((USERSTACK - as->stackEnd) >= /*5001216*/0x500000) {
    return 1; // Here, we are limiting the maximum stack size to be 5 megabytes
  }
  if (faultaddress <= USERSTACK && faultaddress >= as->stackEnd) return 1;
  if (faultaddress <= USERSTACK && faultaddress >= (as->stackEnd - PAGE_SIZE)) return 1;
  return 0;
}

int onHeap (struct addrspace* as, vaddr_t faultaddress) {
  if (faultaddress >= as->heapstart && faultaddress <= as->heapend) return 1;
  return 0;
}

int findFrame (struct addrspace* as, vaddr_t faultaddress, paddr_t* _paddr) {
  assert((faultaddress & PAGE_FRAME) == faultaddress);
  assert(as != NULL);

  u_int32_t lvlOnePageTableIndex = (unsigned)faultaddress>>22;
  u_int32_t lvlTwoPageTableIndex = ((unsigned)faultaddress>>12)&0x3ff;
  struct lvonepagetable* lvlOnePageTable = &(as->pageTable);
  struct lvtwopagetable* lvlTwoPageTable = lvlOnePageTable->lvtwo[lvlOnePageTableIndex];

  if (lvlTwoPageTable == NULL) return -1;

  struct pagetableentry* pte = &(lvlTwoPageTable->entry[lvlTwoPageTableIndex]);
  if (pte->valid == 0) return -1;
  //kprintf("This is the faultaddress: %x\n",faultaddress);
  //kprintf("This is the matching address: %x\n", pte->pagePtr->va);
  assert(pte->pageframeadd == (pte->pageframeadd)&0x3ff);
  *_paddr = pte->pageframeadd;
  //kprintf("FOUND\n");
  return 0;
}

int increaseStack (struct addrspace* as, vaddr_t faultaddress, paddr_t* _paddr) {
  assert((faultaddress & PAGE_FRAME) == faultaddress);
  assert(as != NULL);

  struct page* freePageFrame = NULL;
  int paddr;
  if (getOnePhysicalFrame(&freePageFrame)) {
    assert(freePageFrame == NULL);
    return -1; // fail
  }
  assert(freePageFrame != NULL);
  if (addMapping(as, faultaddress, &freePageFrame)) {
    return -1; // fail
  }

  paddr = coremap.FirstPageAdd + (freePageFrame - coremap.firstPage)*4096;
  bzero((void*)PADDR_TO_KVADDR(paddr), PAGE_SIZE);
  as->stackEnd = as->stackEnd - PAGE_SIZE;
  *_paddr = paddr;
  return 0;
}

int getOnePhysicalFrame (struct page** freePageFrame) {
  assert(*freePageFrame == NULL);
  assert(curspl > 0);
  struct page* pageEntry = coremap.firstPage;
  while (pageEntry < (coremap.firstPage + coremap.numofpage)) {
    if ((pageEntry < (coremap.firstPage + coremap.numofpage)) && pageEntry->allocated == 0) {
      pageEntry->allocated = 1;
      *freePageFrame = pageEntry;
      return 0;
    }
    pageEntry++;
  }
  return -1;
}

int addMapping (struct addrspace* as, vaddr_t faultaddress, struct page** freePageFrame) {
  assert((*freePageFrame)->allocated == 1);
  assert((faultaddress & PAGE_FRAME) == faultaddress);
  assert(as != NULL);

  u_int32_t lvlOnePageTableIndex = (unsigned)faultaddress>>22;
  u_int32_t lvlTwoPageTableIndex = ((unsigned)faultaddress>>12)&0x3ff;
  struct lvonepagetable* lvlOnePageTable = &(as->pageTable);
  struct lvtwopagetable* lvlTwoPageTable = lvlOnePageTable->lvtwo[lvlOnePageTableIndex];

  if (lvlTwoPageTable == NULL) {
    lvlTwoPageTable = kmalloc(sizeof(struct lvtwopagetable));
    if (lvlTwoPageTable == NULL) return ENOMEM;
    bzero(lvlTwoPageTable, sizeof(struct lvtwopagetable));
    struct pagetableentry* pte = lvlTwoPageTable->entry;
    int i;
    for (i = 0; i < 1024; i++) {
      pte[i].valid = 0;
      pte[i].pagePtr = NULL;
    }
    lvlOnePageTable->lvtwo[lvlOnePageTableIndex] = lvlTwoPageTable;
  }

  struct pagetableentry* pte = &(lvlTwoPageTable->entry[lvlTwoPageTableIndex]);
  /* Not sure if these signals are correct */
  /* Everything is readable and everything is writable */
  pte->dirty = 1;
  pte->writable = 1;
  /* Not sure if these signals are correct --END-- */
  pte->valid = 1; // I think this should be correct...
  pte->pageframeadd = coremap.FirstPageAdd + (*freePageFrame - coremap.firstPage)*PAGE_SIZE;
  assert(pte->pageframeadd == (pte->pageframeadd)&0x3ff);
  //kprintf("pte->pageframeadd: %x\n",pte->pageframeadd);
  pte->pagePtr = *freePageFrame; // Perhaps redundant pointer
  assert((*freePageFrame)->allocated == 1); // Extra assertions to make sure
  (*freePageFrame)->isKernelPage = 0;
  (*freePageFrame)->as = as; // Perhaps another redundant pointer
  (*freePageFrame)->pte = pte;
  (*freePageFrame)->va = faultaddress; // This is stored unshifted
  (*freePageFrame)->consec = 1;
  return 0;
}

int increaseHeap (struct addrspace* as, vaddr_t faultaddress, paddr_t* _paddr) {
  assert((faultaddress & PAGE_FRAME) == faultaddress);
  assert(as != NULL);

  struct page* freePageFrame = NULL;
  int paddr;
  if (getOnePhysicalFrame(&freePageFrame)) {
    assert(freePageFrame == NULL);
    return -1; // fail
  }
  if (addMapping(as, faultaddress, &freePageFrame)) return -1; // fail

  paddr = coremap.FirstPageAdd + (freePageFrame - coremap.firstPage)*4096;
  bzero((void*)PADDR_TO_KVADDR(paddr), PAGE_SIZE);
  //as->heapend += PAGE_SIZE;
  *_paddr = paddr;
  return 0;
}

int loadPage (struct addrspace* as, vaddr_t faultaddress, paddr_t* _paddr, int region) {
  if (region == 1) assert((faultaddress >= as->as_vbase1) && (faultaddress < as->as_vbase1 + as->as_npages1 * PAGE_SIZE));
  if (region == 2) assert((faultaddress >= as->as_vbase2) && (faultaddress < as->as_vbase2 + as->as_npages2 * PAGE_SIZE));
  assert((faultaddress & PAGE_FRAME) == faultaddress);
  assert(as != NULL);
  assert(as->v != NULL);

  struct page* freePageFrame = NULL;
  int paddr, i;
  if (getOnePhysicalFrame(&freePageFrame)) {
    assert(freePageFrame == NULL);
    //kprintf("failed to getOnePhysicalFrame\n");
    return -1; // fail
  }
  assert(freePageFrame != NULL);
  if (addMapping(as, faultaddress, &freePageFrame)) {
    //kprintf("failed to add mapping\n");
    return -1; // fail
  }
  assert(freePageFrame != NULL);
  assert(freePageFrame->va == faultaddress);
  paddr = coremap.FirstPageAdd + (freePageFrame - coremap.firstPage)*4096;
  bzero((void*)PADDR_TO_KVADDR(paddr), PAGE_SIZE);

  /* start */
  u_int32_t p_vaddr1 = as->progheader[0].p_vaddr;
  u_int32_t p_vaddr2 = as->progheader[1].p_vaddr;
  //struct vnode* v = as->v;
  size_t offset1 = as->offset1;
  size_t offset2 = as->offset2;
  int memSize1 = as->memSize1;
  int memSize2 = as->memSize2;
  int fileSize1 = as->fileSize1;
  int fileSize2 = as->fileSize2;
  int base1 = as->as_vbase1;
  int base2 = as->as_vbase2;

  int result = -1;
  if (faultaddress >= p_vaddr1 && faultaddress < p_vaddr1 + memSize1) {
    //panic("out1\n");
    assert(region == 1);
    result = load(as, faultaddress, base1, paddr, p_vaddr1, offset1, memSize1, fileSize1);
  } else if (faultaddress >= p_vaddr2 && faultaddress < p_vaddr2 + memSize2) {
    //panic("out2\n");
    assert(region == 2);
    result = load(as, faultaddress, base2, paddr, p_vaddr2, offset2, memSize2, fileSize2);
  } else {
    panic("I shouldn't be here.");
  }
  if (result) return result;
  /* start --END-- */

  *_paddr = paddr;
  return 0;
}

int load (struct addrspace* as, vaddr_t faultaddress, u_int32_t base, paddr_t paddr, u_int32_t vaddr, off_t offset, int memSize, int fileSize) {
  assert((faultaddress&PAGE_FRAME) == faultaddress);
  //kprintf("This is the faultaddress: %x\n",faultaddress);
  struct uio u;
  int result;
  size_t fillamt;
  struct vnode* v = as->v;


  if (fileSize > memSize) {
		kprintf("ELF: warning: segment filesize > segment memsize\n");
		//fileSize = memSize;
  }

  int pageOffset = (faultaddress - base)/PAGE_SIZE;
  assert(pageOffset >= 0);
  int check = fileSize - (pageOffset + 1)*PAGE_SIZE;
  int residue;
  if (check >= 0) residue = PAGE_SIZE;
  else if (check < -PAGE_SIZE) residue = 0;
  else if (check < 0) residue = fileSize - pageOffset*PAGE_SIZE;
  else panic("Shouldn't be here\n");

	u.uio_iovec.iov_kbase = PADDR_TO_KVADDR(paddr);
	u.uio_iovec.iov_len = PAGE_SIZE; // PAGE_SIZE
	u.uio_resid = residue;
	u.uio_offset = pageOffset*PAGE_SIZE+offset;
	u.uio_segflg = UIO_SYSSPACE;
	u.uio_rw = UIO_READ;
	u.uio_space = NULL;

	result = VOP_READ(v, &u);
	if (result) {
		return result;
	}

	if (u.uio_resid != 0) {
		/* short read; problem with executable? */
		kprintf("ELF: short read on segment - file truncated?\n");
		return ENOEXEC;
	}

#if 0
  memSize = (pageOffset + 1)*PAGE_SIZE;
	fillamt = memSize - fileSize;
	if (fillamt > 0) {
		DEBUG(DB_EXEC, "ELF: Zero-filling %lu more bytes\n", 
		      (unsigned long) fillamt);
		u.uio_resid += fillamt;
		result = uiomovezeros(fillamt, &u);
	}
#endif
	
	return result;

}
#endif

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	paddr_t paddr;
	int i;
	u_int32_t ehi, elo;
	struct addrspace *as;
	int spl;

	spl = splhigh();

	faultaddress &= PAGE_FRAME;

	DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

	switch (faulttype) {
	    case VM_FAULT_READONLY:
		/* We always create pages read-write, so we can't get this */
		panic("dumbvm: got VM_FAULT_READONLY\n");
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		break;
	    default:
		splx(spl);
		return EINVAL;
	}

	as = curthread->t_vmspace;
	if (as == NULL) {
		/*
		 * No address space set up. This is probably a kernel
		 * fault early in boot. Return EFAULT so as to panic
		 * instead of getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

  // Tim added
  int result = -1;
  result = findFrame(as, faultaddress, &paddr);
  if (result) {
    if (onStack(as, faultaddress)) {
      //kprintf("updating stack\n");
      result = increaseStack(as, faultaddress, &paddr);
    } else if (onHeap(as, faultaddress)) {
      //kprintf("updating heap\n");
      result = increaseHeap(as, faultaddress, &paddr);
    } else {
      //kprintf("updating data and text\n");
      if ((faultaddress >= as->as_vbase1) && (faultaddress < (as->as_vbase1 + as->as_npages1 * PAGE_SIZE))) {
        result = loadPage(as, faultaddress, &paddr, 1);
      } else if ((faultaddress >= as->as_vbase2) && (faultaddress < (as->as_vbase2 + as->as_npages2 * PAGE_SIZE))) {
        result = loadPage(as, faultaddress, &paddr, 2);
      } else {
#if 0
        kprintf("This is as->as_vbase1: %x\n",as->as_vbase1);
        kprintf("This is as->as_vbase2: %x\n",as->as_vbase2);
        kprintf("This is the fault address: %x\n",faultaddress);
#endif
      }
    }
  }

  if (result) {
#if 0
    kprintf("THIS IS FAULINT LIKE CRAXY \n");
    kprintf("This is USERSTACK - as->heapstart: %x\n",USERSTACK-as->heapstart);
    kprintf("This is USERSTACK - as->stackEnd: %x\n",USERSTACK - as->stackEnd);
    kprintf("This is as->heapend - as->heapstart: %x\n",as->heapend-as->heapstart);
    kprintf("This is as->as_vbase1: %x\n",as->as_vbase1);
    kprintf("This is as->as_vbase2: %x\n",as->as_vbase2);
    kprintf("This is the fault address: %x\n",faultaddress);
    kprintf("This is as->as_vbase1 + as->as_npages1 * PAGE_SIZE: %x\n",as->as_vbase1 + as->as_npages1 * PAGE_SIZE);
    kprintf("This is as->as_vbase2 + as->as_npages2 * PAGE_SIZE: %x\n",as->as_vbase2 + as->as_npages2 * PAGE_SIZE);
    kprintf("DONE\n");
#endif
    splx(spl);
    return EFAULT;
  }

#if 0
	/* Assert that the address space has been set up properly. */
	assert(as->as_vbase1 != 0);
	assert(as->as_pbase1 != 0);
	assert(as->as_npages1 != 0);
	assert(as->as_vbase2 != 0);
	assert(as->as_pbase2 != 0);
	assert(as->as_npages2 != 0);
	assert(as->as_stackpbase != 0);
	assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
	assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
	assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
	assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);

	vbase1 = as->as_vbase1;
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;

  int result = -1;
  result = findFrame(as, faultaddress, &paddr);
  if (result) {
    if (faultaddress >= vbase1 && faultaddress < vtop1) {
      paddr = (faultaddress - vbase1) + as->as_pbase1;
      result = 0;
    }
    else if (faultaddress >= vbase2 && faultaddress < vtop2) {
      paddr = (faultaddress - vbase2) + as->as_pbase2;
      result = 0;
    }
    else if (onStack(as, faultaddress)) {
      result = increaseStack(as, faultaddress, &paddr);
    }
    else {
      splx(spl);
      return EFAULT;
    }
  }

  if (result) {
    splx(spl);
    return EFAULT;
  }
#endif

	/* make sure it's page-aligned */
	assert((paddr & PAGE_FRAME)==paddr);

	for (i=0; i<NUM_TLB; i++) {
		TLB_Read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
		ehi = faultaddress;
		elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		TLB_Write(ehi, elo, i);
		splx(spl);
		return 0;
	}

	kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
	splx(spl);
	return EFAULT;
}


static void swapper(void)
{
int numfree;
int spl;
while (1)
	{
	lock_acquire(&swap_lock);
		spl = splhigh();
        	numfree = coremap.numofpageleft;
        	splx(spl);
	while(numfree>0)
		{
		cv_broadcast(&cv_needmore, &swap_lock);
		cv_wait(&cv_needevict, &swap_lock);
			spl = splhigh();
        		numfree = coremap.numofpageleft;
        		splx(spl);
		}

		spl=splhigh();
		
		splx(spl);
	lock_release(&swap_lock);	
	}
}

int configure (struct lvtwopagetable* old, struct lvtwopagetable* new, struct address* as) {
  int i;
  int paddr;
  struct page* unallocatedPageFrame = NULL;
  for (i = 0; i < 1024; i++) {
    new->entry[i] = old->entry[i];
    if ((new->entry[i]).pagePtr != NULL) {
      if (getOnePhysicalFrame(&unallocatedPageFrame)) return -1;
      if (map(&unallocatedPageFrame, &(new->entry[i]), as)) return -1;
      assert((new->entry[i]).pageframeadd == (old->entry[i]).pageframeadd);
      paddr = coremap.FirstPageAdd + (unallocatedPageFrame - coremap.firstPage)*PAGE_SIZE;
      new->entry[i].pageframeadd = paddr;
      memmove((void*)PADDR_TO_KVADDR(paddr), (const void*)PADDR_TO_KVADDR((old->entry[i]).pageframeadd), PAGE_SIZE);
      unallocatedPageFrame->va = ((old->entry[i]).pagePtr)->va;
    }
  }
  return 0;
}

int map (struct page** frame, struct pagetableentry* pte, struct addrspace* as) {
  assert(pte->pageframeadd == (pte->pageframeadd)&0x3ff);
  assert((*frame)->allocated == 1);
  pte->pagePtr = *frame;
  (*frame)->isKernelPage = 0;
  (*frame)->as = as;
  (*frame)->pte = pte;
  (*frame)->consec = 1;
}

void freePage (struct page* pagePtr) {
  assert(pagePtr != NULL);
  assert(pagePtr->consec == 1);
  assert(pagePtr->isKernelPage == 0);
  assert(pagePtr->as != NULL);
  assert(pagePtr->pte != NULL);
  pagePtr->as = NULL;
  pagePtr->pte = NULL;
  pagePtr->va = 0;
  pagePtr->allocated = 0;
  pagePtr->consec = 0;
  pagePtr->isKernelPage = 0;
}
