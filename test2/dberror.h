#ifndef DBERROR_H
#define DBERROR_H

#include "stdio.h"

/* module wide constants */
#define PAGE_SIZE 4096

/* return code definitions */
typedef int RC;

#define RC_OK 0
#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4
#define RC_FILE_CLOSE_FAILED 5
#define RC_WRITE_NON_EXISTING_PAGE 6
#define RC_BAD_PARAMETER 7
#define RC_BM_NON_EXISTING_PAGE 8

#define RC_WRONG_STRATEGY 100
#define RC_WRONG_NUMPAGE  101
#define RC_NULL_BM        102
#define RC_CLOSE_FIXNODE  103
#define RC_NO_PAGE_FRAME  104
#define RC_NONE_FIX_COUNT 105
#define RC_CURRENT_NODE_ERROE 106
#define RC_ALL_FIXED  107
#define RC_NOT_FROM_TID 108

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205
#define RC_RM_TOMB 206
#define RC_RM_NOT_FROM_TID  207
#define RC_RM_WRONG_ID_SLOT 208
#define RC_RM_DUPLICATE_KEY 209
#define RC_RM_WRONG_DATATYPE  210

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 303
#define RC_IM_NODE_INVALID 304


/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError (RC error);
extern char *errorMessage (RC error);

#define THROW(rc,message) \
  do {			  \
    RC_message=message;	  \
    return rc;		  \
  } while (0)		  \

// check the return code and exit if it is an error
#define CHECK(code)							\
  do {									\
    int rc_internal = (code);						\
    if (rc_internal != RC_OK)						\
      {									\
	char *message = errorMessage(rc_internal);			\
	printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
	free(message);							\
	exit(1);							\
      }									\
  } while(0);


#endif
