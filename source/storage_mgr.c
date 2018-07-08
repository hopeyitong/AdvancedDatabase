#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"

/* manipulating page files */
//***********************************YH***************************************
void initStorageManager (void){}
RC createPageFile (char *fileName){return 0;}
RC openPageFile (char *fileName, SM_FileHandle *fHandle){return 0;}
RC closePageFile (SM_FileHandle *fHandle){return 0;}
RC destroyPageFile (char *fileName){return 0;}
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
    if(fHandle->mgmtInfo == NULL) {
        printError("Write Error! File header not found!\n");
        return RC_WRITE_FAILED;
    }
    if(fHandle->totalNumPages < pageNum) {
        printError("Write Error! Page number not found!\n");
        return RC_WRITE_FAILED;
    }
    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE, SEEK_SET);
    if(fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo) != 1){
        printError("Write Error! Cannot write page to file!\n");
        return RC_WRITE_FAILED;
    }
    return RC_OK;
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){return 0;}
RC appendEmptyBlock (SM_FileHandle *fHandle){return 0;}
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){return 0;}
//*****************************************************************************