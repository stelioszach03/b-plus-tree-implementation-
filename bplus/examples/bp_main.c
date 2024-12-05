#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "bp_file.h"
#include "bp_datanode.h"
#include "bp_indexnode.h"
#include "record.h"

#define RECORDS_NUM 200  // Αριθμός εγγραφών για δοκιμή
#define FILE_NAME "data.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
    printf("=== Δοκιμή Λειτουργιών B+ Δέντρου ===\n\n");

    // Αρχικοποίηση του BF επιπέδου
    BF_Init(LRU);

    // 1. Δοκιμή BP_CreateFile
    printf("1. Δημιουργία αρχείου B+ δέντρου...\n");
    remove(FILE_NAME);  // Διαγραφή προηγούμενου αρχείου
    if (BP_CreateFile(FILE_NAME) < 0) {
        printf("Σφάλμα: Αποτυχία δημιουργίας αρχείου!\n");
        return -1;
    }
    printf("Επιτυχής δημιουργία αρχείου.\n\n");

    // 2. Δοκιμή BP_OpenFile
    printf("2. Άνοιγμα αρχείου...\n");
    int file_desc;
    BPLUS_INFO* info = BP_OpenFile(FILE_NAME, &file_desc);
    if (info == NULL) {
        printf("Σφάλμα: Αποτυχία ανοίγματος αρχείου!\n");
        return -1;
    }
    printf("Επιτυχές άνοιγμα αρχείου.\n\n");

    // 3. Δοκιμή BP_InsertEntry
    printf("3. Εισαγωγή εγγραφών...\n");

    // 3.1 Δοκιμή εισαγωγής κανονικών εγγραφών
    int successful_inserts = 0;
    Record record;
    for (int i = 0; i < RECORDS_NUM; i++) {
        record = randomRecord();
        if (BP_InsertEntry(file_desc, info, record) >= 0) {
            successful_inserts++;
        }
    }
    printf("Επιτυχής εισαγωγή %d/%d εγγραφών.\n", successful_inserts, RECORDS_NUM);
    printf("Ύψος δέντρου: %d\n", info->height);
    printf("Αριθμός κόμβων: %d\n\n", info->numNodes);

    // 3.2 Δοκιμή εισαγωγής διπλότυπης εγγραφής
    Record duplicate;
    duplicate.id = 1;  // Υπάρχον ID
    strncpy(duplicate.name, "Test", sizeof(duplicate.name) - 1);
    strncpy(duplicate.surname, "Test", sizeof(duplicate.surname) - 1);
    strncpy(duplicate.city, "Test", sizeof(duplicate.city) - 1);
    printf("Δοκιμή εισαγωγής διπλότυπης εγγραφής (ID=1)...\n");
    if (BP_InsertEntry(file_desc, info, duplicate) >= 0) {
        printf("Σφάλμα: Η διπλότυπη εγγραφή έγινε δεκτή!\n");
    } else {
        printf("Επιτυχής απόρριψη διπλότυπης εγγραφής.\n\n");
    }

    // 4. Δοκιμή BP_GetEntry
    printf("4. Αναζήτηση εγγραφών...\n");

    // 4.1 Αναζήτηση υπαρχόντων εγγραφών
    int successful_searches = 0;
    for (int i = 1; i <= RECORDS_NUM; i++) {
        Record* result = NULL;
        if (BP_GetEntry(file_desc, info, i, &result) == 0) {
            successful_searches++;
            if (i % 50 == 0) {  // Εκτύπωση κάθε 50ής εγγραφής
                printf("Βρέθηκε εγγραφή με ID=%d\n", result->id);
            }
            free(result);
        }
    }
    printf("Επιτυχής εύρεση %d/%d εγγραφών.\n", successful_searches, RECORDS_NUM);

    // 4.2 Αναζήτηση μη υπάρχουσας εγγραφής
    Record* not_found = NULL;
    printf("\nΔοκιμή αναζήτησης μη υπάρχουσας εγγραφής (ID=%d)...\n", RECORDS_NUM + 1);
    if (BP_GetEntry(file_desc, info, RECORDS_NUM + 1, &not_found) < 0) {
        printf("Επιτυχής χειρισμός μη υπάρχουσας εγγραφής.\n\n");
    } else {
        printf("Σφάλμα: Βρέθηκε ανύπαρκτη εγγραφή!\n\n");
        free(not_found);
    }

    // 5. Δοκιμή BP_CloseFile
    printf("5. Κλείσιμο αρχείου...\n");
    if (BP_CloseFile(file_desc, info) < 0) {
        printf("Σφάλμα: Αποτυχία κλεισίματος αρχείου!\n");
        return -1;
    }
    printf("Επιτυχές κλείσιμο αρχείου.\n\n");

    BF_Close();
    printf("=== Ολοκλήρωση Δοκιμών ===\n");
    return 0;
}
