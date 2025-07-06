#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <numeric>
#include <climits>

using namespace std;

int N, E, P;

vector<string> DNAS;

void LoadData() {
    string inpath;
    cin >> inpath;

    ifstream fin(inpath);
    if (!fin.is_open()) {
        cerr << "can not open " << inpath << endl;
        return;
    }
    string first_line;
    getline(fin, first_line);

    stringstream ss(first_line);
    ss >> N >> E >> P;

    int max_len = 0;
    int min_len = INT_MAX;
    for (int i = 0; i < N; i++) {
        string line;
        getline(fin, line);
        if (line.length() > max_len) max_len = line.length();
        if (line.length() < min_len) min_len = line.length();
        DNAS.push_back(line);
    }

    cout << min_len << " " << max_len << endl;
}

int LevDist(const string &a, const string &b) {
    vector<vector<int>> dp(a.length() + 1, vector<int>(b.length() + 1, 0));

    for (int i = 0; i <= a.length(); i++) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= b.length(); j++) {
        dp[0][j] = j;
    }

    for (int i = 1; i <= a.length(); i++) {
        for (int j = 1; j <= b.length(); j++) {
            dp[i][j] = min(dp[i - 1][j] + 1, dp[i][j - 1] + 1);

            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = min(dp[i][j], dp[i - 1][j - 1]);
            } else {
                dp[i][j] = min(dp[i][j], dp[i - 1][j - 1] + 1);
            }
        }
    }

    return dp[a.length()][b.length()];
}

vector<string> RangeQuery(const vector<string> &dna_set, const string &target_dna, int eps) {
    vector<string> result;
    for (auto dna : dna_set) {
        if (LevDist(dna, target_dna) <= eps) {
            result.push_back(dna);
        }
    }
    return result;
}

struct DSU {
    vector<int> parent;
    int components;

    DSU(int n) {
        parent.resize(n + 1);
        iota(parent.begin(), parent.end(), 0);
        components = n;
    }

    int find(int i) {
        if (parent[i] == i) return i;
        return parent[i] = find(parent[i]);
    }

    void unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);

        if (root_i != root_j) {
            parent[root_i] = root_j;
            components--;
        } 
    }
};

vector<vector<string>> DBSCAN(const vector<string> &dna_set,
                                                int eps,
                                                int minpts,
                                                int &num_cores,
                                                int &num_borders,
                                                int &num_outliers,
                                                int &num_clusters) {
    vector<int> type(dna_set.size(), 0);
    vector<int> cores;
    for (int i = 0; i < dna_set.size(); i++) {
        int cnt = 1;
        for (int j = 0; j < dna_set.size(); j++) {
            if (j == i) continue;
            if (LevDist(dna_set[i], dna_set[j]) <= eps) {
                cnt++;
                if (cnt >= minpts) break;
            }
        }
        if (cnt >= minpts) {
            type[i] = 1;
            num_cores++;
            cores.push_back(i);
        }
    }

    for (int i = 0; i < dna_set.size(); i++) {
        if (type[i] == 1) continue;
        for (int j = 0; j < dna_set.size(); j++) {
            if (j == i || type[j] != 1) continue;
            if (LevDist(dna_set[i], dna_set[j]) <= eps) {
                type[i] = 2;
                num_borders++;
                break;
            }
        }
    }

    num_outliers = dna_set.size() - num_cores - num_borders;

    DSU dsu(num_cores);
    for (int i = 0; i < cores.size(); i++) {
        for (int j = i + 1; j < cores.size(); j++) {
            if (LevDist(dna_set[cores[i]], dna_set[cores[j]]) <= eps) {
                dsu.unite(i + 1, j + 1);
            }
        }
    }

    num_clusters = dsu.components;
    return vector<vector<string>>();
}

int MinEPS(const vector<string> &dna_set, int minpts) {
    int eps = 1;
    while (1) {
        int num_cores = 0, num_borders = 0, num_outliers = 0, num_clusters = 0;
        DBSCAN(dna_set, eps, minpts, num_cores, num_borders, num_outliers, num_clusters);

        if (num_outliers > 0) {
            eps++;
        } else {
            break;
        }
    }

    return eps;
}

int main () {
    // 2.1
    LoadData();

    // 2.2
    LevDist(DNAS[0], DNAS[1]);

    // 2.3
    vector<string> dna_set = RangeQuery(DNAS, DNAS[0], E);
    cout << dna_set.size() << endl;

    // 2.4
    int num_cores, num_borders, num_outliers, num_clusters;
    DBSCAN(DNAS, E, P, num_cores, num_borders, num_outliers, num_clusters);
    cout << num_cores << " " << num_borders << " " << num_outliers << " " << num_clusters << endl;

    // 2.5
    int min_eps = MinEPS(DNAS, P);
    cout << min_eps << endl;

    return 0;
}