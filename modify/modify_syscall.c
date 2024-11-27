#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <asm/io.h>
#include <asm/unistd.h>  // 包含系统调用号等相关定义
#include <asm/pgtable.h>  // 为了访问页面表相关的函数和定义
#include <linux/uaccess.h>
#include <asm/paravirt.h>
#include <linux/kallsyms.h>

#define sys_No 78  // 需要修改的系统调用号

unsigned long old_sys_call_func;
unsigned long *p_sys_call_table = 0;  // 查找到的 sys_call_table 地址，依据 /boot/System.map 确定
// ffff000008af0698 R sys_call_table
// ffff000008af3a98 R a32_sys_call_table
void (*update_mapping_prot)(phys_addr_t phys, unsigned long virt, phys_addr_t size, pgprot_t prot);
// .rodata segment 区间
unsigned long start_rodata;
unsigned long init_begin;

#define section_size init_begin - start_rodata


//修改指定内核地址范围的内存属性为只读
static inline void protect_memory(void)
{
    update_mapping_prot(__pa_symbol(start_rodata), (unsigned long)start_rodata,
            section_size, PAGE_KERNEL_RO);
}

//修改指定内核地址范围的内存属性为可读可写等
static inline void unprotect_memory(void)
{
    update_mapping_prot(__pa_symbol(start_rodata), (unsigned long)start_rodata,
            section_size, PAGE_KERNEL);
}


// 新的系统调用函数
asmlinkage int hello(/* int *a, int *b */void) {
    int a = (int)current_pt_regs()->regs[0]; // 从寄存器 x0 获取参数 a
    int b = (int)current_pt_regs()->regs[1]; // 从寄存器 x1 获取参数 b
    
    printk("No 78 syscall has changed to hello\n");
    printk("a: %d, b: %d\n", a, b);
    
    return a + b; // 返回 a 和 b 的和
}


// 修改系统调用
void modify_syscall(void) {
    update_mapping_prot = (void *)kallsyms_lookup_name("update_mapping_prot");

    start_rodata = (unsigned long)kallsyms_lookup_name("__start_rodata");
    init_begin = (unsigned long)kallsyms_lookup_name("__init_begin");

    p_sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    printk("sys_call_addr: %p\n", p_sys_call_table);

    old_sys_call_func = (p_sys_call_table[sys_No]);  // 保存原始系统调用函数
    printk("old_sys_call_func: %lx\n", old_sys_call_func);

    unprotect_memory();
    p_sys_call_table[sys_No] = (unsigned long)&hello;
    protect_memory();

    printk("syscall 78 replaced successfully with hello function\n");
}

// 恢复原始的 sys_78 系统调用
void restore_syscall(void) {
    unprotect_memory();
    p_sys_call_table[sys_No] = old_sys_call_func;  // 恢复为原始的系统调用函数
    protect_memory();

    printk("syscall 78 restored to original\n");
}

// 模块初始化
static int mymodule_init(void) {
    printk("Module loading...\n");
    modify_syscall();
    return 0;
}

// 模块退出
static void mymodule_exit(void) {
    printk("Module unloading...\n");
    restore_syscall();
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yuaay");
MODULE_DESCRIPTION("A simple syscall hook module for openEuler ARM");