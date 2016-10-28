#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

void insertSort(int *a, int n)
{
    int i, j, key;

    for(i=1; i<n; i++)
    {
        key = a[i];
        for(j=i-1; j>=0; j--)
        {
            if (a[j] > key)
                a[j+1] = a[j];
            else
                break;
        }         
        a[j+1] = key;
    }
}

void shellSort(int *a, int n)
{
    int i, j, key, increment, m;
    
    for(increment=n/2; increment>0; increment/=2)
    {
        for(i=increment; i<n; i++)
        {
            key = a[i];
            for(j=i-increment; j>=0; j-=increment)
            {
                if (a[j] > key)
                    a[j+increment] = a[j];
                else
                    break;
            }
            a[j+increment] = key;
        }
    }
}

void bubbleSort(int *a, int n)
{
    int i, j, exchange, tmp;

    for(j=n-1; j>0; j--)
    {
        exchange = 0;
        for(i=1; i<=j; i++)
        {
            if (a[i] < a[i-1])
            {
                tmp = a[i];
                a[i] = a[i-1];
                a[i-1] = tmp;
                exchange = 1;
            }
        }

        if (!exchange)
            break;
    }
}

int partion(int *a, int low, int high)
{
    int pivot = a[low];
    int tmp;

    while (low < high)
    {
        while (low < high && a[high] >= pivot)
            high --;

        if (low < high)
        {
            a[low] = a[high];
            low ++;
        }
                   
        while (low < high && a[low] <= pivot)
            low ++;
        
        if (low < high)
        {
            a[high] = a[low];
            high --;
        }
    }

    a[low] = pivot;
    return low;
}

void quickSort(int *a, int low, int high)
{
    int n;
  
    if (low < high)
    {
        n = partion(a, low, high);
        quickSort(a, low, n - 1);
        quickSort(a, n + 1, high);
    }
}

void selectSort(int *a, int n)
{
    int i, j, pos, tmp;

    for(i=0; i<n-1; i++)
    {
        pos = i;
        for(j=i+1; j<n; j++)
        {
            if (a[pos] > a[j])
                pos = j;
        }
        
        if (pos != i) 
            tmp = a[i], a[i] = a[pos], a[pos] = tmp;
    } 
}


void adjustHeap(int *a, int pos, int n)
{
    int i;
    int rc = a[pos];
    int nchild;
    int tmp = pos;

    for(i=2*pos+1; i<n; i=2*nchild+1)
    {
        nchild = i;        
        if (i+1 < n && a[i+1] > a[i])
            nchild++;

        if (rc < a[nchild])
        {
            a[tmp] = a[nchild];
            tmp = nchild;
        }
        else
            break; 
    }    

    a[tmp] = rc;
}

void heapSort(int *a, int n)
{
    int i, tmp;

    for(i=n/2-1; i>=0; i--)
    {
        adjustHeap(a, i, n);
    }

    for (i=n-1; i>0; i--)
    {
        tmp = a[0], a[0] = a[i], a[i] = tmp;
        adjustHeap(a, 0, i);
    } 
}

void merge(int *a, int start, int middle, int end)
{
    int n = end - start + 1;
    int tmp[n];
    int s1 = start;
    int s2 = middle + 1;
    int p = 0;
    int i;

    while (s1 != middle + 1 && s2 != end + 1)
    {
        if (a[s1] <= a[s2])
            tmp[p++] = a[s1++];
        else
            tmp[p++] = a[s2++];
    }

   for (i=s1; i<=middle; i++)
       tmp[p++] = a[i];

   for (i=s2; i<=end; i++)
       tmp[p++] = a[i];

   for (i=0; i<n; i++)
       a[start++] = tmp[i];
}

void mergeSort(int *a, int start, int end)
{
    int middle;

    if (start < end)
    {
        middle = (end + start) / 2;
        mergeSort(a, start, middle);
        mergeSort(a, middle + 1, end);
        merge(a, start, middle, end); 
    }
}

int main(int argc, char* argv[])
{
    int i, n;

    if (argc < 2)
    {    
        printf("%s numbers\n", argv[0]);
        return 0;
    }
    
    n = atoi(argv[1]);
      
    int a[n+1];
 
    printf("Input %d numbers:\n", n);
    for(i=0; i<n; i++)
    {
        scanf("%d", a + i);
    }

    for(i=0; i<n; i++)
    {
        printf("%d ", a[i]);
    }

    printf("\n");

    shellSort(a, n);
    //mergeSort(a, 0, n-1);

    for(i=0; i<n; i++)
    {
        printf("%d ", a[i]);
    }

    printf("\n");
}

