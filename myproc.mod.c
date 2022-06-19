#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x63e2e522, "module_layout" },
	{ 0x81d07da9, "mmput" },
	{ 0x35fdb46e, "put_page" },
	{ 0xd46253b, "set_page_dirty_lock" },
	{ 0xca35c2d8, "up_read" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xb272e5b5, "get_user_pages" },
	{ 0xc51cb91a, "down_read" },
	{ 0xb50fccdd, "get_task_mm" },
	{ 0xb259ea9e, "init_task" },
	{ 0xb742fd7, "simple_strtol" },
	{ 0x77e2f33, "_copy_from_user" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xf88159e8, "create_proc_entry" },
	{ 0x6364da16, "remove_proc_entry" },
	{ 0x27e1a049, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "DA517038068F29ED80ACE79");
