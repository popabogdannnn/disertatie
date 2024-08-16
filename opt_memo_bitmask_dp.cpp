#include <iostream>
#include <climits>
#include "cost_matrix.h"

using namespace std;

const int INF = INT_MAX / 2;

int main() {
    int M, N, E;
    
    cin >> M >> N >> E;

    int first_half = N / 2, second_half = N - N / 2;
    vector<vector<int>> G(N);
    vector<vector<int>> memo[] = {
        vector<vector<int>>(1 << first_half, vector<int>(N, -1)),
        vector<vector<int>>(1 << second_half, vector<int>(N, -1)),
    };

    vector<int> dp;

    while(E--) {
        int x, y;
        cin >> x >> y;
        G[y].push_back(x);
    }

    vector<vector<int>> c = compute_cost_matrix(G);

    dp.resize(1 << N);

    auto compute_memo = [](vector<vector<int>> &memo, int delta, int size, int N, vector<vector<int>> &c) {
        fill(memo[0].begin(), memo[0].end(), 0);
        for(int conf = 1; conf < (1 << size); conf++) {
            for(int i = 0; i < N; i++) {
                int p = __builtin_ctz(conf);
                memo[conf][i] = memo[conf ^ (1 << p)][i] + c[delta + p][i];
            }
        }
    };

    compute_memo(memo[0], 0, first_half, N, c);
    compute_memo(memo[1], first_half, second_half, N, c);

    auto F = [&](int conf, int i) {
        int curr_cost = dp[conf ^ (1 << i)] 
            + memo[0][(conf ^ (1 << i)) & ((1 << first_half) - 1)][i]
            + memo[1][(conf ^ (1 << i)) >> first_half][i];
        return curr_cost;
    };

    for(int conf = 1; conf < (1 << N); conf++) {
        dp[conf] = INF;
        for(int i = 0; i < N; i++) {
            if(conf & (1 << i)) {
                int curr_cost = F(conf, i);
                if(dp[conf] > curr_cost) {
                    dp[conf] = curr_cost;
                }
            }
        }
    }

    vector <int> ans;
    for(int conf = (1 << N) - 1; conf;) {
        int node = -1;
        for(int i = 0; i < N; i++) {
            if(conf & (1 << i)) {
                if(node == -1) {
                    node = i;
                }
                if(F(conf, i) < F(conf, node)) {
                    node = i;
                }
            }   
        }
        ans.push_back(node);
        conf ^= (1 << node);
    }
    reverse(ans.begin(), ans.end());
    for(auto it : ans) {
        cout << it + M + 1 << "\n";
    }
    return 0;
}