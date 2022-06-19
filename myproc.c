#include <asm/io.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/ptrace.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/delayacct.h>
#include <linux/elf.h>
#include <linux/gfp.h>
#include <linux/highmem.h>
#include <linux/hugetlb.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/ksm.h>
#include <linux/memcontrol.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmu_notifier.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/proc_fs.h>
#include <linux/ptrace.h>
#include <linux/rmap.h>
#include <linux/sched.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/writeback.h>

struct task_struct *task;
#define MAX_PROC_SIZE 100
static char proc_data[MAX_PROC_SIZE];
char *endp;
int ipid = 0;

static struct proc_dir_entry *proc_write_entry;

static int __access_remote_vm(struct task_struct *tsk, struct mm_struct *mm,
                              unsigned long addr, void *buf, int len,
                              int write) {
  struct vm_area_struct *vma;
  void *old_buf = buf;

  down_read(&mm->mmap_sem);
  /* ignore errors, just check how much was successfully transferred */
  while (len) {
    int bytes, ret, offset;
    void *maddr;
    struct page *page = NULL;

    ret = get_user_pages(tsk, mm, addr, 1, write, 1, &page, &vma);
    if (ret <= 0) {
      /*
       *                          * Check if this is a VM_IO | VM_PFNMAP VMA,
       * which
       *                                                   * we can access using
       * slightly different code.
       *                                                                            */
#ifdef CONFIG_HAVE_IOREMAP_PROT
      vma = find_vma(mm, addr);
      if (!vma || vma->vm_start > addr) break;
      if (vma->vm_ops && vma->vm_ops->access)
        ret = vma->vm_ops->access(vma, addr, buf, len, write);
      if (ret <= 0)
#endif
        break;
      bytes = ret;
    } else {
      bytes = len;
      offset = addr & (PAGE_SIZE - 1);
      if (bytes > PAGE_SIZE - offset) bytes = PAGE_SIZE - offset;

      maddr = kmap(page);
      if (write) {
        copy_to_user_page(vma, page, addr, maddr + offset, buf, bytes);
        set_page_dirty_lock(page);
      } else {
        copy_from_user_page(vma, page, addr, buf, maddr + offset, bytes);
      }
      kunmap(page);
      page_cache_release(page);
    }
    len -= bytes;
    buf += bytes;
    addr += bytes;
  }
  up_read(&mm->mmap_sem);

  return buf - old_buf;
}

int access_process_vm(struct task_struct *tsk, unsigned long addr, void *buf,
                      int len, int write) {
  struct mm_struct *mm;
  int ret;

  mm = get_task_mm(tsk);
  if (!mm) return 0;

  ret = __access_remote_vm(tsk, mm, addr, buf, len, write);
  mmput(mm);

  return ret;
}

int read_proc(char *buf, char **start, off_t offset, int count, int *eof,
              void *data) {
  int len = 0;
  int res = 0;
  struct page *page;
  char *my_page_address;
  char *old;

  for_each_process(task) {
    if (task->pid == ipid) {
      // int* addr;
      // addr = task->mm->start_stack-0xF0;
      // int* buf; int len=1; int write=0;
      // access_process_vm(task, addr, buf, len, write);
      unsigned long uaddr;
      uaddr = task->mm->start_stack - 0xFC;
      down_read(&task->mm->mmap_sem);
      res = get_user_pages(task, task->mm, uaddr,
                           1,  // only want 1 page
                           1,  // do want to write into it
                           1,  // do force
                           &page, NULL);
      if (res == 1) {
        my_page_address = kmap(page);
        old = &my_page_address;
        // memset(my_page_address, 7, sizeof(int));
        *((int *)my_page_address) = 777;
        kunmap(page);
        if (!PageReserved(page)) SetPageDirty(page);
        page_cache_release(page);
      }
      len = sprintf(buf,
                    "\nPID:%d\nMAP "
                    "COUNT:%d\nUADDR:%p\nRES:%d\nOld:%d\nNEW:%d\nPAddr:%p\n",
                    task->pid, task->mm->map_count, uaddr, res, *((int *)old),
                    *((int *)my_page_address), my_page_address);
    }
  }
  //   len = sprintf(buf,"\n%s\n",proc_data);
  // up_read(&task->mm->mmap_sem);
  return len;
}

int write_proc(struct file *file, const char *buf, int count, void *data) {
  if (count > MAX_PROC_SIZE) count = MAX_PROC_SIZE;
  if (copy_from_user(proc_data, buf, count)) return -EFAULT;
  ipid = simple_strtol(proc_data, &endp, 10);
  return count;
}

void create_new_proc_entry() {
  proc_write_entry = create_proc_entry("myproc", 0666, (void *)NULL);
  if (!proc_write_entry) {
    printk(KERN_INFO "Error creating proc entry");
    return;
  }
  proc_write_entry->read_proc = (void *)read_proc;
  proc_write_entry->write_proc = (void *)write_proc;
  printk(KERN_INFO "proc initialized");
}

int proc_init(void) {
  create_new_proc_entry();
  return 0;
}

void proc_cleanup(void) {
  printk(KERN_INFO " Inside cleanup_module\n");
  remove_proc_entry("myproc", NULL);
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);
