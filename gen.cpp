#include <bits/stdc++.h>

using namespace std;

using u32 = uint32_t;

mt19937 get_rng() {
    seed_seq seq {
            (uint64_t) chrono::duration_cast<chrono::nanoseconds>(
                    chrono::high_resolution_clock::now().time_since_epoch()).count(),
            (uint64_t) __builtin_ia32_rdtsc(),
            (uint64_t) (uintptr_t) unique_ptr<char>(new char).get()
    };
    return mt19937(seq);
}

auto rng = get_rng();

void random(int N, int M, int E) {
    vector<pair<int, int>> edges;
    for(int i = 1; i <= N; i++) {
        for(int j = N + 1; j <= N + M; j++) {
            edges.push_back({i, j});
        }
    }

    shuffle(edges.begin(), edges.end(), rng);

    edges.resize(E);

    cout << "p ocr " << N << " " << M << " " << E << "\n";
    for(auto e : edges) {
        cout << e.first << " " << e.second << "\n";
    }
}

int main(int argc, char *argv[]) {
    int N, M, E;
    N = atoi(argv[1]);
    M = atoi(argv[2]);
    E = atoi(argv[3]);

    random(N, M, E);
    return 0;
}