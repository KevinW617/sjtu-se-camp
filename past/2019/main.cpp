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
        elems = 0;

        for (int i = 0; i < old_size; i++) {
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
        int pos = -1;
        
        // 1. 找到要删除的元素
        for (int i = hkey; i < hkey + size; i++) {
            if (occupy[i % size] != 0 && tbl[i % size].first == key) {
                pos = i % size;
                occupy[pos] = 0;
                tbl[pos] = {0, 0};
                elems--;
                break;
            }
        }
        
        if (pos == -1) return; // 没找到要删除的元素
        
        // 2. 重新排列后续元素
        int j = (pos + 1) % size;
        while (occupy[j] != 0) {
            int element_key = tbl[j].first;
            int original_hash = element_key % size;
            
            // 判断这个元素是否需要前移
            bool should_move = false;
            
            if (original_hash <= pos) {
                // 元素的原始位置在删除位置之前或等于删除位置
                if (pos < j) {
                    // 正常情况：original_hash <= pos < j
                    should_move = true;
                } else if (j < original_hash) {
                    // 环形情况：j < original_hash <= pos
                    should_move = true;
                }
            } else {
                // 元素的原始位置在删除位置之后
                if (j < original_hash && pos < j) {
                    // 环形情况：pos < j < original_hash
                    should_move = true;
                }
            }
            
            if (should_move) {
                // 移动元素到空位
                tbl[pos] = tbl[j];
                occupy[pos] = 1;
                occupy[j] = 0;
                tbl[j] = {0, 0};
                pos = j; // 更新空位位置
            }
            
            j = (j + 1) % size;
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
        if (occupy1[hk1(key)] && tbl1[hk1(key)].first == key) { // 应该用hk1(key)而不是key % size
            return tbl1[hk1(key)].second;
        }
    
        if (occupy2[hk2(key)] && tbl2[hk2(key)].first == key) { // 应该用hk2(key)而不是(key / size) % size
            return tbl2[hk2(key)].second;
        }
    
        return INVALID;
    }

    void Del(int key) {
        if (occupy1[hk1(key)] && tbl1[hk1(key)].first == key) {
            tbl1[hk1(key)] = {0, 0};
            occupy1[hk1(key)] = 0;
        }
    
        if (occupy2[hk2(key)] && tbl2[hk2(key)].first == key) {
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
                Set(old_tbl1[i].first, old_tbl1[i].second);
            }
            if (old_occupy2[i]) {
                Set(old_tbl2[i].first, old_tbl2[i].second);
            }
        }
    }

    void Set(int key, int value) {
        if (tbl1[hk1(key)].first == key) {
            tbl1[hk1(key)].second = value;
            return;
        }
        if (tbl2[hk2(key)].first == key) {
            tbl2[hk2(key)].second = value;
            return;
        }
        int tbidx = 1;
        int cur_key = key, cur_val = value;

        stack<pair<int, int>> stk;
        bool done = false;
        bool loop = false;

        int cnt = 1;
        // 迭代找空位
        while (cnt++) {
            if (cnt > 100000) assert(false);
            int hk;
            if (tbidx == 1) {
                hk = hk1(cur_key);

                if (occupy1[hk] == 0) {
                    break;
                } else if (occupy1[hk] != 0 && tbl1[hk].first == key) {
                    tbl1[hk].second = value;
                    done = true;
                    break;
                } else {
                    // 判断是否成环
                    stack<pair<int, int>> tmp_stk = stk;
                    while (!tmp_stk.empty()) {
                        if (tbl1[hk] == tmp_stk.top()) {
                            loop = true;
                            break;
                        } else {
                            tmp_stk.pop();
                        }
                    }
                    if (loop) break;
                    stk.push(tbl1[hk]);
                }
                tbidx = 2;
                cur_key = tbl1[hk].first;
                cur_val = tbl1[hk].second;
            } else {
                hk = hk2(cur_key);
                if (occupy2[hk] == 0) {
                    break;
                } else if (occupy2[hk] != 0 && tbl2[hk].first == key) {
                    tbl2[hk].second = value;
                    done = true;
                    break;
                } else {
                    stack<pair<int, int>> tmp_stk = stk;
                    while (!tmp_stk.empty()) {
                        if (tbl2[hk] == tmp_stk.top()) {
                            loop = true;
                            break;
                        } else {
                            tmp_stk.pop();
                        }
                    }
                    if (loop) break;
                    stk.push(tbl2[hk]);
                }
                tbidx = 1;
                cur_key = tbl2[hk].first;
                cur_val = tbl2[hk].second;
            }
        }

        if (loop) {
            // 成环后扩容
            Enlarge();
            Set(key, value);
            done = true;
        }

        if (!done) {
            // 采用栈来保存要移动的kv对，然后replay
            while (!stk.empty()) {
                if (tbidx == 1) {
                    pair<int, int> kv = stk.top();
                    int hk = hk1(kv.first);
                    tbl1[hk] = kv;
                    occupy1[hk] = 1;
                    stk.pop();
                    tbidx = 2;
                } else {
                    pair<int, int> kv = stk.top();
                    int hk = hk2(kv.first);
                    tbl2[hk] = kv;
                    occupy2[hk] = 1;
                    stk.pop();
                    tbidx = 1;
                }
            }

            if (tbidx == 1) {
                int hk = hk1(key);
                tbl1[hk] = {key, value};
                occupy1[hk] = 1;
            } else {
                int hk = hk2(key);
                tbl2[hk] = {key, value};
                occupy2[hk] = 1;
            }
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