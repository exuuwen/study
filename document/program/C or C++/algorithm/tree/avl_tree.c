#include <stdio.h>
#include <stdlib.h>

typedef struct Avl Avl;

struct Avl
{
    int data;
    int bf;
    Avl *left;
    Avl *right;
};

Avl* new_avl(int data)
{
    Avl *node = (Avl*)malloc(sizeof(Avl));
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    node->bf = 0;

    return node;
}

void l_rotate(Avl **tree)   
{  
//以p为根节点的二叉排序树进行单向左旋处理  
    Avl *rc = (*tree)->right;  
    (*tree)->right = rc->left;  
    rc->left = (*tree);  
    (*tree) = rc;  
}  
  
void r_rotate(Avl **tree)  
{  
//以p为根节点的二叉排序树进行单向右旋处理  
    Avl *lc = (*tree)->left;  
    (*tree)->left = lc->right;  
    lc->right = (*tree);  
    (*tree) = lc;  
}  
  
void left_balance(Avl **tree)  
{  
//以T为根节点的二叉排序树进行左平衡旋转处理  
    Avl *lc,*rd;  
    lc = (*tree)->left;  
    switch(lc->bf)  
    {  
    case 1:  
    //新结点插在T的左孩子的左子树上，做单向右旋处理  
        (*tree)->bf = lc->bf = 0;  
        r_rotate(tree);  
        break; 
    case 0:  
    //used for delete avl  
        (*tree)->bf = 1;
        lc->bf = -1;  
        r_rotate(tree);  
        break;  
    case -1:  
    //新结点插在T的左孩子的右子树上，要进行双旋平衡处理（先左后右）  
        rd = lc->right;  
        switch(rd->bf)  
        {  
        case 1:  
        //插在右子树的左孩子上  
            (*tree)->bf = -1;  
            lc->bf = 0;  
            break;  
        case 0:  
            (*tree)->bf = lc->bf = 0;  
            break;  
        case -1:  
            (*tree)->bf = 0;  
            lc->bf = 1;  
            break;  
        }  
        rd->bf = 0;  
        l_rotate(&(*tree)->left);//先对T的左子树进行单向左旋处理  
        r_rotate(tree);        //再对T进行单向右旋处理  
        break;
    }  
}  
  
void right_balance(Avl **tree)  
{  
    Avl *rc,*ld;  
    rc = (*tree)->right;  
    switch(rc->bf)  
    {  
    case -1:  
        //新结点插在右孩子的右子树上，进行单向左旋处理  
        (*tree)->bf = rc->bf = 0;  
        l_rotate(tree);  
        break;  
    case 0:  
        //used for delete avl
        (*tree)->bf = -1;
        rc->bf = 1;  
        l_rotate(tree);  
        break;
    case 1:  
        //新结点插在T的右孩子的左子树上，要进行右平衡旋转处理（先右再左）  
        ld = rc->left;  
        switch(ld->bf)  
        {  
        case 1:  
            (*tree)->bf = 1;  
            rc->bf = 0;  
            break;  
        case 0:  
            (*tree)->bf = rc->bf = 0;  
            break;  
        case -1:  
            (*tree)->bf = 0;  
            rc->bf = -1;  
            break;  
        }  
        ld->bf = 0;  
        r_rotate(&(*tree)->right);//先对T的右子树进行单向右旋处理  
        l_rotate(tree);        //再对T进行单向左旋处理  
        break;
    }  
}  

int insert(Avl **tree, int data, int *taller)
{
    if (*tree == NULL)
    {
        *tree = new_avl(data);
        *taller = 1;
        return 0;
    }

    if (data < (*tree)->data)
    {
        insert(&(*tree)->left, data, taller);
        if (*taller)
        {  
            switch((*tree)->bf)  
            {  
            case 1:    
                left_balance(tree);  
                *taller = 0;  
                break;         
            case 0:    
                (*tree)->bf = 1;  
                *taller = 1;  
                break;  
            case -1:
                (*tree)->bf = 0;  
                *taller = 0;  
                break;  
            }  
        }  
    }
    else
    {
        insert(&(*tree)->right, data, taller);
        if (*taller)
        {  
            switch((*tree)->bf)  
            {  
            case -1:    
                right_balance(tree);  
                *taller = 0;  
                break;         
            case 0:    
                (*tree)->bf = -1;  
                *taller = 1;  
                break;  
            case 1:
                (*tree)->bf = 0;  
                *taller = 0;  
                break;  
            }  
        } 
    }
    
    return 0;
}



Avl* search(Avl *tree, int data)
{
    if (tree == NULL)
        return NULL;
    
    if (tree->data == data)
        return tree;
    else if (tree->data < data)
        return search(tree->right, data);
    else
        return search(tree->left, data);
}


void delete(Avl **tree, int data, int *shorter)
{
    if (*tree == NULL)
        return; 

    if ((*tree)->data == data)
    {
        Avl *q = NULL;
        if ((*tree)->left == NULL)
        {
            q = *tree;
            *tree = (*tree)->right;
            free(q);
            *shorter = 1;
        }
        else if ((*tree)->right == NULL)
        {
            q = *tree;
            *tree = (*tree)->left;
            free(q);
            *shorter = 1;
        }
        else
        {
            q = (*tree)->left;
            while (q->right)
                q = q->right;
            (*tree)->data = q->data;
            delete(&(*tree)->left, q->data, shorter);
        }
    }
    else if ((*tree)->data > data)
    {
        delete(&(*tree)->left, data, shorter);
        if (*shorter)
        {
            switch((*tree)->bf)
            {
            case 1:
                (*tree)->bf = 0;
                *shorter = 1;
                break;
            case 0:
                (*tree)->bf = -1;
                *shorter = 0;
                break;
            case -1:
                 right_balance(tree);
                 if ((*tree)->right->bf == 0)
                     *shorter = 0;
                 else
                     *shorter = 1;
                 break;
            }
        }
    }
    else
    {
        delete(&(*tree)->right, data, shorter);
        if (*shorter)
        {
            switch((*tree)->bf)
            {
            case -1:
                (*tree)->bf = 0;
                *shorter = 1;
                break;
            case 0:
                (*tree)->bf = 1;
                *shorter = 0;
                break;
            case 1:
                 left_balance(tree);
                 if ((*tree)->left->bf == 0)
                     *shorter = 0;
                 else
                     *shorter = 1;
                 break;
            }
        }
    }

}

void clear(Avl **tree)
{
    if (*tree != NULL)
    {
        clear(&(*tree)->left);
        clear(&(*tree)->right);
    }

    free(*tree);   
}

void inOrderTraverse(Avl *tree)
{
    if(tree->left)
        inOrderTraverse(tree->left);

     printf("%d ", tree->data);

    if(tree->right)
        inOrderTraverse(tree->right);
}

void preOrderTraverse(Avl *tree)
{
    printf("%d ", tree->data);

    if(tree->left)
        preOrderTraverse(tree->left);
   
    if(tree->right)
        preOrderTraverse(tree->right);
}

void postOrderTraverse(Avl *tree)
{
    if(tree->left)
        postOrderTraverse(tree->left);

    if(tree->right)
        postOrderTraverse(tree->right);

    printf("%d ", tree->data);
}

int treeDepth(Avl* tree)
{
     // the depth of a empty tree is 0
     if (!tree)
         return 0;

     // the depth of left sub-tree
     int nLeft = treeDepth(tree->left);
     // the depth of right sub-tree
     int nRight = treeDepth(tree->right);

     // depth is the binary tree
     return (nLeft > nRight) ? (nLeft + 1) : (nRight + 1);
}

int treeNodeCount(Avl *tree)
{
    if(tree == NULL)
        return 0;
   
    int l = treeNodeCount(tree->left);
    int r = treeNodeCount(tree->right);
    
    return l + r + 1;

}

int treeLeafNodeCount(Avl *tree)
{
    if(tree == NULL)
        return 0;
    
    if (tree->left == NULL && tree->right == NULL)
        return 1;
 
    int l = treeLeafNodeCount(tree->left);
    int r = treeLeafNodeCount(tree->right);

    return l + r;
}

int treeNodeNumKthLevel(Avl *tree, int k)
{
	if(tree == NULL || k < 1)
		return 0;
	if(k == 1)
		return 1;
	int numLeft = treeNodeNumKthLevel(tree->left, k-1);
	int numRight = treeNodeNumKthLevel(tree->right, k-1); 
	return (numLeft + numRight);
}

int main()
{
    Avl *root = NULL;
    int i;
    int data;
    int lenth;
    int taller = -1;
    int shorter = -1;

    printf("input datas and end with Ctrl + Z\n");
    while(scanf("%d",&data)!= EOF)
    {
        insert(&root, data, &taller);
    }

    printf("preOrderTraverse : ");
    preOrderTraverse(root);

    printf("\ninOrderTraverse : ");
    inOrderTraverse(root);

    printf("\npostOrderTraverse : ");
    postOrderTraverse(root);

    printf("\ntreeDepth : %d\n", treeDepth(root));

    printf("treeNodeCount：%d\n", treeNodeCount(root));
    printf("treeLeafNodeCount：%d\n", treeLeafNodeCount(root));

    printf("ttreeNodeNumKthLevel 4：%d\n", treeNodeNumKthLevel(root, 4));

    if (search(root, 4))
        printf("search 4 ok\n");
    else
        printf("search 4 fail\n");

    if (search(root, 1000))
        printf("search 1000 ok\n");
    else
        printf("search 1000 fail\n");

    printf("input data to delete\n");
    scanf("%d", &data);
    delete(&root, data, &shorter);

    printf("preOrderTraverse : ");
    preOrderTraverse(root);

    printf("\ninOrderTraverse : ");
    inOrderTraverse(root);

    printf("\npostOrderTraverse : ");
    postOrderTraverse(root);

    printf("\ntreeDepth : %d\n", treeDepth(root));

    printf("treeNodeCount：%d\n", treeNodeCount(root));
    printf("treeLeafNodeCount：%d\n", treeLeafNodeCount(root));

    printf("ttreeNodeNumKthLevel 3：%d\n", treeNodeNumKthLevel(root, 3));
    
    clear(&root);
    
    return 0;
}



