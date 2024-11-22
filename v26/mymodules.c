#include <linux/init.h>                                            /*必须要包含的头文件*/
#include <linux/kernel.h>
#include <linux/module.h>                                    /*必须要包含的头文件*/
static int mymodule_init(void)                                //模块初始化函数
{
  printk("hello,my module wored! \n");                    /*输出信息到内核日志*/
  return 0;
}
static void mymodule_exit(void) //模块清理函数
{ 
  printk("goodbye,unloading my module.\n");         /*输出信息到内核日志*/
}  
module_init(mymodule_init);                                //注册初始化函数
module_exit(mymodule_exit);                              //注册清理函数
MODULE_LICENSE("GPL");                              //模块许可声明