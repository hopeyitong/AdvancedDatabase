#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"


#define ASSERT_EQUALS_RECORDS(_l,_r, schema, message)			\
  do {									\
    Record *_lR = _l;                                                   \
    Record *_rR = _r;                                                   \
    ASSERT_TRUE(memcmp(_lR->data,_rR->data,getRecordSize(schema)) == 0, message); \
    int i;								\
    for(i = 0; i < schema->numAttr; i++)				\
      {									\
        Value *lVal, *rVal;                                             \
		char *lSer, *rSer; \
        getAttr(_lR, schema, i, &lVal);                                  \
        getAttr(_rR, schema, i, &rVal);                                  \
		lSer = serializeValue(lVal); \
		rSer = serializeValue(rVal); \
        ASSERT_EQUALS_STRING(lSer, rSer, "attr same");	\
		free(lVal); \
		free(rVal); \
		free(lSer); \
		free(rSer); \
      }									\
  } while(0)

#define ASSERT_EQUALS_RECORD_IN(_l,_r, rSize, schema, message)		\
  do {									\
    int i;								\
    boolean found = false;						\
    for(i = 0; i < rSize; i++)						\
      if (memcmp(_l->data,_r[i]->data,getRecordSize(schema)) == 0)	\
	found = true;							\
    ASSERT_TRUE(0, message);						\
  } while(0)

#define OP_TRUE(left, right, op, message)		\
  do {							\
    Value *result = (Value *) malloc(sizeof(Value));	\
    op(left, right, result);				\
    bool b = result->v.boolV;				\
    free(result);					\
    ASSERT_TRUE(b,message);				\
   } while (0)

// test methods
static void testDuplicateKeyInsert (void);

// struct for test records
typedef struct TestRecord {
  int a;
  char *b;
  int c;
} TestRecord;

// helper methods
Record *testRecord(Schema *schema, int a, char *b, int c);
Schema *testSchema (void);
Record *fromTestRecord (Schema *schema, TestRecord in);

// test name
char *testName;

// main method
int 
main (void) 
{
  testName = "";
  turnOnChedkMode();
  testDuplicateKeyInsert();
  return 0;
}


void 
testDuplicateKeyInsert(void)
{
  RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
  TestRecord inserts[] = { 
    {1, "aaaa", 3}, 
    {2, "bbbb", 2},
    {3, "cccc", 1},
    {4, "dddd", 3},
    {5, "eeee", 5},
    {6, "ffff", 1},
    {7, "gggg", 3},
    {8, "hhhh", 3},
    {9, "iiii", 2},
    {10, "jjjj", 5},
  };
  TestRecord realInserts[10000];
  TestRecord updates[] = {
    {8, "iiii", 6},
    {3, "iiii", 10},
  };
  int numInserts = 10, i;
  int randomRec = 8;
  Record *r;
  RID *rids;
  Schema *schema;
  testName = "test creating a new table and inserting 10 records then insert/updating duplicate record";
  schema = testSchema();
  rids = (RID *) malloc(sizeof(RID) * numInserts);
  
  TEST_CHECK(initRecordManager(NULL));
  TEST_CHECK(createTable("test_table_t",schema));
  TEST_CHECK(openTable(table, "test_table_t"));

  // insert rows into table
  for(i = 0; i < numInserts; i++)
    {
      realInserts[i] = inserts[i];
      r = fromTestRecord(schema, realInserts[i]);
      TEST_CHECK(insertRecord(table,r)); 
      rids[i] = r->id;
      freeRecord(r);
    }

  r = fromTestRecord(schema, realInserts[randomRec]);
  if(insertRecord(table,r) == RC_RM_DUPLICATE_KEY){
    printf("Duplicate Key Check Success: insert operation return RC_RM_DUPLICATE_KEY\n");
    freeRecord(r);
  }else{
    printf("Error in checking dupliacte key after insert.\n");
    exit(1);
  }
  
  r = fromTestRecord(schema, updates[0]);
  if(updateRecord(table,r) == RC_RM_DUPLICATE_KEY){
    printf("Duplicate Key Check Success: update operation return RC_RM_DUPLICATE_KEY\n");
    freeRecord(r);
  }else{
    printf("Error in checking dupliacte key after update.\n");
    exit(1);
  }

  //Update the same key in same RID.
  r = fromTestRecord(schema, updates[1]);
  r->id = rids[3-1];
  realInserts[3-1] = updates[1];
  if(updateRecord(table,r) == RC_OK){
    printf("Update Success: Update the same key in same RID place should be success.\n");
  }else{
    printf("Error in checking dupliacte key after update.\n");
    exit(1);
  }

  TEST_CHECK(closeTable(table));
  TEST_CHECK(openTable(table, "test_table_t"));

  // retrieve records from the table and compare to expected final stage
  for(i = 0; i < numInserts; i++)
    {
      RID rid = rids[i];
      TEST_CHECK(getRecord(table, rid, r));
      Record *r2 = fromTestRecord(schema, realInserts[i]);
      ASSERT_EQUALS_RECORDS(r2, r, schema, "compare records");
      freeRecord(r2);
    }


  TEST_CHECK(closeTable(table));
  TEST_CHECK(deleteTable("test_table_t"));
  TEST_CHECK(shutdownRecordManager());

  freeSchema(schema);
  freeRecord(r);
  free(rids);
  free(table);
  TEST_DONE();
}


Schema *
testSchema (void)
{
  Schema *result;
  char *names[] = { "a", "b", "c" };
  DataType dt[] = { DT_INT, DT_STRING, DT_INT };
  int sizes[] = { 0, 4, 0 };
  int keys[] = {0};
  int i;
  char **cpNames = (char **) malloc(sizeof(char*) * 3);
  DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
  int *cpSizes = (int *) malloc(sizeof(int) * 3);
  int *cpKeys = (int *) malloc(sizeof(int));

  for(i = 0; i < 3; i++)
    {
      cpNames[i] = (char *) malloc(2);
      strcpy(cpNames[i], names[i]);
    }
  memcpy(cpDt, dt, sizeof(DataType) * 3);
  memcpy(cpSizes, sizes, sizeof(int) * 3);
  memcpy(cpKeys, keys, sizeof(int));

  result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

  return result;
}

Record *
fromTestRecord (Schema *schema, TestRecord in)
{
  return testRecord(schema, in.a, in.b, in.c);
}

Record *
testRecord(Schema *schema, int a, char *b, int c)
{
  Record *result;
  Value *value;

  TEST_CHECK(createRecord(&result, schema));

  MAKE_VALUE(value, DT_INT, a);
  TEST_CHECK(setAttr(result, schema, 0, value));
  freeVal(value);

  MAKE_STRING_VALUE(value, b);
  TEST_CHECK(setAttr(result, schema, 1, value));
  freeVal(value);

  MAKE_VALUE(value, DT_INT, c);
  TEST_CHECK(setAttr(result, schema, 2, value));
  freeVal(value);

  return result;
}
