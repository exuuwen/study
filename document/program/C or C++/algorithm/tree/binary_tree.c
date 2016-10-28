#include <stdio.h>
#include <stdlib.h>

typedef struct Bst Bst;

struct Bst
{
    int data;
    Bst *left;
    Bst *right;
};


static void insert_(Bst **tree, Bst *node)
{
    if (*tree == NULL)
    {
        *tree = node;
        return;
    }

    if (node->data <= (*tree)->data)
    {
        insert_(&(*tree)->left, node);
    }
    else
    {
        insert_(&(*tree)->right, node);
    }
}

void insert(Bst **tree, int data)
{
    Bst *node = (Bst*)malloc(sizeof(Bst));
    node->data = data;
    node->left = NULL;
    node->right = NULL;

    insert_(tree, node);
}


Bst* search(Bst *tree, int data)
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

Bst** find(Bst **tree, int data)
{
    if (*tree == NULL)
        return NULL;
    
    if ((*tree)->data == data)
        return tree;
    else if ((*tree)->data < data)
        return find(&(*tree)->right, data);
    else
        return find(&(*tree)->left, data);
}

void delete(Bst **tree, int data)
{
    if (*tree == NULL)
        return; 

    Bst **node = find(tree, data);
    if (node == NULL || *node == NULL)
        return;

    if ((*node)->left)
        insert_(&(*node)->right, (*node)->left);
    Bst *tmp = *node;
    *node = (*node)->right;

    free(tmp);
}

void clear(Bst **tree)
{
    if (*tree != NULL)
    {
        clear(&(*tree)->left);
        clear(&(*tree)->right);
    }

    free(*tree);   
}

void inOrderTraverse(Bst *tree)
{
    if(tree->left)
        inOrderTraverse(tree->left);

     printf("%d ", tree->data);

    if(tree->right)
        inOrderTraverse(tree->right);
}

void preOrderTraverse(Bst *tree)
{
    printf("%d ", tree->data);

    if(tree->left)
        preOrderTraverse(tree->left);
   
    if(tree->right)
        preOrderTraverse(tree->right);
}

void postOrderTraverse(Bst *tree)
{
    if(tree->left)
        postOrderTraverse(tree->left);

    if(tree->right)
        postOrderTraverse(tree->right);

    printf("%d ", tree->data);
}

int treeDepth(Bst* tree)
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

int treeNodeCount(Bst *tree)
{
    if(tree == NULL)
        return 0;
   
    int l = treeNodeCount(tree->left);
    int r = treeNodeCount(tree->right);
    
    return l + r + 1;

}

int treeLeafNodeCount(Bst *tree)
{
    if(tree == NULL)
        return 0;
    
    if (tree->left == NULL && tree->right == NULL)
        return 1;
 
    int l = treeLeafNodeCount(tree->left);
    int r = treeLeafNodeCount(tree->right);

    return l + r;
}

int treeNodeNumKthLevel(Bst *tree, int k)
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
    Bst *root = NULL;
    int i;
    int data;
    int lenth;

    printf("input datas and end with Ctrl + Z\n");
    while(scanf("%d",&data)!= EOF)
    {
        insert(&root, data);
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
    delete(&root, data);

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



