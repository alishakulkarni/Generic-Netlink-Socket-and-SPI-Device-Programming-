#ifndef HCSR_H
#define HCSR_H
#include <linux/ioctl.h>

#ifdef __KERNEL__
#include <linux/hrtimer.h>
#endif

 
struct config_pins {

int trigger_pin;
int echo_pin;
} pins;


struct set_params{

int samples;
int period;
} params;

#define CONFIG_PINS _IOW('k', 0, struct config_pins *)
#define SET_PARAMETERS _IOW('k', 3, struct set_params *)



#ifdef __KERNEL__
uint64_t get_tsc(void);

int r_int_release(void* handle);

static irqreturn_t hcsr_handler(int irq, void *dev_id);

//void reset_buffer(struct FIFO_buf *buffer);

void timer_callback_work(struct work_struct *work);

enum hrtimer_restart timer_callback( struct hrtimer *temp_hr_timer );

#endif

//bool buffer_empty(struct FIFO_buf *buffer);

#endif