#include "cost_matrix.h"
#include <iostream>
#include <cassert>
#include <bitset>

using namespace std;

vector<vector<int>> c;

struct Instance {
    int N;
    vector<vector<bool>> D;
    int crossings;

    Instance() = default;
    Instance(int _N): N(_N), D(_N, vector<bool>(_N, false)), crossings(0) {
        for(int i = 0; i < N; i++) {
            D[i][i] = true;
        }
    }

    pair<int, int> find_choice() {
        for(int i = 0; i < N; i++) {
            for(int j = i + 1; j < N; j++) {
                if(D[i][j] == false && D[j][i] == false && c[i][j] != c[j][i]) {
                    return make_pair(i, j);
                }
            }
        }
        return make_pair(-1, -1);
    }

    Instance commit(int a, int b) {
        Instance ret = *this;
        ret.D[a][b] = true;
        ret.crossings += c[a][b];
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                if(ret.D[i][a] && ret.D[b][j] && ret.D[i][j] == false) {
                    ret.D[i][j] = true;
                    ret.crossings += c[i][j];
                }
            }
        }
        return ret;
    }

    vector<int> top_sort() {
        vector<bool> vis(N);
        vector<int> ret;

        function<void(int)> dfs;
        dfs = [&](int node) {
            vis[node] = true;
            for(int i = 0; i < N; i++) {
                if(D[node][i] && !vis[i]) {
                    dfs(i);
                }
            }
            ret.push_back(node);
        };

        for(int i = 0; i < N; i++) {
            if(!vis[i]) {
                dfs(i);
            }
        }
        reverse(ret.begin(), ret.end());
        return ret;
    }
};

Instance sol;
int states = 0;

void search(Instance I, int K) {
    if(I.crossings > K) {
        return;
    }

    pair<int, int> p = I.find_choice();
    
    if(p.first == -1) {
        if(sol.crossings > I.crossings) {
            sol = I;
        }
        return;
    }

    Instance I_a = I.commit(p.first, p.second);
    Instance I_b = I.commit(p.second, p.first);

    search(I_a, K);
    search(I_b, K);
}

vector<int> solve(int K, int N, Instance I) {
    if(K < 0) {
        return vector<int>();
    }
    
    sol.crossings = 2e9;
    search(I, K);

    if(sol.crossings == 2e9) {
        return vector<int>();
    }
    return sol.top_sort();
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
    int lb = 0;
    Instance I(N);
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            if(i != j) {
                if(c[i][j] == 0 && c[j][i] > 0) {
                    I.D[i][j] = true;
                }
            }
        }
    }

    for(int i = 0; i < N; i++) {
        for(int j = i + 1; j < N; j++) {
            int t = min(c[i][j], c[j][i]);
            c[i][j] -= t;
            c[j][i] -= t;
            lb += t;
        }
    }

    int K = 1;
    vector<int> x;
    while(x.empty()) {
        x = solve(K, N, I);
        K *= 2;
    }
    for(auto i : x) {
        cout << i + M + 1 << "\n";
    }
    return 0;
}