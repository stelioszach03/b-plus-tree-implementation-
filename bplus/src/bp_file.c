#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_datanode.h"
#include "bp_indexnode.h"
#include <stdbool.h>

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return -1;              \
    }                         \
  }

#define CALL_BF_PTR(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return NULL;            \
    }                         \
  }

// Πίνακας ανοιχτών αρχείων B+ δέντρου (μέγιστο 20 αρχεία)
static BPLUS_INFO* openFiles[20] = {NULL};

// Βοηθητική συνάρτηση για την εύρεση του κατάλληλου φύλλου
static int findLeafBlock(int file_desc, BPLUS_INFO* bplus_info, int key) {
    if (bplus_info->root == -1) return -1;

    BF_Block *block;
    BF_Block_Init(&block);
    int current = bplus_info->root;

    while (true) {
        CALL_BF(BF_GetBlock(file_desc, current, block));
        BPLUS_INDEX_NODE* node = (BPLUS_INDEX_NODE*)BF_Block_GetData(block);

        if (node->isLeaf) {
            CALL_BF(BF_UnpinBlock(block));
            BF_Block_Destroy(&block);
            return current;
        }

        // Αναζήτηση του κατάλληλου παιδιού
        int i;
        for (i = 0; i < node->numKeys; i++) {
            if (key < node->keys[i]) break;
        }

        current = node->children[i];
        CALL_BF(BF_UnpinBlock(block));
    }
}

// Βοηθητική συνάρτηση για τη διάσπαση φύλλου
static int splitLeafNode(int file_desc, BPLUS_INFO* bplus_info, BF_Block* block, Record* record) {
    BF_Block *newBlock;
    BF_Block_Init(&newBlock);

    // Δημιουργία νέου block
    CALL_BF(BF_AllocateBlock(file_desc, newBlock));
    int newBlockId = bplus_info->blockCounter++;

    // Λήψη δεδομένων από τα blocks
    BPLUS_DATA_NODE* oldNode = (BPLUS_DATA_NODE*)BF_Block_GetData(block);
    BPLUS_DATA_NODE* newNode = (BPLUS_DATA_NODE*)BF_Block_GetData(newBlock);

    // Αρχικοποίηση του νέου κόμβου
    newNode->numRecords = 0;
    newNode->nextBlock = oldNode->nextBlock;
    oldNode->nextBlock = newBlockId;

    // Υπολογισμός του μέσου σημείου
    int midPoint = (MAX_RECORDS + 1) / 2;
    int totalRecords = oldNode->numRecords + 1;  // +1 για τη νέα εγγραφή

    // Προσωρινός πίνακας για ταξινόμηση
    Record* tempRecords = malloc((MAX_RECORDS + 1) * sizeof(Record));
    if (tempRecords == NULL) {
        BF_Block_Destroy(&newBlock);
        return -1;
    }
    int insertPos = 0;

    // Αντιγραφή και ταξινόμηση των εγγραφών
    for (int i = 0; i < oldNode->numRecords; i++) {
        if (insertPos == midPoint && record->id < oldNode->records[i].id) {
            tempRecords[insertPos++] = *record;
        }
        tempRecords[insertPos++] = oldNode->records[i];
    }
    if (insertPos <= midPoint) {
        tempRecords[insertPos] = *record;
    }

    // Διανομή των εγγραφών
    oldNode->numRecords = midPoint;
    memcpy(oldNode->records, tempRecords, midPoint * sizeof(Record));

    newNode->numRecords = totalRecords - midPoint;
    memcpy(newNode->records, &tempRecords[midPoint], newNode->numRecords * sizeof(Record));

    free(tempRecords);

    // Σήμανση των blocks ως τροποποιημένα
    BF_Block_SetDirty(block);
    BF_Block_SetDirty(newBlock);

    // Απελευθέρωση του νέου block
    CALL_BF(BF_UnpinBlock(newBlock));
    BF_Block_Destroy(&newBlock);

    return newBlockId;
}

int BP_CreateFile(char *fileName) {
    int file_desc;
    BF_Block *block;
    BF_Block_Init(&block);

    // Δημιουργία του αρχείου
    if (BF_CreateFile(fileName) != BF_OK) {
        BF_Block_Destroy(&block);
        return -1;
    }

    // Άνοιγμα του αρχείου
    if (BF_OpenFile(fileName, &file_desc) != BF_OK) {
        BF_Block_Destroy(&block);
        return -1;
    }

    // Δέσμευση του πρώτου block για μεταδεδομένα
    if (BF_AllocateBlock(file_desc, block) != BF_OK) {
        BF_CloseFile(file_desc);
        BF_Block_Destroy(&block);
        return -1;
    }

    // Αρχικοποίηση των μεταδεδομένων
    BPLUS_INFO* info = (BPLUS_INFO*)BF_Block_GetData(block);
    info->root = -1;          // Κενό δέντρο
    info->height = 0;         // Μηδενικό ύψος
    info->numNodes = 0;       // Κανένας κόμβος
    info->blockCounter = 1;   // Το πρώτο block είναι τα μεταδεδομένα

    // Ενημέρωση και απελευθέρωση του block
    BF_Block_SetDirty(block);
    if (BF_UnpinBlock(block) != BF_OK) {
        BF_Block_Destroy(&block);
        BF_CloseFile(file_desc);
        return -1;
    }
    BF_Block_Destroy(&block);

    // Κλείσιμο του αρχείου
    if (BF_CloseFile(file_desc) != BF_OK) {
        return -1;
    }

    return 0;
}

BPLUS_INFO* BP_OpenFile(char *fileName, int *file_desc) {
    BF_Block *block;
    BF_Block_Init(&block);

    // Άνοιγμα του αρχείου
    if (BF_OpenFile(fileName, file_desc) != BF_OK) {
        BF_Block_Destroy(&block);
        return NULL;
    }

    // Εύρεση κενής θέσης στον πίνακα ανοιχτών αρχείων
    int i;
    for (i = 0; i < 20; i++) {
        if (openFiles[i] == NULL) break;
    }

    if (i == 20) {
        printf("Σφάλμα: Υπερβολικός αριθμός ανοιχτών αρχείων\n");
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    // Ανάκτηση των μεταδεδομένων
    if (BF_GetBlock(*file_desc, 0, block) != BF_OK) {
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    // Αποθήκευση των μεταδεδομένων στη μνήμη
    openFiles[i] = malloc(sizeof(BPLUS_INFO));
    if (openFiles[i] == NULL) {
        BF_UnpinBlock(block);
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    memcpy(openFiles[i], BF_Block_GetData(block), sizeof(BPLUS_INFO));

    // Απελευθέρωση του block
    if (BF_UnpinBlock(block) != BF_OK) {
        free(openFiles[i]);
        openFiles[i] = NULL;
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    BF_Block_Destroy(&block);
    return openFiles[i];
}

int BP_CloseFile(int file_desc, BPLUS_INFO* info) {
    // Εύρεση και απελευθέρωση της εγγραφής στον πίνακα
    for (int i = 0; i < 20; i++) {
        if (openFiles[i] == info) {
            // Ενημέρωση των μεταδεδομένων στο αρχείο
            BF_Block *block;
            BF_Block_Init(&block);
            CALL_BF(BF_GetBlock(file_desc, 0, block));


            // Αντιγραφή των ενημερωμένων μεταδεδομένων
            memcpy(BF_Block_GetData(block), info, sizeof(BPLUS_INFO));
            BF_Block_SetDirty(block);
            CALL_BF(BF_UnpinBlock(block));
            BF_Block_Destroy(&block);

            // Απελευθέρωση μνήμης
            free(openFiles[i]);
            openFiles[i] = NULL;
            break;
        }
    }

    // Κλείσιμο του αρχείου
    CALL_BF(BF_CloseFile(file_desc));
    return 0;
}

int BP_InsertEntry(int file_desc, BPLUS_INFO *bplus_info, Record record) {
    BF_Block *block;
    BF_Block_Init(&block);

    // Εάν το δέντρο είναι άδειο, δημιουργούμε το πρώτο φύλλο
    if (bplus_info->root == -1) {
        if (BF_AllocateBlock(file_desc, block) != BF_OK) {
            BF_Block_Destroy(&block);
            return -1;
        }
        BPLUS_DATA_NODE* root = (BPLUS_DATA_NODE*)BF_Block_GetData(block);
        root->numRecords = 1;
        root->nextBlock = -1;
        root->records[0] = record;

        bplus_info->root = bplus_info->blockCounter;
        bplus_info->height = 1;
        bplus_info->numNodes = 1;
        bplus_info->blockCounter++;

        BF_Block_SetDirty(block);
        if (BF_UnpinBlock(block) != BF_OK) {
            BF_Block_Destroy(&block);
            return -1;
        }
        BF_Block_Destroy(&block);
        return bplus_info->root;
    }

    // Εύρεση του κατάλληλου φύλλου
    int leaf_block = findLeafBlock(file_desc, bplus_info, record.id);
    if (leaf_block < 0 || BF_GetBlock(file_desc, leaf_block, block) != BF_OK) {
        BF_Block_Destroy(&block);
        return -1;
    }
    BPLUS_DATA_NODE* leaf = (BPLUS_DATA_NODE*)BF_Block_GetData(block);

    // Έλεγχος για διπλότυπο ID
    for (int i = 0; i < leaf->numRecords; i++) {
        if (leaf->records[i].id == record.id) {
            printf("Σφάλμα: Το ID %d υπάρχει ήδη\n", record.id);
            CALL_BF(BF_UnpinBlock(block));
            BF_Block_Destroy(&block);
            return -1;
        }
    }

    // Εισαγωγή της εγγραφής στο φύλλο εάν υπάρχει χώρος
    if (leaf->numRecords < MAX_RECORDS) {
        // Εύρεση της σωστής θέσης εισαγωγής
        int pos = 0;
        while (pos < leaf->numRecords && leaf->records[pos].id < record.id) {
            pos++;
        }

        // Μετακίνηση των εγγραφών για να κάνουμε χώρο
        if (pos < leaf->numRecords) {
            memmove(&leaf->records[pos + 1],
                   &leaf->records[pos],
                   (leaf->numRecords - pos) * sizeof(Record));
        }

        leaf->records[pos] = record;
        leaf->numRecords++;

        BF_Block_SetDirty(block);
        CALL_BF(BF_UnpinBlock(block));
        BF_Block_Destroy(&block);
        return leaf_block;
    }

    // Διάσπαση του φύλλου όταν είναι γεμάτο
    int new_block = splitLeafNode(file_desc, bplus_info, block, &record);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    return new_block;
}

int BP_GetEntry(int file_desc, BPLUS_INFO *bplus_info, int value, Record** record) {
    *record = NULL;
    if (bplus_info->root == -1) return -1;

    BF_Block *block;
    BF_Block_Init(&block);

    // Εύρεση του κατάλληλου φύλλου
    int leaf_block = findLeafBlock(file_desc, bplus_info, value);
    if (leaf_block < 0) {
        BF_Block_Destroy(&block);
        return -1;
    }

    if (BF_GetBlock(file_desc, leaf_block, block) != BF_OK) {
        BF_Block_Destroy(&block);
        return -1;
    }

    BPLUS_DATA_NODE* leaf = (BPLUS_DATA_NODE*)BF_Block_GetData(block);

    // Αναζήτηση της εγγραφής στο φύλλο
    for (int i = 0; i < leaf->numRecords; i++) {
        if (leaf->records[i].id == value) {
            *record = malloc(sizeof(Record));
            if (*record == NULL) {
                CALL_BF(BF_UnpinBlock(block));
                BF_Block_Destroy(&block);
                return -1;
            }
            memcpy(*record, &leaf->records[i], sizeof(Record));
            CALL_BF(BF_UnpinBlock(block));
            BF_Block_Destroy(&block);
            return 0;
        }
    }

    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    return -1;
}

