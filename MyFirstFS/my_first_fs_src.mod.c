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
	{ 0xcbbb05e8, "d_make_root" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x758557ac, "inc_nlink" },
	{ 0x9ec6ca96, "ktime_get_real_ts64" },
	{ 0x7086ff56, "inode_init_owner" },
	{ 0xe953b21f, "get_next_ino" },
	{ 0x69181652, "new_inode" },
	{ 0xa91cceb7, "mount_bdev" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "C8E9DCCBCEE334F6B6BFB45");
