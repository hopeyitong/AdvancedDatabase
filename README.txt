///////////////////////////////////////////////////////////////////////////////
//////////////////////////// CS 525 Summer 2018 ///////////////////////////////
////////////////////// Yanbo Deng, Xinghang Ma, YH //////////////////
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

HOW TO RUN
1. Go to the folder which contains source/ and Makefile
2. > make
3. > ./build/assign1.out

Methods Description In storage_mgr.c

/////////////////////////////
/* manipulating page files */
/////////////////////////////

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
