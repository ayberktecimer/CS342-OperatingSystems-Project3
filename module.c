#include <linux/highmem.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/sched/signal.h>

// Module parameters
static int processpid; 
static unsigned long virtaddr;
module_param (processpid, int, 0);
module_param (virtaddr, ulong, 0);


static void printVirtualMemoryAreas(struct mm_struct *mm) {
	struct vm_area_struct *mmap = mm -> mmap;
	struct vm_area_struct *temp = mmap;

	printk(KERN_INFO "Virtual Memory Areas \n");

	int i = 1;
	do {
		printk(KERN_INFO "Area %d: Start: %lx End: %lx \n", i, temp -> vm_start, temp -> vm_end);
		i++;
	} while ((temp = temp -> vm_next) != NULL);	
}

static void printMemoryLayout(struct mm_struct *mm) {
	printk(KERN_INFO "Virtual Memory Layout \n");

	printk(KERN_INFO "DATA Start: %lx, End: %lx, Size: %lu \n", mm -> start_data, mm -> end_data, (mm -> end_data - mm -> start_data));
	printk(KERN_INFO "STACK Start: %lx, Size: %lu \n", mm -> start_stack, mm -> stack_vm);
	printk(KERN_INFO "HEAP Start: %lx, End: %lx, Size: %lu \n", mm -> start_brk, mm->brk, (mm -> brk - mm -> start_brk));
	printk(KERN_INFO "MAIN ARGUMENTS Start: %lx, End: %lx, Size: %lu \n", mm -> arg_start, mm -> arg_end, (mm -> arg_end - mm -> arg_start));
	printk(KERN_INFO "ENVIRONMENT VARIABLES  Start: %lx, End: %lx, Size: %lu \n", mm -> env_start, mm -> env_end, (mm -> env_end - mm -> env_start));
	printk(KERN_INFO "NUMBER OF FRAMES: %lu \n", mm -> hiwater_rss);
	printk(KERN_INFO "TOTAL VIRTUAL MEMORY: %lu \n", mm -> hiwater_vm);
}

static unsigned long virt2phys(struct mm_struct *mm, unsigned long vpage){
	unsigned long page_offset = vpage & ~PAGE_MASK;
	pgd_t *pgd = pgd_offset(mm, vpage);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return 0;
	printk(KERN_INFO "pgd: %lx ", pgd);

	p4d_t *p4d = p4d_offset(pgd, vpage);
	if (p4d_none(*p4d) || p4d_bad(*p4d))
		return 0;
	printk(KERN_INFO "p4d: %lx ", p4d);

	pud_t *pud = pud_offset(p4d, vpage);
	if (pud_none(*pud) || pud_bad(*pud))
		return 0;
	printk(KERN_INFO "pud: %lx ", pud);

	pmd_t *pmd = pmd_offset(pud, vpage);
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		return 0;
	printk(KERN_INFO "pmd: %lx ", pmd);

	if (!(pte_offset_kernel(pmd, vpage)))
		return 0;

	pte_t *pte = pte_offset_kernel(pmd, vpage);
	printk(KERN_INFO "pte: %lx ", pte);

	if (!(pte_page(*pte)))
		return 0;

	struct page *page = pte_page(*pte);
	unsigned long phys = page_to_phys(page);
	unsigned long phys_p_offset = phys | page_offset;
	pte_unmap(pte);
	return phys_p_offset;
}
int cnt=0;
static void printPageTable2(struct mm_struct *mm){
	printk(KERN_INFO "Page Table Entries \n");
	struct vm_area_struct *vma = 0;
	unsigned long vpage;
	if(mm && mm -> mmap){
		for(vma = mm -> mmap; vma; vma = vma -> vm_next){
			for(vpage = vma -> vm_start; vpage < vma -> vm_end; vpage += PAGE_SIZE){
				unsigned long phys = virt2phys(mm, vpage);
				unsigned long temp_phys = (phys >> 12);
				if(phys == 70368744173568 || temp_phys == 17179869183 )
					phys =0;
				printk("-------------- \n--------------");
				printk("VA: %lx PA: %lx\n", vpage, phys);
				
			}
		}
	}
}

static void translateVA(struct mm_struct *mm, unsigned long virtualAddress){
	printk(KERN_INFO "Searching physical address of %lx \n", virtualAddress);
	struct vm_area_struct *vma = 0;
	unsigned long vpage;
	if(mm && mm -> mmap){
		for(vma = mm -> mmap; vma; vma = vma -> vm_next){
			for(vpage = vma -> vm_start; vpage < vma -> vm_end; vpage += PAGE_SIZE){
				if(vpage == virtualAddress){
					unsigned long phys = virt2phys(mm, virtualAddress);

					printk(KERN_INFO "VA: %lx <--> PA: %lx", virtualAddress, phys);
					return;
				}
			}
		}
		printk(KERN_INFO "Could not found PA for VA: %lx", virtualAddress);
		return;
	}	
}

int init_module (void) {
	printk (KERN_INFO "Memory analyzing module started for PID %u\n", processpid);
	
	struct task_struct *currentTask = &init_task;
	struct mm_struct *currentMM = currentTask -> mm;

	do {
		if (currentTask -> pid == processpid) {
			currentMM = currentTask -> mm;
			printPageTable2(currentMM);
			printVirtualMemoryAreas(currentMM);
			printMemoryLayout(currentMM);
			if(virtaddr)
				translateVA(currentMM, virtaddr);
			else
				printk(KERN_INFO "You did not provide a virtual address!\n");

			break;
		}
	} while ((currentTask = next_task(currentTask)) != &init_task);

	return 0;
}

void cleanup_module (void) {
	printk (KERN_INFO "Memory analyzing module ended.\n");
}
