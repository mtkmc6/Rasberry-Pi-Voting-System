#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif
   
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h> // for kthreads
#include <linux/sched.h> // for task_struct
#include <linux/time.h> // for using jiffies
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#define MSG_SIZE 50
#define CDEV_NAME "Chardevice" // "Custom Character Device made on terminal"


MODULE_LICENSE("GPL");
int mydev_id;
int delay=0;

static int major;
static char msg[MSG_SIZE];

// structure for the kthread.
static struct task_struct *kthread1;


static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
// Whatever is in msg will be placed into buffer, which will be copied into user space
ssize_t dummy = copy_to_user(buffer, msg, length); // dummy will be 0 if successful

// msg should be protected (e.g. semaphore). Not implemented here, but you can add it.
msg[0] = '\0'; // "Clear" the message, in case the device is read again.
// This way, the same message will not be read twice.
// Also convenient for checking if there is nothing new, in user space.
return length;
}



static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
	ssize_t dummy;
	if(len > MSG_SIZE)
		return -EINVAL;
// unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
	dummy = copy_from_user(msg, buff, len); // Transfers the data from user space to kernel space
	if(len == MSG_SIZE)
		msg[len-1] = '\0'; // will ignore the last character received.
	else
		msg[len] = '\0';
	// You may want to remove the following printk in your final version.
	printk("Message from user space: %s\n", msg);
	if (msg[0]=='A') {
		delay= 250;
	}
	if (msg[0]=='B') {
		delay= 500;
	}
	if (msg[0]=='C') {
		delay= 750;
	}
	if (msg[0]=='D') {
		delay= 1000;
	}
	if (msg[0]=='E') {
		delay= 1500;
	}
	return len; // the number of bytes that were written to the Character Device.
}

// structure needed when registering the Character Device. Members are the callback
// functions when the device is read from or written to.
static struct file_operations fops = {
.read = device_read,
.write = device_write,
};



static irqreturn_t button_isr(int irq, void *dev_id)
{
disable_irq_nosync(79);
unsigned long* gpeds0=(unsigned long*)ioremap(0x3F200040,4096);
//changing delay, changes frequency
	if(ioread32(gpeds0)== 0b10000000000000000){
		delay=1500;
	}
	if(ioread32(gpeds0)== 0b100000000000000000){
		delay=1000;
	}
	if(ioread32(gpeds0)== 0b1000000000000000000){
		delay=750;
	}
	if(ioread32(gpeds0)== 0b10000000000000000000){
		delay=500;
	}
//Turn off buttons
	if(ioread32(gpeds0)== 0b100000000000000000000){
		delay=250;
	}
	
	iowrite32(0x1F0000,gpeds0); //Clears event detection
	printk("Interrupt handled\n");
	enable_irq(79); // re-enable interrupt
    return IRQ_HANDLED;
}

// Function to be associated with the kthread; what the kthread executes.
int kthread_fn(void *ptr)

{
	unsigned long *gpset0=(unsigned long*)ioremap(0x3F20001C,4096);
	unsigned long*gpclr0=(unsigned long*)ioremap(0x3F200028,4096);
	unsigned long j0, j1;
	int count = 0;

	printk("In kthread1\n");
	j0 = jiffies; // number of clock ticks since system started;
	// current "time" in jiffies
	j1 = j0 + 10*HZ; // HZ is the number of ticks per second, that is
	// 1 HZ is 1 second in jiffies
	while(time_before(jiffies, j1)) // true when current "time" is less than j1
        schedule(); // voluntarily tell the scheduler that it can schedule
	// some other process
	printk("Before loop\n");
	// The ktrhead does not need to run forever. It can execute something
	// and then leave.
	while(1)
	{
		iowrite32(0b1000000,gpset0);
		udelay(delay); // good for > 10 ms
		iowrite32(0b1000000,gpclr0);
		udelay(delay);
	
	if(kthread_should_stop()) {
		do_exit(0);
	}
	// comment out if your loop is going "fast". You don't want to
	// printk too often. Sporadically or every second or so, it's okay.
	printk("Count: %d\n", ++count);
	}
	return 0;
}

int init_module(void)

{
	int dummy = 0;
	unsigned long*gpsel0=(unsigned long*)ioremap(0x3F200000,4096);
	unsigned long*gparen0=(unsigned long*)ioremap(0x3F20007C,4096);
	unsigned long*gpud=(unsigned long*)ioremap(0x3F200094,4096);
	unsigned long*gpudclk0=(unsigned long*)ioremap(0x3F200098,4096);
	unsigned long*gpsel1=(unsigned long*)ioremap(0x3F200004,4096);
	unsigned long*gpsel2=(unsigned long*)ioremap(0x3F200008,4096);


	iowrite32(0x0, gpsel1); //Sets BCM 16-19 to input
	iowrite32(0x0, gpsel2); //Sets BCM 20 to input
	iowrite32(0b1001001001001000000,gpsel0); //Sets speaker to output
	iowrite32(0x1,gpud); //Enable Pull down clock, which converts 01 to Hex.
	udelay(100);
	iowrite32(0x1F0000,gpudclk0);   //111110000000000000000 converted to Hex. Sets clock to BCM 16-20
	udelay(100);
	iowrite32(0x0,gpud);  //Turns off clk signal
	iowrite32(0x1F0000,gpudclk0); //Turns off clk for each push button
	iowrite32(0x1F0000,gparen0);  //111110000000000000000 converted to Hex. Sets BCM 16-20 to asynchronous
	major = register_chrdev(0, CDEV_NAME, &fops);
	if (major < 0) {
		printk("Registering the character device failed with %d\n", major);
		return major;
	}
	printk("Lab6_cdev_kmod example, assigned major: %d\n", major);
	printk("Create Char Device (node) with: sudo mknod /dev/%s c %d 0\n", CDEV_NAME, major);
	char kthread_name[11]="my_kthread"; // try running  ps -ef | grep my_kthread
	// when the thread is active.
	printk("In init module\n");
       
    kthread1 = kthread_create(kthread_fn, NULL, kthread_name);
    if((kthread1)) // true if kthread creation is successful
    {
        printk("Inside if\n");
		// kthread is dormant after creation. Needs to be woken up
        wake_up_process(kthread1);
    }
   
    dummy = request_irq(79, button_isr, IRQF_SHARED, "Button_handler", &mydev_id);

    return 0;
}

void cleanup_module(void) {
	int ret;
	unsigned long*gpclr0=(unsigned long*)ioremap(0x3F200028,4096);
	unsigned long* gpeds0=(unsigned long*)ioremap(0x3F200040,4096);

	iowrite32(0x1F0000,gpeds0); //Clears event detection just in case
	iowrite32(0b1000000,gpclr0);
	// the following doesn't actually stop the thread, but signals that
	// the thread should stop itself (with do_exit above).
	// kthread should not be called if the thread has already stopped.
	ret = kthread_stop(kthread1);
	if(!ret){
		printk("Kthread stopped\n");
	}
	free_irq(79, &mydev_id);
	unregister_chrdev(major, CDEV_NAME);
	printk("Char Device /dev/%s unregistered.\n", CDEV_NAME);
}



//module_init(init_module);
//module_exit(cleanup_module);