#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <limits>
#include <unordered_map>
#include <queue>

// 定义键值对结构
struct KeyValue {
    uint32_t key;
    uint32_t value;
    bool isEmpty;

    KeyValue() : key(0), value(0), isEmpty(true) {}
    KeyValue(uint32_t k, uint32_t v) : key(k), value(v), isEmpty(false) {}
};

// Linear Hashing 实现
class LinearHashing {
private:
    std::vector<KeyValue> table;
    size_t capacity;
    size_t size;

    // 哈希函数
    size_t hash(uint32_t key) const {
        return key % capacity;
    }

    // 扩容操作
    void resize() {
        size_t oldCapacity = capacity;
        capacity *= 2;
        std::vector<KeyValue> newTable(capacity);
        
        // 重新插入所有元素
        for (const auto& kv : table) {
            if (!kv.isEmpty) {
                size_t pos = hash(kv.key);
                while (!newTable[pos].isEmpty) {
                    pos = (pos + 1) % capacity;
                }
                newTable[pos] = kv;
            }
        }
        
        table = std::move(newTable);
    }

public:
    LinearHashing(size_t initialCapacity = 8) : capacity(initialCapacity), size(0) {
        table.resize(capacity);
    }

    // 插入或更新键值对
    void set(uint32_t key, uint32_t value) {
        // 检查是否需要扩容
        if (size >= capacity / 2) {
            resize();
        }

        size_t pos = hash(key);
        // 查找键是否存在
        size_t startPos = pos;
        do {
            if (table[pos].isEmpty) {
                // 找到空位，插入新键值对
                table[pos] = KeyValue(key, value);
                size++;
                return;
            }
            
            if (table[pos].key == key) {
                // 键已存在，更新值
                table[pos].value = value;
                return;
            }
            
            pos = (pos + 1) % capacity;
        } while (pos != startPos);
    }

    // 获取键对应的值
    uint32_t* get(uint32_t key) {
        size_t pos = hash(key);
        size_t startPos = pos;
        
        do {
            if (table[pos].isEmpty) {
                // 遇到空位，说明键不存在
                return nullptr;
            }
            
            if (table[pos].key == key) {
                // 找到键，返回值
                return &table[pos].value;
            }
            
            pos = (pos + 1) % capacity;
        } while (pos != startPos);
        
        return nullptr;
    }

    // 删除键值对
    void del(uint32_t key) {
        size_t pos = hash(key);
        size_t startPos = pos;
        
        // 找到要删除的键
        do {
            if (table[pos].isEmpty) {
                // 遇到空位，说明键不存在
                return;
            }
            
            if (table[pos].key == key) {
                break;
            }
            
            pos = (pos + 1) % capacity;
        } while (pos != startPos);
        
        // 如果键不存在
        if (pos == startPos && (table[pos].isEmpty || table[pos].key != key)) {
            return;
        }
        
        // 删除键值对
        table[pos].isEmpty = true;
        size--;
        
        // 重新排列后续元素
        size_t nextPos = (pos + 1) % capacity;
        while (!table[nextPos].isEmpty) {
            uint32_t nextKey = table[nextPos].key;
            size_t correctPos = hash(nextKey);
            
            // 判断是否需要移动
            bool needMove = false;
            size_t checkPos = correctPos;
            while (checkPos != nextPos) {
                if (checkPos == pos) {
                    needMove = true;
                    break;
                }
                checkPos = (checkPos + 1) % capacity;
            }
            
            if (needMove) {
                table[pos] = table[nextPos];
                table[nextPos].isEmpty = true;
                pos = nextPos;
            }
            
            nextPos = (nextPos + 1) % capacity;
        }
    }

    // 获取当前哈希表大小
    size_t getSize() const {
        return size;
    }

    // 获取当前哈希表容量
    size_t getCapacity() const {
        return capacity;
    }

    // 获取表中所有键值对（用于调试）
    std::vector<KeyValue> getAll() const {
        std::vector<KeyValue> result;
        for (const auto& kv : table) {
            if (!kv.isEmpty) {
                result.push_back(kv);
            }
        }
        return result;
    }
};

// Cuckoo Hashing 实现
class CuckooHashing {
private:
    std::vector<KeyValue> table1;
    std::vector<KeyValue> table2;
    size_t capacity;
    size_t size;
    static const int MAX_LOOP = 1000000; // 防止无限循环
    
    // 存储替换路径的结构
    struct ReplacePath {
        std::vector<std::pair<int, size_t>> path; // 表号(1或2)和位置
    };
    
    ReplacePath lastReplacePath; // 存储最近一次插入的替换路径

    // 哈希函数 1
    size_t hash1(uint32_t key) const {
        return key % capacity;
    }

    // 哈希函数 2
    size_t hash2(uint32_t key) const {
        return (key / capacity) % capacity;
    }

    // 扩容操作
    void resize() {
        size_t oldCapacity = capacity;
        capacity *= 2;
        
        std::vector<KeyValue> oldTable1 = table1;
        std::vector<KeyValue> oldTable2 = table2;
        
        table1.clear();
        table2.clear();
        table1.resize(capacity);
        table2.resize(capacity);
        
        size = 0;
        lastReplacePath.path.clear();
        
        // 重新插入所有元素
        for (const auto& kv : oldTable1) {
            if (!kv.isEmpty) {
                set(kv.key, kv.value);
            }
        }
        
        for (const auto& kv : oldTable2) {
            if (!kv.isEmpty) {
                set(kv.key, kv.value);
            }
        }
    }

public:
    CuckooHashing(size_t initialCapacity = 8) : capacity(initialCapacity), size(0) {
        table1.resize(capacity);
        table2.resize(capacity);
    }

    // 插入或更新键值对
    bool set(uint32_t key, uint32_t value) {
        // 先检查是否已存在
        uint32_t* existingValue = get(key);
        if (existingValue) {
            *existingValue = value;
            return true;
        }
        
        // 清空上次的替换路径
        lastReplacePath.path.clear();
        
        // 尝试插入到第一个表
        size_t pos1 = hash1(key);
        if (table1[pos1].isEmpty) {
            table1[pos1] = KeyValue(key, value);
            size++;
            lastReplacePath.path.push_back({1, pos1});
            return true;
        }
        
        // 尝试插入到第二个表
        size_t pos2 = hash2(key);
        if (table2[pos2].isEmpty) {
            table2[pos2] = KeyValue(key, value);
            size++;
            lastReplacePath.path.push_back({2, pos2});
            return true;
        }
        
        // 两个位置都被占用，开始替换过程
        KeyValue current(key, value);
        int table = 1; // 从第一个表开始
        int count = 0;
        
        while (1) {
            if (table == 1) {
                // 从表1替换到表2
                KeyValue temp = table1[pos1];
                table1[pos1] = current;
                lastReplacePath.path.push_back({1, pos1});
                
                current = temp;
                pos2 = hash2(current.key);
                
                if (table2[pos2].isEmpty) {
                    table2[pos2] = current;
                    for (auto p : lastReplacePath.path) {
                        if (p == std::make_pair(2, pos2)) {
                            break;
                        }
                    }
                    lastReplacePath.path.push_back({2, pos2});
                    size++;
                    return true;
                }
                
                table = 2;
            } else {
                // 从表2替换到表1
                KeyValue temp = table2[pos2];
                table2[pos2] = current;
                lastReplacePath.path.push_back({2, pos2});
                
                current = temp;
                pos1 = hash1(current.key);
                
                if (table1[pos1].isEmpty) {
                    table1[pos1] = current;
                    for (auto p : lastReplacePath.path) {
                        if (p == std::make_pair(1, pos1)) {
                            break;
                        }
                    }
                    lastReplacePath.path.push_back({1, pos1});
                    size++;
                    return true;
                }
                
                table = 1;
            }
            
            count++;
        }
        
        // 无法插入，需要扩容
        resize();
        return set(key, value); // 重新尝试插入
    }

    // 获取键对应的值
    uint32_t* get(uint32_t key) {
        size_t pos1 = hash1(key);
        if (!table1[pos1].isEmpty && table1[pos1].key == key) {
            return &table1[pos1].value;
        }
        
        size_t pos2 = hash2(key);
        if (!table2[pos2].isEmpty && table2[pos2].key == key) {
            return &table2[pos2].value;
        }
        
        return nullptr;
    }

    // 删除键值对
    void del(uint32_t key) {
        size_t pos1 = hash1(key);
        if (!table1[pos1].isEmpty && table1[pos1].key == key) {
            table1[pos1].isEmpty = true;
            size--;
            return;
        }
        
        size_t pos2 = hash2(key);
        if (!table2[pos2].isEmpty && table2[pos2].key == key) {
            table2[pos2].isEmpty = true;
            size--;
            return;
        }
    }

    // 获取当前哈希表大小
    size_t getSize() const {
        return size;
    }

    // 获取当前哈希表容量
    size_t getCapacity() const {
        return capacity;
    }

    // 获取表1的所有键值对
    const std::vector<KeyValue>& getTable1() const {
        return table1;
    }

    // 获取表2的所有键值对
    const std::vector<KeyValue>& getTable2() const {
        return table2;
    }
    
    // 获取最近插入的替换路径
    const ReplacePath& getLastReplacePath() const {
        return lastReplacePath;
    }
};

// 正确性测试函数
void correctnessTest(const std::string& filename, bool isSmall = true) {
    std::cout << "测试" << (isSmall ? "小规模" : "大规模") << "数据..." << std::endl;
    
    std::ifstream inFile(filename + ".in");
    std::ifstream ansFile(filename + ".ans");
    
    if (!inFile.is_open() || !ansFile.is_open()) {
        std::cerr << "无法打开测试文件或答案文件！" << std::endl;
        return;
    }
    
    LinearHashing linearHash;
    CuckooHashing cuckooHash;
    
    std::string operation;
    uint32_t key, value;
    std::vector<std::string> linearResults, cuckooResults;
    std::vector<std::string> expectedResults;
    
    // 读取期望结果
    std::string line;
    while (std::getline(ansFile, line)) {
        expectedResults.push_back(line);
    }
    
    int getCount = 0;
    
    // 执行操作
    while (inFile >> operation) {
        if (operation == "Set") {
            inFile >> key >> value;
            linearHash.set(key, value);
            cuckooHash.set(key, value);
        } else if (operation == "Get") {
            inFile >> key;
            uint32_t* linearValue = linearHash.get(key);
            uint32_t* cuckooValue = cuckooHash.get(key);
            
            if (linearValue) {
                linearResults.push_back(std::to_string(*linearValue));
            } else {
                linearResults.push_back("null");
            }
            
            if (cuckooValue) {
                cuckooResults.push_back(std::to_string(*cuckooValue));
            } else {
                cuckooResults.push_back("null");
            }
            
            getCount++;
        } else if (operation == "Del") {
            inFile >> key;
            linearHash.del(key);
            cuckooHash.del(key);
        }
    }
    
    // 检查结果
    bool linearCorrect = (linearResults == expectedResults);
    bool cuckooCorrect = (cuckooResults == expectedResults);
    
    std::cout << "Linear Hashing: " << (linearCorrect ? "通过" : "失败") << std::endl;
    std::cout << "Cuckoo Hashing: " << (cuckooCorrect ? "通过" : "失败") << std::endl;
    
    // 如果失败，显示详细信息
    if (!linearCorrect) {
        std::cout << "Linear Hashing 失败详情：" << std::endl;
        for (size_t i = 0; i < expectedResults.size(); i++) {
            if (i < linearResults.size() && linearResults[i] != expectedResults[i]) {
                std::cout << "第" << (i + 1) << "个Get：期望 " << expectedResults[i] 
                          << "，实际 " << linearResults[i] << std::endl;
            }
        }
    }
    
    if (!cuckooCorrect) {
        std::cout << "Cuckoo Hashing 失败详情：" << std::endl;
        for (size_t i = 0; i < expectedResults.size(); i++) {
            if (i < cuckooResults.size() && cuckooResults[i] != expectedResults[i]) {
                std::cout << "第" << (i + 1) << "个Get：期望 " << expectedResults[i] 
                          << "，实际 " << cuckooResults[i] << std::endl;
            }
        }
    }
    
    inFile.close();
    ansFile.close();
}

// 性能测试函数
void performanceTest() {
    std::cout << "开始性能测试..." << std::endl;
    
    const int NUM_KEYS = 10000;
    const int TEST_KEYS = 1000;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist;
    
    // 生成随机键值对
    std::vector<std::pair<uint32_t, uint32_t>> keyValues;
    for (int i = 0; i < NUM_KEYS; i++) {
        keyValues.push_back({dist(gen), dist(gen)});
    }
    
    // 初始化测试数据
    std::vector<double> linearInsertLatencies;
    std::vector<double> cuckooInsertLatencies;
    linearInsertLatencies.reserve(TEST_KEYS);
    cuckooInsertLatencies.reserve(TEST_KEYS);
    
    // 初始化哈希表
    LinearHashing linearHash;
    CuckooHashing cuckooHash;
    
    // 测试0：初始插入延迟
    for (int i = 0; i < NUM_KEYS; i++) {
        auto key = keyValues[i].first;
        auto value = keyValues[i].second;
        
        if (i < TEST_KEYS) {
            // 测量Linear Hashing插入时间
            auto start = std::chrono::high_resolution_clock::now();
            linearHash.set(key, value);
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::micro>(end - start).count();
            linearInsertLatencies.push_back(duration);
            
            // 测量Cuckoo Hashing插入时间
            start = std::chrono::high_resolution_clock::now();
            cuckooHash.set(key, value);
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration<double, std::micro>(end - start).count();
            cuckooInsertLatencies.push_back(duration);
        } else {
            // 直接插入
            linearHash.set(key, value);
            cuckooHash.set(key, value);
        }
    }
    
    // 保存测试0的结果到CSV文件
    std::ofstream insertLatencyFile("insert_latency.csv");
    insertLatencyFile << "操作ID,Linear Hashing (μs),Cuckoo Hashing (μs)\n";
    for (size_t i = 0; i < TEST_KEYS; i++) {
        insertLatencyFile << i << "," << linearInsertLatencies[i] << "," << cuckooInsertLatencies[i] << "\n";
    }
    insertLatencyFile.close();
    
    // 测试1：全Get请求
    std::vector<uint32_t> getKeys;
    for (int i = 0; i < NUM_KEYS; i++) {
        getKeys.push_back(keyValues[i].first);
    }
    std::shuffle(getKeys.begin(), getKeys.end(), gen);
    
    // 执行测试1
    double linearMinLatency = std::numeric_limits<double>::max();
    double linearMaxLatency = 0;
    double linearTotalLatency = 0;
    
    double cuckooMinLatency = std::numeric_limits<double>::max();
    double cuckooMaxLatency = 0;
    double cuckooTotalLatency = 0;
    
    auto linearStart = std::chrono::high_resolution_clock::now();
    for (auto key : getKeys) {
        auto start = std::chrono::high_resolution_clock::now();
        linearHash.get(key);
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::micro>(end - start).count();
        linearMinLatency = std::min(linearMinLatency, duration);
        linearMaxLatency = std::max(linearMaxLatency, duration);
        linearTotalLatency += duration;
    }
    auto linearEnd = std::chrono::high_resolution_clock::now();
    double linearTime = std::chrono::duration<double>(linearEnd - linearStart).count();
    double linearThroughput = NUM_KEYS / linearTime;
    
    auto cuckooStart = std::chrono::high_resolution_clock::now();
    for (auto key : getKeys) {
        auto start = std::chrono::high_resolution_clock::now();
        cuckooHash.get(key);
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::micro>(end - start).count();
        cuckooMinLatency = std::min(cuckooMinLatency, duration);
        cuckooMaxLatency = std::max(cuckooMaxLatency, duration);
        cuckooTotalLatency += duration;
    }
    auto cuckooEnd = std::chrono::high_resolution_clock::now();
    double cuckooTime = std::chrono::duration<double>(cuckooEnd - cuckooStart).count();
    double cuckooThroughput = NUM_KEYS / cuckooTime;
    
    double linearAvgLatency = linearTotalLatency / NUM_KEYS;
    double cuckooAvgLatency = cuckooTotalLatency / NUM_KEYS;
    
    std::cout << "测试1（全Get请求）结果：" << std::endl;
    std::cout << "Linear Hashing：" << std::endl;
    std::cout << "  吞吐量：" << std::fixed << std::setprecision(2) << linearThroughput << " 请求/秒" << std::endl;
    std::cout << "  最小延迟：" << std::fixed << std::setprecision(2) << linearMinLatency << " μs" << std::endl;
    std::cout << "  最大延迟：" << std::fixed << std::setprecision(2) << linearMaxLatency << " μs" << std::endl;
    std::cout << "  平均延迟：" << std::fixed << std::setprecision(2) << linearAvgLatency << " μs" << std::endl;
    
    std::cout << "Cuckoo Hashing：" << std::endl;
    std::cout << "  吞吐量：" << std::fixed << std::setprecision(2) << cuckooThroughput << " 请求/秒" << std::endl;
    std::cout << "  最小延迟：" << std::fixed << std::setprecision(2) << cuckooMinLatency << " μs" << std::endl;
    std::cout << "  最大延迟：" << std::fixed << std::setprecision(2) << cuckooMaxLatency << " μs" << std::endl;
    std::cout << "  平均延迟：" << std::fixed << std::setprecision(2) << cuckooAvgLatency << " μs" << std::endl;
    
    // 测试2：混合Get和Set请求
    std::vector<std::pair<std::string, uint32_t>> mixedOps;
    for (int i = 0; i < NUM_KEYS / 2; i++) {
        mixedOps.push_back({"Get", keyValues[i].first});
        mixedOps.push_back({"Set", i});
    }
    std::shuffle(mixedOps.begin(), mixedOps.end(), gen);
    
    // 执行测试2
    linearMinLatency = std::numeric_limits<double>::max();
    linearMaxLatency = 0;
    linearTotalLatency = 0;
    
    cuckooMinLatency = std::numeric_limits<double>::max();
    cuckooMaxLatency = 0;
    cuckooTotalLatency = 0;
    
    linearStart = std::chrono::high_resolution_clock::now();
    for (const auto& op : mixedOps) {
        if (op.first == "Get") {
            auto start = std::chrono::high_resolution_clock::now();
            linearHash.get(op.second);
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::micro>(end - start).count();
            linearMinLatency = std::min(linearMinLatency, duration);
            linearMaxLatency = std::max(linearMaxLatency, duration);
            linearTotalLatency += duration;
        } else {
            auto start = std::chrono::high_resolution_clock::now();
            linearHash.set(op.second, dist(gen));
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::micro>(end - start).count();
            linearMinLatency = std::min(linearMinLatency, duration);
            linearMaxLatency = std::max(linearMaxLatency, duration);
            linearTotalLatency += duration;
        }
    }
    linearEnd = std::chrono::high_resolution_clock::now();
    linearTime = std::chrono::duration<double>(linearEnd - linearStart).count();
    linearThroughput = NUM_KEYS / linearTime;
    
    cuckooStart = std::chrono::high_resolution_clock::now();
    for (const auto& op : mixedOps) {
        if (op.first == "Get") {
            auto start = std::chrono::high_resolution_clock::now();
            cuckooHash.get(op.second);
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::micro>(end - start).count();
            cuckooMinLatency = std::min(cuckooMinLatency, duration);
            cuckooMaxLatency = std::max(cuckooMaxLatency, duration);
            cuckooTotalLatency += duration;
        } else {
            auto start = std::chrono::high_resolution_clock::now();
            cuckooHash.set(op.second, dist(gen));
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::micro>(end - start).count();
            cuckooMinLatency = std::min(cuckooMinLatency, duration);
            cuckooMaxLatency = std::max(cuckooMaxLatency, duration);
            cuckooTotalLatency += duration;
        }
    }
    cuckooEnd = std::chrono::high_resolution_clock::now();
    cuckooTime = std::chrono::duration<double>(cuckooEnd - cuckooStart).count();
    cuckooThroughput = NUM_KEYS / cuckooTime;
    
    linearAvgLatency = linearTotalLatency / NUM_KEYS;
    cuckooAvgLatency = cuckooTotalLatency / NUM_KEYS;
    
    std::cout << "\n测试2（混合Get和Set请求）结果：" << std::endl;
    std::cout << "Linear Hashing：" << std::endl;
    std::cout << "  吞吐量：" << std::fixed << std::setprecision(2) << linearThroughput << " 请求/秒" << std::endl;
    std::cout << "  最小延迟：" << std::fixed << std::setprecision(2) << linearMinLatency << " μs" << std::endl;
    std::cout << "  最大延迟：" << std::fixed << std::setprecision(2) << linearMaxLatency << " μs" << std::endl;
    std::cout << "  平均延迟：" << std::fixed << std::setprecision(2) << linearAvgLatency << " μs" << std::endl;
    
    std::cout << "Cuckoo Hashing：" << std::endl;
    std::cout << "  吞吐量：" << std::fixed << std::setprecision(2) << cuckooThroughput << " 请求/秒" << std::endl;
    std::cout << "  最小延迟：" << std::fixed << std::setprecision(2) << cuckooMinLatency << " μs" << std::endl;
    std::cout << "  最大延迟：" << std::fixed << std::setprecision(2) << cuckooMaxLatency << " μs" << std::endl;
    std::cout << "  平均延迟：" << std::fixed << std::setprecision(2) << cuckooAvgLatency << " μs" << std::endl;
    
    // 保存性能测试结果到CSV文件以便绘图
    std::ofstream latencyFile("latency.csv");
    latencyFile << "测试,方法,最小延迟(μs),平均延迟(μs),最大延迟(μs)\n";
    latencyFile << "全Get,Linear," << linearMinLatency << "," << linearAvgLatency << "," << linearMaxLatency << "\n";
    latencyFile << "全Get,Cuckoo," << cuckooMinLatency << "," << cuckooAvgLatency << "," << cuckooMaxLatency << "\n";
    latencyFile << "混合,Linear," << linearMinLatency << "," << linearAvgLatency << "," << linearMaxLatency << "\n";
    latencyFile << "混合,Cuckoo," << cuckooMinLatency << "," << cuckooAvgLatency << "," << cuckooMaxLatency << "\n";
    latencyFile.close();
    
    std::ofstream throughputFile("throughput.csv");
    throughputFile << "测试,方法,吞吐量(请求/秒)\n";
    throughputFile << "全Get,Linear," << linearThroughput << "\n";
    throughputFile << "全Get,Cuckoo," << cuckooThroughput << "\n";
    throughputFile << "混合,Linear," << linearThroughput << "\n";
    throughputFile << "混合,Cuckoo," << cuckooThroughput << "\n";
    throughputFile.close();
}

int main(int argc, char** argv) {
    // 正确性测试
    std::cout << "=== 正确性测试 ===" << std::endl;
    correctnessTest("small", true);
    correctnessTest("large", false);
    
    // 性能测试
    std::cout << "\n=== 性能测试 ===" << std::endl;
    performanceTest();
    
    return 0;
}
