//YH

#include<stdlib.h>
#include<string.h>
#include <stdio.h>

#include "dberror.h"
#include "storage_mgr.h"


/* manipulating page files */
//***********************************YH***************************************
void initStorageManager (void){}

RC createPageFile(char *fileName)
{
    //open a file for writing and reading
    FILE *fp = fopen(fileName, "w+");

    //disable the buffer so that following process are executed directly to disk
    setbuf(fp, NULL);

    //check if the file exists
    if(fp == NULL){
          return RC_FILE_NOT_FOUND;
    }

    //create a string with size of block_size and fill out the memory with 0s
    char * str = (char *)malloc(sizeof(char)*PAGE_SIZE);
    memset(str, 0, sizeof(str));

    //assign the block to file
    long file_size=fwrite(str, sizeof(char), PAGE_SIZE, fp);

    //check if the file has enough space to write the page
    if (PAGE_SIZE != file_size) {
        return RC_WRITE_FAILED;
    }

    //close the file and free cache
    fclose(fp);
    free(str);

    return RC_OK;
}

RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
    //open a file for reading and writing and check if file exists
    FILE *fp = fopen(fileName, "r+");
    if(fp == NULL){
        return RC_FILE_NOT_FOUND;
    }

    //move pointer to end of the file
    long reposit=fseek(fp, 0, SEEK_END);
    if(reposit == -1){
        return RC_READ_NON_EXISTING_PAGE;
}

    //add 1 to the file size to make sure below total number of pages is correct
    long fileSize = ftell(fp)+1;

    //assign fhandle variables
    fHandle->fileName = fileName;
    fHandle->totalNumPages = fileSize/PAGE_SIZE;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = fp;

    return RC_OK;
}

RC closePageFile(SM_FileHandle *fHandle)
{
    //check if file exists, if yes close it
    if(fclose(fHandle->mgmtInfo) == -1){
        return RC_FILE_NOT_FOUND;
    }
    return RC_OK;
}

RC destroyPageFile(char *fileName)
{
    //check if file exists, if yes destroy it
    if(remove(fileName) == -1) {
        return RC_FILE_NOT_FOUND;
    }
    return RC_OK;
}
//****************************************************************************

/* reading blocks from disc */
//**********************************ZZ***************************************
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
int getBlockPos (SM_FileHandle *fHandle){return 0;}
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
//****************************************************************************

//************************************XM**************************************
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  //the currnt block postion should be start from current page
      return readBlock(fHandle->curPagePos, fHandle, memPage);
}


RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  //if the NextBlock is beyond the last pagefile
  //then it should return RC_READ_NON_EXISTING_PAGE
  if (fHandle->curPagePos >= fHandle->totalNumPages - 1 )
  {
      return RC_READ_NON_EXISTING_PAGE;
  }
  else{
    //else, it should return the current page position +1
      return readBlock(fHandle->curPagePos + 1, fHandle, memPage);
  }
}
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  //the last page of blkock should be equals to the totalNumPages-1
      return readblock(fHandle->totalNumPages - 1, fHandle, memPage);
    }
//*****************************************************************************

/* writing blocks to a page file */
//************************************YD***************************************
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
RC appendEmptyBlock (SM_FileHandle *fHandle){return 0;}
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){return 0;}
//*****************************************************************************
