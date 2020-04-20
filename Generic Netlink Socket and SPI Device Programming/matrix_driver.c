/* Driver for integrating HCSR sensor and LED Matrix - Once the input arguments are obtained via the Generic netlink socket,
the values are used to initialise the sensor pins as well as decide the pattern to display. A kernel thread is spawned if the user 
requests to measure distance.*/


#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/timer.h>
#include <linux/export.h>
#include <net/genetlink.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>    
#include <linux/spi/spidev.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/time.h>
#include <linux/hrtimer.h>



#include "matrix_driver.h"
#include "hcsr.h"

#define INT_NAME   "gpio_name"

struct FIFO_buf{                      // the fifo buffer
    uint64_t timestamp;
    unsigned long long int distance[5];
    
} buffer;




#define BUS_NUM 1

struct spidev_data {
   
struct spi_device *spi;

int echo;

int trigger;

long period;

int samples;
unsigned int irq_handler;

struct FIFO_buf buffer[1];

uint64_t diff[5];

uint64_t start;

uint64_t finish;

int enable;

int pattern;

struct task_struct *task1;

}device_data;


static unsigned char ch1_tx, ch2_tx;
struct spi_master *master;
struct message_cmd mcmd;
static struct timer_list timer;
static struct genl_family genl_test_family;
uint64_t avg=0;



/* TSC implementaion to find the clock cycles */

uint64_t get_tsc(void){


uint32_t a,d;
asm volatile("rdtsc" : "=a" (a), "=d" (d));
return (((uint64_t)a)|((uint64_t)d)<<32);
}



static void greet_group(unsigned int group)
{   
    void *hdr;
    int res, flags = GFP_ATOMIC;
    char msg[GENL_TEST_ATTR_MSG_MAX];
    struct sk_buff* skb = genlmsg_new(NLMSG_DEFAULT_SIZE, flags);

  
    if (!skb) {
        printk(KERN_ERR "%d: OOM!!", __LINE__);
        return;
    }

    hdr = genlmsg_put(skb, 0, 0, &genl_test_family, flags, GENL_TEST_C_MSG);
    if (!hdr) {
        printk(KERN_ERR "%d: Unknown err !", __LINE__);
        goto nlmsg_fail;
    }

    snprintf(msg, GENL_TEST_ATTR_MSG_MAX, genl_test_mcgrp_names[group]);
    res = nla_put(skb, GENL_TEST_ATTR_MSG, sizeof(struct message_cmd), (struct message_cmd *)&mcmd);

    //res = nla_put_string(skb, GENL_TEST_ATTR_MSG, msg);
    if (res) {
        printk(KERN_ERR "%d: err %d ", __LINE__, res);
        goto nlmsg_fail;
    }

    genlmsg_end(skb, hdr);
    genlmsg_multicast(&genl_test_family, skb, 0, group, flags);
    return;

nlmsg_fail:
    genlmsg_cancel(skb, hdr);
    nlmsg_free(skb);
    return;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,14,0)
void genl_test_periodic(unsigned long data)
#else
void genl_test_periodic(struct timer_list *unused)
#endif
{
    greet_group(GENL_TEST_MCGRP0);
    greet_group(GENL_TEST_MCGRP1);
    greet_group(GENL_TEST_MCGRP2);

    mod_timer(&timer, jiffies + msecs_to_jiffies(GENL_TEST_HELLO_INTERVAL));
}




static void spi_led_transfer(unsigned char ch1, unsigned char ch2)
{
    ch1_tx = ch1;
    ch2_tx = ch2;
   gpio_set_value_cansleep(40,0);
   spi_write(device_data.spi, &ch1_tx, sizeof(ch1_tx));
   spi_write(device_data.spi, &ch2_tx, sizeof(ch2_tx));
   gpio_set_value_cansleep(40,1);
   return;
}



/* IRQ Handler for the device. The trigger pin  triggers the echo pin for m+2
    samples in delta period */

static irqreturn_t hcsr_handler(int irq, void *dev_id){

if(gpio_get_value(4)){

device_data.start = get_tsc();
}

else{
device_data.finish = get_tsc();
}

return IRQ_HANDLED;

}




/* To initialise the buffer of the device struct */

void reset_buffer(struct FIFO_buf *buffer){

int i;
for(i=0;i<5;i++){
    buffer[i].timestamp =0;
    buffer[i].distance[0] =0;

    }
}



// /*Interrupt release function*/

int r_int_release(void* handle)
{
struct spidev_data *hcsr04_devp = (struct spidev_data *)handle;
if(hcsr04_devp->irq_handler!=0)
free_irq(hcsr04_devp->irq_handler, (void *)(hcsr04_devp));
return 0;
}




bool buffer_empty(struct FIFO_buf *buffer){

int i,flag=0;
for(i=0;i<1;i++){
    if(buffer[i].timestamp!=0)
        flag = 1;

}
return flag;
}



void gpio_setup(void){


/*SPI config */
//printk(KERN_INFO "I should be lighting up now!\n");

gpio_request(40, "sysfs");
gpio_direction_output(40, 0);
gpio_set_value_cansleep(40, 1);


// gpio_request(44, "sysfs");
// gpio_set_value_cansleep(44, 1);

// gpio_request(72, "sysfs");
// gpio_set_value_cansleep(72, 0);


// gpio_request(46, "sysfs");
// gpio_set_value_cansleep(46, 1);


/* HCSR config */

/*trigger pin */
//{ 0, 11,32 }
gpio_request(11 , "sysfs");
gpio_direction_output(11 , 0);

gpio_request(32, "sysfs");
gpio_direction_output(32, 0);

/*echo pin */
//{ 9, 4, 70,22 }
gpio_request(4, "sysfs");
gpio_direction_input(4);

gpio_request(22, "sysfs");
gpio_direction_output(22,1);

gpio_request(70, "sysfs");
gpio_set_value_cansleep(70, 0);
        
}


static int thread_SPI_Matrix(int i){

int val;

if(device_data.buffer[0].distance[i] >= 0 && device_data.buffer[0].distance[i] <= 15)
    val = 0;

else if (device_data.buffer[0].distance[i] > 15 && device_data.buffer[0].distance[i] < 30)
   val = 200;

else if (device_data.buffer[0].distance[i] >= 30 && device_data.buffer[0].distance[i] < 60)
val = 500;   

else
    val = 1000;
if(device_data.pattern==2){

spi_led_transfer(0x0F, 0x00);
spi_led_transfer(0x0A, 0x0F);
spi_led_transfer(0x09, 0x00);
spi_led_transfer(0x0B, 0x07);
spi_led_transfer(0x0C, 0x00);

spi_led_transfer(0x01, 0x3c);
spi_led_transfer(0x02, 0x4E);
spi_led_transfer(0x03, 0xCF);
spi_led_transfer(0x04, 0x3F);
spi_led_transfer(0x05, 0x9F);
spi_led_transfer(0x06, 0x3F);
spi_led_transfer(0x07, 0xFE);
spi_led_transfer(0x08, 0x3C);
spi_led_transfer(0x0C, 0x01);

//{0x1c,0x1c,0x08,0x3e,0x49,0x08,0x3e,0x22}

mdelay(val);


spi_led_transfer(0x0F, 0x00);
spi_led_transfer(0x0A, 0x0F);
spi_led_transfer(0x09, 0x00);
spi_led_transfer(0x0B, 0x07);
spi_led_transfer(0x0C, 0x00);

spi_led_transfer(0x01, 0x3c);
spi_led_transfer(0x02, 0x4E);
spi_led_transfer(0x03, 0xCF);
spi_led_transfer(0x04, 0xFF);
spi_led_transfer(0x05, 0x1F);
spi_led_transfer(0x06, 0xFF);
spi_led_transfer(0x07, 0xFE);
spi_led_transfer(0x08, 0x3C);
spi_led_transfer(0x0C, 0x01);

//{0x1c,0x1c,0x08,0x3e,0x49,0x08,0x3e,0x22}

mdelay(val);

}

else{
spi_led_transfer(0x0F, 0x00);
spi_led_transfer(0x0A, 0x0F);
spi_led_transfer(0x09, 0x00);
spi_led_transfer(0x0B, 0x07);
spi_led_transfer(0x0C, 0x00);

// printk("I'm in here to glow!\n");   
spi_led_transfer(0x01, 0x1c);
spi_led_transfer(0x02, 0x1c);
spi_led_transfer(0x03, 0x08);
spi_led_transfer(0x04, 0x3e);
spi_led_transfer(0x05, 0x49);
spi_led_transfer(0x06, 0x08);
spi_led_transfer(0x07, 0x3e);
spi_led_transfer(0x08, 0x22);
spi_led_transfer(0x0C, 0x01);

//{0x1c,0x1c,0x08,0x3e,0x49,0x08,0x3e,0x22}

mdelay(val);


spi_led_transfer(0x0F, 0x00);
spi_led_transfer(0x0A, 0x0F);
spi_led_transfer(0x09, 0x00);
spi_led_transfer(0x0B, 0x07);
spi_led_transfer(0x0C, 0x00);

spi_led_transfer(0x01, 0x1c);
spi_led_transfer(0x02, 0x1c);
spi_led_transfer(0x03, 0x49);
spi_led_transfer(0x04, 0x3e);
spi_led_transfer(0x05, 0x08);
spi_led_transfer(0x06, 0x49);
spi_led_transfer(0x07, 0x3e);
spi_led_transfer(0x08, 0x00);
spi_led_transfer(0x0C, 0x01);

mdelay(val);
//{0x1c,0x1c,0x49,0x3e,0x08,0x49,0x3e,0x00}
}




return 0;

}


static int distance_measure(void *data){

uint64_t timestamp=0, sum=0,i=0,j;

for(j=0;j<5;j++)
device_data.buffer[0].distance[j] =0;
device_data.start =0;
device_data.finish =0;
printk(KERN_INFO"Starting the interrupt handler as there is no on-going measurement.....\n");
while(!kthread_should_stop()){
    if(i>4)
        i=0;
        
    gpio_set_value_cansleep(11,1);
    udelay(100);
    gpio_set_value_cansleep(11,0);
    
    msleep_interruptible(100);
    device_data.diff[i] = device_data.finish - device_data.start;
    sum = device_data.diff[i];
    timestamp= div_u64(sum,1); 
    device_data.buffer[0].timestamp = timestamp;
    timestamp= div_u64(timestamp,400);
    timestamp=timestamp*34000;
    timestamp = div_u64(timestamp,1000000);
    timestamp = div_u64(timestamp,2);
    if(timestamp>200)
       timestamp = 200;
    device_data.buffer[0].distance[i]  = timestamp;
    if(i==4){
         avg =0;
         for(j=0;j<5;j++){
    //printk("\nThe distance is: %d  %llucm\n", i, device_data.buffer[0].distance[i] );
    avg+= device_data.buffer[0].distance[j];
    }
    avg =  div_u64(avg,5);
    mcmd.distance = avg;

    }

    //printk("\nThe distance is: %d  %dcm\n", i, device_data.buffer[0].distance[i] );
    thread_SPI_Matrix(i);
    i++;

   }  

  

return 0;

}

/* If distance is requested, this function creates kthreads that run until distance request is stopped */

void HCSR_write(void){

   

//printk(KERN_INFO"Starting the interrupt handler as there is no on-going measurement.....\n");
device_data.finish = 0;
device_data.start = 0;
device_data.diff[0] = 0;
device_data.diff[1] = 0;
device_data.diff[2] = 0;
device_data.diff[3] = 0;
device_data.diff[4] = 0;

if(device_data.enable){
device_data.task1 = kthread_run(&distance_measure,NULL, "distance");
//device_data.task2 = kthread_run(&thread_SPI_Matrix,NULL, "matrix");
}

//else {

// ret = kthread_stop(device_data.task1);
// if (ret != -EINTR)
//     printk("Distance measurement has stopped\n");
// spi_led_transfer(0x0C, 0x00);



//}

}


static int genl_test_rx_msg(struct sk_buff* skb, struct genl_info* info)
{
struct message_cmd *temp ;
int ret;



  //Register information about your slave device
struct spi_board_info spi_device_info = {
    .modalias = "LEDMatrix",
    .max_speed_hz = 10000,
    .bus_num = BUS_NUM,
    .chip_select = 1,
    .mode = 3,
};


if (!info->attrs[GENL_TEST_ATTR_MSG]) {
    printk(KERN_ERR "empty message from %d!!\n",
        info->snd_portid);
    printk(KERN_ERR "%p\n", info->attrs[GENL_TEST_ATTR_MSG]);
    return -EINVAL;
}
temp = (struct message_cmd *)(nla_data(info->attrs[GENL_TEST_ATTR_MSG]));
    
nla_memcpy(temp, info->attrs[GENL_TEST_ATTR_MSG], sizeof(struct message_cmd));
printk(KERN_NOTICE "\n%u says = trigger pin: GPIO %d   echo pin: GPIO %d  chip select: GPIO %d Pattern: %d  \n", 
info->snd_portid, temp->trigger_pin, temp->echo_pin,temp->chip_pin, temp->pattern);

device_data.trigger = temp->trigger_pin;
device_data.echo = temp->echo_pin;
device_data.enable = temp->enable;
device_data.pattern = temp->pattern;

if(device_data.enable){

gpio_setup();

/* SPI initialisations begin here based on CHip Select received from the user */


/*To send data we have to know what spi port/pins should be used. This information 
can be found in the device-tree. */


master = spi_busnum_to_master(spi_device_info.bus_num);
if(!master){
    printk("Master not found\n");
    return -ENODEV;
}


if(device_data.spi){
    printk(KERN_INFO "Unregistering SPI\n");
    spi_unregister_device(device_data.spi);
}

/* Create a new slave device, given the master and device info */

device_data.spi = spi_new_device(master, &spi_device_info);

if(!device_data.spi){
    printk("Failed to create a slave\n");
    return -ENODEV;
}


device_data.spi->bits_per_word = 8;
device_data.spi->chip_select = 1;
//device_data.spi->master = master;
device_data.spi->mode = 3;
device_data.spi->max_speed_hz = 5000;


ret = spi_setup(device_data.spi);

if( ret ){
    printk("FAILED to setup slave.\n");
    spi_unregister_device( device_data.spi);
    return -ENODEV;
}

//thread_SPI_Matrix();
HCSR_write();
}

else {
if(device_data.task1){
ret = kthread_stop(device_data.task1);
if (ret != -EINTR)
    printk("Distance measurement has stopped\n");
spi_led_transfer(0x0C, 0x00);
}

else
    printk("There is no on going distance measurement to stop, please try ./main -s -m start\n");
}


return 0;
}

static const struct genl_ops genl_test_ops[] = {
    {
        .cmd = GENL_TEST_C_MSG,
        .policy = genl_test_policy,
        .doit = genl_test_rx_msg,
        .dumpit = NULL,
    },
};

static const struct genl_multicast_group genl_test_mcgrps[] = {
    [GENL_TEST_MCGRP0] = { .name = GENL_TEST_MCGRP0_NAME, },
    [GENL_TEST_MCGRP1] = { .name = GENL_TEST_MCGRP1_NAME, },
    [GENL_TEST_MCGRP2] = { .name = GENL_TEST_MCGRP2_NAME, },
};

static struct genl_family genl_test_family = {
    .name = GENL_TEST_FAMILY_NAME,
    .version = 1,
    .maxattr = GENL_TEST_ATTR_MAX,
    .netnsok = false,
    .module = THIS_MODULE,
    .ops = genl_test_ops,
    .n_ops = ARRAY_SIZE(genl_test_ops),
    .mcgrps = genl_test_mcgrps,
    .n_mcgrps = ARRAY_SIZE(genl_test_mcgrps),
};

static int __init genl_test_init(void)
{
    int rc;

    printk(KERN_INFO "genl_test: initializing netlink\n");

    rc = genl_register_family(&genl_test_family);
    if (rc)
        goto failure;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,14,0)
    init_timer(&timer);
    timer.data = 0;
    timer.function = genl_test_periodic;
    timer.expires = jiffies + msecs_to_jiffies(GENL_TEST_HELLO_INTERVAL);
    add_timer(&timer);
#else
    timer_setup(&timer, genl_test_periodic, 0);
    mod_timer(&timer, jiffies + msecs_to_jiffies(GENL_TEST_HELLO_INTERVAL));
#endif

   

device_data.echo = 4;
device_data.trigger = 11;
device_data.period = 100;
device_data.samples = 5;
device_data.irq_handler = 0;
device_data.diff[0] = 0;
device_data.diff[1] = 0;
device_data.diff[2] = 0;
device_data.diff[3] = 0;
device_data.diff[4] = 0;
device_data.pattern = 0;
device_data.task1 = NULL;

if(device_data.irq_handler)
    free_irq(device_data.irq_handler, (void *)&device_data);


//printk("The echo pin is: %d \n",device_data.echo);
device_data.irq_handler = gpio_to_irq(device_data.echo);

if(device_data.irq_handler < 0){
    printk(KERN_INFO "IRQ not registered\n");
    return -1;
}

else
    printk(KERN_INFO "Driver module has been inserted\n");               

if(request_irq(device_data.irq_handler, hcsr_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING  , INT_NAME, (void*)&device_data)<0){
    printk(KERN_DEBUG "IRQ number request failed.\n");
    return -1;
}


return 0;

failure:
    printk(KERN_DEBUG "genl_test: error occurred in %s\n", __func__);
    return -EINVAL;
}



static void genl_test_exit(void)
{

int ret;

if(device_data.spi){
printk(KERN_INFO "Unregistering SPI\n");
spi_unregister_device(device_data.spi);
}

ret = r_int_release((void *)&device_data);
if(ret!=0)
    printk("Couldn't free interrupt\n");

printk(KERN_INFO"\n Removing the Driver module\n");
gpio_set_value_cansleep(40, 0);
gpio_free(40);
// gpio_free(44);
// gpio_free(72);
// gpio_free(46);
gpio_free(11);
gpio_free(32);
gpio_free(4);
gpio_free(22);
gpio_free(70);

del_timer(&timer);
genl_unregister_family(&genl_test_family);
}


module_init(genl_test_init);
module_exit(genl_test_exit);
MODULE_AUTHOR("Alisha Kulkarni <akulka27@asu.edu>");
MODULE_LICENSE("GPL");
