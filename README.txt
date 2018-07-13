///////////////////////////////////////////////////////////////////////////////
//////////////////////////// CS 525 Summer 2018 ///////////////////////////////
////////////////////// Yanbo Deng, Xinghang Ma, Yitong Huang //////////////////
///////////////////////////////////////////////////////////////////////////////
   #
  # #     ####    ####      #     ####   #    #  #    #  ######  #    #   #####
 #   #   #       #          #    #    #  ##   #  ##  ##  #       ##   #     #
#     #   ####    ####      #    #       # #  #  # ## #  #####   # #  #     #
#######       #       #     #    #  ###  #  # #  #    #  #       #  # #     #
#     #  #    #  #    #     #    #    #  #   ##  #    #  #       #   ##     #
#     #   ####    ####      #     ####   #    #  #    #  ######  #    #     #

                                       #
                                      ##
                                     # #
                                       #
                                       #
                                       #
                                     #####

********** HOW TO RUN **********
1. Go to the folder which contains source/ README.txt and Makefile
2. > make
3. > ./build/assign1.out
4. > make clean

********** Extra Test Cases ***********
1. testWriteMethods
2. testReadMethods

********* Methods Description In storage_mgr.c **********

/////////////////////////////
/* manipulating page files */
/////////////////////////////

1. readBlock 
1) Checks if the file exists.
2) Check if the pageNum is invalid, returns RC_READ_NON_EXISTING_PAGE.
3) If the file exists and pageNum is valid, sets the file pointer to the pageNum
  position, reads its contents to memPage and I the read failed print “Read failed”.
  Otherwise Returns RC_OK.

2. getBlockPos 
1) Check If the file already exists. 
2) Return current page position. 

3. readFirstBlock 
1) Read the First block is to set the pointer at 0 position

4. readPreviousBlock 
1) If the current page position is at the first page or smaller and the total page
  numbers are smaller than current page position it returns RC_READ_NON_EXISTING_PAGE;
2) In other case it will return to the current page position minus one which is previous position..

5. readCurrentBlock 
1) Checks if the file exists and if current Page Position is valid.
3) If the file does not exist, returns RC_READ_NON_EXISTING_PAGE.
4) If the file exists and current Page Position is valid, return RC_OK  

6. readNextBlock 
1) If the last Next block is exceed the last page file ,then it should
  returns RC_READ_NON_EXISTING_PAGE
2) In other cases ,it returns reading the current positon plus 1 which is the next block 

7. readLastBlock 
1) Read the block at the position which is total number pages minus 1.the last block
  should returns RC_OK.

/////////////////////////////
/*    reading page files   */
/////////////////////////////

1. readBlock 
1) Checks if the file exists.
2) Check if the pageNum is invalid, returns RC_READ_NON_EXISTING_PAGE.
3) If the file exists and pageNum is valid, sets the file pointer to the pageNum
  position, reads its contents to memPage and I the read failed print “Read failed”.
  Otherwise Returns RC_OK.

2. getBlockPos 
1) Check If the file already exists. 
2) Return current page position. 

3. readFirstBlock 
1) Read the First block is to set the pointer at 0 position

4. readPreviousBlock 
1) If the current page position is at the first page or smaller and the total page
  numbers are smaller than current page position it returns RC_READ_NON_EXISTING_PAGE;
2) In other case it will return to the current page position minus one which is previous position..

5. readCurrentBlock 
1) Checks if the file exists and if current Page Position is valid.
3) If the file does not exist, returns RC_READ_NON_EXISTING_PAGE.
4) If the file exists and current Page Position is valid, return RC_OK  

6. readNextBlock 
1) If the last Next block is exceed the last page file ,then it should
  returns RC_READ_NON_EXISTING_PAGE
2) In other cases ,it returns reading the current positon plus 1 which is the next block 

7. readLastBlock 
1) Read the block at the position which is total number pages minus 1.the last block
  should returns RC_OK.

/////////////////////////////
/*    writing page files   */
/////////////////////////////

1. writeBlock
1) check the file handle has been initialized 
2) check the pageNum is valid
3) set the file pointer to correct block and write PAGE_SIZE from memory to file

2. writeCurrentBlock
1）get current block position (current block) from file handle
2) call writeBlock to write current block to file

3. appendEmptyBlock
1) check file Handle has correctly initialized
2) initialize a page handle, allocate PAGE_SIZE memory and fill it with '\0'
3) set the file header to the (end + 1) of the file handle pointer and write the page handle here

4. ensureCapacity
1) check the file handle has been initialized
2) if the file handle has enough pages, return RC_OK
  else compute how many pages we need to add to file handle
3) call appendEmptyBlock to add the number of empty pages to file handle
