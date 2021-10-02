#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xf654425, "module_layout" },
	{ 0x4f8505dd, "unregister_filesystem" },
	{ 0x108c4e2f, "register_filesystem" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0xcbbb05e8, "d_make_root" },
	{ 0x9ec6ca96, "ktime_get_real_ts64" },
	{ 0x7086ff56, "inode_init_owner" },
	{ 0x69181652, "new_inode" },
	{ 0xf05c7b8, "__x86_indirect_thunk_r15" },
	{ 0x3c75f900, "pv_ops" },
	{ 0xdbf17652, "_raw_spin_lock" },
	{ 0xc6e6580f, "__brelse" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xaf38333d, "__bread_gfp" },
	{ 0xa91cceb7, "mount_bdev" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "79651BF9BBCA31ED11AABE2");
