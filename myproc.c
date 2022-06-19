#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/highmem.h> /* for kmap() */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h> /* for copy_to_user_page() */
#include <linux/module.h>
#include <linux/pagemap.h> /* for page_cache_release() */
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/utsname.h>

struct task_struct *task;
#define MAX_PROC_SIZE 100
static char proc_data[MAX_PROC_SIZE];
char *endp;
int ipid = 0;

static struct proc_dir_entry *proc_write_entry;

int read_proc(char *buf, char **start, off_t offset, int count, int *eof,
              void *data) {
  int len = 0;
  int res = 0;
  struct page *page = NULL;
  char *my_page_address = NULL;
  struct mm_struct *mm;
  struct vm_area_struct *vma;
  int new_number = 777;

  for_each_process(task) {
    if (task->pid == ipid) {
      unsigned long uaddr;
      uaddr = task->mm->start_stack - 0xF0; 
      // calculate by the value from printf("%p", &a) in test.c

      mm = get_task_mm(task); // do not try to replace this mm with task->mm
      down_read(&mm->mmap_sem);
      res = get_user_pages(task, mm, uaddr,
                           1,  // only want 1 page
                           1,  // do want to write into it
                           1,  // do force
                           &page, &vma);
      if (res == 1) {
        my_page_address = kmap(page);
        int my_offset = 0;
        my_offset = uaddr & (PAGE_SIZE - 1);
        copy_to_user_page(vma, page, uaddr, my_page_address + my_offset, &new_number,
                          sizeof(int));
        // not work 1: memset(my_page_address, 7, sizeof(int));
        // not work 2: *((int *)my_page_address) = 777;
        set_page_dirty_lock(page);
        kunmap(page);
        page_cache_release(page);
        mmput(mm);
      }
      len = sprintf(buf,
                    "\nPID:%d\nMAP COUNT:%d\nUADDR:%p\nRES:%d\n",
                    task->pid, task->mm->map_count, (void *)uaddr, res);
    }
  }
  up_read(&mm->mmap_sem);
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
