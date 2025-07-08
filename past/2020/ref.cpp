// 姓名：[您的姓名]
// 学号：[您的学号]
// 哈夫曼压缩程序实现

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <cstring>

using namespace std;

// 哈夫曼树节点结构
struct HuffmanNode {
    char ch;           // 字符
    int freq;          // 频率
    int minAscii;      // 子树中最小ASCII码
    HuffmanNode* left;
    HuffmanNode* right;
    
    HuffmanNode(char c, int f) : ch(c), freq(f), minAscii(c), left(nullptr), right(nullptr) {}
    HuffmanNode(int f, int minAsc) : ch(0), freq(f), minAscii(minAsc), left(nullptr), right(nullptr) {}
};

// 用于优先队列的比较器
struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        if (a->freq != b->freq) {
            return a->freq > b->freq;  // 频率小的优先
        }
        return a->minAscii > b->minAscii;  // ASCII码小的优先
    }
};

// 全局变量
map<char, int> freqTable;           // 字符频率表
HuffmanNode* huffmanTree = nullptr;  // 哈夫曼树根节点
map<char, string> codingTable;      // 字符编码表

// 步骤一：统计字符频率
void do_statistics(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }
    
    freqTable.clear();
    char ch;
    while (file.get(ch)) {
        freqTable[ch]++;
    }
    file.close();
    
    // 输出统计结果
    cout << freqTable.size() << endl;
    
    // 按频率排序找出前三个
    vector<pair<int, char>> freqList;
    for (auto& p : freqTable) {
        freqList.push_back({p.second, p.first});
    }
    
    // 按频率降序，ASCII码升序排序
    sort(freqList.begin(), freqList.end(), [](const pair<int, char>& a, const pair<int, char>& b) {
        if (a.first != b.first) return a.first > b.first;
        return a.second < b.second;
    });
    
    // 输出前三个
    for (int i = 0; i < min(3, (int)freqList.size()); i++) {
        cout << freqList[i].second << " " << freqList[i].first << endl;
    }
}

// 计算树的深度
int getTreeDepth(HuffmanNode* node) {
    if (!node) return -1;
    if (!node->left && !node->right) return 0;
    return 1 + max(getTreeDepth(node->left), getTreeDepth(node->right));
}

// 步骤二：建立哈夫曼树
void build_tree() {
    if (freqTable.empty()) return;
    
    // 如果只有一个字符，特殊处理
    if (freqTable.size() == 1) {
        auto it = freqTable.begin();
        huffmanTree = new HuffmanNode(it->first, it->second);
        cout << 0 << endl;  // 深度为0
        return;
    }
    
    // 创建优先队列
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;
    
    // 将所有字符作为叶子节点加入队列
    for (auto& p : freqTable) {
        pq.push(new HuffmanNode(p.first, p.second));
    }
    
    // 构建哈夫曼树
    while (pq.size() > 1) {
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* left = pq.top(); pq.pop();
        
        // 确保左子树ASCII码小于右子树
        if (left->minAscii > right->minAscii) {
            swap(left, right);
        }
        
        // 创建新的内部节点
        int newFreq = left->freq + right->freq;
        int newMinAscii = min(left->minAscii, right->minAscii);
        HuffmanNode* newNode = new HuffmanNode(newFreq, newMinAscii);
        newNode->left = left;
        newNode->right = right;
        
        pq.push(newNode);
    }
    
    huffmanTree = pq.top();
    cout << getTreeDepth(huffmanTree) << endl;
}

// 生成编码表
void generateCodes(HuffmanNode* node, string code) {
    if (!node) return;
    
    if (!node->left && !node->right) {
        // 叶子节点
        codingTable[node->ch] = code.empty() ? "0" : code;  // 特殊情况：只有一个字符
        return;
    }
    
    generateCodes(node->left, code + "0");
    generateCodes(node->right, code + "1");
}

// 步骤三：编码
void encode(const string& outputFile) {
    codingTable.clear();
    generateCodes(huffmanTree, "");
    
    // 输出编码表到文件
    ofstream file(outputFile);
    if (!file) {
        cerr << "无法创建文件: " << outputFile << endl;
        return;
    }
    
    // 按ASCII码顺序输出
    for (auto& p : codingTable) {
        file << p.first << " " << p.second << endl;
    }
    file.close();
    
    // 输出字符e的编码
    if (codingTable.find('e') != codingTable.end()) {
        cout << codingTable['e'] << endl;
    }
}

// 步骤四：压缩文件
void compress(const string& inputFile, const string& outputFile) {
    ifstream inFile(inputFile, ios::binary);
    if (!inFile) {
        cerr << "无法打开输入文件: " << inputFile << endl;
        return;
    }
    
    // 读取原文件并转换为比特流
    string bitStream = "";
    char ch;
    while (inFile.get(ch)) {
        bitStream += codingTable[ch];
    }
    inFile.close();
    
    // 计算有效比特数
    long long validBits = bitStream.length();
    
    // 补足到8的倍数
    while (bitStream.length() % 8 != 0) {
        bitStream += "0";
    }
    
    // 写入压缩文件
    ofstream outFile(outputFile, ios::binary);
    if (!outFile) {
        cerr << "无法创建输出文件: " << outputFile << endl;
        return;
    }
    
    // 写入有效比特数（小端模式）
    outFile.write(reinterpret_cast<const char*>(&validBits), sizeof(validBits));
    
    // 将比特流转换为字节流并写入
    for (size_t i = 0; i < bitStream.length(); i += 8) {
        string byte = bitStream.substr(i, 8);
        unsigned char byteValue = 0;
        for (int j = 0; j < 8; j++) {
            if (byte[j] == '1') {
                byteValue |= (1 << (7 - j));  // 大端顺序
            }
        }
        outFile.write(reinterpret_cast<const char*>(&byteValue), 1);
    }
    outFile.close();
    
    cout << validBits << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "用法: " << argv[0] << " <输入文件> <编码文件> <压缩文件>" << endl;
        return 1;
    }
    
    string inputFile = argv[1];
    string huffidxFile = argv[2];
    string huffzipFile = argv[3];
    
    // 步骤一：统计字符频率
    do_statistics(inputFile);
    
    // 步骤二：建立哈夫曼树
    build_tree();
    
    // 步骤三：编码
    encode(huffidxFile);
    
    // 步骤四：压缩文件
    compress(inputFile, huffzipFile);
    
    return 0;
}
