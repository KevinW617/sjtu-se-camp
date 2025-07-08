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

string txt, idx, zip;

unordered_map<char, int> freqTable;

void do_statistics() {
    ifstream fin(txt);

    if (!fin.is_open()) {
        assert(false);
    }

    char ch;
    int cnt = 0;
    while (fin.get(ch)) {
        if (freqTable.find(ch) == freqTable.end()) {
            freqTable[ch] = 1;
            cnt++;
        } else {
            freqTable[ch]++;
        }
    }

    int max0 = 0;
    char maxch0;
    for (auto kv : freqTable) {
        if (kv.second > max0 || (kv.second == max0 && kv.first < maxch0)) {
            max0 = kv.second;
            maxch0 = kv.first;
        }
    }

    int max1 = 0;
    char maxch1;
    for (auto kv : freqTable) {
        if (kv.second == max0 && kv.first == maxch0) continue;
        if (kv.second > max1 || (kv.second == max1 && kv.first < maxch1)) {
            max1 = kv.second;
            maxch1 = kv.first;
        }
    }

    int max2 = 0;
    char maxch2;
    for (auto kv : freqTable) {
        if (kv.second > max1 || (kv.second == max1 && kv.first == maxch1)) continue;
        if (kv.second > max2 || (kv.second == max2 && kv.first < maxch2)) {
            max2 = kv.second;
            maxch2 = kv.first;
        }
    }

    cout << cnt << endl << maxch0 << " " << max0 << endl << maxch1 << " " << max1 << endl << maxch2 << " " << max2 << endl;
}

struct HFTREE {
    char asc;
    int w;
    int h;

    HFTREE* lchild;
    HFTREE* rchild;

    HFTREE(char ascii, int weight) : asc(ascii), w(weight), h(0), lchild(NULL), rchild(NULL) {}
};

// pair of weight, ascii
typedef pair<int, int> PII;
typedef pair<PII, HFTREE*> PPH;
priority_queue<PPH, vector<PPH>, greater<PPH>> pq;

HFTREE* huffmanTree;

void build_tree() {
    for (auto kv : freqTable) {
        HFTREE* hf = new HFTREE(kv.first, kv.second);
        pq.push({{kv.second, kv.first}, hf});
    }

    while (pq.size() > 1) {
        PPH p1 = pq.top();
        pq.pop();
        PPH p2 = pq.top();
        pq.pop();

        char asc = min(p1.first.second, p2.first.second);
        int w = p1.first.first + p2.first.first;

        HFTREE* hf = new HFTREE(w, asc);
        hf->lchild = p1.second;
        hf->rchild = p2.second;
        hf->h = max(p1.second->h, p2.second->h) + 1;
        pq.push({{w, asc}, hf});
    }

    assert(pq.size() == 1);
    huffmanTree = pq.top().second;

    cout << huffmanTree->h << endl;
}

map<char, string> codingTable;
vector<char> path;
void encode(HFTREE* node) {
    if (node->lchild == NULL) {
        string pth(path.begin(), path.end());
        codingTable[node->asc] = pth;
    } else {
        path.push_back('0');
        encode(node->lchild);
        path.pop_back();
        path.push_back('1');
        encode(node->rchild);
        path.pop_back();
    }
}

void compress() {
    ifstream fin(txt);
    char ch;

    string bitstream = "";
    while(fin.get(ch)) {
        if (codingTable.find(ch) == codingTable.end()) {
            assert(false);
        } else {
            bitstream += codingTable[ch];
        }
    }
    fin.close();

    long long validBits = bitstream.length();

    ofstream fout(zip);
    fout.write(reinterpret_cast<const char*>(&validBits), sizeof(validBits));

    if (bitstream.length() % 8 != 0) {
        int padding = 8 - (bitstream.length() % 8);
        while (padding--) {
            bitstream += '0';
        }
    }

    for (int i = 0; i < bitstream.length(); i += 8) {
        string byte = bitstream.substr(i, 8);
        unsigned char byteValue = 0;

        for (int j = 0; j < 8; j++) {
            if (byte[j] == '1') {
                byteValue |= (1 << (7 - j));
            }
        }

        fout.write(reinterpret_cast<const char*>(&byteValue), 1);
    }

    fout.close();

    cout << validBits << endl;
}

int main() {
    cin >> txt >> idx >> zip;
    // 2.1
    do_statistics();

    // 2.2
    build_tree();

    // 2.3
    encode(huffmanTree);
    ofstream fo(idx);
    for (auto kv : codingTable) {
        fo << kv.first << " " << kv.second << endl;
    }
    fo.close();
    cout << codingTable['e'] << endl;

    // 2.4
    compress();

    return 0;
}