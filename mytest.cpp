#include "hash.h"
#include <iostream>
#include <random>
#include <vector>
using namespace std;
enum RANDOM {UNIFORM, NORMAL};
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORM) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 and standard deviation of 20
            m_normdist = std::normal_distribution<>(50,20);
        }
        else{
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
    }

    int getRandNum(){
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else{
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//uniform distribution

};

// The hash function used by HashTable class
unsigned int hashCode(const string str);

class Tester{ // Tester class to implement test functions
public:
    Tester(){}
    ~Tester(){}
private:
};

int main(){
    Random diskBlockGen(DISKMIN,DISKMAX);
    int tempDiskBlocks[200] = {0};
    HashTable aTable(MINPRIME,hashCode);
    int temp = 0;
    int secondIndex = 0;
    for (int i=0;i<200;i++){
        temp = diskBlockGen.getRandNum();
        tempDiskBlocks[secondIndex] = temp;
        secondIndex++;

        cout << "Insertion # " << i << " => " << temp << endl;
        if (i%2 == 0)
            aTable.insert(File("test.txt", temp));
        else
            // these will be deleted
            aTable.insert(File("driver.cpp", temp));
    }

    cout << "\nMessage: dump after 200 insertions in a table with MINPRIME (101) buckets:" << endl;
    aTable.dump();
    cout << endl;

    for (int i = 0;i<160;i++) {
        cout << "Remove # " << i << " => " << tempDiskBlocks[i] << endl;
        aTable.remove(File("driver.cpp", tempDiskBlocks[i]));
        aTable.remove(File("test.txt", tempDiskBlocks[i]));
    }
    cout << "\nMessage: dump after removing 160 buckets," << endl;
    aTable.dump();

    cout << "\nTesting the find operation on colliding file" << endl;
    if (! (aTable.getFile("test.txt", tempDiskBlocks[160]) == EMPTY)){
        cout << "File found!" << endl;
    }

    cout << "\nTesting the find operation on NON-colliding file" << endl;
    temp = diskBlockGen.getRandNum();
    aTable.insert(File("myfile.txt", temp));
    if (! (aTable.getFile("myfile.txt", temp) == EMPTY)){
        cout << "File found!" << endl;
    }

    return 0;
}

unsigned int hashCode(const string str) {
    unsigned int val = 0 ;
    const unsigned int thirtyThree = 33 ;  // magic number from textbook
    for ( int i = 0 ; i < str.length(); i++)
        val = val * thirtyThree + str[i] ;
    return val ;
}