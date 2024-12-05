# Database Systems Implementation  
## Winter Semester 2024-2025  
### Assignment 2: B+ Trees  

**Student:** Stylianos Zacharioudakis  
**Student ID:** 1115202200243  

---

## Design Choices  

### File Structure  
- The first block contains the metadata of the B+ tree (BPLUS_INFO).  
- Internal nodes (BPLUS_INDEX_NODE) store keys and pointers to child nodes.  
- Leaf nodes (BPLUS_DATA_NODE) store records and are linked in a list.  

### Memory Management  
- Utilization of the BF layer for block management.  
- Careful use of BF_UnpinBlock and BF_Block_SetDirty.  
- Dynamic memory allocation for data structures.  

### Record Handling  
- Duplicate ID checks during insertion.  
- Efficient search using the index structure.  
- Maintaining tree balance during insertions.  

---

## Assumptions  
1. The maximum number of records per block is calculated dynamically.  
2. Block size is 512 bytes.  
3. LRU is used for block replacement policy.  
4. A maximum of 20 files can be open simultaneously.  

---

## Observed Issues  

1. **Valgrind Warning for Uninitialized Memory during Block Writes:**  
   - Originates from the BF layer.  
   - Does not affect program functionality.  

2. **Compilation Warnings for Signed/Unsigned Integer Comparisons:**  
   - Does not affect functionality.  
   - Could be resolved with appropriate type casting.  

---

## Functionality Verification  
- All functions return correct values.  
- Successful handling of duplicate records.  
- Proper memory management (no leaks).  
- Successful execution on Linux with gcc 5.4+.  
