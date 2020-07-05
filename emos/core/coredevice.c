#include "lmosem.h"

/*用于保护全局设备表的自旋锁*/
static DEFINE_SPINLOCK(chrdevs_lock);
static DEFINE_SPINLOCK(drvlist_lock);

static void device_init(device_t *dev)
{
	spin_lock_init(&dev->dev_lock);
	init_list_head(&dev->dev_list);
	krnl_sem_init(&dev->dev_sem, 1);
	dev->dev_status = 0;
	dev->dev_flags = 0;
	dev->dev_id = 0;
	dev->dev_drv = NULL;
	dev->dev_attrb = NULL;
	dev->dev_private = NULL;
	dev->rev_extend1 = NULL;
	dev->rev_extend2 = NULL;
	memset(dev->dev_name, 0, sizeof(dev->dev_name));
	dev->next = NULL;
}


/*
* register_chrdev_region() - 注册一组设备号
* @dev_id:  希望注册的设备号
* @dev: 申请的设备结构
* @name： 设备名字
* 返回非0值表示成功，返回0表示失败
*/
dev_t register_chrdev(dev_t dev_id, device_t **dev, const char *name)
{
	device_t *cd, **cp;
	u32_t major, minor;
	cpuflg_t flag;
	int i;

	cd = (device_t *)krnl_alloc(sizeof(device_t));

	if (cd == NULL)
		return 0;

	device_init(cd);

	spin_lock_save(&chrdevs_lock, flag);

	major = MAJOR(dev_id);
	minor = MINOR(dev_id);

	/*当设备号为0时，表示自动寻找设备号*/
	if (major == 0)
	{
		for (i = ARRAY_SIZE(chrdevs)-1; i >= 0; i--)
		{
			if (chrdevs[i] == NULL)
				break;
		}

		if(i == 0)
		{

			goto out;
		}

		major = i;
		minor = 0;
	}

	cd->dev_id = MKDEV(major, minor);

	strlcpy(cd->dev_name, name, sizeof(cd->dev_name));

	i = major;

	/*遍历主设备号下所有的设备，如果设备号相等，则注册失败*/
	for (cp = &chrdevs[i]; *cp; cp = &(*cp)->next)
	{
		/*设备号已经被申请*/
		if((*cp)->dev_id == dev_id)
			goto out;

		/*保证设备号是有序的*/
		if((*cp)->next)
		{
			if((*cp)->dev_id < dev_id && (*cp)->next->dev_id > dev_id)
				break;
		}

	}

	/*加入设备到链表*/
	cd->next = *cp;
	*cp = cd;

	*dev = cd;
	spin_unlock_restore(&chrdevs_lock, flag);
	return cd->dev_id;
out:
	*dev = NULL;
	spin_unlock_restore(&chrdevs_lock, flag);
	krnl_free((adr_t)cd, sizeof(device_t));
	return 0;
}

KLINE sint_t default_read(device_t *dev, char *buf, size_t count, long *ppos){ return DFCERRSTUS;}
KLINE sint_t default_write(device_t *dev, const char *buf, size_t count, long *ppos){ return DFCERRSTUS;}
KLINE long default_ioctl(device_t *dev, unsigned int cmd, unsigned long arg){ return DFCERRSTUS;}
KLINE int default_open(device_t *dev){ return DFCERRSTUS;}

static void driver_init(driver_t *drv)
{
	spin_lock_init(&drv->drv_lock);
	init_list_head(&drv->drv_list);
	krnl_sem_init(&drv->drv_sem, 1);
	drv->drv_status = 0;
	drv->drv_flags = 0;
	drv->drv_private = NULL;
	drv->file_ops = NULL;
	init_list_head(&drv->drv_devicelist);
	drv->dev_count = 0;
	drv->drv_entry = NULL;
	drv->drv_exit = NULL;
	drv->rev_extend1 = NULL;
	drv->rev_extend2 = NULL;
	memset(drv->drv_name, 0, sizeof(drv->drv_name));		
}


/*
* driver_add_device() - 向驱动添加设备
* @drv:  驱动结构
* @dev: 设备结构
*/

void driver_add_device(driver_t *drv, device_t *dev)
{
	cpuflg_t flag;
	struct list_head *list;

	list_for_each(list, &drv->drv_devicelist)
		if(list == &dev->dev_list)							/*设备已经在设备表，则不添加*/
			return;

	spin_lock_save(&chrdevs_lock, flag);

	dev->dev_drv = drv;

	/*将设备添加到驱动的链表*/
	list_add(&dev->dev_list, &drv->drv_devicelist);

	spin_unlock_restore(&chrdevs_lock, flag);
}

/*
* driver_alloc() - 分配一个driver结构
* name: 驱动名
* ops: 驱动操作函数集
* 返回driver结构，NULL表示失败，非NULL表示成功
*/
driver_t *driver_alloc(const char *name, struct drv_operations *ops)
{
	driver_t *drv;

	drv = (driver_t *)krnl_alloc(sizeof(driver_t));

	if(drv == NULL)
		return NULL;

	driver_init(drv);

	drv->file_ops = ops;

	if(!ops->read)
		drv->file_ops->read = default_read;
	if(!ops->write)
		drv->file_ops->write = default_write;
	if(!ops->unlocked_ioctl)
		drv->file_ops->unlocked_ioctl = default_ioctl;
	if(!ops->open)
		drv->file_ops->open = default_open;

	strlcpy(drv->drv_name, name, sizeof(drv->drv_name));

	return drv;
}

/*
* driver_add_system() -向内核添加驱动
* @drv: 需要添加的驱动
*
*/
bool_t driver_add_system(driver_t *drv)
{
	cpuflg_t flag;
	struct list_head *list;

	spin_lock_save(&drvlist_lock, flag);

	list_for_each(list, &drvlist)
		if(list == &drv->drv_list)								/*驱动已经在驱动表，则不添加*/
			return FALSE;

	list_add_tail(&drv->drv_list, &drvlist);

	spin_unlock_restore(&drvlist_lock, flag);

	return TRUE;
}

/*
*driver_system_init() - 初始化驱动架构全局数据结构
*/
void driver_system_init(void)
{
	for(u32_t i = 1; i < ARRAY_SIZE(chrdevs); i++)
		chrdevs[i] = NULL;

	init_list_head(&drvlist);
}

/*
* driver_load() - 加载内核驱动程序
*/
void driver_load(void)
{
	drvfunc_t drv_func;
	for(uint_t i = 0; drventrytlb[i] != NULL; i++)
	{
		drv_func = drventrytlb[i];
		if(drv_func() == DFCERRSTUS)
			system_error("load device driver error!\n");
	}
}

/*
* driver_list_all() 打印内核所有的驱动及其下管理的设备
*/
void driver_list_all(void)
{
	struct list_head *drv_list, *dev_list;
	driver_t *drv;
	device_t *dev;

	printk("drvier\tdevice\t\tmajor\tminor\n");

	list_for_each(drv_list, &drvlist)
	{
		drv = list_entry(drv_list, driver_t, drv_list);

		printk("%s", drv->drv_name);

		list_for_each(dev_list, &drv->drv_devicelist)
		{
			dev = list_entry(dev_list, device_t, dev_list);

			printk("\t%s\t\t%d\t%d\n", dev->dev_name, MAJOR(dev->dev_id), MINOR(dev->dev_id));
		}
	}
}