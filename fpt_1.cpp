#include "cost_matrix.h"
#include <iostream>

using namespace std;

struct Instance {
    vector<int> nodes;
    vector<vector<bool>> mat;

    Instance RR1(vector<vector<int>> &c) {
        Instance ret = *this;
        for(int a : ret.nodes) {
            for(int b : ret.nodes) {
                if(a != b) {
                    if(c[a][b])
                }
            }
        }
        return ret;
    };
};

int main() {
    int M, N, E;
    cin >> M >> N >> E;
    int K;
    cin >> K;
    
    vector<vector<int>> G(N);
    while(E--) {
        int x, y;
        cin >> x >> y;
        G[y].push_back(x);
    }
    
    vector<vector<int>> c = compute_cost_matrix(G);

    return 0;
}