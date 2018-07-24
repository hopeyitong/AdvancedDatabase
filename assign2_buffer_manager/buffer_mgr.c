#include<stdlib.h>
#include<string.h>
#include "dberror.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <stdio.h>

typedef struct BM_PageContent {
   //check if dirty
    int dirty;
    //initial count
    int fixCount;
    //initial total pages
    int totalPage;
    //initial read pages
    int totalRead;
    //initial write pages
    int totalWrite;
  }BM_PageContent;


typedef struct BM_DataManager {
    //record the order of writting into the buffer
    BM_PageArray *pageHeader;
    BM_PageArray *pageTail;
    //page array
    BM_PageHandle handleData[1000];
    BM_PageContent  content[1000];

  }BM_DataManager;

typedef struct BM_PageArray {

    int pageNumber;
    int index;
    struct BM_PageArray* next;

  }BM_PageArray;

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                   const int numPages, ReplacementStrategy strategy,
                   void *stratData)
{
   //Check bm exist or not
	 if (bm == NULL ）
	  return RC_BAD_PARAMETER;

   if (pageFile == NULL)
    return RC_BAD_PARAMETER;

   if( pageFileName == NULL)
    return RC_BM_NON_EXISTING_PAGE
    //init bm pool
    bm->numPages = numPages;
    bm->pageFile = (varchar*)pageFileName;
    bm->strategy = strategy;
    //apply the space which has size 'numPages' to store page

    BM_DataManager *data = (BM_DataManager *)malloc(sizeof(BM_DataManager));
    data->pageHeader = NULL;
    data->pageTail = NULL;

    for(int i = 0; i < numPages; i++)
    {
        (data->content[i]).dirty = 0;
        (data->content[i]).fixCount = 0;
        (data->content[i]).totalPage = 0;
        (data->content[i]).totalRead = 0;
        (data->content[i]).totalWrite = 0;
    }
    bm->mgmtData = data;
    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm)
{
     return RC_OK;
}
