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
//这个method里把大括号对齐一下，还有，凡是第一个if里面包含有return的，接下来可以不用else
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // check if the file is opened or not
  if (fHandle->mgmtInfo==NULL){
    return RC_FILE_NOT_FOUND;
  }
  //check if the current page number exceed the total pages
  else if (fHandle->totalNumPages < fHandle->curPagePos) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    fseek(fHandle->mgmtInfo, (fHandle->curPagePos-1) * PAGE_SIZE, SEEK_SET);

    if(fread(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo) != 1){
      printf("Read ERROR,cannot read page to file");
        return RC_READ_NON_EXISTING_PAGE;
      }
    return RC_OK;
}

//这里需要考虑万一fHandle是NULL的情况，需要return handle not init
int getBlockPos (SM_FileHandle *fHandle)
{
    return fHandle->curPagePos;
}
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  return readBlock(0, fHandle, memPage);
}
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  // check if the current page is the first page 
  if (fHandle->curPagePos <= 0) {//curPagePos会不会>=TotalPageNum呢？
      return RC_READ_NON_EXISTING_PAGE;
  }
  return readBlock(fHandle->curPagePos - 1, fHandle, memPage);//这里的问题（也许？）是，读了前一个block，我觉得是不是该把curPagePos也往前移动一位
}//所以我觉得可以写成 (fHandle->curPagePos--, ...)
//****************************************************************************

//************************************XM**************************************
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  //the currnt block postion should be start from current page
      return readBlock(fHandle->curPagePos, fHandle, memPage);//虽然可能多余了一点，我觉得还是判断一下current block是不是valid吧
}


RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  //if the NextBlock is beyond the last pagefile
  //then it should return RC_READ_NON_EXISTING_PAGE
  if (fHandle->curPagePos >= fHandle->totalNumPages - 1 )
  {
      return RC_READ_NON_EXISTING_PAGE;
  }
  else{//麻烦把下面这个注释对齐一下
    //else, it should return the current page position +1
      return readBlock(fHandle->curPagePos + 1, fHandle, memPage);
  }
}
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  //the last page of blkock should be equals to the totalNumPages-1
      return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
    }//把这个括号对齐...
//*****************************************************************************

/* writing blocks to a page file */
//************************************YD***************************************
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle->mgmtInfo == NULL) {
        printf("Write Error! File header not found!\n");
        return RC_FILE_HANDLE_NOT_INIT;
    }
    if(fHandle->totalNumPages < pageNum) {
        printf("Write Error! Page number not found!\n");
        return RC_WRITE_FAILED;
    }
    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE, SEEK_SET);
    if(fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo) != 1){
        printf("Write Error! Cannot write page to file!\n");
        return RC_WRITE_FAILED;
    }
    return RC_OK;
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    //int pos = getBlockPos(fHandle);
    if(fHandle->mgmtInfo == NULL) {
        printf("Write Error! File header not found!\n");
        return RC_FILE_HANDLE_NOT_INIT;
    }
    if(fHandle->totalNumPages < fHandle->curPagePos) {
        printf("Write Error! Page number not found!\n");
        return RC_WRITE_FAILED;
    }
    fseek(fHandle->mgmtInfo, (fHandle->curPagePos-1) * PAGE_SIZE, SEEK_SET);
    if(fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo) != 1){
        printf("Write Error! Cannot write page to file!\n");
        return RC_WRITE_FAILED;
    }
    return RC_OK;
}

RC appendEmptyBlock (SM_FileHandle *fHandle){
    SM_PageHandle emptyPh;
    if((emptyPh = (SM_PageHandle) malloc(PAGE_SIZE)) == NULL){
        printf("Appending Error! Cannot allocate memory for one page!\n");
        return RC_WRITE_FAILED;
    }
    strcpy(emptyPh, "");
    for(int i = 0; i < PAGE_SIZE; ++i){
        strcat(emptyPh, "\0");
    }
    if(fHandle->mgmtInfo == NULL) {
        printf("Appending Error! File header not found!\n");
        return RC_FILE_HANDLE_NOT_INIT;
    }
    fseek(fHandle->mgmtInfo, fHandle->totalNumPages * PAGE_SIZE, SEEK_SET);
    fprintf(fHandle->mgmtInfo, "%s", emptyPh);
    fHandle->totalNumPages +=  1;
    free(emptyPh);
    return RC_OK;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    if(fHandle->mgmtInfo == NULL){
        printf("Capacity Error! File header not found!\n");
        return RC_FILE_HANDLE_NOT_INIT;
    }
    if(numberOfPages <= fHandle->totalNumPages){
        printf("Capacicty is enough.\n");
        return RC_OK;
    }
    int numAppendingPage = numberOfPages - fHandle->totalNumPages;
    for(int i = 0; i < numAppendingPage; ++i){
        int RC_RETURNED = appendEmptyBlock(fHandle);
        if(RC_RETURNED == RC_OK) continue;
        return RC_RETURNED;
    }
    return RC_OK;
}
//*****************************************************************************
