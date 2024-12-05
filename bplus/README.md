# Υλοποίηση Συστημάτων Βάσεων Δεδομένων
## Χειμερινό Εξάμηνο 2024-2025
### Εργασία 2: B+ Δέντρα

**Φοιτητής:** Στέλιος Ζαχαριουδάκης
**Αριθμός Μητρώου:** 1115202200243

---

## Σχεδιαστικές Επιλογές

### Δομή Αρχείου
- Το πρώτο block περιέχει τα μεταδεδομένα του B+ δέντρου (BPLUS_INFO)
- Οι εσωτερικοί κόμβοι (BPLUS_INDEX_NODE) περιέχουν κλειδιά και δείκτες σε παιδιά
- Οι κόμβοι φύλλα (BPLUS_DATA_NODE) αποθηκεύουν τις εγγραφές και συνδέονται σε λίστα

### Διαχείριση Μνήμης
- Χρήση του BF επιπέδου για διαχείριση blocks
- Προσεκτική χρήση των BF_UnpinBlock και BF_Block_SetDirty
- Δυναμική δέσμευση μνήμης για τις δομές δεδομένων

### Χειρισμός Εγγραφών
- Έλεγχος για διπλότυπα IDs κατά την εισαγωγή
- Αποτελεσματική αναζήτηση μέσω της δομής ευρετηρίου
- Διατήρηση της ισορροπίας του δέντρου κατά τις εισαγωγές

## Παραδοχές
1. Μέγιστος αριθμός εγγραφών ανά block υπολογίζεται δυναμικά
2. Τα blocks είναι μεγέθους 512 bytes
3. Χρήση LRU για την πολιτική αντικατάστασης blocks
4. Μέγιστο 20 ανοιχτά αρχεία ταυτόχρονα

## Παρατηρηθέντα Προβλήματα
1. Προειδοποίηση valgrind για μη αρχικοποιημένη μνήμη κατά το γράψιμο blocks
   - Προέρχεται από το BF επίπεδο
   - Δεν επηρεάζει τη λειτουργικότητα του προγράμματος
2. Προειδοποιήσεις μεταγλώττισης για σύγκριση προσημασμένων/μη προσημασμένων ακεραίων
   - Δεν επηρεάζουν τη λειτουργικότητα
   - Θα μπορούσαν να διορθωθούν με κατάλληλες μετατροπές τύπων

## Επαλήθευση Λειτουργικότητας
- Όλες οι συναρτήσεις επιστρέφουν τις σωστές τιμές
- Επιτυχής χειρισμός διπλότυπων εγγραφών
- Σωστή διαχείριση μνήμης (χωρίς διαρροές)
- Επιτυχής εκτέλεση σε Linux με gcc 5.4+