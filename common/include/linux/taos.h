#ifndef __TAOS_H__
#define __TAOS_H__


/* i2c */
#define I2C_M_WR 0 /* for i2c */
#define I2c_M_RD 1 /* for i2c */

/* sensor gpio */
#define REGS_PROX 	0x0 // Read  Only
#define REGS_GAIN 	0x1 // Write Only
#define REGS_HYS	0x2 // Write Only
#define REGS_CYCLE	0x3 // Write Only
#define REGS_OPMOD	0x4 // Write Only
#define REGS_CON	0x6 // Write Only

/* sensor type */
#define LIGHT           0
#define PROXIMITY	1
#define ALL		2

/* power control */
#define ON              1
#define OFF		0

/* IOCTL for proximity sensor */
#define TAOS_PROX_IOC_MAGIC   'C'                                 
#define TAOS_PROX_OPEN    _IO(TAOS_PROX_IOC_MAGIC,1)            
#define TAOS_PROX_CLOSE   _IO(TAOS_PROX_IOC_MAGIC,2)      

/* input device for proximity sensor */
#define USE_INPUT_DEVICE 	0  /* 0 : No Use  ,  1: Use  */

#define USE_INTERRUPT		1
#define INT_CLEAR    1 /* 0 = by polling operation, 1 = by interrupt operation */

/* Register value  for TMD2771x */    // hm83.cho 100817
#define ATIME 0xff   // 2.7ms - minimum ALS intergration time
#define WTIME 0xff  // 2.7ms - minimum Wait time
#define PTIME  0xff  // 2.7ms - minimum Prox integration time
#define PPCOUNT  1
#define WEN  0x8     // Enable Wait
#define PEN  0x4     // Enable Prox
#define AEN  0x2     // Enable ALS
#define PON 0x1     //Enable Power on
#define PDRIVE 0
#define PDIODE 0x20
#define PGAIN 0
#define AGAIN 0


/* driver data */
struct taos_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct work_struct work_prox;  /* for proximity sensor */
	int             irq;
	struct hrtimer timer;
	struct timer_list light_init_timer;
};
struct taos_data *taos_global;


struct workqueue_struct *taos_wq;

/* prototype */
extern short taos_get_proximity_value(void);
static int proximity_open(struct inode *ip, struct file *fp);
static int proximity_release(struct inode *ip, struct file *fp);
static long proximity_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);


#endif
