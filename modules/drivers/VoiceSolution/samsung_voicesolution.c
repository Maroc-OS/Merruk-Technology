/*******************************************************************************

*******************************************************************************/

#include <asm/uaccess.h>
#include <asm/system.h>

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/capability.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/in.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>

#include <net/arp.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/ip_fib.h>


#define SOLUTION_RUN  1
#define SOLUTION_STOP 2
#define IOCTL_NAS_READ_WRITE	_IOWR(0xA1, 0x01, int)

#define GDEBUG
#ifdef  GDEBUG
#    define dprintk( x... )  printk(KERN_INFO x )
#else
#    define dprintk( x... )
#endif


int voicesolution_open(struct inode *inode, struct file *filp)
{
    int ret = -ENODEV;

    if( !try_module_get(THIS_MODULE) )
        goto out;	

    ret = 0;

out:	
    return ret;    
}

int voicesolution_release(struct inode *inode, struct file *filp)
{
    module_put(THIS_MODULE);
    return 0;
}

ssize_t voicesolution_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    return 0;
}


int voicesolution_start(unsigned int cmd)
{
    printk(KERN_INFO "voicesolution_start: %d\n", cmd); 
    return 0;
}


int voicesolution_stop(unsigned int cmd)
{
    printk(KERN_INFO "voicesolution_stop: %d\n", cmd); 
    return 0;
}


int voicesolution_callback(unsigned int cmd)
{
    printk(KERN_INFO "voicesolution_callback: %d\n", cmd); 
    return 0;
}


int solution_ioctl(unsigned int cmd, void __user *arg)
{
    int err;
    int data;

    // user 영역으로부터 kernel 영역으로 data copy:: cmd tuning 용도
    err = copy_from_user(&data, arg, sizeof(int));

    if( err ) return -EFAULT;

    switch( data )
    {
        case SOLUTION_RUN:
            printk(KERN_INFO "SOLUTION_RUN\n" );     
            voicesolution_start(SOLUTION_RUN);
            break;

        case SOLUTION_STOP:
            printk(KERN_INFO "SOLUTION_STOP\n" );  
            voicesolution_stop(SOLUTION_STOP);
            break;

        default:
            err = -EFAULT;
            break;
    }

    if( err != -EFAULT )
    {
        err = copy_to_user(arg, &data, sizeof(int));
    }

    if(err) err = -EFAULT;
    else    err = 0;

    return err;
}


int voicesolution_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{    
    int err = 0;

    switch (cmd)
    {
        case IOCTL_NAS_READ_WRITE:
            err = solution_ioctl( cmd, (void __user *)arg );
            break;
            
        default:    	
            err = -ENOIOCTLCMD;
            break;
    }

    return err;
}


ssize_t voicesolution_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    return 0;
}


struct file_operations voicesolution_fops = {
    .owner    = THIS_MODULE,
    .open     = voicesolution_open,
    .read	= voicesolution_read,  //
    .write    = voicesolution_write,  //
    .ioctl	= voicesolution_ioctl,
    .release  = voicesolution_release,
};


struct miscdevice voicesolution_miscdev = {
    253, "voicesolution", &voicesolution_fops
};


static int __init
voicesolution_init(void)
{
    int ret = 0;

    ret = misc_register(&voicesolution_miscdev);
    if( ret < 0 )
    {
        printk(KERN_ERR "voicesolution misc driver register error\n");
        goto err;
    }

    printk("--------------simple misc device <voicesolution> INIT ------------------\n");

    return 0;

err: 
    return ret;
}


static void __exit
voicesolution_exit(void)
{
	misc_deregister(&voicesolution_miscdev);
	printk("--------------simple misc device <voicesolution> EXIT ------------------\n");
}


module_init(voicesolution_init);
module_exit(voicesolution_exit);

EXPORT_SYMBOL(voicesolution_start);
EXPORT_SYMBOL(voicesolution_stop);
EXPORT_SYMBOL(voicesolution_callback);

MODULE_AUTHOR("voice solution");
MODULE_LICENSE("GPL");

