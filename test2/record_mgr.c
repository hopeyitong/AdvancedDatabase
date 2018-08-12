#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "record_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dberror.h"
#include "expr.h"
#include "tables.h"


#define SLOTSIZE 64
RC addNewDataBlock(SM_FileHandle *fh, int recordInSlotSize);
int getSuperPageNumber(BM_BufferPool *bm);
int getEndPageNumber(BM_BufferPool *bm);
int getStartSlotInDataPage();
void turnOnChedkMode();
void turnOffChedkMode();

bool checkDuplicateKey = false; 
//Whether active the mode of check duplicate primary key.

RC initRecordManager (void *mgmtData) {
    printf("Start to generate record manager5!\n");
    return RC_OK;
}

RC shutdownRecordManager () {
    printf("Shutdown record manager!\n");
    return RC_OK;
}


// table and manager
RC createTable (char *name, Schema *schema) {
    createPageFile(name);
    SM_FileHandle fh; 
    openPageFile(name, &fh);
//use function serializeSchema in rm_serializer to retieve schema info
    char* getSchema = serializeSchema(schema);
//get how many page to save schema info
    int schemaPageNum = (sizeof(getSchema)  + PAGE_SIZE)/(PAGE_SIZE+1);
//allocate memory to save shema info, store shema page numberand schema info to this memory
    char* metadataPage = (char *)malloc(PAGE_SIZE);
    memset(metadataPage, 0, sizeof(metadataPage));
    memmove(metadataPage, &schemaPageNum, sizeof(schemaPageNum));
    memmove(metadataPage+sizeof(schemaPageNum), getSchema, strlen(getSchema)); 
    writeBlock(0, &fh, metadataPage);
    int i=1;
    while(i < schemaPageNum){
	memset(metadataPage, 0, sizeof(metadataPage));
        memmove(metadataPage, i * PAGE_SIZE + getSchema, PAGE_SIZE);
	writeBlock(i, &fh, metadataPage);
        free(metadataPage);
	i=i+1;
    }
    return RC_OK;
}

RC openTable (RM_TableData *rel, char *name) {
    SM_FileHandle fh;
    openPageFile(name, &fh);
    BM_BufferPool *bm = (BM_BufferPool*) malloc(sizeof(BM_BufferPool));
    initBufferPool(bm, name, 1000, RS_FIFO, NULL);
//get how many page to save schema info
    BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
    pinPage(bm, ph, 0);
    int metadataPage =*ph->data;
//allocate memory for schema and read it
    char* metadata = (char *)malloc(sizeof(char)*PAGE_SIZE);
    int i=0;
    while(i < metadataPage){
	readBlock(i, &fh, i * PAGE_SIZE + metadata);
	i=i+1;
    }
//initialize keySize and keys memory
    int keySize=1;
    int *keys=(int *)malloc(sizeof(int));
//check schema info
    char *attrInfo = strstr(metadata + 4, "<")+1;
    printf("Schema has %s \n",  attrInfo-1 );
    char* attrList = (char *)malloc(sizeof(char));
    memset(attrList, 0, sizeof(attrList));
//read number of attributes and assign it to numAttr
    i = 0;
    while(*(attrInfo + i) != '>'){
        attrList[i] = attrInfo[i];
	i = i+1;
    }
    int numAttr = atol(attrList);
    char **attrNames = (char**) malloc(sizeof(char *)*numAttr);
    free(attrList);
//get posistion of "("
    attrInfo = strstr(attrInfo, "(")+1;
    DataType *dataType = (DataType*) malloc(sizeof(DataType)*numAttr);
    int *typeLength = (int*) malloc(sizeof(int)*numAttr);
    i=0;
    int j;
//read datatype and typelenth from schema
    while(i<numAttr){
        for(j = 0; ; j++){
		int size = 50;
		char *str = (char *)malloc(sizeof(char)*size);
		str=attrInfo + j;
  		char *dest= (char *)malloc(sizeof(char)*size);
		strncpy(dest, str, 1);
		if(strcmp(dest,":")==0){
		attrNames[i] = (char*) malloc(sizeof(char)*j);
                memcpy(attrNames[i], attrInfo, j);

		char *str1 = (char *)malloc(sizeof(char)*size);
		str1=attrInfo + j + 2;
  		char *dest1= (char *)malloc(sizeof(char)*size);
		char *dest2= (char *)malloc(sizeof(char)*size);
		char *dest3= (char *)malloc(sizeof(char)*size);
		strncpy(dest1, str1, 6);
		strncpy(dest2, str1, 3);
		strncpy(dest3, str1, 5);

		if(strcmp(dest1,"STRING")==0){                      
		    attrList=(char *)malloc(sizeof(char));
                    int k=0;
		    while(*(attrInfo + j + 9 + k)!=']'){
		        attrList[k] = attrInfo[k+j+9];
		        k=k+1;
		    }
                    dataType[i] = DT_STRING;
                    typeLength[i] = atol(attrList);           
                }else if(strcmp(dest2,"INT")==0){ 
                    dataType[i] = DT_INT;       
                    typeLength[i] = 0;
                }else if(strcmp(dest3,"FLOAT")==0){
                    dataType[i] = DT_FLOAT;              
                    typeLength[i] = 0;
                } else {
                    dataType[i] = DT_BOOL;              
                    typeLength[i] = 0;
                } 
//check if read to the last attribute
               if (i != numAttr-1){
                    attrInfo = strstr(attrInfo, ",");
                    attrInfo = attrInfo + 2;
                }
                break;

            }
        }i++;
    }
//assign schema details
    Schema *schema = createSchema(numAttr, attrNames, dataType, typeLength, keySize, keys);
    rel->name = name;
    rel->schema = schema;
    rel->mgmtData = bm;
    return RC_OK;
}


RC closeTable (RM_TableData *rel){

	
    shutdownBufferPool((BM_BufferPool*)rel->mgmtData);

    freeSchema(rel->schema);
    free(rel->mgmtData);
    int i = 0;
    while(i < rel->schema->numAttr){
	free(rel->schema->attrNames[i]);
        i = i+1;
    }
    return RC_OK;
}

RC deleteTable (char *name){
     remove(name);
     return RC_OK;
}


int getNumTuples (RM_TableData *rel){
    BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
    int tupleNum;
    pinPage(rel->mgmtData, ph, 0);
    return *ph->data+ sizeof(int);

}

// handling records in a table

//Firstly use checkNewRecord() check whether the new record keep the duplicate key values, if have, return error.
//If there exist tomb in a page, the record will be priorly saved in tomb palce. The pagetomb value at the beginning of page will update.
//Otherwise, it will saved in a empty slot. Additionally, if all pages are full, we will add a new data page.
RC insertRecord (RM_TableData *rel, Record *record){
	
	//If the check mode is on.
	if(checkDuplicateKey){
		RID id;
		id.page = -1;	
		//If the new record keep the duplicate key values, return error.
		if (checkNewRecord(rel, record, id) == false){
			return RC_RM_DUPLICATE_KEY;
		}
	}

	BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
	int recordSize = getRecordSize(rel->schema);
	int recordInSlotSize = (recordSize + sizeof(bool) + sizeof(RID) + SLOTSIZE -1)/SLOTSIZE;
	int startSlot = getStartSlotInDataPage(); //The first slot of data position.
	int startPage = getSuperPageNumber(rel->mgmtData);
	int endPage = getEndPageNumber(rel->mgmtData);

	int aimSlot;
	for(; startPage <= endPage; startPage++){
		pinPage(rel->mgmtData, ph, startPage); //Get the page.

		int pagetomb;
		memcpy(&pagetomb, ph->data + 3*sizeof(int), sizeof(int)); //Get the tomb slot postion in this page at the beginning.
		if(pagetomb > 0){ //If tomb exists, 
			aimSlot = pagetomb; //Aim the insert slot as the tomb position.
			RID TID;
			memcpy(&TID, ph->data + SLOTSIZE * aimSlot + sizeof(bool), sizeof(RID));
			
			//If tomb point to another tomb, update this value as first tomb position into pagetomb. 
			if(TID.slot > 0){
				pagetomb = TID.slot;
			}else pagetomb = -1;
			memcpy(ph->data + 3*sizeof(int), &pagetomb,  sizeof(int));
			markDirty(rel->mgmtData, ph);
			break;
		}

		//If no tomb, check whther the recordNum is full.
		int recordNum;
		memcpy(&recordNum, ph->data + sizeof(int), sizeof(int)); //Get the used record number in this page.
		int maxNum;
		memcpy(&maxNum, ph->data + 2*sizeof(int), sizeof(int)); //Get the max record number in this page.
		if(recordNum < maxNum){
			aimSlot = startSlot + recordNum*recordInSlotSize; //Set the aimed slot at the first empty slot.
			recordNum++;

			memcpy(ph->data + sizeof(int), &recordNum, sizeof(int));
			markDirty(rel->mgmtData, ph);
			break;
		}
		unpinPage(rel->mgmtData, ph);
	}

	//If all existed pages do not has slots to insert this record, we should add a new page to save.
	if(startPage == endPage+1){
		SM_FileHandle fHandle;
		RC ans =  openPageFile(((BM_BufferPool *)(rel->mgmtData))->pageFile, &fHandle);
		if(ans != RC_OK) return ans;
		ans = addNewDataBlock(&fHandle, recordInSlotSize);
		if(ans != RC_OK) return ans;
		pinPage(rel->mgmtData, ph, startPage);
		aimSlot = startSlot;
		int recordNum = 1;
		memcpy(ph->data + sizeof(int), &recordNum, sizeof(int));
		markDirty(rel->mgmtData, ph);
		closePageFile(&fHandle);
	}

	//Fresh the tomb value in this record.
	bool tomb = false;
	memcpy(ph->data + SLOTSIZE * aimSlot, &tomb, sizeof(bool));
	//Fresh the tid value in this record.
	RID TID;
	TID.page = -1;
	TID.slot = -1;
	memcpy(ph->data + SLOTSIZE * aimSlot + sizeof(bool), &TID, sizeof(RID));
	//Save the record into these slots.
	memcpy(ph->data + SLOTSIZE * aimSlot + sizeof(bool) + sizeof(RID), record->data, recordSize);
	markDirty(rel->mgmtData, ph);
	unpinPage(rel->mgmtData, ph);

	//Update the tuple number on the super page.
	pinPage(rel->mgmtData, ph, 0);
	int tupleNum;
	memcpy(&tupleNum, ph->data + 2 * sizeof(int), sizeof(int));
	tupleNum++;
	memcpy(ph->data + 2 * sizeof(int), &tupleNum, sizeof(int));
	markDirty(rel->mgmtData, ph);
	unpinPage(rel->mgmtData, ph);


	record->id.slot = aimSlot;
	record->id.page = startPage;

	free(ph);
	return RC_OK;
}

//Similar to insertRecord. 
//Find the postion and pin the page to bufferPool.
//Firstly, check the tomb, if it is true, return tomb error, otherwise, mark this place as a tomb and update TID and total tuple number.
RC deleteRecord (RM_TableData *rel, RID id){
	
	BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
	pinPage(rel->mgmtData, ph, id.page);

	int pagetomb;
	memcpy(&pagetomb, ph->data + 3*sizeof(int), sizeof(int));
	
	//Check whether it is already a tomb.
	bool tomb;
	memcpy(&tomb, ph->data + id.slot * SLOTSIZE, sizeof(bool));
	if(tomb == true){
		unpinPage(rel->mgmtData, ph);
		free(ph);
		return RC_RM_TOMB;
	}
	//Update tomb.
	tomb = true;
	memcpy(ph->data + id.slot * SLOTSIZE, &tomb, sizeof(bool));
	//Update TID to next tomb.
	RID TID;
	TID.slot = pagetomb;
	TID.page = id.page;
	memcpy(ph->data + id.slot * SLOTSIZE + sizeof(bool), &TID, sizeof(RID));
	//Update the first tomb postion into pagetomb. 
	pagetomb = id.slot;
	memcpy(ph->data + 3*sizeof(int), &pagetomb, sizeof(int));
	markDirty(rel->mgmtData, ph);
	unpinPage(rel->mgmtData, ph);

	//Update the tuple number on the super page.
	pinPage(rel->mgmtData, ph, 0);
	int tupleNum;
	memcpy(&tupleNum, ph->data + 2 * sizeof(int), sizeof(int));
	tupleNum--;
	memcpy(ph->data + 2 * sizeof(int), &tupleNum, sizeof(int));
	markDirty(rel->mgmtData, ph);
	unpinPage(rel->mgmtData, ph);

	free(ph);
	return RC_OK;
}

extern RC updateRecord (RM_TableData *rel, Record *record){
	
	//If the check mode is on.
	if(checkDuplicateKey){
		//If the new record keep the duplicate key values, return error.
		if (checkNewRecord(rel, record, record->id) == false){
			return RC_RM_DUPLICATE_KEY;
		}
	}

	BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
	pinPage(rel->mgmtData, ph, record->id.page);
	int recordSize = getRecordSize(rel->schema);


	
	//Check whether it is already a tomb.
	bool tomb;
	memcpy(&tomb, ph->data + record->id.slot * SLOTSIZE, sizeof(bool));
	if(tomb == true){
		unpinPage(rel->mgmtData, ph);
		free(ph);
		return RC_RM_TOMB;
	}


	memcpy(ph->data +  record->id.slot * SLOTSIZE + sizeof(bool) + sizeof(RID), record->data, recordSize);
	markDirty(rel->mgmtData, ph);
	unpinPage(rel->mgmtData, ph);
	free(ph);

	return RC_OK;
}

//Check whether the new record keep the same key values with some tuples in the table. 
bool checkNewRecord(RM_TableData *rel, Record *record, RID updateID){
	int keynum = rel->schema->keySize;
	int *keyIndex = rel->schema->keyAttrs;
	//DataType *types = rel->schema->dataTypes;
	Value **recordKey = (Value **) malloc(sizeof(Value*) * keynum);

	int i;
	for(i = 0; i < keynum; i++){
		getAttr(record, rel->schema, keyIndex[i], &(recordKey[i]));
	}

	BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
	int recordSize = getRecordSize(rel->schema);
	int recordInSlotSize = (recordSize + sizeof(bool) + sizeof(RID) + SLOTSIZE -1)/SLOTSIZE;
	int startSlot = getStartSlotInDataPage(); 
	int startPage = getSuperPageNumber(rel->mgmtData);
	int endPage = getEndPageNumber(rel->mgmtData);

	Record *recordTry;
	createRecord(&recordTry, rel->schema);
	
	for(; startPage <= endPage; startPage++){
		pinPage(rel->mgmtData, ph, startPage); //Get the page.

		int recordNum;
		memcpy(&recordNum, ph->data + sizeof(int), sizeof(int)); //Get the used record number in this page.
		
		int curSlot;
		int flag = 0;
		for(curSlot = startSlot; curSlot < startSlot + recordNum*recordInSlotSize; curSlot += recordInSlotSize){
			RID id;
			id.page = startPage;
			id.slot = curSlot;
			//If the checking position is the same with update record position, skip it. 
			if(updateID.page != -1 && updateID.page == id.page && updateID.slot == id.slot)
				continue;
			if(getRecord(rel, id, recordTry) == RC_OK){
				for(i = 0;  i < keynum; i++){
					Value *tuplekey;
					getAttr(recordTry, rel->schema, keyIndex[i], &tuplekey);

					switch(tuplekey->dt){
						case 0:
							if(tuplekey->v.intV == recordKey[i]->v.intV) flag++;
							break;
						case 1:
							if(strcmp(tuplekey->v.stringV, recordKey[i]->v.stringV) == 0) flag++;
							break;
						case 2:
							if(tuplekey->v.floatV == recordKey[i]->v.floatV) flag++;
							break;
						case 3:
							if(tuplekey->v.boolV == recordKey[i]->v.boolV) flag++;
							break;

					}
					free(tuplekey);
					if(flag != i+1){
						flag = 0;
						break;
					}
				}

				if(flag == keynum){//Means all the key values match, so the insert record is incorrect with these tuples.
					//Free space.
					for(i = 0; i < keynum; i++){
						free(recordKey[i]);
					}
					free(recordKey);
					freeRecord(recordTry);
					unpinPage(rel->mgmtData, ph);
					free(ph);
					return false;
				}
			}
		}
		unpinPage(rel->mgmtData, ph);
	}

	//Free space.
	for(i = 0; i < keynum; i++){
		free(recordKey[i]);
	}
	free(recordKey);
	freeRecord(recordTry);
	free(ph);
	return true;
}

//The Record is not from TID.
RC getRecord (RM_TableData *rel, RID id, Record *record){
	return getRecordWithTID (rel, id, record, false);
}

// If FromTID is true, it means the RID is from a TID.
RC getRecordWithTID (RM_TableData *rel, RID id, Record *record, bool fromTID){
	//Check the currectness of slot position.
	int recordSize = getRecordSize(rel->schema);
	int recordSizeInSlots = (recordSize + sizeof(bool) + sizeof(RID) + SLOTSIZE -1)/SLOTSIZE;//Calculate one record with tomb indetifier and TID need how many slots.
	int stateSlotSize = getStartSlotInDataPage();
	
	//The RID slot minus startSlot should can be divided by slot size of each record.
	if((id.slot - stateSlotSize) % recordSizeInSlots != 0)
		return RC_RM_WRONG_ID_SLOT;
	
	BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
	RC ans = pinPage(rel->mgmtData, ph, id.page);
	if(ans != RC_OK) return ans;

	bool tomb;
	memcpy(&tomb, ph->data + SLOTSIZE * id.slot, sizeof(bool));
	if(tomb){//If it is a tomb, return RC_RM_TOMB.
		unpinPage(rel->mgmtData, ph);
		free(ph);
		return RC_RM_TOMB;
	}else{
		RID TID;
		memcpy(&TID, ph->data + SLOTSIZE * id.slot + sizeof(bool), sizeof(RID));
		if(TID.page == -1){//Page = -1 means that TID does not exists, just get the data.
			memcpy(record->data, ph->data + + SLOTSIZE * id.slot + sizeof(bool) + sizeof(RID), recordSize);
		}else if(TID.page == 0){//Page = 0 means this record is pointed by a TID from other place.
			if(fromTID == true){//If this call is from a TID.
				memcpy(record->data, ph->data + + SLOTSIZE * id.slot + sizeof(bool) + sizeof(RID), recordSize);
			}
			else{
				unpinPage(rel->mgmtData, ph);
				free(ph);
				return RC_RM_NOT_FROM_TID; //If not from TID, return error.
			}
		}else{//Other wise go to find record in the TID position.
			ans = getRecordWithTID(rel, TID, record, true);
			if(ans != RC_OK){
				unpinPage(rel->mgmtData, ph);
				free(ph);
				return ans;
			}
		}
	}

	record->id = id;
	unpinPage(rel->mgmtData, ph);
	free(ph);
	return RC_OK;
}

// scans

//Simply record position and expression into the table structure.
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
	scan->rel = rel;
	scan->expr = cond;
	scan->cur.page = getSuperPageNumber(scan->rel->mgmtData); //Get the start page of tuples data.
	scan->cur.slot = getStartSlotInDataPage(); 

	return RC_OK;
}

//Find the next tuple which suits the expr.
RC next (RM_ScanHandle *scan, Record *record){
	BM_BufferPool *bm = (BM_BufferPool*)scan->rel->mgmtData;
	BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
	
	//Calculate one record with tomb indetifier and TID need how many slots.
	int recordSizeInSlots = (getRecordSize(scan->rel->schema) + sizeof(bool) + sizeof(RID) + SLOTSIZE -1)/SLOTSIZE;

	int stateSlotSize = getStartSlotInDataPage();

	//Get the number of last page from fileHandle.
	int endPage = getEndPageNumber(bm);

	Record *recordTry;
	createRecord(&recordTry, scan->rel->schema);
	Value *result = (Value*) malloc(sizeof(Value));
	while(scan->cur.page <= endPage){
		RC ans = pinPage(bm, ph, scan->cur.page); //Get the page where scan points.
		if(ans != RC_OK){
			unpinPage(bm, ph);
			freeRecord(recordTry);
			free(result);
			free(ph);
			return ans;
		}
		int recordNum;
		memcpy(&recordNum, ph->data + sizeof(int), sizeof(int)); //Get the used record number in this page.

		for(; scan->cur.slot < recordNum*recordSizeInSlots + stateSlotSize; scan->cur.slot += recordSizeInSlots){
			if(getRecord(scan->rel, scan->cur, recordTry) == RC_OK){
				evalExpr(recordTry, scan->rel->schema, scan->expr, &result);
				
				//If the recordTry suits the expr, save the rid to record.
				if(result->v.boolV){
					record->id.page = recordTry->id.page;
					record->id.slot = recordTry->id.slot;
					memcpy(record->data, recordTry->data, getRecordSize(scan->rel->schema));

					//Move the RID cur of scan to the next position.
					if(scan->cur.slot + recordSizeInSlots < (recordNum-1)*recordSizeInSlots){
						scan->cur.slot += recordSizeInSlots;
					}else{
						scan->cur.slot = 0;
						scan->cur.page++;
					}

					unpinPage(bm, ph);
					freeRecord(recordTry);
					free(result);
					free(ph);
					return RC_OK;
				}
			}
		}
		scan->cur.slot = 0;
		scan->cur.page++;
		unpinPage(bm, ph);

	}

	freeRecord(recordTry);
	free(result);
	free(ph);
	return RC_RM_NO_MORE_TUPLES;
}

RC closeScan (RM_ScanHandle *scan){
	//Nothing need be freed in scan.
	return RC_OK;
}

// dealing with schemas

//Simply calculate total size according to the size or length of each type 
int getRecordSize (Schema *schema){
	int index;
	int size = 0;

	for(index = 0; index < schema->numAttr; index++){
		switch(schema->dataTypes[index]){ //Correspondingly add the attribute size.
			case 0:
				size += sizeof(int);
				break;
			case 1:
				size += schema->typeLength[index];
				break;
			case 2:
				size += sizeof(float);
				break;
			case 3:
				size += sizeof(bool);
				break;
		}
	}
	return size;
}

//Simply allocate space and save all the parameter into it.
Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){
	Schema *schema = (Schema*) malloc(sizeof(Schema));
    schema->numAttr = numAttr;
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->typeLength = typeLength;
    schema->keyAttrs = keys;
    schema->keySize = keySize;
    return schema;
}

//Simply free the space allocated for schema pointer.
RC freeSchema (Schema *schema){
	int index;
	for(index = 0; index < schema->numAttr; index++){
		free(schema->attrNames[index]);
	}
	free(schema->attrNames);
	free(schema->dataTypes);
	free(schema->typeLength);
	free(schema->keyAttrs);
	free(schema);

	return RC_OK;
}

// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema){
	*record = (Record *) malloc(sizeof(Record));
	(*record)->data = (char *) malloc(getRecordSize(schema)* sizeof(char));

	return RC_OK;
}

//Simply free the space in record pointer.
RC freeRecord (Record *record){
	free(record->data);
	free(record);

	return RC_OK;
}

//Simply calculte the position of aimed attribute and memcpy it out according to its type and length.
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
	int i;
	int pos = 0;

	//Caculate the position of aimed attribute.
	for(i = 0; i < attrNum; i++){
		switch(schema->dataTypes[i]){ //Correspondingly add the attribute size.
			case 0:
				pos += sizeof(int);
				break;
			case 1:
				pos += schema->typeLength[i];
				break;
			case 2:
				pos += sizeof(float);
				break;
			case 3:
				pos += sizeof(bool);
				break;
		}
	}

	*value = (Value *) malloc(sizeof(Value));
	(*value)->dt = schema->dataTypes[attrNum];
	switch((*value)->dt){
		case 0:
			memcpy(&((*value)->v.intV), record->data + pos, sizeof(int));
			break;
		case 1:
			(*value)->v.stringV = (char *) malloc(schema->typeLength[attrNum] + 1);
        	memcpy((*value)->v.stringV, record->data + pos, schema->typeLength[attrNum]);
        	char stringEnd = '\0';
        	memcpy((*value)->v.stringV + schema->typeLength[attrNum], &stringEnd, sizeof(char));
			break;
		case 2:
			memcpy(&((*value)->v.floatV), record->data + pos, sizeof(float));
			break;
		case 3:
			memcpy(&((*value)->v.boolV), record->data + pos, sizeof(bool));
			break;
	}
	return RC_OK;
}

//Simply calculte the position of aimed attribute and memcpy the value into it according to its type and length.
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
	int i;
	int pos = 0;

	//Caculate the position of aimed attribute.
	for(i = 0; i < attrNum; i++){
		switch(schema->dataTypes[i]){ //Correspondingly add the attribute size.
			case 0:
				pos += sizeof(int);
				break;
			case 1:
				pos += schema->typeLength[i];
				break;
			case 2:
				pos += sizeof(float);
				break;
			case 3:
				pos += sizeof(bool);
				break;
		}
	}

	int size = 0;
	switch(value->dt){
		case 0:
			memcpy(record->data + pos, &(value->v.intV), sizeof(int));
			break;
		case 1:
			//Set the size to the minimum of input string length or recorded type length.
			if (strlen(value->v.stringV) >= schema->typeLength[attrNum]) {
				size = schema->typeLength[attrNum];
	        } else {
	            size = strlen(value->v.stringV);
	        }
	        memcpy(record->data + pos, value->v.stringV, size);
			break;
		case 2:
			memcpy(record->data + pos, &(value->v.floatV), sizeof(float));
			break;
		case 3:
			memcpy(record->data + pos, &(value->v.boolV), sizeof(int));
			break;
	}
	return RC_OK;
}

//Add a new data page to file.
RC addNewDataBlock(SM_FileHandle *fh, int recordInSlotSize){

	RC ans = appendEmptyBlock(fh);
    if (ans != RC_OK) return ans;

    //Fresh the four state attribute into the empty data page.
    char* dataPage = (char *) calloc(PAGE_SIZE, sizeof(char));
    int pageNum = fh->totalNumPages - 1;
    memcpy(dataPage, &pageNum, sizeof(int));
    int recordNum = 0;
    memcpy(dataPage + sizeof(int), &recordNum, sizeof(int));
    int maxNum = (PAGE_SIZE - 4*sizeof(int)) / (recordInSlotSize * SLOTSIZE);
    memcpy(dataPage + 2*sizeof(int), &maxNum, sizeof(int));
    int tomb = -1;
    memcpy(dataPage + 3*sizeof(int), &tomb, sizeof(int));

    ans = writeBlock(pageNum, fh, dataPage);
    if (ans != RC_OK) return ans;

    free(dataPage);
    return RC_OK;
}

//Get the size of super pages which contains info of table.
//In other words, get the start page number of data page which stores tuples.
int getSuperPageNumber(BM_BufferPool *bm){
	int size;
	BM_PageHandle *ph = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
	pinPage(bm, ph, 0);
    memcpy(&size, ph->data, sizeof(int));
    unpinPage(bm, ph);
    free(ph);
    return size;
}


//Get the number of last page from fileHandle.
int getEndPageNumber(BM_BufferPool *bm){
	SM_FileHandle fHandle;
	RC ans =  openPageFile(bm->pageFile, &fHandle);
	if(ans != RC_OK) return ans;
	int endPage = fHandle.totalNumPages - 1;
	closePageFile(&fHandle);
	return endPage;
}

//Find the first slot saved tuple, skipping the four title attributes.
//In otherword, calculate the number of slots occupied by the four state attributes(pageNo, recordNum, maxNum, tomb).
int getStartSlotInDataPage(){
	return (4*sizeof(int) + SLOTSIZE -1)/ SLOTSIZE;
}

//Turn on the mode of checking duplicate primary key.
void turnOnChedkMode(){
	checkDuplicateKey = true;
}

//Turn off the mode of checking duplicate primary key.
void turnOffChedkMode(){
	checkDuplicateKey = false;
}
