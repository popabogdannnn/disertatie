#include <iostream>
#include <climits>
#include "cost_matrix.h"
#include <thread>
#include <future>
#include <cassert>

using namespace std;

using i64 = int64_t;

const int INF = INT_MAX / 2;
const int PROCESSORS = thread::hardware_concurrency();

vector<int> compute_node_memo(int node, int N, const vector<vector<int>> &c) {
    vector<int> memo(1 << N);
    for(int conf = 1; conf < (1 << N); conf++) {
        int p = __builtin_ctz(conf);
        memo[conf] = memo[conf ^ 1 << p] + c[p][node];
    }
    return memo;
}

void compute_memo(int N, vector<vector<int>> &memo, vector<vector<int>> &c) {
    memo.resize(N);
    vector<future<vector<int>>> threads(PROCESSORS);
    for(int i = 0; i < N; i += PROCESSORS) {
        for(int j = 0; i + j < N && j < PROCESSORS; j++) {
            threads[j] = async(launch::async, compute_node_memo, i + j, N, c);
        }
        for(int j = 0; i + j < N && j < PROCESSORS; j++) {
            memo[i + j] = threads[j].get();
        }
    }
}

vector<vector<int>> compute_comb(int N) {
    vector<vector<int>> comb(N + 5);
    for(int i = 0; i <= N; i++) {
        comb[i].resize(N + 5);
        comb[i][0] = 1;
        for(int j = 1; j <= i; j++) {
            comb[i][j] = comb[i - 1][j] + comb[i - 1][j - 1];
        }
    }
    return comb;
}

vector<int> dp;
vector<vector<int>> memo;
vector<vector<int>> comb;
vector<int> T;
vector<vector<int>> c;

int search_upper_bound(int N, int layer, int add) {
    int ret = 0;
    for(int bit = N - 1; bit >= 0 && layer > 0; bit--) {
        if(comb[bit][layer] < add) {
            add -= comb[bit][layer];
            layer--;
            ret += 1 << bit;
        }
    }
    return ret;
}

void compute_layer_range_dp(int N, int layer, int a, int b) {
    for(int i = a; i <= b; i++) {
        int conf = search_upper_bound(N, layer, i);
        dp[conf] = INF;
        for(int i = 0; i < N; i++) {
            if(conf & (1 << i)) {
                int curr_cost = dp[conf ^ (1 << i)] + memo[i][conf ^ (1 << i)];
                if(dp[conf] > curr_cost) {
                    dp[conf] = curr_cost;
                    T[conf] = conf ^ (1 << i);
                }
            }
        }
    }
}

int main() {
    int M, N, E;
    
    cin >> M >> N >> E;

    vector<vector<int>> G(N);

    while(E--) {
        int x, y;
        cin >> x >> y;
        G[y].push_back(x);
    }

    c = compute_cost_matrix(G);

    dp.resize(1 << N);
    T.resize(1 << N);

    compute_memo(N, memo, c);
    comb = compute_comb(N);

    for(int layer = 1; layer <= N; layer++) {
        vector<i64> work(PROCESSORS);
        for(int i = 0; i < PROCESSORS; i++) {
            work[i] = comb[N][layer] / PROCESSORS + (i < comb[N][layer] % PROCESSORS);
        }
        vector<thread> threads(PROCESSORS);
        i64 add = 0, last = 0;
        for(int i = 0; i < PROCESSORS; i++) {
            add += work[i];
            threads[i] = thread(compute_layer_range_dp, N, layer, last + 1, add);
            last = add;
        }

        for(int i = 0; i < PROCESSORS; i++) {
            threads[i].join();
        }
    }
    vector <int> ans;
    for(int conf = (1 << N) - 1; conf; conf = T[conf]) {
        int node = __builtin_ctz(conf ^ T[conf]);
        ans.push_back(node);
    }
    reverse(ans.begin(), ans.end());
    for(auto it : ans) {
        cout << it + M + 1 << "\n";
    }
    return 0;
}