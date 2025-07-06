#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <queue>
#include <unordered_set>

using namespace std;

// 全局变量
int N, E, P;
vector<string> DNAS;

// 2.1 数据读取
void LoadData() {
    string inpath;
    cin >> inpath;
    
    ifstream fin(inpath);
    if (!fin.is_open()) {
        cerr << "Cannot open file: " << inpath << endl;
        return;
    }
    
    fin >> N >> E >> P;
    DNAS.resize(N);
    
    int min_len = INT_MAX;
    int max_len = 0;
    
    for (int i = 0; i < N; i++) {
        fin >> DNAS[i];
        min_len = min(min_len, (int)DNAS[i].length());
        max_len = max(max_len, (int)DNAS[i].length());
    }
    
    cout << min_len << " " << max_len << endl;
    fin.close();
}

// 2.2 编辑距离计算
int LevDist(const string &a, const string &b) {
    int m = a.length();
    int n = b.length();
    
    // 创建DP表
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));
    
    // 初始化边界条件
    for (int i = 0; i <= m; i++) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= n; j++) {
        dp[0][j] = j;
    }
    
    // 填充DP表
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (a[i-1] == b[j-1]) {
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = min({
                    dp[i-1][j] + 1,     // 删除
                    dp[i][j-1] + 1,     // 插入
                    dp[i-1][j-1] + 1    // 替换
                });
            }
        }
    }
    
    return dp[m][n];
}

// 2.3 计算相似DNA序列集合
vector<string> RangeQuery(const vector<string> &dna_set, 
                         const string &target_dna, 
                         int eps) {
    vector<string> result;
    
    for (const string &dna : dna_set) {
        if (LevDist(dna, target_dna) <= eps) {
            result.push_back(dna);
        }
    }
    
    return result;
}

// 2.4 DBSCAN算法实现
vector<vector<string>> DBSCAN(const vector<string> &dna_set,
                             int eps,
                             int minpts,
                             int &num_cores,
                             int &num_borders,
                             int &num_outliers,
                             int &num_clusters) {
    int n = dna_set.size();
    vector<int> point_type(n, 0); // 0: 未处理, 1: 核心点, 2: 边界点, 3: 噪声点
    vector<int> cluster_id(n, -1); // -1: 未分配簇
    vector<bool> visited(n, false);
    
    num_cores = 0;
    num_borders = 0;
    num_outliers = 0;
    num_clusters = 0;
    
    // 第一步：识别核心点
    for (int i = 0; i < n; i++) {
        vector<string> neighbors = RangeQuery(dna_set, dna_set[i], eps);
        if (neighbors.size() >= minpts) {
            point_type[i] = 1; // 核心点
            num_cores++;
        }
    }
    
    // 第二步：聚类核心点并形成簇
    for (int i = 0; i < n; i++) {
        if (point_type[i] == 1 && !visited[i]) {
            // 开始新的簇
            queue<int> q;
            q.push(i);
            visited[i] = true;
            cluster_id[i] = num_clusters;
            
            while (!q.empty()) {
                int current = q.front();
                q.pop();
                
                vector<string> neighbors = RangeQuery(dna_set, dna_set[current], eps);
                
                for (const string &neighbor : neighbors) {
                    // 找到邻居在dna_set中的索引
                    auto it = find(dna_set.begin(), dna_set.end(), neighbor);
                    if (it != dna_set.end()) {
                        int neighbor_idx = distance(dna_set.begin(), it);
                        
                        if (!visited[neighbor_idx]) {
                            visited[neighbor_idx] = true;
                            cluster_id[neighbor_idx] = num_clusters;
                            
                            if (point_type[neighbor_idx] == 1) {
                                q.push(neighbor_idx);
                            }
                        } else if (cluster_id[neighbor_idx] == -1) {
                            cluster_id[neighbor_idx] = num_clusters;
                        }
                    }
                }
            }
            num_clusters++;
        }
    }
    
    // 第三步：识别边界点和噪声点
    for (int i = 0; i < n; i++) {
        if (point_type[i] != 1) { // 非核心点
            bool is_border = false;
            
            // 检查是否在某个核心点的邻域内
            for (int j = 0; j < n; j++) {
                if (point_type[j] == 1 && LevDist(dna_set[i], dna_set[j]) <= eps) {
                    point_type[i] = 2; // 边界点
                    is_border = true;
                    num_borders++;
                    break;
                }
            }
            
            if (!is_border) {
                point_type[i] = 3; // 噪声点
                num_outliers++;
            }
        }
    }
    
    // 构建返回的簇集合
    vector<vector<string>> clusters(num_clusters);
    for (int i = 0; i < n; i++) {
        if (cluster_id[i] != -1) {
            clusters[cluster_id[i]].push_back(dna_set[i]);
        }
    }
    
    return clusters;
}

// 2.5 最小半径计算
int MinEPS(const vector<string> &dna_set, int minpts) {
    int eps = 1;
    
    while (true) {
        int num_cores = 0, num_borders = 0, num_outliers = 0, num_clusters = 0;
        DBSCAN(dna_set, eps, minpts, num_cores, num_borders, num_outliers, num_clusters);
        
        if (num_outliers == 0) {
            break;
        }
        eps++;
        
        // 防止无限循环，设置上限
        if (eps > 1000) {
            break;
        }
    }
    
    return eps;
}

int main() {
    // 2.1 数据读取
    LoadData();
    
    // 2.2 编辑距离
    int edit_dist = LevDist(DNAS[0], DNAS[1]);
    cout << edit_dist << endl;
    
    // 2.3 计算相似DNA序列集合
    vector<string> similar_dnas = RangeQuery(DNAS, DNAS[0], E);
    cout << similar_dnas.size() << endl;
    
    // 2.4 DBSCAN
    int num_cores = 0, num_borders = 0, num_outliers = 0, num_clusters = 0;
    vector<vector<string>> clusters = DBSCAN(DNAS, E, P, num_cores, num_borders, num_outliers, num_clusters);
    cout << num_cores << " " << num_borders << " " << num_outliers << " " << num_clusters << endl;
    
    // 2.5 最小半径
    int min_eps = MinEPS(DNAS, P);
    cout << min_eps << endl;
    
    return 0;
}