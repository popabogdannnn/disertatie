#include "cost_matrix.h"
#include <iostream>
#include <cassert>
#include <bitset>
#include <map>
#include <algorithm>
#include <set>
#include <thread>

using namespace std;

const int PROCESSORS = thread::hardware_concurrency();
const int INF = 2e9;
const int DP_MAX = 30;
vector <int> dp;
vector <vector<int>> comb;
 vector<vector<int>> memo[] = {
    vector<vector<int>>(DP_MAX, vector<int>(1 << (DP_MAX / 2), 0)),
    vector<vector<int>>(DP_MAX, vector<int>(1 << (DP_MAX / 2), 0))
};

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

vector<pair<int, int>> compute_I(int N, vector<vector<int>>&G) {
    vector<pair<int, int>> I(N);
    for(int i = 0; i < N; i++) {
        if(!G[i].empty()) {
            I[i] = make_pair(*G[i].begin(), *G[i].rbegin());
        }
    }
    return I;
}


vector <pair<int, int>> compute_J(int N, int M, vector<vector<int>>&G) {
    auto I = compute_I(N, G);
    struct Triplet {
        int y, x, t;  
    };
    vector<Triplet>P;
    vector<vector<Triplet>> s(M + 1); 
    for(int i = 0; i < N; i++) {
        s[I[i].first + 1].push_back({i, I[i].first, 0});
        s[I[i].second + 1].push_back({i, I[i].second, 1});
    }
    for(int x = 0; x <= M; x++) {
        copy_if(s[x].begin(), s[x].end(), back_inserter(P), [&](Triplet a) {
            return G[a.y].size() > 1 && a.t == 1;
        });
        copy_if(s[x].begin(), s[x].end(), back_inserter(P), [&](Triplet a) {
            return G[a.y].size() == 1;
        });
        copy_if(s[x].begin(), s[x].end(), back_inserter(P), [&](Triplet a) {
            return G[a.y].size() > 1 && a.t == 0;
        });
    }

    vector<pair<int, int>> J(N);
    
    for(int i = 0; i < P.size(); i++) {
        if(P[i].t == 0) {
            J[P[i].y].first = i + 1;
        }
        else {
            J[P[i].y].second = i + 1;
        }
    }
    return J;
}

void compute_layer_range_dp(int N, int layer, int a, int b) {
    int first_half = N / 2, second_half = N - N / 2;
    for(int i = a; i <= b; i++) {
        int conf = search_upper_bound(N, layer, i);
        dp[conf] = INF;
        for(int i = 0; i < N; i++) {
            if(conf & (1 << i)) {
                int curr_cost = dp[conf ^ (1 << i)] 
                    + memo[0][i][(conf ^ (1 << i)) & ((1 << first_half) - 1)]
                    + memo[1][i][(conf ^ (1 << i)) >> first_half];
                if(dp[conf] > curr_cost) {
                    dp[conf] = curr_cost;
                }
            }
        }
    }
}

vector <int> compute_dp(set<int>active, vector<vector<int>>&c, vector<int>&clt, int until) {
    vector<int> nodes(active.begin(), active.end());
    int N = nodes.size();
    assert(N <= DP_MAX);
    dp.resize(1 << N);

    int first_half = N / 2, second_half = N - N / 2;
    for(int i = 0; i < N; i++) {
        memo[0][i][0] = clt[nodes[i]];
    }
    
    auto compute_memo = [](vector<vector<int>> &memo, int delta, int size, int N, vector<vector<int>> &c, vector<int>&nodes) {
        fill(memo[0].begin(), memo[0].end(), 0);
        for(int conf = 1; conf < (1 << size); conf++) {
            for(int i = 0; i < N; i++) {
                int p = __builtin_ctz(conf);
                memo[i][conf] = memo[i][conf ^ (1 << p)] + c[nodes[delta + p]][nodes[i]];
            }
        }
    };

    compute_memo(memo[0], 0, first_half, N, c, nodes);
    compute_memo(memo[1], first_half, second_half, N, c, nodes);

    auto F = [&](int conf, int i) {
        int curr_cost = dp[conf ^ (1 << i)] 
            + memo[0][i][(conf ^ (1 << i)) & ((1 << first_half) - 1)]
            + memo[1][i][(conf ^ (1 << i)) >> first_half];
        return curr_cost;
    };

    dp[0] = 0;
    for(int layer = 1; layer <= N; layer++) {
        vector<int> work(PROCESSORS);
        for(int i = 0; i < PROCESSORS; i++) {
            work[i] = comb[N][layer] / PROCESSORS + (i < comb[N][layer] % PROCESSORS);
        }
        vector<thread> threads(PROCESSORS);
        int add = 0, last = 0;
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
        ans.push_back(nodes[node]);
        conf ^= (1 << node);
        if(nodes[node] == until) {
            break;
        }
    }
    return ans;
}

vector<int> solve(int K, int N, int M, int E, vector<vector<int>>&G, vector<vector<int>> &c) {
    if(E >= N + K) {
        return vector<int>();
    }
    vector<int> ret;

    auto J = compute_J(N, M, G);
    vector<pair<int, int>> event(2 * N + 1);

    for(int i = 0; i < N; i++) {
        event[J[i].first] = make_pair(i, 1);
        event[J[i].second] = make_pair(i, -1);
    }

    vector<int> clt(N);

    int s = 0;
    int ans = 0;
    long long t = 0;

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            clt[i] += c[j][i];
        }
    }

    set <int> active;
 
    for(int i = 2 * N; i >= 1; i--) {
        
        if(event[i].second == -1) {
            active.insert(event[i].first);
            for(int j = 0; j < N; j++) {
                clt[j] -= c[event[i].first][j];
            }
        }
        else if(active.count(event[i].first)) {
            auto get_perm = compute_dp(active, c, clt, event[i].first);
            ret.insert(ret.end(), get_perm.begin(), get_perm.end());
            for(auto x : get_perm) {
                active.erase(x);
            }
        }
    }
    reverse(ret.begin(), ret.end());
    return ret;
}

int main() {
    int M, N, E;
    cin >> M >> N >> E;
    vector<int> x;
    vector<vector<int>> G(N);

    comb = compute_comb(DP_MAX + 5);

    while(E--) {
        int x, y;
        cin >> x >> y;
        G[y].push_back(x);
    }

    vector<vector<int>> c = compute_cost_matrix(G);

    for(int i = 0; i < N; i++) {
        if(G[i].empty()) {
            G[i].push_back(-1);
        }
        sort(G[i].begin(), G[i].end());
    }
   
    x = solve(1e9, N, M, E, G, c);
    for(auto i : x) {
        cout << i + M + 1 << "\n";
    }
    return 0;
}