// CMSC 341 - Fall 2021 - Project 4
#include "hash.h"
#include "math.h"


//The alternative constructor, size is an unsigned integer to specify the length of hash table, and hash is a function pointer to a hash function.
//The type of hash is defined in hash.h.
//The table size must be a prime number between MINPRIME and MAXPRIME. If the user passes a size less than MINPRIME,
// the capacity must be set to MINPRIME. If the user passes a size larger than MAXPRIME, the capacity must be set to MAXPRIME.
// If the user passes a non-prime number the capacity must be set to the smallest prime number greater than user's value.
//Moreover, the constructor creates memory for table 1 and initializes all member variables. And, it initializes m_newTable to TABLE1
HashTable::HashTable(unsigned size, hash_fn hash){
    if(size < MINPRIME && isPrime(signed(size))){
        // restrict to range
        m_capacity1 = MINPRIME;
    }
    else if (size > MAXPRIME && isPrime(signed(size))){
        // restrict to range
        m_capacity1 = MAXPRIME;
    }
    else if (!isPrime(signed(size))){
        //find smallest prime greater than user value
        m_capacity1 = findNextPrime(signed(size));
    }
    else{
        m_capacity1 = size;
    }

    // create memory for table 1
    m_table1 = new File[m_capacity1];

    // initialize all member variables
    m_size1 = 0;
    m_numDeleted1 = 0;
    m_hash = hash;
    m_newTable = TABLE1;

    m_table2 = nullptr;
    m_capacity2 = 0;
    m_size2 = 0;
    m_numDeleted2 = 0;
}

//Destructor, deallocates the memory for both table 1 and table 2
HashTable::~HashTable(){
    delete[] m_table1;
    delete[] m_table2;
}

//This function looks for the File object with name and diskBlock in the hash table, if the object is found the function returns it,
// otherwise the function returns empty object.
// If there are two hash tables at the time, the function needs to look into both tables.
File HashTable::getFile(string name, unsigned int diskBlock){
    for (unsigned int i = 0; i < m_capacity1; ++i) {
        if(m_table1[i].diskBlock() == diskBlock && m_table1[i].key() == name){
            return m_table1[i];
        }
    }
    for (unsigned int i = 0; i < m_capacity2; ++i) {
        if(m_table2[i].diskBlock() == diskBlock && m_table2[i].key() == name){
            return m_table2[i];
        }
    }
    return File();
}


//This function inserts an object into the hash table. The insertion index is determined by applying the hash function m_hash that is set in the
// HashTable constructor call and then reducing the output of the hash function modulo the table size.
// A hash function is provided in the sample driver.cpp file to be used in this project.
//Hash collisions should be resolved using the quadratic probing policy. We insert into the table indicated by m_newTable.

// After every insertion we need to check for the proper criteria, and if it is required, we need to rehash the entire table incrementally into
// a new table. The incremental transfer proceeds with 25% of the nodes at a time. Once we transferred 25% of the nodes for the first time,
// the second 25% will be transferred at the next operation (insertion or removal). Once all data is transferred to the new table,
// the old table will be removed, and its memory will be deallocated.

//If the "file" object is inserted, the function returns true, otherwise it returns false. A File object can only be inserted once.
// The hash table does not contain duplicate objects. Moreover, the disk block value should be a valid one falling in the range [DISKMIN-DISKMAX].
// Every File object is a unique object carrying the file's name and the disk block number. The file's name is the key which is used for hashing.
bool HashTable::insert(File file){
    // determine insertion index
    // still missing: safe guard for index within range of table
    int index;
    int i = 0;
    do {
        index = signed(((m_hash(file.key()) % tableSize(m_newTable)) + (i * i)) % tableSize(m_newTable));
        // if file already exists in table, fail to insert
        if (getFileByIndex(index) == file){
            return false;
        }
        i ++;
    } while (! (getFileByIndex(index) == EMPTY) && i < signed(tableSize(m_newTable)));

    // insert
    if (!setFile(index, file)){
        return false;
    }
    // increment m_size
    if (m_newTable == TABLE1){
        m_size1 ++;
    }
    else {
        m_size2++;
    }

    // check for proper criteria
    // After an insertion, if the load factor becomes greater than 0.5, we need to rehash to a new hash table.
    float loadFactor = lambda(m_newTable);
    if (loadFactor > 0.5){
        rehash();
    }
    return true;
}

//This function removes a data point from the hash table. In a hash table we do not empty the bucket, we only tag it as deleted. To tag a removed
// bucket we assign the DELETED object to the bucket. The DELETED object is defined in hash.h. To find the bucket of the object we should use the quadratic probing policy.

//After every deletion we need to check for the proper criteria, and if it is required, we need to rehash the entire table incrementally into a new table.
// The incremental transfer proceeds with 25% of the nodes at a time. Once we transferred 25% of the nodes for the first time, the second 25% will be
// transferred at the next operation(insertion or removal).Once all data is transferred to the new table,the old table will be removed,and its memory will be deallocated.

//If the "file" object is found and is deleted, the function returns true, otherwise it returns false.
bool HashTable::remove(File file){

    // if empty file, fail to delete
    if (file == EMPTY){
        return false;
    }

    // find index of file in table using quadratic probing policy
    int index = -1;
    int curr;
    int i = 0;
    do {
        curr = signed(((m_hash(file.key()) % tableSize(m_newTable)) + (i * i)) % tableSize(m_newTable));
        if (getFileByIndex(curr) == file){
            index = curr;
        }
        i++;
    } while (! (getFileByIndex(curr) == file) && i < signed(tableSize(m_newTable)));

    // if file is not found, return false
    if (index == -1){
        return false;
    }

    // else file is found, assign bucket to DELETED object
    if (!setFile(index, DELETED)){
        return false;
    }
    // increment m_numDeleted1
    if (m_newTable == TABLE1){
        m_numDeleted1 ++;
    }
    else{
        m_numDeleted2 ++;
    }

    // check for proper criteria
    // After a deletion, if the number of deleted buckets is more than 80 percent of the total number of occupied buckets, we need to rehash to a new table
    if (deletedRatio(m_newTable) > 0.8){
        rehash();
    }
    return true;
}

//This function returns the load factor of the hash table. The load factor is the ratio of occupied buckets to the table capacity. The number of occupied
// buckets is the total of available buckets and deleted buckets. The parameter tablename specifies the table that should be used for the calculation.
float HashTable::lambda(TABLENAME tablename) const {
    if (m_newTable == TABLE1){
        return (float)m_size1/(float)m_capacity1;
    }
    else{
        return (float)m_size2/(float)m_capacity2;
    }
}

//This function returns the ratio of the deleted buckets to the total number of occupied buckets.
// The parameter tablename specifies the table that should be used for the calculation.
float HashTable::deletedRatio(TABLENAME tableName) const {
    if (m_newTable == TABLE1){
        return (float)m_numDeleted1/(float)m_size1;
    }
    else{
        return (float)m_numDeleted2/(float)m_size2;
    }
}

//dump
void HashTable::dump() const {
    cout << "Dump for table 1: " << endl;
    if (m_table1 != nullptr)
        for (unsigned int i = 0; i < m_capacity1; i++) {
            cout << "[" << i << "] : " << m_table1[i] << endl;
        }
    cout << "Dump for table 2: " << endl;
    if (m_table2 != nullptr)
        for (unsigned int i = 0; i < m_capacity2; i++) {
            cout << "[" << i << "] : " << m_table2[i] << endl;
        }
}

//checks if prime
bool HashTable::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

//finds next prime
int HashTable::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) {
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0)
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}

bool HashTable::setFile(unsigned int index, File file){
    if (m_newTable == TABLE1){
        if (index > m_capacity1 - 1 || index < 0){
            return false;
        }
        m_table1[index] = file;
    }
    else{
        if (index > m_capacity2 - 1 || index < 0){
            return false;
        }
        m_table2[index] = file;
    }
    return true;
}

unsigned int HashTable::getNumDeleted(){
    if (m_newTable == TABLE1){
        return m_numDeleted1;
    }
    else{
        return m_numDeleted2;
    }
}
int HashTable::getNewCapacity() {
    // The capacity of the new table would be the smallest prime number greater than 4 times the current number of data points.
    // The current number of data points is total number of occupied buckets minus total number of deleted buckets.
    return findNextPrime( signed(4 * (numEntries(m_newTable) - getNumDeleted())));
}

File HashTable::getFileByIndex(int index){
    if (m_newTable == TABLE1){
        return m_table1[index];
    }
    else{
        return m_table2[index];
    }
}

bool HashTable::rehash(){
    // if new hashtable isn't created yet, allocate new memory for it
    if (m_newTable == TABLE1){
        if (m_table2 == nullptr){
            cout << "Rehashing... " << endl;
            dump();
            int newSize = getNewCapacity();
            m_table2 = new File[newSize];
            m_capacity2 = newSize;
        }
    }
    else{
        if (m_table1 == nullptr){
            cout << "Rehashing... " << endl;
            dump();
            int newSize = getNewCapacity();
            m_table1 = new File[newSize];
            m_capacity1  = newSize;
        }
    }

    // move 25% of the data over
    // do not move over DELETED files
    int numMove = floor(numEntries(m_newTable)/4);
    int index = 0;
    while (numMove > 0 && index < signed(tableSize(m_newTable))){
        if (! (getFileByIndex(index) == EMPTY) && ! (getFileByIndex(index) == DELETED)){
            if (m_newTable == TABLE1){
                unsigned int index2;
                int i = 0;
                do {
                    index2 = ((m_hash(m_table1[index].key()) % m_capacity2) + (i * i)) % m_capacity2;
                    i++;
                } while (! (m_table2[index2] == EMPTY) && i < m_capacity2);

                // insert
                m_table2[index2] = m_table1[index];
                m_size2++;
                m_numDeleted1++;
            }
            else{
                unsigned int index2;
                int i = 0;
                do {
                    index2 = ((m_hash(m_table2[index].key()) % m_capacity1) + (i * i)) % m_capacity1;
                    i ++;
                } while (! (m_table1[index2] == EMPTY) && i < m_capacity1);

                // insert
                m_table1[index2] = m_table2[index];
                m_size1++;
                m_numDeleted2++;
            }
            setFile(index, DELETED);
            numMove--;
        }
        index++;
    }

    // if 100% of data is moved over, delete old table
    unsigned int used;
    unsigned int deleted;
    if (m_newTable == TABLE1){
        used = m_size1;
        deleted = m_numDeleted1;
    }
    else{
        used = m_size2;
        deleted = m_numDeleted2;
    }
    if (used == deleted){
        if (m_newTable == TABLE1) {
            delete[] m_table1;
            m_table1 = nullptr;
            m_capacity1 = 0;
            m_size1 = 0;
            m_numDeleted1 = 0;
            m_newTable = TABLE2;
            cout << "Rehashing complete..." << endl;
            dump();
        } else {
            delete[] m_table2;
            m_table2 = nullptr;
            m_capacity2 = 0;
            m_size2 = 0;
            m_numDeleted2 = 0;
            m_newTable = TABLE1;
            cout << "Rehashing complete..." << endl;
            dump();
        }
    }

    // questions
    // which 25% to move over?
    // say after inserting 4, you have 1 2 3 4 and you need to rehash and move 1 to the new table
    // then you insert 5, you have 1 2 3 4 5 what do you move to the new table?

    // say after inserting 4, you have 1 2 3 4 and you need to rehash and move 1 to the new table and delete it from the old "DELETED" 2 3 4
    // then you insert another 1, you have 1 2 3 4 do you still move the new 1 over to the new table?

    // say after inserting 5, you have 2 3 4 5 and you need to rehash and move 2 to the new table 2
    // then you delete 2, you have 3 4 5 and you move 3 to the new table 2 3
    // then you insert 1, you have 1 3 4 5 what do you move to the new table
    return true;
}