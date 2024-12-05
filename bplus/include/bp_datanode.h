#ifndef BP_DATANODE_H
#define BP_DATANODE_H

#include <record.h>
#include <bf.h>

// Forward declaration
struct BPLUS_INFO;

#define MAX_RECORDS ((BF_BLOCK_SIZE - sizeof(int) * 2) / sizeof(Record))

typedef struct {
    int numRecords;
    int nextBlock;
    Record records[MAX_RECORDS];
} BPLUS_DATA_NODE;

#endif 