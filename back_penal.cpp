#include <bits/stdc++.h>

using namespace std;

const int N_MAX = 5000;

int M, N, E;
vector <int> G[N_MAX + 5];
int cnt[N_MAX + 5];

int get_perm_cost(vector <int> perm) {
    memset(cnt, 0, sizeof(cnt));
    int ret = 0;
    for(int i = 0; i < N; i++) {
        for(auto it : G[perm[i]]) {
            for(int x = it + 1; x < M; x++) {
                ret += cnt[x];
            }
        }
        for(auto it : G[perm[i]]) {
            cnt[it]++;
        }
    }
    return ret;
}

int main() {
    cin >> M >> N >> E;

    while(E--) {
        int x, y;
        cin >> x >> y;
        G[y].push_back(x);
    }

    vector <int> perm(N);
    iota(perm.begin(), perm.end(), 0);

    int ans = INT_MAX;
    vector <int> perm_ans;
    do {
        int curr = get_perm_cost(perm);
        if(curr < ans) {
            ans = curr;
            perm_ans = perm;
        }
       
    }while(next_permutation(perm.begin(), perm.end()));
    
    for(auto it : perm_ans) {
        cout << it + M + 1 << '\n';
    }

    return 0;
}