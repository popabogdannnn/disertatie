#include "cost_matrix.h"

vector<vector<int>> compute_cost_matrix(vector<vector<int>> G) {
    int N = G.size();
    vector<vector<int>> c(N, vector<int>(N, 0));

    for(auto &v : G) {
        sort(v.begin(), v.end());
    }

    auto get_cost = [](vector<int> &a, vector<int> &b) {
        int ret = 0;
        int j = 0;
        for(int i = 0; i < a.size(); i++) {
            while(j < b.size() && b[j] < a[i]) {
                j++;
            }
            ret += j;
        }
        return ret;
    };

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            if(i != j) {
                c[i][j] = get_cost(G[i], G[j]);
            }
        }
    }

    return c;
}