#include "cost_matrix.h"
#include <iostream>
#include <cassert>
#include <bitset>

using namespace std;

const int INF = 2e9;
const int N_MAX = 17000;
using u64 = uint_fast64_t;

vector<vector<int>> c;
struct DynamicBitset {
    vector<u64> data;
    DynamicBitset() = default;
    DynamicBitset(int N): data(N / 64 + (N % 64 > 0), 0) {}
    
    int operator[](int pos) {
        return (data[pos / 64] >> (pos % 64)) & 1; 
    }

    void operator|=(const DynamicBitset &oth) {
        for(int i = 0; i < oth.data.size() && i < data.size(); i++) {
            data[i] |= oth.data[i];
        }
    }

    void set(int pos) {
        data[pos / 64] |= (1ll << (pos % 64));
    }
};

struct Instance {
    int N;
    vector<vector<bool>> D;
    int crossings;
    int depth;

    Instance() = default;
    Instance(int _N): N(_N), D(_N, vector<bool>(_N, false)), crossings(0), depth(0) {
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
        ret.depth++;
        return ret;
    }

    void transitive_closure() {
        auto srt = top_sort();
        vector <DynamicBitset> b(N, DynamicBitset(N));
        for(int i = 0; i < srt.size(); i++) {
            for(int j = 0; j < i; j++) {
                if(D[srt[i]][srt[j]]) {
                    crossings = INF;
                    return;
                }
            }
        }
        for(int i = srt.size() - 1; i >= 0; i--) {
            for(int j = i + 1; j < srt.size(); j++) {
                if(D[srt[i]][srt[j]]) {
                    b[i] |= b[j];
                    b[i].set(srt[j]);
                }
            }
            for(int j = 0; j < N; j++) {
                if(D[srt[i]][j] == false && b[i][j]) {
                    crossings += c[srt[i]][j];
                    D[srt[i]][j] = true;
                }
            }
        }
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
    if(sol.crossings == 0) {
        return;
    }
    
    if(I.crossings > K || I.crossings >= sol.crossings) {
        return;
    }

    pair<int, int> p = I.find_choice();
    
    if(p.first == -1) {
        if(sol.crossings > I.crossings) {
            sol = I;
        }
        return;
    }

    if(c[p.first][p.second] > c[p.second][p.first]) {
        swap(p.first, p.second);
    }
    search(I.commit(p.first, p.second), K);
    search(I.commit(p.second, p.first), K);
}

vector<int> solve(int K, int N, Instance I) {
    if(K < 0) {
        return vector<int>();
    }
    
    sol.crossings = 2e9;

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            if(i != j) {
                if(c[i][j] > K) {
                    I.D[j][i] = true;
                }
            }
            
        }
    }

    I.transitive_closure();

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
    cerr << K << " " << sol.crossings << "\n";
    for(auto i : x) {
        cout << i + M + 1 << "\n";
    }
    return 0;
}