#ifndef __KRNLDEVICE_H__
#define __KRNLDEVICE_H__

#define CHRDEV_MAJOR_HASH_SIZE 256				/*主设备号最大大小*/

#define MINORBITS	20
#define MINORMASK	((1U << MINORBITS) - 1)

#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))

/*
*驱动数据结构
*/
typedef struct driver
{
	spinlock_t drv_lock;
	struct list_head drv_list;
	uint_t drv_status;
	uint_t drv_flags;
	sem_t drv_sem;								/*驱动程序的信号量*/
	void *drv_safedrv;							/*安全体*/
	void *drv_attrb;							/*属性体*/
	void *drv_private;							/*私有数据指针*/
	struct drv_operations *file_ops;			/*驱动操作函数*/
	struct list_head drv_devicelist;			/*设备链表*/
	uint_t dev_count;							/*设备计数*/
	drvfunc_t  drv_entry;						/*入口函数*/
	drvfunc_t  drv_exit;						/*出口函数*/
	void *rev_extend1;							/*保留扩展1*/
	void *rev_extend2;							/*保留扩展2*/
	char_t drv_name[32];						/*驱动名字*/
}driver_t;

/*
*设备数据结构
*/
typedef struct device
{
	spinlock_t dev_lock;
	struct list_head dev_list;
	sem_t dev_sem;								/*信号量*/
	uint_t dev_status;							/*设备状态*/
	uint_t dev_flags;							/*设备标记*/
	dev_t dev_id;								/*设备id*/
	driver_t *dev_drv;							/*设备驱动指针*/
	void *dev_attrb;							/*设备属性指针*/
	void *dev_private;							/*设备私有数据*/
	void *rev_extend1;							/*保留扩展1*/
	void *rev_extend2;							/*保留扩展2*/
	char_t dev_name[32];						/*设备名字*/
	struct device *next;						/*指向同类设备的下一个次设备*/
}device_t;

struct drv_operations{
	sint_t (*read) (device_t *, char *, size_t, long *);
	sint_t (*write) (device_t *, const char *, size_t, long *);
	long (*unlocked_ioctl) (device_t *, unsigned int, unsigned long);
	int (*open) (device_t *);
};

dev_t register_chrdev(dev_t dev_id, device_t **dev, const char *name);
driver_t *driver_alloc(const char *name, struct drv_operations *ops);
void driver_add_device(driver_t *drv, device_t *dev);
bool_t driver_add_system(driver_t *drv);

void driver_system_init(void);
void driver_load(void);
void driver_list_all(void);

#endif