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

#define INVALID -114514

struct LinearHash {
    vector<pair<int, int>> tbl;
    // 1 for occupied
    vector<int> occupy;
    int size;
    int elems;

    LinearHash(int sz) : size(sz) {
        elems = 0;
        tbl = vector<pair<int, int>>(size, {0, 0});
        occupy = vector<int>(size, 0);
    }

    void Enlarge() {
        int old_size = size;
        vector<pair<int, int>> old_tbl = tbl;
        vector<int> old_occupy = occupy;
        size = size * 2;
        tbl = vector<pair<int, int>>(size, {0, 0});
        occupy = vector<int>(size, 0);

        for (int i = 0; i < size; i++) {
            if (old_occupy[i % size] != 0) {
                Set(old_tbl[i].first, old_tbl[i].second);
            }
        }
    }

    int Get(int key) {
        int hkey = key % size;

        for (int i = hkey; i < hkey + size; i++) {
            if (occupy[i % size] != 0 && tbl[i % size].first == key) {
                return tbl[i % size].second;
            }
        }

        return INVALID;
    }

    void Set(int key, int value) {
        int hkey = key % size;

        for (int i = hkey; i < hkey + size; i++) {
            if (occupy[i % size] == 0) {
                occupy[i % size] = 1;
                tbl[i % size] = {key, value};
                elems++;
                if (elems > (size / 2)) {
                    Enlarge();
                }
                break;
            } else if (occupy[i % size] != 0 && tbl[i % size].first == key) {
                tbl[i % size].second = value;
                break;
            }
        }
    }

    void Del(int key) {
        int hkey = key % size;
        bool fnd = false;
        int i;
        for (i = hkey; i < hkey + size; i++) {
            if (occupy[i % size] != 0 && tbl[i % size].first == key) {
                occupy[i % size] = 0;
                tbl[i % size] = {0, 0};
                fnd = true;
                break;
            }
        }

        if (fnd) {
            int lastj = INVALID;
            for (int j = i; j < i + size; j++) {
                if (occupy[j % size] != 0 && (tbl[j % size].first % size == key % size)) {
                    lastj = j;
                } else if (!(occupy[j % size] != 0 && (tbl[j % size].first % size == key % size)) && lastj != INVALID) {
                    occupy[i % size] = 1;
                    tbl[i % size] = {tbl[lastj % size]};

                    occupy[lastj % size] = 0;
                    tbl[lastj % size] = {0, 0};
                    break;
                } else {
                    break;
                }
            }
        }
    }
};

struct CuckooHash {
    vector<pair<int, int>> tbl1;
    vector<pair<int, int>> tbl2;
    // 1 for occupied
    vector<int> occupy1;
    vector<int> occupy2;
    int size;

    CuckooHash(int sz) : size(sz) {
        tbl1 = vector<pair<int, int>>(size, {0, 0});
        tbl2 = vector<pair<int, int>>(size, {0, 0});
        occupy1 = vector<int>(size, 0);
        occupy2 = vector<int>(size, 0);
    }

    int hk1(int key) {
        return key % size;
    }

    int hk2(int key) {
        return (key / size) % size;
    }

    int Get(int key) {
        if (occupy1[key % size] && tbl1[key % size].first == key) {
            return tbl1[key % size].second;
        }

        if (occupy2[(key / size) % size] && tbl2[(key / size) % size].first == key) {
            return tbl2[(key / size) % size].second;
        }

        return INVALID;
    }

    void Del(int key) {
        if (occupy1[key % size] && tbl1[key % size].first == key) {
            tbl1[key % size] = {0, 0};
            occupy1[key % size] = 0;
        }

        if (occupy2[(key / size) % size] && tbl2[(key / size) % size].first == key) {
            tbl2[hk2(key)] = {0, 0};
            occupy2[hk2(key)] = 0;
        }
    }
    

    void Enlarge() {
        vector<pair<int, int>> old_tbl1 = tbl1;
        vector<pair<int, int>> old_tbl2 = tbl2;
        // 1 for occupied
        vector<int> old_occupy1 = occupy1;
        vector<int> old_occupy2 = occupy2;
        int old_size = size;

        size = 2 * size;
        tbl1 = vector<pair<int, int>>(size, {0, 0});
        tbl2 = vector<pair<int, int>>(size, {0, 0});
        occupy1 = vector<int>(size, 0);
        occupy2 = vector<int>(size, 0);

        for (int i = 0; i < old_size; i++) {
            if (old_occupy1[i]) {
                // cout << "Enlarge set " << old_tbl1[i].first << " " << old_tbl1[i].second << endl;
                Set(old_tbl1[i].first, old_tbl1[i].second);
            }
            if (old_occupy2[i]) {
                // cout << "Enlarge set " << old_tbl2[i].first << " " << old_tbl2[i].second << endl;
                Set(old_tbl2[i].first, old_tbl2[i].second);
            }
        }
    }

    void Set(int key, int value) {

        while (1) {
            int tbidx = 1;
            int cur_key = key, cur_val = value;

            stack<pair<int, int>> stk;
            bool done = false;
            bool loop = false;

            int cnt = 1;
            while (cnt++) {
                if (cnt > 100000) assert(false);
                int hk;
                if (tbidx == 1) {
                    hk = hk1(cur_key);

                    if (occupy1[hk] == 0) {
                        // cout << "occupy1[" << hk << "] = 0" << endl;
                        break;
                    } else if (occupy1[hk] != 0 && tbl1[hk].first == key) {
                        tbl1[hk].second = value;
                        // cout << "found " << endl;
                        done = true;
                        break;
                    } else {
                        // cout << "hello " << endl;
                        stack<pair<int, int>> tmp_stk = stk;
                        while (!tmp_stk.empty()) {
                            if (tbl1[hk] == tmp_stk.top()) {
                                loop = true;
                                break;
                            } else {
                                tmp_stk.pop();
                            }
                        }
                        // cout << "loop " << loop << endl;
                        if (loop) break;
                        stk.push(tbl1[hk]);
                        // cout << "stack push " << tbl1[hk].first << " " << tbl1[hk].second << endl;
                    }
                    tbidx = 2;
                    cur_key = tbl1[hk].first;
                    cur_val = tbl1[hk].second;
                } else {
                    hk = hk2(cur_key);
                    if (occupy2[hk] == 0) {
                        // cout << "occupy2[" << hk << "] = 0" << endl;
                        break;
                    } else if (occupy2[hk] != 0 && tbl2[hk].first == key) {
                        tbl2[hk].second = value;
                        done = true;
                        // cout << "found " << endl;
                        break;
                    } else {
                        // cout << "hello " << endl;
                        stack<pair<int, int>> tmp_stk = stk;
                        while (!tmp_stk.empty()) {
                            if (tbl2[hk] == tmp_stk.top()) {
                                loop = true;
                                break;
                            } else {
                                tmp_stk.pop();
                            }
                        }
                        // cout << "loop " << loop << endl;
                        if (loop) break;
                        stk.push(tbl2[hk]);
                        // cout << "stack push " << tbl2[hk].first << " " << tbl2[hk].second << endl;
                    }
                    tbidx = 1;
                    cur_key = tbl2[hk].first;
                    cur_val = tbl2[hk].second;
                }
            }

            if (loop) {
                // cout << "loop" << endl;
                Enlarge();
                continue;
            }

            if (!done) {
                while (!stk.empty()) {
                    if (tbidx == 1) {
                        pair<int, int> kv = stk.top();
                        int hk = hk1(kv.first);
                        tbl1[hk] = kv;
                        occupy1[hk] = 1;
// cout << "move " << kv.first << " " << kv.second << " in table 1 " << hk << endl;
                        stk.pop();
                        tbidx = 2;
                    } else {
                        pair<int, int> kv = stk.top();
                        int hk = hk2(kv.first);
                        tbl2[hk] = kv;
                        occupy2[hk] = 1;
// cout << "move " << kv.first << " " << kv.second << " in table 2 " << hk << endl;
                        stk.pop();
                        tbidx = 1;
                    }
                }

                if (tbidx == 1) {
                    int hk = hk1(key);
                    tbl1[hk] = {key, value};
                    occupy1[hk] = 1;
// cout << "set " << key << " " << value << " in table1 " << hk << endl;
                } else {
                    int hk = hk2(key);
                    tbl2[hk] = {key, value};
                    occupy2[hk] = 1;
// cout << "set " << key << " " << value << " in table1 " << hk << endl;
                }
            }


            break;
        }
    }
};

int main(int argc, char* argv[]) {
    string mode;
    cin >> mode;
    string input_path;
    cin >> input_path;
    ifstream fin(input_path);

    if (!fin.is_open()) assert(false);

    string line;
    ofstream fout("ans");
    if (mode == "linear") {
        LinearHash ltbl(8);
        while (getline(fin, line)) {
            // cout << line << endl;
            stringstream ss(line);
            string op;
            ss >> op;
            if (op == "Set") {
                string key, value;
                ss >> key >> value;
                ltbl.Set(stoi(key), stoi(value));
            } else if (op == "Get") {
                string key;
                ss >> key;
                int value = ltbl.Get(stoi(key));
                if (value != INVALID) {
                    fout << value << endl;
                } else {
                    fout << "null" << endl;
                }
            } else if (op == "Del") {
                string key;
                ss >> key;
                ltbl.Del(stoi(key));
            } else {
                assert(false);
            }
        }
    } else if (mode == "cuckoo") {
        CuckooHash ltbl(8);
        while (getline(fin, line)) {
            // cout << line << endl;
            stringstream ss(line);
            string op;
            ss >> op;
            if (op == "Set") {
                string key, value;
                ss >> key >> value;
                ltbl.Set(stoi(key), stoi(value));
            } else if (op == "Get") {
                string key;
                ss >> key;
                int value = ltbl.Get(stoi(key));
                if (value != INVALID) {
                    fout << value << endl;
                } else {
                    fout << "null" << endl;
                }
            } else if (op == "Del") {
                string key;
                ss >> key;
                ltbl.Del(stoi(key));
            } else {
                assert(false);
            }
        }
    } else {
        cerr << mode << endl;
    }
    return 0;
}