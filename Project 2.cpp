#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
using namespace std;

map<int, int> memory;
vector<int> sequence;
vector<int> writes; // stores the index of write instructions
vector<int> writeData;

bool writeThrough = 0;
bool WA = 0; //write allocate 
class cacheEntry {
    public:
    bool valid;
    bool dirty;
    int tag;
    int index;
    vector<int> data; // vector for num of blocks / line size

    cacheEntry(bool v, int t, int i, int datasize) { // create the bytes
        valid = v; tag = t; index = i;
        dirty = 0;
        for (int i = 0; i < datasize; i++) {
            data.push_back(0);
        }
    }
};
class cache {
    public:
    vector<cacheEntry> entries;
    cache* upperlvlCache = NULL; // closest to mem
    cache* lowerlvlCache = NULL; // closest to cpu
    int entryCycles;
    int Size;
    int LineSize;

    int cacheLvl = 1;
    
    //stats
    int accesses = 0;
    int hits = 0;
    
    bool missed = 0;

    cache() {
        entryCycles = 0;
        Size=0;
        LineSize=0;
    }
    cache(int size, int lineSize, int lvl, int cyclesDelay) {
        Size = size;
        LineSize = lineSize;
        for (int i = 0; i < size; i++) {
            cacheEntry entry(0, 0, i, lineSize);
            entries.push_back(entry);
        }
        cacheLvl = lvl;
        entryCycles = cyclesDelay;
    }

    int getEntry(int addr, int& cyclestaken) {
        int temp = addr;
        int offsetBits = int(log2(LineSize));
        int offset = addr % LineSize;
        temp >>= offsetBits; // temp = tag with index
        int x = temp % Size; // index only
        
        if (entries[x].valid) {
            int t = temp;
            t >>= int(log2(Size));
            if (entries[x].tag == t) {
                if (!missed) {
                   cout << "HIT L" << cacheLvl << endl;
                   hits++;
                   accesses++;
                   cyclestaken += entryCycles;
                }
                return entries[x].data[offset];
            }
            else {
                getFromUpper(temp, cyclestaken); // complete here
                //cyclestaken += entryCycles;
                hits--;
                missed = 0;
                return getEntry(addr, cyclestaken);
            }
        }
        else {
            getFromUpper(temp, cyclestaken);
            //cyclestaken += entryCycles;
            hits--;
            missed = 0;
            return getEntry(addr, cyclestaken);
        }
    }

    void getFromUpper(int addr, int& cyclestaken) {
        missed = 1;
        cout << "MISS    ";
        if (upperlvlCache) { // get from upper cache
            int index = addr % Size;
            int tag = addr;
            tag >>= int(log2(Size));
            addr <<= int(log2(LineSize));
            cout << "requesting line ";
            cout << hex << addr;
            cout << dec << " from cache level " << cacheLvl + 1 << endl; 
            for (int i = 0; i < LineSize; i++) {
                if (!writeThrough && entries[index].dirty && upperlvlCache) { // if write back
                    int oldaddr = entries[index].tag;
                    oldaddr <<= int(log2(Size));
                    oldaddr += index;
                    oldaddr <<= int(log2(LineSize));
                    upperlvlCache->write(oldaddr + i, entries[index].data[i], cyclestaken);
                }
                entries[index].data[i] = upperlvlCache->getEntry(addr + i, cyclestaken);
                upperlvlCache->missed = 1;
            }
            upperlvlCache->missed = 0;
            entries[index].valid = 1;
            entries[index].dirty = false;
            entries[index].tag = tag;
        }
        else { // get from memory
            int index = addr % Size;
            int tag = addr;
            tag >>= int(log2(Size));
            addr <<= int(log2(LineSize));
            cout << "requesting line ";
            cout << hex << addr;
            cout << dec << " from Memory" << endl; 
            for (int i = 0; i < LineSize; i++) {
                if (!writeThrough && entries[index].dirty) { // if write back
                    int oldaddr = entries[index].tag;
                    oldaddr <<= int(log2(Size));
                    oldaddr += index;
                    oldaddr <<= int(log2(LineSize));
                    memory[oldaddr + i] = entries[index].data[i];
                }
                if (memory.find(addr + i) != memory.end()) {
                    entries[index].data[i] = memory[addr + i];
                }
                else {
                    entries[index].data[i] = 0;
                }
            }
            entries[index].valid = 1;
            entries[index].tag = tag;
            entries[index].dirty = false;
            cyclestaken += 100;
        }
    }
    
    void printData() {
        cout << "L" << cacheLvl << " Cache" << endl;
        cout << "Valid" << '\t' << "Tag" << '\t' << "Index" << '\t' << "Data (bytes)" << endl;
        for (int i= 0; i < entries.size(); i++) {
            cout << entries[i].valid << '\t' <<  hex << entries[i].tag << '\t' << dec <<  entries[i].index << '\t';   // std::bitset<16>(entries[i].tag)   for binary
            for(int j=0; j < entries[i].data.size(); j++) {
                cout << dec << entries[i].data[j] << " ";
            }
            cout << endl;
        }
        cout << endl;
        cout << "Accesses: " << accesses << endl;
        cout << "Hit ratio: " << float(hits) / accesses << endl;
        cout << "Miss ratio: " << 1-(float(hits) / accesses) << endl;
        cout << endl;
    }

    void write(int addr, int val, int& cyclestaken) {  
        int temp = addr;
        int offsetBits = int(log2(LineSize));
        int offset = addr % LineSize;
        temp >>= offsetBits; // temp = tag with index
        int x = temp % Size; // index only
        if (WA) {
            if (entries[x].valid) {
                int t = temp;
                t >>= int(log2(Size));
                if (entries[x].tag == t) {
                    entries[x].data[offset] = val;
                    entries[x].dirty = 1;
                    cyclestaken += entryCycles;
                    if (writeThrough) {
                        if (memory[addr] != val) {
                            memory[addr] = val;
                            cyclestaken += 100;
                        }
                        if (upperlvlCache) {
                            upperlvlCache->write(addr, val, cyclestaken);
                        }
                    }
                    cout << "Write hit" << endl;
                    return;
                }
                else { // write miss
                    cout << "Write miss, fetching.." << endl;
                    getEntry(addr, cyclestaken);
                    write(addr, val, cyclestaken);
                    return;
                }
            }
            else { // write miss
                cout << "Write miss, fetching.." << endl;
                getEntry(addr, cyclestaken);
                write(addr, val, cyclestaken);
                return;
            }
        }
        else {
            if (entries[x].valid) {
                int t = temp;
                t >>= int(log2(Size));
                if (entries[x].tag == t) {
                    entries[x].data[offset] = val;
                    entries[x].dirty = 1;
                    cyclestaken += entryCycles;
                    if (writeThrough) {
                        if (memory[addr] != val) {
                            memory[addr] = val;
                            cyclestaken += 100;
                        }
                        if (upperlvlCache) {
                            upperlvlCache->write(addr, val, cyclestaken);
                        }
                    }
                    cout << "Write hit" << endl;
                    return;
                }
                else { // write miss
                    cout << "Write miss, writing to memory" << endl;
                    memory[addr] = val;
                    cyclestaken += 100;
                    return;
                }
            }
            else { // write miss
                cout << "Write miss, writing to memory" << endl;
                memory[addr] = val;
                cyclestaken += 100;
                return;
            }
        }
    }
};

void preloadData(string name) {
    ifstream file(name);
    string line;
    while(getline(file, line)) {
        istringstream ss(line);
        int address;
        int value;
        string dummy;
        ss >> std::hex >> address;
        ss >> dummy;
        ss >> std::dec >> value;
        //cout << address << " " << value << endl;
        memory[address] = value;
    }
    file.close();
}

void readSeq(string name) {
    ifstream file(name);
    int t;
    int i = 0;
    while(file >> std::hex >> t) {
        sequence.push_back(t);
        char let;
        file >> let;
        if (let == 'w' || let == 'W') {
            writes.push_back(i);
            int val;
            file >> val;
            writeData.push_back(val);
        }
        i++;
    }
    file.close();
}

int main() {
    cout << "Welcome to the memory hierarchy simulator!" << endl << endl;
    cout << "Please enter the filename of the file containing the initial memory content:" << endl;
    string n = "preload.txt";
    cin >> n;
    preloadData(n); // load intitial data
    
    cout << "Enter the number of cache levels" << endl;
    int lvls = 2;
    cin >> lvls;
    cache* caches = new cache[lvls];

    cout << "(L1 cache is closest to CPU)" << endl << endl;
    for (int i = 1 ; i <= lvls; i++) {
        cout << "Enter the cache size for cache L" << i << ": (number of form 2^n)" << endl;
        int size = 4;
        cin >> size;
        cout << "Enter the cache line size for cache L" << i << ": (number of form 2^n)" << endl;
        int lineSize = 4;
        cin >> lineSize;
        cout << "Enter the number of cycles needed to access the cache (1 to 10) :" << endl;
        int cycles = 10;
        cin >> cycles;
        cache c(size, lineSize, i, cycles);
        caches[i-1] = c;
        if (i >= 2) {
            caches[i - 1].lowerlvlCache = &caches[i-2];
            caches[i - 2].upperlvlCache = &caches[i-1];
        }
    }
    
    cout << "Enter the name of the file contatining the access sequence. (eg. input.txt)" << endl;
    string seqname = "sequence.txt";
    cin >> seqname;
    readSeq(seqname);

    float cyclessum;

    cout << "Choose the writing policy: (1 : write through, 0 : write back)" << endl;
    cin >> writeThrough;
    cout << "Choose the write miss policy: (1 : write allocate, 0 : no write allocate)" << endl;
    cin >> WA;
    
    int w = 0;
    for (int i = 0; i < sequence.size(); i++) {
        cout << "--------------------------------------------------" << endl;
        int cyclesTaken = 0;
        if (i != writes[w]) {
            cout << "Getting address: " << hex << sequence[i] << endl;
            caches[0].getEntry(sequence[i], cyclesTaken);
        }
        else {
            cout << "Writing " << writeData[w] << " to address: " << hex << sequence[i] << endl;
            caches[0].write(sequence[i], writeData[w], cyclesTaken);
            w++;
        }
        cout << "Access time cycles: " << dec << cyclesTaken << endl;
        cyclessum += cyclesTaken;
        cout << "AMAT: " << cyclessum / (i+1) << endl << endl;
        
        for (int x = 0; x < lvls; x++) {
            caches[x].printData();
        }
    }

    cout << "-------------------END OF PROGRAM-------------------";

    return 0;
}