#include <stdlib.h>
#include <stdio.h>
#include "..\..\..\bwlib.h"
#include <time.h>
#include <windows.h>
#include "..\..\..\bwoss\bwoss_iface.h"
//#include "osdep.h"

#define NUM 50 * 1024

typedef struct radix_tree_item
{
	uint32_t index; 
}radix_tree_item;

typedef struct Para
{
	bwlib_radix_tree_root* root;
	uint32_t start;
}Para;

int  insert_50k_nodes(void* tp)
{
     uint32_t i = 0 ;
	bwoss_lock_write();

	for(i = ((Para *)tp)->start; i < ((Para *)tp)->start + NUM; ++i)
	{
		struct radix_tree_item* leaf = (radix_tree_item*)malloc(sizeof(radix_tree_item));
		leaf->index = i;
		if(1 == bwlib_radix_tree_insert(((Para *)tp)->root,i,(void*)leaf))
		{
			printf("indexΪ%dʱ��insert������������\n",i);
			bwoss_unlock_write();
			return 0;
		}
	}
printf("thread that adds Nodes indexed from %d to %d has been exit!\n",((Para *)tp)->start, ((Para *)tp)->start + NUM -1);
	bwoss_unlock_write();	
	return 1;
}


uint32_t search_250k_nodes(void* lp)
{   
		uint32_t i = 0;
	bwoss_lock_read();



    for( i = ((Para *)lp)->start; i < ((Para *)lp)->start + 5 * NUM; ++i)
	{
		radix_tree_item *leaf1 = (radix_tree_item*)bwlib_radix_tree_lookup(((Para *)lp)->root,i);
		if(leaf1)
			;//printf("indexΪ%d\n",leaf1->index);
		else
		{
			printf("���ҵĽڵ㲻���ڣ�\n");
			bwoss_unlock_read();
		    return 0;
		}
	}
     
    bwoss_unlock_read();
	return 1;
}


uint32_t main()
{
	uint32_t j = 0 , i =0;
	HANDLE hThread[6];
	DWORD  threadID[6];
	Para para[6];
	DWORD Code;
	clock_t start, after_tree_creates, after_lookups, after_deletes;
	uint32_t node_count[8] = {        1024, 
									 	 4*1024,
									    64*1024,
									   256*1024, 
									  1024*1024,
								    4*1024*1024,
								   64*1024*1024,
								  256*1024*1024};

	struct bwlib_radix_tree_root* root = NULL;
	root =(bwlib_radix_tree_root*) malloc(sizeof(bwlib_radix_tree_root));
	bwlib_radix_tree_init(root);
/*	printf("Performance test:\n");
	for(j = 0; j < 6; ++j)
	{
		start = clock();
		//test bwlib_radix_tree_insert
		for(i = 0 ; i < node_count[j]; i++)
		{
			struct radix_tree_item* leaf = (radix_tree_item*)malloc(sizeof(radix_tree_item));
			leaf->index = i;
			if(1 == bwlib_radix_tree_insert(root,i,(void*)leaf))
			{
				//printf("indexΪ%dʱ��insert funcion error\n",i);

				system("pause");
				return -1;
			}
		}
		after_tree_creates = clock();
		//test bwlib_radix_tree_lookup
		for(i = 0; i < node_count[j]; ++i)
		{
			radix_tree_item *leaf1 = (radix_tree_item*)bwlib_radix_tree_lookup(root,i);
			if(leaf1)
				;//printf("indexΪ%d\n",leaf1->index);
			else
			{
				printf("the node does not exist!\n");
			}
		}
		after_lookups = clock();
		//test bwlib_radix_tree_delete
		for(i = 0; i < node_count[j]; ++i)
		{
			bwlib_radix_tree_node* node = (bwlib_radix_tree_node*)bwlib_radix_tree_delete(root,i);
			radix_tree_item *leaf2 = (radix_tree_item*)bwlib_radix_tree_lookup(root,i);
			if(leaf2)
				printf("indexΪ%d\n",leaf2->index);
			else
			{
			//	printf("the node does not exist!\n");
			}
		}
		after_deletes = clock();

		printf("Test case : Nodes Total\t%u\n	Time consumed for tree creation: %f\n	Time consumed for lookups: %f\n	Time consumed for deletes: %f\n", node_count[j], (double)(after_tree_creates - start)/CLOCKS_PER_SEC, (double)(after_lookups - after_tree_creates)/CLOCKS_PER_SEC, (double)(after_deletes - after_lookups)/CLOCKS_PER_SEC);
	}//for loop ends
	
	//to create six threads to manipulate the tree in parallel 
	//using the header file: osdep.cpp for the compatibility and convinience.
*/
	
	bwoss_initialize_lock();

	
	for(i = 0; i < 5; i++ )
	{
		para[i].root  = root;
		para[i].start = i * NUM + 1;
		hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)insert_50k_nodes, &(para[i]), 0, &(threadID[i]));
	}
   WaitForMultipleObjects(5,hThread,TRUE,INFINITE);
	
    para[5].root = root;
	para[5].start = 1;
    hThread[5]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)search_250k_nodes, &(para[5]), 0, &(threadID[5]));

	WaitForSingleObject(hThread[5],INFINITE);
    
	
	GetExitCodeThread(hThread[5], &Code);
	if(Code == 1)
        printf("Find all the nodes\n"); 

	for(i = 0; i < 6; ++i)
		CloseHandle(hThread[i]);

    bwoss_clean_lock();

	system("pause");
	
	return 0;
}