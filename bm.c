#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define MMA8452_DRV_NAME "gs_mma8452"

/** MMA8452 registers **/
 
enum {
	MMA8452_STATUS = 0x00,
	MMA8452_OUT_X_MSB,
	MMA8452_OUT_X_LSB,
	MMA8452_OUT_Y_MSB,
	MMA8452_OUT_Y_LSB,
	MMA8452_OUT_Z_MSB,
	MMA8452_OUT_Z_LSB,
	
	MMA8452_SYSMOD = 0x0B,
	MMA8452_INT_SOURCE,
	MMA8452_WHO_AM_I,
	MMA8452_XYZ_DATA_CFG,
	MMA8452_HP_FILTER_CUTOFF,
	
	MMA8452_PL_STATUS,
	MMA8452_PL_CFG,
	MMA8452_PL_COUNT,
	MMA8452_PL_BF_ZCOMP,
	MMA8452_PL_P_L_THS_REG,
	
	MMA8452_FF_MT_CFG,
	MMA8452_FF_MT_SRC,
	MMA8452_FF_MT_THS,
	MMA8452_FF_MT_COUNT,

	MMA8452_TRANSIENT_CFG = 0x1D,
	MMA8452_TRANSIENT_SRC,
	MMA8452_TRANSIENT_THS,
	MMA8452_TRANSIENT_COUNT,
	
	MMA8452_PULSE_CFG,
	MMA8452_PULSE_SRC,
	MMA8452_PULSE_THSX,
	MMA8452_PULSE_THSY,
	MMA8452_PULSE_THSZ,
	MMA8452_PULSE_TMLT,
	MMA8452_PULSE_LTCY,
	MMA8452_PULSE_WIND,
	
	MMA8452_ASLP_COUNT,
	MMA8452_CTRL_REG1,
	MMA8452_CTRL_REG2,
	MMA8452_CTRL_REG3,
	MMA8452_CTRL_REG4,
	MMA8452_CTRL_REG5,
	
	MMA8452_OFF_X,
	MMA8452_OFF_Y,
	MMA8452_OFF_Z,
	
	MMA8452_REG_END,
};

/** Scale modes **/

enum {
	MODE_2G = 0,
	MODE_4G,
	MODE_8G,
};

static struct i2c_client *bm_client;

ssize_t bm_read(struct file *f, char __user *buf, size_t count, loff_t *offp) 
{	
	unsigned char data[6];		
	unsigned long pos = *offp;	

	int status = i2c_smbus_read_byte_data(bm_client, MMA8452_STATUS);

	if(status & (1<<3)) {		
		data[0] = i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_X_MSB);
		data[1] = i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_X_LSB);
		data[2] = i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_Y_MSB);
		data[3] = i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_Y_LSB);
		data[4] = i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_Z_MSB);
		data[5] = i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_Z_LSB);
	}

	/*
	 *
	 *

	printk(KERN_INFO "BM: STATUS is %x\n", i2c_smbus_read_byte_data(bm_client, MMA8452_STATUS));

	printk(KERN_INFO "BM: X_MSB is %x\n", i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_X_MSB));
	printk(KERN_INFO "BM: X_LSB is %x\n", i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_X_LSB));
	printk(KERN_INFO "BM: Y_MSB is %x\n", i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_Y_MSB));
	printk(KERN_INFO "BM: Y_LSB is %x\n", i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_Y_LSB));
	printk(KERN_INFO "BM: Z_MSB is %x\n", i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_Z_MSB));
	printk(KERN_INFO "BM: Z_LSB is %x\n", i2c_smbus_read_byte_data(bm_client, MMA8452_OUT_Z_LSB));

	*
	*
	*/

	if (pos + count > sizeof(data))
		count = sizeof(data) - pos;
	if (copy_to_user(buf, data + pos, count))
		return -EFAULT;
	*offp += count;

	return count;
}


static struct file_operations bm_fops = {
        .owner = THIS_MODULE,
	.read = bm_read,
};

static struct miscdevice bm_device = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "bm",
        .fops = &bm_fops,
};

static int bm_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk(KERN_INFO "BM: Chip ID is 0x%x\n", i2c_smbus_read_byte_data(client, MMA8452_WHO_AM_I));

	/** MMA8452 chip initialization **/
	// TODO: check write return values
	i2c_smbus_write_byte_data(client, MMA8452_CTRL_REG2, 0x40); // Device reset
	mdelay(20); // Waiting for reset
	i2c_smbus_write_byte_data(client, MMA8452_CTRL_REG1, 0x39); // Output data rate set with period 640ms
	i2c_smbus_write_byte_data(client, MMA8452_XYZ_DATA_CFG, MODE_2G); // Full scale mode = 2g
	i2c_smbus_write_byte_data(client, MMA8452_CTRL_REG2, 0x00); // Device activation in normal mode (power)
	i2c_smbus_write_byte_data(client, MMA8452_CTRL_REG4, 0x00); // Disable all interrupts	
	
	/** Initial Debug (verification of activation by reading status) **/
	
	/*
	 *
	 *
	
	mdelay(5000);

	printk(KERN_INFO "BM: STATUS is %x\n", i2c_smbus_read_byte_data(client, MMA8452_STATUS));

	printk(KERN_INFO "BM: X_MSB is %x\n", i2c_smbus_read_byte_data(client, MMA8452_OUT_X_MSB));
	printk(KERN_INFO "BM: X_LSB is %x\n", i2c_smbus_read_byte_data(client, MMA8452_OUT_X_LSB));
	printk(KERN_INFO "BM: Y_MSB is %x\n", i2c_smbus_read_byte_data(client, MMA8452_OUT_Y_MSB));
	printk(KERN_INFO "BM: Y_LSB is %x\n", i2c_smbus_read_byte_data(client, MMA8452_OUT_Y_LSB));
	printk(KERN_INFO "BM: Z_MSB is %x\n", i2c_smbus_read_byte_data(client, MMA8452_OUT_Z_MSB));
	printk(KERN_INFO "BM: Z_LSB is %x\n", i2c_smbus_read_byte_data(client, MMA8452_OUT_Z_LSB));
	
	*	
	*	
	*/

	bm_client = client;

	// TODO: check registration return value
	misc_register(&bm_device);

	return 0;
} 

static int bm_remove(struct i2c_client *client) 
{
	return 0;
}

static const struct i2c_device_id bm_id[] = {
	{ MMA8452_DRV_NAME, 0 },
	{ }
};

static struct i2c_driver bm_driver = {
	.probe		= bm_probe,
	.remove		= bm_remove,
	.id_table	= bm_id,
	.driver = {
		.name = MMA8452_DRV_NAME,
	},
};

static int __init bm_init(void)
{	
	return i2c_add_driver(&bm_driver);
}

static void __exit bm_exit(void)
{
	i2c_del_driver(&bm_driver);
}

module_init(bm_init);
module_exit(bm_exit);

MODULE_LICENSE("GPL");
