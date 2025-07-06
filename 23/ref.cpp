#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <climits>
#include <algorithm>

using namespace std;

struct TrieNode {
    map<char, TrieNode*> children;
    bool isWord;
    
    TrieNode() : isWord(false) {}
};

class CaesarCipher {
private:
    vector<string> dictionary;
    vector<string> encryptedWords;
    TrieNode* root;
    vector<string> matrix;
    int rows, cols;
    
public:
    CaesarCipher() {
        root = new TrieNode();
    }
    
    // 步骤1: 读取字典和加密信息
    void LoadData() {
        string dictPath, encPath;
        cin >> dictPath >> encPath;
        
        // 读取字典文件
        ifstream dictFile(dictPath);
        string word;
        int cCount = 0;
        
        while (getline(dictFile, word)) {
            if (!word.empty()) {
                dictionary.push_back(word);
                if (word[0] == 'c') cCount++;
            }
        }
        dictFile.close();
        
        // 读取加密文件
        ifstream encFile(encPath);
        while (getline(encFile, word)) {
            if (!word.empty()) {
                encryptedWords.push_back(word);
            }
        }
        encFile.close();
        
        cout << cCount << " " << encryptedWords.size() << endl;
    }
    
    // 步骤2: 构建字典树
    void BuildTrie() {
        for (const string& word : dictionary) {
            TrieNode* curr = root;
            for (char c : word) {
                if (curr->children.find(c) == curr->children.end()) {
                    curr->children[c] = new TrieNode();
                }
                curr = curr->children[c];
            }
            curr->isWord = true;
        }
        
        // 计算叶子节点数量
        int leafCount = countLeafNodes(root);
        cout << leafCount << endl;
    }
    
    int countLeafNodes(TrieNode* node) {
        if (!node) return 0;
        if (node->children.empty()) return 1;
        
        int count = 0;
        for (auto& pair : node->children) {
            count += countLeafNodes(pair.second);
        }
        return count;
    }
    
    bool searchWord(const string& word) {
        TrieNode* curr = root;
        for (char c : word) {
            if (curr->children.find(c) == curr->children.end()) {
                return false;
            }
            curr = curr->children[c];
        }
        return curr->isWord;
    }
    
    // 步骤3: 单调密码序列
    void CrackCodeInc() {
        int n = encryptedWords.size();
        vector<int> offsets(n);
        
        // 尝试找到单调递增的偏移序列
        for (int maxOffset = 1; maxOffset <= 26; maxOffset++) {
            bool found = true;
            
            for (int start = 0; start <= maxOffset; start++) {
                fill(offsets.begin(), offsets.end(), 0);
                
                // 生成单调递增序列
                for (int i = 0; i < n; i++) {
                    offsets[i] = start + (maxOffset - start) * i / (n - 1);
                }
                
                // 检查所有解密后的单词是否在字典中
                bool validSequence = true;
                for (int i = 0; i < n; i++) {
                    string decrypted = decrypt(encryptedWords[i], offsets[i]);
                    if (!searchWord(decrypted)) {
                        validSequence = false;
                        break;
                    }
                }
                
                if (validSequence) {
                    cout << offsets[0] << " " << offsets[n-1] << endl;
                    
                    // 构建解密后的矩阵
                    matrix.clear();
                    for (int i = 0; i < n; i++) {
                        matrix.push_back(decrypt(encryptedWords[i], offsets[i]));
                    }
                    rows = matrix.size();
                    cols = rows > 0 ? matrix[0].length() : 0;
                    return;
                }
            }
        }
    }
    
    string decrypt(const string& word, int offset) {
        string result = word;
        for (char& c : result) {
            c = ((c - 'a' - offset + 26) % 26) + 'a';
        }
        return result;
    }
    
    // 步骤4: 密文中文
    void CodeInCode() {
        string longestWord = "";
        int bestRow = 0, bestCol = 0;
        
        // 重新构建密文矩阵（未解密的）
        vector<string> cipherMatrix = encryptedWords;
        int cipherRows = cipherMatrix.size();
        int cipherCols = cipherRows > 0 ? cipherMatrix[0].length() : 0;
        
        // 水平方向搜索密文
        for (int i = 0; i < cipherRows; i++) {
            for (int j = 0; j < cipherCols; j++) {
                for (int len = 1; j + len <= cipherCols; len++) {
                    string cipherCandidate = cipherMatrix[i].substr(j, len);
                    
                    // 尝试所有可能的偏移量解密
                    for (int offset = 1; offset <= 26; offset++) {
                        string decrypted = decrypt(cipherCandidate, offset);
                        if (searchWord(decrypted) && 
                            (decrypted.length() > longestWord.length() || 
                             (decrypted.length() == longestWord.length() && decrypted < longestWord))) {
                            longestWord = decrypted;
                            bestRow = i;
                            bestCol = j;
                        }
                    }
                }
            }
        }
        
        // 垂直方向搜索密文
        for (int j = 0; j < cipherCols; j++) {
            for (int i = 0; i < cipherRows; i++) {
                for (int len = 1; i + len <= cipherRows; len++) {
                    string cipherCandidate = "";
                    for (int k = 0; k < len; k++) {
                        cipherCandidate += cipherMatrix[i + k][j];
                    }
                    
                    // 尝试所有可能的偏移量解密
                    for (int offset = 1; offset <= 26; offset++) {
                        string decrypted = decrypt(cipherCandidate, offset);
                        if (searchWord(decrypted) && 
                            (decrypted.length() > longestWord.length() || 
                             (decrypted.length() == longestWord.length() && decrypted < longestWord))) {
                            longestWord = decrypted;
                            bestRow = i;
                            bestCol = j;
                        }
                    }
                }
            }
        }
        
        // 对角线方向搜索密文
        for (int i = 0; i < cipherRows; i++) {
            for (int j = 0; j < cipherCols; j++) {
                for (int len = 1; i + len <= cipherRows && j + len <= cipherCols; len++) {
                    string cipherCandidate = "";
                    for (int k = 0; k < len; k++) {
                        cipherCandidate += cipherMatrix[i + k][j + k];
                    }
                    
                    // 尝试所有可能的偏移量解密
                    for (int offset = 1; offset <= 26; offset++) {
                        string decrypted = decrypt(cipherCandidate, offset);
                        if (searchWord(decrypted) && 
                            (decrypted.length() > longestWord.length() || 
                             (decrypted.length() == longestWord.length() && decrypted < longestWord))) {
                            longestWord = decrypted;
                            bestRow = i;
                            bestCol = j;
                        }
                    }
                }
            }
        }
        
        cout << bestRow << " " << bestCol << " " << longestWord << endl;
    }
    
    // 步骤5: 密文最短路径
    void PathInCode() {
        if (rows == 0 || cols == 0) return;
        
        // 使用密文矩阵而不是解密后的矩阵
        vector<string> cipherMatrix = encryptedWords;
        int cipherRows = cipherMatrix.size();
        int cipherCols = cipherRows > 0 ? cipherMatrix[0].length() : 0;
        
        // 使用Dijkstra算法
        vector<vector<int>> dist(cipherRows, vector<int>(cipherCols, INT_MAX));
        vector<vector<string>> path(cipherRows, vector<string>(cipherCols));
        
        // 优先队列: {代价, 行, 列, 路径}
        priority_queue<tuple<int, int, int, string>, 
                      vector<tuple<int, int, int, string>>, 
                      greater<tuple<int, int, int, string>>> pq;
        
        dist[0][0] = 0;
        pq.push(make_tuple(0, 0, 0, string("")));
        
        // 方向: 下, 右, 左
        int dx[] = {1, 0, 0};
        int dy[] = {0, 1, -1};
        char moves[] = {'d', 'r', 'l'};
        
        while (!pq.empty()) {
            tuple<int, int, int, string> current = pq.top();
            pq.pop();
            
            int cost = get<0>(current);
            int x = get<1>(current);
            int y = get<2>(current);
            string currentPath = get<3>(current);
            
            if (cost > dist[x][y]) continue;
            if (cost == dist[x][y] && currentPath > path[x][y]) continue;
            
            for (int i = 0; i < 3; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                
                if (nx >= 0 && nx < cipherRows && ny >= 0 && ny < cipherCols) {
                    // 使用密文矩阵计算代价
                    int newCost = cost + getCost(cipherMatrix[x][y], cipherMatrix[nx][ny]);
                    string newPath = currentPath + moves[i];
                    
                    if (newCost < dist[nx][ny] || 
                        (newCost == dist[nx][ny] && newPath < path[nx][ny])) {
                        dist[nx][ny] = newCost;
                        path[nx][ny] = newPath;
                        pq.push(make_tuple(newCost, nx, ny, newPath));
                    }
                }
            }
        }
        
        string finalPath = path[cipherRows-1][cipherCols-1];
        cout << finalPath.length() << " " << dist[cipherRows-1][cipherCols-1] << endl;
        
        // 输出到文件
        ofstream pathFile("path.txt");
        for (int i = 0; i < finalPath.length(); i++) {
            if (i > 0 && i % 20 == 0) {
                pathFile << "\n";
            }
            pathFile << finalPath[i];
        }
        if (!finalPath.empty()) {
            pathFile << "\n";
        }
        pathFile.close();
    }
    
    int getCost(char from, char to) {
        int diff = (to - from + 26) % 26;
        return diff == 0 ? 26 : diff;
    }
    
    void solve() {
        LoadData();
        BuildTrie();
        CrackCodeInc();
        CodeInCode();
        PathInCode();
    }
};

int main() {
    CaesarCipher cipher;
    cipher.solve();
    return 0;
}
