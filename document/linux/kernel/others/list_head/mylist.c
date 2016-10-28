#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Xie");
MODULE_DESCRIPTION("List Module");
MODULE_ALIAS("List module");

struct student
{
    char name[100];
    int num;
    struct list_head list;
};

struct student *pstudent;
struct student *tmp_student;
struct list_head student_list;
struct list_head *pos;

int mylist_init(void)
{
	int i = 0;
	
	INIT_LIST_HEAD(&student_list);
	
	pstudent = kmalloc(sizeof(struct student)*5,GFP_KERNEL);
	memset(pstudent,0,sizeof(struct student)*5);
	
	for(i=0;i<5;i++)
	{
	    sprintf(pstudent[i].name,"Student%d",i+1);
		pstudent[i].num = i+1; 
		list_add( &(pstudent[i].list), &student_list);
	} 
	
	
	list_for_each(pos,&student_list)
	{
		tmp_student = list_entry(pos,struct student,list);
		printk("student %d name: %s\n",tmp_student->num,tmp_student->name);
	}
	printk("\n");
	list_for_each_entry(tmp_student,&student_list,list)
	{
		printk("student %d name: %s\n",tmp_student->num,tmp_student->name);
	}
	
	return 0;
}


void mylist_exit(void)
{	
	int i ;
	struct list_head *temp;
	/* 实验：将for换成list_for_each来遍历删除结点，观察要发生的现象，并考虑解决办法 */
	/*for(i=0;i<5;i++)
	{
		list_del(&(pstudent[i].list));     
	}*/
	
	list_for_each(pos,&student_list)
	{
		temp=pos->next;
		list_del(pos); 
		pos=temp;
	}
	
	kfree(pstudent);
	printk("mylist_exit ok...");
}

module_init(mylist_init);
module_exit(mylist_exit);
