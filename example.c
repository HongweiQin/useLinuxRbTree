/*
* An example of using linux kernel rb tree.
* Refer to [https://lwn.net/Articles/184495/] and [Documentation/rbtree.txt] for more information.
* Note that the website article may have small problems.
* Author: Hongwei Qin
* Huazhong University of Science and Technology
* December, 2016
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rbtree.h>

struct mynode{
	struct rb_node rbnode;
	int value;
	int value2;
};


struct rb_root myroot;

struct mynode *my_rb_search(struct rb_root *root, int value)
{
	struct rb_node *node = root->rb_node;  /* top of the tree */
	while (node)
	{
		struct mynode *stuff = rb_entry(node, struct mynode, rbnode);
		if (stuff->value > value)
			node = node->rb_left;
		else if (stuff->value < value)
			node = node->rb_right;
	    else
			return stuff;  /* Found it */
  	}
	return NULL;
}

int my_rb_insert(struct rb_root *root, struct mynode *newnode)
{
	struct rb_node **link = &root->rb_node, *parent=NULL;
	int value = newnode->value;
	struct mynode *stuff;

	/* Go to the bottom of the tree */
	while (*link)
	{
	    parent = *link;
	    stuff = rb_entry(parent, struct mynode, rbnode);

	    if (stuff->value > value)
			link = &parent->rb_left;
	    else if (stuff->value < value)
			link = &parent->rb_right;
		else
			return 1;//insert failed, node exist
	}

	/* link the new node */
	rb_link_node(&newnode->rbnode, parent, link);
	/* re-adjust the tree */
	rb_insert_color(&newnode->rbnode, root);
	return 0;
}


void initTree(void)
{
	myroot = RB_ROOT;
	printk(KERN_NOTICE"init finished\n");
}

void insertNodes(void)
{
	//19,34,21,6,90
	int lucky[5]={19,34,21,6,90};
	int i;
	struct mynode *newnode;
	for(i=0;i<5;i++)
	{
		newnode = (struct mynode *)kmalloc(sizeof(struct mynode),GFP_KERNEL);
		if(!newnode)
		{
			printk(KERN_NOTICE"not enough memory\n");
			return;
		}
		newnode->value = lucky[i];
		newnode->value2 = 1;
		my_rb_insert(&myroot,newnode);
	}
	printk(KERN_NOTICE"insert nodes finished\n");
	return;
}

/*
* struct rb_node *rb_first(struct rb_root *tree);
* struct rb_node *rb_next(struct rb_node *node);
*/
void traverseTree1(struct rb_root *tree)
{
	struct rb_node *thisnode;
	struct mynode *cnode;
	printk(KERN_NOTICE"traverseTree1:\n");
	thisnode = rb_first(tree);
	while(thisnode)
	{
		//print this node
		cnode = rb_entry(thisnode, struct mynode, rbnode);
		printk(KERN_NOTICE"%d %d\n",cnode->value,cnode->value2);
		//goto next node
		thisnode = rb_next(thisnode);
	}
	return;
}

/*
* struct rb_node *rb_last(struct rb_root *tree);
* struct rb_node *rb_prev(struct rb_node *node);
*/
void traverseTree2(struct rb_root *tree)
{
	struct rb_node *thisnode;
	struct mynode *cnode;
	printk(KERN_NOTICE"traverseTree2:\n");
	thisnode = rb_last(tree);
	while(thisnode)
	{
		//print this node
		cnode = rb_entry(thisnode, struct mynode, rbnode);
		printk(KERN_NOTICE"%d %d\n",cnode->value,cnode->value2);
		//goto next node
		thisnode = rb_prev(thisnode);
	}
	return;
}


/*
* replace(the new node must have the same value with the old one):
*   void rb_replace_node(struct rb_node *old,struct rb_node *new,struct rb_root *tree);
*/
void searchAndReplace(void)
{
	int target=21;
	struct mynode *pnode,*newnode;
	printk(KERN_NOTICE"searchAndReplace %d\n",target);
	pnode = my_rb_search(&myroot,target);
	if(pnode)
	{
		printk(KERN_NOTICE"target found,replace it with the same value\n");
		newnode = (struct mynode *)kmalloc(sizeof(struct mynode),GFP_KERNEL);
		if(!newnode)
		{
			printk(KERN_NOTICE"not enough memory for new node\n");
			return;
		}
		newnode->value = target;
		newnode->value2 = 2;
		rb_replace_node(&pnode->rbnode,&newnode->rbnode,&myroot);
		kfree(pnode);
	}
	else
	{
		printk(KERN_NOTICE"can't find target\n");
	}
	return;

}

/*
* The rb tree doesn't implement a deleteTree() operation since it's not commonly used.
* In a more common case, our structure should also contains a struct list_head, with which we can easily delete them all.
* At this example, I just travers the whole tree and show the usage of rb_erase():
*     void rb_erase(struct rb_node *victim, struct rb_root *tree);
*/
void deleteTree(void)
{
	struct rb_node *thisnode,*nextnode;
	struct mynode *containernode;
	printk(KERN_NOTICE"delete tree\n");
	thisnode = rb_first(&myroot);
	while(thisnode)
	{
		nextnode = rb_next(thisnode);
		rb_erase(thisnode,&myroot);
		containernode = rb_entry(thisnode, struct mynode, rbnode);
		kfree(containernode);
		thisnode = nextnode;
	}
	return;
}

static int __init re_init(void)
{
	printk(KERN_NOTICE"init rb tree example\n");
	initTree();
	insertNodes();
	traverseTree1(&myroot);
	searchAndReplace();
	traverseTree2(&myroot);
	return 0;
}

static void __exit re_exit(void)
{
	printk(KERN_NOTICE"exit rb tree example\n");
	deleteTree();
	return;
}

MODULE_AUTHOR("Hongwei Qin");
MODULE_DESCRIPTION("an example of using linux kernel rb tree");
MODULE_LICENSE("GPL");
module_init(re_init);
module_exit(re_exit);

