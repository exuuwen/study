/****************************************************************
*
* FILENAME
*   evt_dev.c
*
*
* AUTHOR
*
* DATE
*   9 Nov 2010
*
* HISTORY
*   2010.11.09  Rev.1.0  1st revision
*
****************************************************************/
/****************************************************************/
/*          INCLUDE FILES                                       */
/****************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>


#include "evt.h"


/****************************************************************/
/*          MACROS/DEFINES                                      */
/****************************************************************/
struct evt_content {
	/* Configuration parameters */
	int type;		/* input event type (EV_KEY, EV_SW) */
	int code;		/* input event code (KEY_*, SW_*) */
	int value;
};

struct evt_data {
	struct evt_content content;
	struct input_dev *input;
	struct evt_data* prv;
	struct evt_data* next;
};

struct evt_drvdata {
	struct input_dev *input;
	struct evt_data* head;
};
//#define  TEST
/****************************************************************/
/*          VARIABLES                                           */
/****************************************************************/
static struct evt_drvdata *evt_ddata;
static int evt_IsInit = -1;
#ifdef  TEST
static DEV_EVT_CODE code[40];//wx
static struct timer_list timer;
#endif


/****************************************************************/
/*          PUBLIC FUNCTIONS                                    */
/****************************************************************/
DEV_EVT_RESULT_CODE dev_evt_RegEvt(DEV_EVT_TYPE event_type, int event_code, int event_value, DEV_EVT_CODE* code)
{
	DEV_EVT_RESULT_CODE ret = DEV_EVT_OK;
	unsigned int evtype;
	int evcode;
	struct evt_data* pdata = NULL;
	struct evt_data* tail = NULL;
	
	if(evt_IsInit == -1){
		ret = DEV_EVT_NOT_INITIALISED;
		return ret;
	}
	
	if(event_type < 0 || code == NULL){
		ret = DEV_EVT_INVALID_PARAM;
		return ret;
	}
	
	switch(event_type){
	case EVT_PLUG:
		evtype = EV_SW;
		evcode = event_code;
		break;
	case EVT_SIGNAL:
		evtype = EV_MSC;
		evcode = event_code;
		break;
	case EVT_SYS:
		evtype = EV_KEY;
		evcode = event_code;
		break;
	default:
		ret = DEV_EVT_NOT_SUPPORTED;
		return ret;
		
	}
	
	pdata = evt_ddata->head;
	while(pdata){
		if(pdata->content.type == evtype && pdata->content.code == evcode){
			if(pdata->content.value == event_value){
				ret = DEV_EVT_ALREADY_REGISTERED;
				return ret;
			}
		}
		tail = pdata;
		pdata = pdata->next;
	}
	
	if(evt_ddata->head == NULL){
		evt_ddata->head = kzalloc(sizeof(struct evt_data),GFP_KERNEL);
		if(evt_ddata->head == NULL){
			ret = DEV_EVT_MEMORY_ALLOC_FAIL;
			return ret;
		}
		evt_ddata->head->content.type = evtype;
		evt_ddata->head->content.code = evcode;
		evt_ddata->head->content.value = event_value;
		evt_ddata->head->input = evt_ddata->input;
		evt_ddata->head->prv = NULL;
		evt_ddata->head->next = NULL;
		*code = (unsigned int)evt_ddata->head;
		input_set_capability(evt_ddata->head->input, evt_ddata->head->content.type, evt_ddata->head->content.code);
		//printk("int reg content.type is %d, content.code is %d, content.value is %d\n", evt_ddata->head->content.type, evt_ddata->head->content.code, evt_ddata->head->content.value);
		return ret;
	}
	
	if(tail == NULL){
		ret = DEV_EVT_FAIL;
		return ret;
	}
	
	pdata = kzalloc(sizeof(struct evt_data),GFP_KERNEL);
	if(pdata == NULL){
		ret = DEV_EVT_MEMORY_ALLOC_FAIL;
		return ret;
	}
	pdata->content.type = evtype;
	pdata->content.code = evcode;
	pdata->content.value = event_value;
	pdata->input = evt_ddata->input;
	pdata->prv = tail;
	pdata->next = NULL;
	tail->next = pdata;
	input_set_capability(pdata->input,pdata->content.type,pdata->content.code);
	//printk("int reg content.type is %d, content.code is %d, content.value is %d\n", pdata->content.type, pdata->content.code, pdata->content.value);
	*code = (unsigned int)pdata;
	return ret;
}



DEV_EVT_RESULT_CODE dev_evt_UnRegEvt(DEV_EVT_CODE code)
{
	DEV_EVT_RESULT_CODE ret = DEV_EVT_OK;
	struct evt_data* pdata = NULL;
	struct evt_data* tail = NULL;
	
	if(evt_IsInit == -1){
		ret = DEV_EVT_NOT_INITIALISED;
		return ret;
	}
	
	if(code == 0){
		ret = DEV_EVT_INVALID_PARAM;
		return ret;
	}
	
	pdata = (struct evt_data*)code;
	tail = evt_ddata->head;
	while(tail){
		if(tail == pdata){
			break;
		}else{
			tail = tail->next;
		}
	}
	
	if(tail == NULL){
		ret = DEV_EVT_INVALID_PARAM;
		return ret;
	}
	
	tail->prv->next = tail->next;
	tail->next->prv = tail->prv;
	
	kfree(pdata);
	
	return ret;
}

DEV_EVT_RESULT_CODE dev_evt_InjectEvt(DEV_EVT_CODE code)
{
	DEV_EVT_RESULT_CODE ret = DEV_EVT_OK;
	struct evt_data* pdata = NULL;
	struct evt_data* tail = NULL;
	
	if(evt_IsInit == -1){
		ret = DEV_EVT_NOT_INITIALISED;
		return ret;
	}
	
	if(code == 0){
		ret = DEV_EVT_INVALID_PARAM;
		return ret;
	}
	
	pdata = (struct evt_data*)code;
	tail = evt_ddata->head;
	while(tail){
		if(tail == pdata){
			break;
		}else{
			tail = tail->next;
		}
	}
	
	if(tail == NULL){
		ret = DEV_EVT_INVALID_PARAM;
		return ret;
	}
	
	input_event(pdata->input, pdata->content.type, pdata->content.code, pdata->content.value);
	//printk("content.type is %d, content.code is %d, content.value is %d\n", pdata->content.type, pdata->content.code, pdata->content.value);
	input_sync(pdata->input);
	
	return ret;
}

EXPORT_SYMBOL(dev_evt_RegEvt);
EXPORT_SYMBOL(dev_evt_UnRegEvt);
EXPORT_SYMBOL(dev_evt_InjectEvt);

/****************************************************************/
/*          PRIVATE FUNCTIONS                                   */
/****************************************************************/

static int evt_Setup(void)
{
	struct input_dev *input;
	int error;

	evt_ddata = kzalloc(sizeof(struct evt_drvdata) ,GFP_KERNEL);
	input = input_allocate_device();
	if (!evt_ddata || !input) {
		error = -ENOMEM;
		pr_err("evt: Unable to create evt_ddata or inputdevice,error %d\n", error);
		goto fail;
	}

	input->name = "evt device";
	input->phys = "event/input1";
	input->dev.parent = NULL;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;
	
		
	evt_ddata->input = input;
	evt_ddata->head = NULL;
	
	error = input_register_device(input);
	if (error) {
		pr_err("evt: Unable to register input device, "
			"error: %d\n", error);
		goto fail;
	}
	return 0;

fail:
	input_free_device(input);
	kfree(evt_ddata);
	return error;
}

static void evt_CleanUp(void)
{
	struct input_dev *input = evt_ddata->input;
	struct evt_data* pdata = NULL;
	struct evt_data* temp = NULL;
	
	pdata = evt_ddata->head;
	while(pdata){
		temp = pdata;
		pdata = pdata->next;
		kfree(temp);
	}

	input_unregister_device(input);
	kfree(evt_ddata);
}

#ifdef TEST
void reg_my_code()
{
	DEV_EVT_RESULT_CODE ret;
	int i = 0;

	for(i = 0; i < 20; i++)
	{
		ret = dev_evt_RegEvt(EVT_SYS, i, 0, &code[i]);
		if(ret != DEV_EVT_OK)
		{
			printk("-----<DRV_IIR LOG> dev_evt_RegEvt 0x03 error:%d \r\n", ret);
		}
		else
			printk("-----<DRV_IIR LOG> dev_evt_RegEvt0 ok:%d \r\n", ret);


		ret = dev_evt_RegEvt(EVT_SYS, i, 1, &code[20 + i]);
		if(ret != DEV_EVT_OK)
		{
			printk("-----<DRV_IIR LOG> dev_evt_RegEvt 0x03 error:%d \r\n", ret);
		}
		else
			printk("-----<DRV_IIR LOG> dev_evt_RegEvt1 ok:%d \r\n", ret);
	}
}
static int times = 0;
static void timer_handle(unsigned long arg)
{
  DEV_EVT_RESULT_CODE ret;

  mod_timer(&timer, jiffies + 5*HZ);

  ret = dev_evt_InjectEvt(code[20 + times]);
  ret = dev_evt_InjectEvt(code[times]);
  times = (++times) % 20;
  if(ret != DEV_EVT_OK)
  {
	printk("-----<timer_handle> dev_evt_RegEvt 0x03 error:%d \r\n", ret);
  }
  else
	  printk("timer_handle input dev ok\n");
  printk(KERN_NOTICE "current jiffies is %ld\n", jiffies);
}
#endif
/****************************************************************/
/*          Module initialize                                   */
/****************************************************************/
static int __init evt_init(void)
{
	int ret = 0;
	ret = evt_Setup();
	if(ret == 0)
		evt_IsInit = 1;
#ifdef TEST
	reg_my_code();
	init_timer(&timer);
	timer.function = &timer_handle;
	timer.expires = jiffies + 5*HZ;

	add_timer(&timer);
#endif
	return ret;
}

static void __exit evt_exit(void)
{
#ifdef TEST
	del_timer(&timer);
#endif
	evt_IsInit = -1;
	evt_CleanUp();
}

module_init(evt_init);
module_exit(evt_exit);

MODULE_LICENSE("GPL");
