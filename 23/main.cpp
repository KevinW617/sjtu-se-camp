#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <tuple>
#include <map>
#include <map>
#include <cmath>
#include <algorithm>
#include <cassert>

using namespace std;

vector<string> dict_words;
vector<string> enc_words;

vector<vector<bool>> explored;
vector<vector<char>> direction;
typedef pair<int, int> PII;
typedef pair<int, PII> IPII;
priority_queue<IPII, vector<IPII>, greater<IPII>> pq;
map<PII, int> dist;


void LoadData(string dict_path, string en_path) {

    fstream dict_file, en_file;
    dict_file.open(dict_path, std::ios::in);
    if (!dict_file.is_open()) {
        cerr << "can not open " << dict_path << endl;
    }

    string line;
    int c_cnt = 0;
    while (getline(dict_file, line)) {
        if (line[0] == 'c') c_cnt++;
        dict_words.push_back(line);
    }
    dict_file.close();
    cout << c_cnt << endl;

    en_file.open(en_path, std::ios::in);
    if (!en_file.is_open()) {
        cerr << "can not open " << en_path << endl;
    }
    int cnt = 0;
    while (getline(en_file, line)) {
        enc_words.push_back(line);
        cnt++;
    }
    en_file.close();
    cout << cnt << endl;
}

struct TrieNode {
    map<char, TrieNode *> children;
    bool isWord;
};

TrieNode* trie_root;

void insert_trie(TrieNode* root, string word) {
    for (int i = 0; i < word.length(); i++) {
        char ch = word[i];
        if (root->children.find(ch) == root->children.end()) {
            TrieNode* child = new TrieNode;
            child->isWord = false;
            root->children[ch] = child;
        }
        root = root->children[ch];
    }

    root->isWord = true;
}

int cnt_leaf_nodes(TrieNode* root) {
    if (root == NULL) return 0;
    if (root->children.empty()) {
        return 1;
    }

    int cnt = 0;
    for (auto leaf : root->children) {
        cnt += cnt_leaf_nodes(leaf.second);
    }

    return cnt;
}

void BuildTree() {
    trie_root = new TrieNode();
    trie_root->isWord = false;
    for (auto word : dict_words)
        insert_trie(trie_root, word);

    int leaves = cnt_leaf_nodes(trie_root);
    cout << leaves << endl;

}

bool find_trie(TrieNode* root, string word) {
    for (auto ch : word) {
        if (root->children.find(ch) == root->children.end()) {
            return false;
        }
        root = root->children[ch];
    }

    if (root->isWord) return true;

    return false;
}

string dec_word(string word, int n) {
    for (char &c : word) {
        c = 'a' + (c - 'a' - n) % 26;
    }
    return word;
}

void CrackCodeInc() {
    int last_n = 0;
    int start = -1;
    for (auto word : enc_words) {
        int n = last_n;
        string shift_word = dec_word(word, n);
        while (!find_trie(trie_root, shift_word)) {
            assert(n <= 26);
            n++;
            shift_word = dec_word(word, n);
        }
        last_n = n;

        if (start == -1) start = n;
    }

    cout << start << " " << last_n << endl;
}

string secret_word = "";

void longest_word(int r, int c) {
    if (r >= enc_words.size() || c >= enc_words[r].size()) return;

    char ch = enc_words[r][c];

    TrieNode* node;

    int i, j;

    i = r;
    j = c;
    string word1 = "";
    node = trie_root;
    while (i < enc_words.size() && j < enc_words[i].size()) {
        ch = enc_words[i][j];
        if (node->children.find(ch) == node->children.end()) break;

        word1 += ch;
        node = node->children[ch];
        if (node->isWord) {
            if (word1.length() > secret_word.length()) {
                secret_word = word1;
            } else if (word1.length() == secret_word.length()) {
                if (word1 < secret_word) secret_word = word1;
            }
        }
        j++;
    }

    i = r;
    j = c;
    word1 = "";
    node = trie_root;
    while (i < enc_words.size() && j < enc_words[i].size()) {
        ch = enc_words[i][j];
        if (node->children.find(ch) == node->children.end()) break;

        word1 += ch;
        node = node->children[ch];
        if (node->isWord) {
            if (word1.length() > secret_word.length()) {
                secret_word = word1;
            } else if (word1.length() == secret_word.length()) {
                if (word1 < secret_word) secret_word = word1;
            }
        }
        i++;
    }

    i = r;
    j = c;
    word1 = "";
    node = trie_root;
    while (i < enc_words.size() && j < enc_words[i].size()) {
        ch = enc_words[i][j];
        if (node->children.find(ch) == node->children.end()) break;

        word1 += ch;
        node = node->children[ch];
        if (node->isWord) {
            if (word1.length() > secret_word.length()) {
                secret_word = word1;
            } else if (word1.length() == secret_word.length()) {
                if (word1 < secret_word) secret_word = word1;
            }
        }
        i++;
        j++;
    }
}

void CodeInCode() {
    assert(trie_root != NULL);

    for (int i = 0; i < enc_words.size(); i++) {
        for (int j = 0; j < enc_words[i].size(); j++) {
            longest_word(i, j);
        }
    }

    cout << secret_word << endl;
}

int ch_cost(char a, char b) {
    int cnt;
    if (b > a) {
        cnt = b - a;
    } else {
        cnt = b - a + 26;
    }
    return cnt;
}

void PathInCode() {
    explored.resize(enc_words.size());
    direction.resize(enc_words.size());
    for (int i = 0; i < enc_words.size(); i++) {
        explored[i].resize(enc_words[i].size(), false);
        direction[i].resize(enc_words[i].size(), ' ');
    }
    IPII src = {0, {0, 0}};
    pq.push(src);
    dist[{0, 0}] = 0;

    while (!pq.empty()) {
        IPII cur = pq.top();
        pq.pop();
        int cost = cur.first;
        int x = cur.second.first;
        int y = cur.second.second;
        char c = enc_words[x][y];
        explored[x][y] = true;

        if (x == enc_words.size() - 1 && y == enc_words[x].size() - 1) {
            break;
        }

        if (y > 0 && !explored[x][y - 1]) {
            char lc = enc_words[x][y - 1];
            int lcost = cost + ch_cost(c, lc);
            if (dist.find({x, y - 1}) == dist.end() || dist[{x, y - 1}] > lcost) {
                dist[{x, y - 1}] = lcost;
                pq.push({lcost, {x, y - 1}});
                direction[x][y - 1] = 'l';
            }
        }

        if (x < enc_words.size() - 1 && !explored[x + 1][y]) {
            char lc = enc_words[x + 1][y];
            int lcost = cost + ch_cost(c, lc);
            if (dist.find({x + 1, y}) == dist.end() || dist[{x + 1, y}] > lcost) {
                dist[{x + 1, y}] = lcost;
                pq.push({lcost, {x + 1, y}});
                direction[x + 1][y] = 'd';
            }
        }

        if (y < enc_words[x].size() - 1 && !explored[x][y + 1]) {
            char lc = enc_words[x][y + 1];
            int lcost = cost + ch_cost(c, lc);
            if (dist.find({x, y + 1}) == dist.end() || dist[{x, y + 1}] > lcost) {
                dist[{x, y + 1}] = lcost;
                pq.push({lcost, {x, y + 1}});
                direction[x][y + 1] = 'r';
            }
        }
    }

    vector<char> path;
    int a = enc_words.size() - 1;
    int b = enc_words[a].size() - 1;
    int x = a, y = b;
    while (x != 0 || y != 0) {
        char dir = direction[x][y];
        path.push_back(dir);
        if (dir == 'l') {
            y = y + 1;
        } else if (dir == 'r') {
            y = y - 1;
        } else if (dir == 'd') {
            x = x - 1;
        } else {
            assert(false);
        }
    }

    reverse(path.begin(), path.end());
    cout << path.size() << " " << dist[{a, b}] << endl;

    ofstream path_file("path.txt");
    if (path_file.is_open()) {
        for (int i = 0; i < path.size(); i++) {
            path_file << path[i];
            if ((i + 1) % 20 == 0) {
                path_file << endl;
            }
        }
        path_file.close();
    } else {
        cerr << "Unable to open file path.txt" << endl;
    }
}

int main() {
    // 2.1
    string dict_path, en_path;
    cin >> dict_path >> en_path;
    cout << dict_path << " " << en_path << endl;
    LoadData(dict_path, en_path);

    // 2.2
    BuildTree();

    // 2.3
    CrackCodeInc();

    // 2.4
    CodeInCode();

    // 2.5
    PathInCode();

    return 0;
}