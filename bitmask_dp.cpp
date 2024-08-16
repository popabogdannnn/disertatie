#include <iostream>
#include <climits>
#include "cost_matrix.h"

using namespace std;

const int INF = INT_MAX / 2;

int main() {
    int M, N, E;
    
    cin >> M >> N >> E;

    vector<vector<int>> G(N);
    vector<vector<int>> memo(1 << N, vector<int>(N, -1));
    vector<int> dp;

    fill(memo[0].begin(), memo[0].end(), 0);

    while(E--) {
        int x, y;
        cin >> x >> y;
        G[y].push_back(x);
    }

    vector<vector<int>> c = compute_cost_matrix(G);

    dp.resize(1 << N);

    for(int conf = 1; conf < (1 << N); conf++) {
        for(int i = 0; i < N; i++) {
            int p = __builtin_ctz(conf);
            memo[conf][i] = memo[conf ^ (1 << p)][i] + c[p][i];
        }
    }

    for(int conf = 1; conf < (1 << N); conf++) {
        dp[conf] = INF;
        for(int i = 0; i < N; i++) {
            if(conf & (1 << i)) {
                int curr_cost = dp[conf ^ (1 << i)] + memo[conf ^ (1 << i)][i];
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
                if(dp[conf ^ (1 << i)] + memo[conf ^ (1 << i)][i] < dp[conf ^ (1 << node)] + memo[conf ^ (1 << node)][node]) {
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