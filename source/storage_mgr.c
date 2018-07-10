#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    SM_PageHandle str = (SM_PageHandle) malloc(PAGE_SIZE);
    memset(str, '\0', PAGE_SIZE);
   
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
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
//*****************************************************************************

/* writing blocks to a page file */
//************************************YD***************************************
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    // check if mgmtInfo point to the correct file header
    if(fHandle->mgmtInfo == NULL) {
        printf("Write Error! File header not found!\n");
        return RC_FILE_HANDLE_NOT_INIT;
    }
    // check if pageNum exists in the fHandle
    if(fHandle->totalNumPages <= pageNum) {
        printf("Write Error! Page number not found!\n");
        return RC_WRITE_FAILED;
    }
    // set the file header to the current block (pageNum)
    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE, SEEK_SET);
    // write page file (memPage) to current block
    if(fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo) != 1){
        printf("Write Error! Cannot write page to file!\n");
        return RC_WRITE_FAILED;
    }
    return RC_OK;
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // get current block position
    //int pos = getBlockPos(fHandle);
    // check fHandle has correct file header
    if(fHandle->mgmtInfo == NULL) {
        printf("Write Error! File header not found!\n");
        return RC_FILE_HANDLE_NOT_INIT;
    }
    // check current block position is valid
    if(fHandle->totalNumPages <= fHandle->curPagePos) {
        printf("Write Error! Page number not found!\n");
        return RC_WRITE_FAILED;
    }
    // set the file header to the current block 
    fseek(fHandle->mgmtInfo, fHandle->curPagePos * PAGE_SIZE, SEEK_SET);
    // write page file (memPage) to current block
    if(fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo) != 1){
        printf("Write Error! Cannot write page to file!\n");
        return RC_WRITE_FAILED;
    }
    return RC_OK;
}

RC appendEmptyBlock (SM_FileHandle *fHandle){
    // check fHandle has correct file header
    if(fHandle->mgmtInfo == NULL) {
        printf("Appending Error! File header not found!\n");
        return RC_FILE_HANDLE_NOT_INIT;
    }
    // Initialize a page handle, allocate PAGE_SIZE memory and fill it with '\0'
    SM_PageHandle emptyPh;
    if((emptyPh = (SM_PageHandle) malloc(PAGE_SIZE)) == NULL){
        printf("Appending Error! Cannot allocate memory for one page!\n");
        return RC_WRITE_FAILED;
    }
    memset(emptyPh, '\0', PAGE_SIZE);

    fHandle->totalNumPages +=  1;
    // set the file header to the start of the empty block
    fseek(fHandle->mgmtInfo, 1, SEEK_END);
    // write the empty page to the file
    fprintf(fHandle->mgmtInfo, "%s", emptyPh);
    // free the memory of pageHandle
    free(emptyPh);
    fHandle->curPagePos = fHandle->totalNumPages - 1;
    return RC_OK;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    // check fHandle has correct file header
    if(fHandle->mgmtInfo == NULL){
        printf("Capacity Error! File header not found!\n");
        return RC_FILE_HANDLE_NOT_INIT;
    }
    // check if the capacity of the file is big enough
    if(numberOfPages <= fHandle->totalNumPages){
        printf("Capacicty is enough.\n");
        return RC_OK;
    }
    // calculate how many empty pages we need to append to the end of the file
    int numAppendingPage = numberOfPages - fHandle->totalNumPages;
    // append # of numAppendingPage empty pages to the end of the file
    for(int i = 0; i < numAppendingPage; ++i){
        int RC_RETURNED = appendEmptyBlock(fHandle);
        if(RC_RETURNED == RC_OK) continue;
        return RC_RETURNED;
    }
    return RC_OK;
}
//*****************************************************************************