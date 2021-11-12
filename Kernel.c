Lab6
Kellermann, Matthew (MU-Student)
Mon 4/26/2021 7:01 PM
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



MODULE_LICENSE("GPL");
int mydev_id;
int delay=0;

// structure for the kthread.
static struct task_struct *kthread1;




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
//msleep_interruptible(1000); // good for > 10 ms
//udelay(unsigned long usecs); // good for a few us (micro s)
//usleep_range(unsigned long min, unsigned long max); // good for 10us - 20 ms
// In an infinite loop, you should check if the kthread_stop
// function has been called (e.g. in clean up module). If so,
// the kthread should exit. If this is not done, the thread
// will persist even after removing the module.
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
iowrite32(0b1001001001001000000,gpsel0);
iowrite32(0x1,gpud); //Enable Pull down clock, which converts 01 to Hex.
udelay(100);
iowrite32(0x1F0000,gpudclk0);   //111110000000000000000 converted to Hex. Sets clock to BCM 16-20
udelay(100);
iowrite32(0x0,gpud);  //Turns off clk signal
iowrite32(0x1F0000,gpudclk0); //Turns off clk for each push button
iowrite32(0x1F0000,gparen0);  //111110000000000000000 converted to Hex. Sets BCM 16-20 to asynchronous
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
}



//module_init(init_module);
//module_exit(cleanup_module);
	