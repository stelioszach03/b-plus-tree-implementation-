#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "bf.h"
#include "bp_file.h"
#include "record.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
    printf("=== Δοκιμή Λειτουργιών B+ Δέντρου ===\n\n");

    CALL_OR_DIE(BF_Init(LRU));

    // 1. Δοκιμή BP_CreateFile
    printf("1. Δοκιμή BP_CreateFile...\n");
    int result = BP_CreateFile("test.db");
    if (result != 0) {
        printf("Σφάλμα: Αποτυχία δημιουργίας αρχείου\n");
        return -1;
    }
    printf("Επιτυχής δημιουργία αρχείου.\n\n");

    // 2. Δοκιμή BP_OpenFile
    printf("2. Δοκιμή BP_OpenFile...\n");
    int file_desc;
    BPLUS_INFO* info = BP_OpenFile("test.db", &file_desc);
    if (info == NULL) {
        printf("Σφάλμα: Αποτυχία ανοίγματος αρχείου\n");
        return -1;
    }
    printf("Επιτυχές άνοιγμα αρχείου.\n\n");

    // 3. Δοκιμή BP_InsertEntry
    printf("3. Δοκιμή BP_InsertEntry...\n");

    // Δοκιμή κανονικής εισαγωγής
    Record record1 = {1, "Όνομα1", "Επώνυμο1", "Πόλη1"};
    result = BP_InsertEntry(file_desc, info, record1);
    if (result == -1) {
        printf("Σφάλμα: Αποτυχία εισαγωγής εγγραφής\n");
        return -1;
    }
    printf("Επιτυχής εισαγωγή εγγραφής 1.\n");

    // Δοκιμή διπλότυπης εισαγωγής
    printf("Δοκιμή διπλότυπης εισαγωγής...\n");
    result = BP_InsertEntry(file_desc, info, record1);
    if (result != -1) {
        printf("Σφάλμα: Επιτράπηκε διπλότυπη εισαγωγή\n");
        return -1;
    }
    printf("Επιτυχής απόρριψη διπλότυπης εγγραφής.\n\n");

    // 4. Δοκιμή BP_GetEntry
    printf("4. Δοκιμή BP_GetEntry...\n");
    Record* found_record = NULL;

    // Αναζήτηση υπάρχουσας εγγραφής
    result = BP_GetEntry(file_desc, info, 1, &found_record);
    if (result != 0 || found_record == NULL) {
        printf("Σφάλμα: Αποτυχία εύρεσης υπάρχουσας εγγραφής\n");
        return -1;
    }
    printf("Επιτυχής εύρεση εγγραφής: %d, %s, %s, %s\n",
           found_record->id, found_record->name, found_record->surname, found_record->city);
    free(found_record);

    // Αναζήτηση μη υπάρχουσας εγγραφής
    result = BP_GetEntry(file_desc, info, 999, &found_record);
    if (result != -1 || found_record != NULL) {
        printf("Σφάλμα: Λανθασμένη επιστροφή για μη υπάρχουσα εγγραφή\n");
        return -1;
    }
    printf("Επιτυχής χειρισμός μη υπάρχουσας εγγραφής.\n\n");

    // 5. Δοκιμή BP_CloseFile
    printf("5. Δοκιμή BP_CloseFile...\n");
    result = BP_CloseFile(file_desc, info);
    if (result != 0) {
        printf("Σφάλμα: Αποτυχία κλεισίματος αρχείου\n");
        return -1;
    }
    printf("Επιτυχές κλείσιμο αρχείου.\n\n");

    printf("Όλες οι δοκιμές ολοκληρώθηκαν επιτυχώς!\n");
    return 0;
}
