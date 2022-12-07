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

class cache {
    vector<cacheEntry> entries;

    //print caches function
};
struct cacheEntry {
    bool valid;
    int tag;
    int index;
    vector<int> data; // vector for num of blocks / line size
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
    while(file >> std::hex >> t) {
        sequence.push_back(t);
    }
    file.close();
}

int main() {
    cout << "welcome to the memory heirarchy simulator" << endl;
    cout << "Please enter the filename of the file containing the initial memory content:" << endl;
    string n;
    cin >> n;
    preloadData(n); // load intitial data
    

    cout << "Please enter the cache size: (number of form 2^n)" << endl;
    int size;
    cin >> size;
    cout << "Please enter the cache line size: (number of form 2^n)" << endl;
    int lineSize;
    cin >> lineSize;

    return 0;
}