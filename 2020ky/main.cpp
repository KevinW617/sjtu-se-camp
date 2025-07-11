#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include <stack>
#include <algorithm>
#include <cmath>
#include <cassert>

using namespace std;

int num;
vector<string> infiles(num);

struct KVPair {
    int key;
    string value;

    int ctime;

    KVPair() {
        key = 0;
        value = "";
    }
};

struct SSTable {
    int time;
    int kv_nums;
    int file_size;
    vector<KVPair> pairs;

    SSTable() {
        time = 0;
        kv_nums = 0;
        file_size = 0;
        pairs = vector<KVPair>();
    }
};

vector<SSTable> sstbls;

void loadSSTables() {
    for (int i = 0; i < num; i++) {
// cout << "loadSSTables " << infiles[i] << endl;
        ifstream fin(infiles[i], ios::in | ios::binary);
        if (!fin.is_open()) assert(false);

        SSTable sst;
        fin.seekg(0, ios::beg);
        fin.read(reinterpret_cast<char *>(&sst.file_size), 4);
        fin.seekg(4, ios::beg);
        fin.read(reinterpret_cast<char *>(&sst.time), 4);
        fin.seekg(8, ios::beg);
        fin.read(reinterpret_cast<char *>(&sst.kv_nums), 4);
// cout << "file size " << sst.file_size << " kv_nums " << sst.kv_nums << " time " << sst.time << endl;


        for (int i = 0; i < sst.kv_nums; i++) {
            KVPair kvp;
            kvp.ctime = sst.time;

            fin.seekg(12 + 8 * i, ios::beg);
            fin.read(reinterpret_cast<char *>(&kvp.key), 4);

            int off = 0;
            fin.seekg(12 + 8 * i + 4, ios::beg);
            fin.read(reinterpret_cast<char *>(&off), 4);
// cout << "off " << off << endl;

            int next_off = 0;
            if (i == sst.kv_nums - 1) next_off = sst.file_size;
            else {
                fin.seekg(12 + 8 * (i + 1) + 4, ios::beg);
                fin.read(reinterpret_cast<char *>(&next_off), 4);
            }
// cout << "next_off " << next_off << endl;

            fin.seekg(off, ios::beg);
            char value[1024];
// cout << "value size " << next_off - off << endl;
            fin.read(value, next_off - off);
            value[next_off - off] = '\0';
            kvp.value = (string)value;

// cout << "kvp " << kvp.key << " " << kvp.value << " " << kvp.ctime << endl; 
            sst.pairs.push_back(kvp);
// cout << "success push back " << endl;
        }

        cout << sst.kv_nums << " " << sst.pairs[0].key << " " << sst.pairs[sst.kv_nums - 1].key << endl;

        sstbls.push_back(sst);
    }
}

vector<KVPair> sortedKVPairs;

void sortSSTables() {
    vector<int> pivot(num, 0);

    while (1) {
        int min_pivot_idx = 0;
        int min_key = INT_MAX;
        int min_ctime = 0;
        string min_value;

        for (int i = 0; i < num; i++) {
            if (pivot[i] >= sstbls[i].kv_nums) continue;

            if (sstbls[i].pairs[pivot[i]].key < min_key) {
                min_key = sstbls[i].pairs[pivot[i]].key;
                min_value = sstbls[i].pairs[pivot[i]].value;
                min_ctime = sstbls[i].pairs[pivot[i]].ctime;

                min_pivot_idx = i;
            }
        }

        if (min_key != INT_MAX) {
            KVPair kvp;
            kvp.key = min_key;
            kvp.value = min_value;
            kvp.ctime = min_ctime;
            sortedKVPairs.push_back(kvp);

            pivot[min_pivot_idx]++;
        } else {
            break;
        }
    }

    cout << sortedKVPairs[0].key << " " << sortedKVPairs[sortedKVPairs.size() - 1].key << endl;
}

vector<KVPair> cleanKVPairs;

void cleanSSTables() {
    KVPair last_latest_kvp = sortedKVPairs[0];
    for (int i = 0; i < sortedKVPairs.size(); i++) {
        if (sortedKVPairs[i].key != last_latest_kvp.key) {
            if (last_latest_kvp.value.size() != 0) {
                cleanKVPairs.push_back(last_latest_kvp);
            }
            last_latest_kvp = sortedKVPairs[i];
        } else {
            if (sortedKVPairs[i].ctime > last_latest_kvp.ctime) {
                last_latest_kvp = sortedKVPairs[i];
            }
        }
    }

    if (last_latest_kvp.value.size() != 0) {
        cleanKVPairs.push_back(last_latest_kvp);
    }

    cout << cleanKVPairs.size() << " " << cleanKVPairs[0].key << " " << cleanKVPairs[cleanKVPairs.size() - 1].key << endl;
}

void saveSSTables() {
    int i = 0;
    int file_idx = 1;

    while (i < cleanKVPairs.size()) {
        int left_space = 256 * 1024;
        string tmp = "output-" + std::to_string(file_idx) + ".sst";
        file_idx++;
        ofstream fout(tmp, ios::out | ios::binary);
        fout.seekp(4, ios::beg);

        int time = 0x00ffffff;
        fout.write(reinterpret_cast<char *>(&time), 4);

        int j = i;
        while (left_space > (8 + cleanKVPairs[i].value.size()) && i < cleanKVPairs.size()) {
            fout.seekp(12 + 8 * (i - j), ios::beg);
            fout.write(reinterpret_cast<char *>(&cleanKVPairs[i].key), 4);
            left_space -= 8 + cleanKVPairs[i].value.size();
            i++;
        }

        int off = 12 + 8 * (i - j);
        for (int k = j; k < i; k++) {
            fout.seekp(12 + 8 * (k - j) + 4, ios::beg);
            fout.write(reinterpret_cast<char *>(&off), 4);

            fout.seekp(off, ios::beg);
            fout.write(cleanKVPairs[k].value.c_str(), cleanKVPairs[k].value.size());

            off += cleanKVPairs[k].value.size();
        }
        fout.seekp(0, ios::beg);
        fout.write(reinterpret_cast<char *>(&off), 4);
        fout.seekp(8, ios::beg);
        int kvps = i - j;
        fout.write(reinterpret_cast<char *>(&kvps), 4);
    }

    cout << file_idx - 1 << endl;
}


int main() {

    cin >> num;

    for (int i = 1; i <= num; i++) {
        string tmp = "small-case/sstable-" + std::to_string(i) + ".sst";
        infiles.push_back(tmp);
    }

    loadSSTables();

    sortSSTables();

    cleanSSTables();

    saveSSTables();

    return 0;
}