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
/*    initial buffer pool   */
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
3)check if dirty and fix count,
4)open pageful and then write data to blocks.Return RC_OK



