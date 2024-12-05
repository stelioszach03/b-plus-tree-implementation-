#ifndef BP_INDEX_NODE_H
#define BP_INDEX_NODE_H

#include <record.h>
#include <bf.h>

// Forward declaration
struct BPLUS_INFO;

typedef struct
{
    int isLeaf;           // 0 για εσωτερικό κόμβο, 1 για φύλλο
    int numKeys;          // Αριθμός κλειδιών στον κόμβο
    int keys[20];         // Πίνακας κλειδιών
    int children[21];     // Δείκτες σε παιδιά (block numbers)
} BPLUS_INDEX_NODE;

#endif
