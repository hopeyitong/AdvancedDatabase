/////////////////////////////
/* table and manager */
/////////////////////////////

1. initRecordManager

2. shutdownRecordManager

3. createTable
1) use function serializeSchema in rm_serializer to retieve schema info
2) get how many page to save schema info
3) allocate memory to save shema info, store shema page numberand schema info to this memory

4. openTable
1) get how many page to save schema info
2) allocate memory for schema and read it
3) initialize keySize and keys memory
4) check schema info and attribute info
5) read datatype and typelenth from schema
6) assign schema details

5. closeTable
1) shutdown buffer pool
2) free schema using function freeSchema

6. deleteTable

7. getNumTuples
1) return number of tuples using pinPage function
