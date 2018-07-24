#include<stdlib.h>
#include<string.h>
#include "dberror.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <stdio.h>

typedef struct BM_PageArray {

    int pageNumber;
    int index;
    struct BM_PageArray* next;

  }BM_PageArray;

typedef struct BM_PageContent {
   //check if dirty
    int dirty;
    //initial count
    int fixCount;

  }BM_PageContent;


typedef struct BM_DataManager {
    //record the order of writting into the buffer
    BM_PageArray *pageHead;
    BM_PageArray *pageTail;
    //initial total pages
    int totalPage;
    //initial read pages
    int totalRead;
    //initial write pages
    int totalWrite;
    //page array
    BM_PageHandle handleData[10000];
    BM_PageContent  content[10000];

  }BM_DataManager;



RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                   const int numPages, ReplacementStrategy strategy,
                   void *stratData)
{
   //Check bm exist or not
	 if (bm == NULL)
	  return RC_BAD_PARAMETER;
    // check number pages
  if (numPages == NULL)
   return RC_BAD_PARAMETER;
   //check file exist or not
   if( pageFileName == NULL)
    return RC_BM_NON_EXISTING_PAGE;
    //init bm pool
    bm->numPages = numPages;
    bm->pageFile = (char*)pageFileName;
    bm->strategy = strategy;
    //apply the space which has size 'numPages' to store page

    BM_DataManager *data = (BM_DataManager *)malloc(sizeof(BM_DataManager));
    data->pageHead = NULL;
    data->pageTail = NULL;
    data->totalPage = 0;
    data->totalRead = 0;
    data->totalWrite = 0;

    for(int i = 0; i < numPages; i++)
    {
        (data->content[i]).dirty = 0;
        (data->content[i]).fixCount = 0;

    }
    bm->mgmtData = data;
    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm)
{

     return RC_OK;
}
