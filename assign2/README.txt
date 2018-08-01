///////////////////////////////////////////////////////////////////////////////
//////////////////////////// CS 525 Summer 2018 ///////////////////////////////
////////////////////// Yanbo Deng, Xinghang Ma, Yitong Huang //////////////////
///////////////////////////////////////////////////////////////////////////////
**************************** Assignment 2 *************************************

********** HOW TO RUN **********
1. Go to the folder which contains source/ README.txt and Makefile
2. > make
3. > ./build/assign2.out
4. > make clean

********* Methods Description In buffer_mgr.c **********

/////////////////////////////
/*    initial buffer pool  */
/////////////////////////////

1. initBufferPool
1) Checks if bm exist or not.
2) Check if the pageNum is invalid, returns RC_BAD_PARAMETER.
3) If the bm exists and pageNum is valid, initial bm buffer pool.
4) giving space to storage the pages.Return RC_OK

2. shutdownBufferPool
1) initial buffer pool. 
2) write dirty pages to memory.
3) free memory. Return RC_OK. 

3. forceFlushPool 
1) Initial buffer pool
2) write dirty pages to blocks
3) check if dirty and fix count,
4) open pageful and then write data to blocks.Return RC_OK

/////////////////////////////
/*Page Management Functions*/
/////////////////////////////

1. markDirty 
1) check if the page exist
2) mark modified page ad dirty, set dirty bit to 1

2. unpinPag
1) check if page exist
2) if the client finishing using the page, notify buffer manager no need of the page once done with the reading or modifying, decrease the fix count and increase the total write count

3. forcePage
1) check if page exist
2) write current page back to the page file on disk, using functions openPageFile and writeBlock in assignment 1

4. pinPage
1) check if page exist
2) assign page number and page size to the PageHandle page
3) check all the occupied page frames in the buffer pool, if the requested page already cached in a page frame, buffer simply returns the pointer
4) if the page not cached in page frame, the buffer manager has to read this page from disk and store it to the buffer
5) store the page number into the page frame's pointer

/////////////////////////////
/*   Statistics Functions  */
/////////////////////////////

1. getFrameContents
1) returns an array of PageNumbers from BM_DataManagement
2) An empty page is represented by NO_PAGE

2. getDirtyFlags
1) returns an array of bools from BM_DataManagement

3. getFixCounts
1) returns an array of integers from BM_DataManagement

4. getNumReadIO
1) returns the number of pages that have been read from disk

5. getNumWriteIO
1) returns the number of pages that have been written to the page file
