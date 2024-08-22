#include "cost_matrix.h"
#include <iostream>
#include <cassert>
#include <bitset>

using namespace std;

const int INF = 2e9;

vector<vector<int>> c;
vector<vector<bool>> D;
vector<pair<int, int>> edges;

void reset(vector<vector<bool>> &D, vector<pair<int, int>> &edges, int stack_size) {
    while(edges.size() > stack_size) {
        D[edges.back().first][edges.back().second] = false;
        edges.pop_back();
    }
}

struct Instance {
    int N;
    int stack_size;
    int crossings;

    Instance() = default;
    Instance(int _N): N(_N), crossings(0), stack_size(0) {
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
        D[a][b] = true;
        ret.crossings += c[a][b];
        edges.push_back(make_pair(a, b));
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                if(D[i][a] && D[b][j] && D[i][j] == false) {
                    D[i][j] = true;
                    edges.push_back(make_pair(i, j));
                    ret.crossings += c[i][j];
                }
            }
        }
        ret.stack_size = edges.size();
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
vector<int> sol_top_sort;
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
            sol_top_sort = I.top_sort();
        }
        return;
    }

    if(c[p.first][p.second] > c[p.second][p.first]) {
        swap(p.first, p.second);
    }
    search(I.commit(p.first, p.second), K);
    reset(D, edges, I.stack_size);
    search(I.commit(p.second, p.first), K);
    reset(D, edges, I.stack_size);
    
}

vector<int> solve(int K, int N, Instance I) {
    if(K < 0) {
        return vector<int>();
    }
    
    sol.crossings = INF;
    search(I, K);

    if(sol.crossings == INF) {
        return vector<int>();
    }
    return sol_top_sort;
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
    D.resize(N, vector<bool>(N, false));
    int lb = 0;
    Instance I(N);
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            if(i != j) {
                if(c[i][j] == 0 && c[j][i] > 0) {
                    D[i][j] = true;
                    edges.push_back(make_pair(i, j));
                }
            }
        }
        D[i][i] = true;
        edges.push_back(make_pair(i, i));
    }
    I.stack_size = edges.size();

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