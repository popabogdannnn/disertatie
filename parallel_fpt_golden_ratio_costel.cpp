#include "cost_matrix.h"
#include <condition_variable>
#include <iostream>
#include <cassert>
#include <bitset>
#include <thread>
#include <mutex>
#include <stack>
#include <utility>
#include <atomic>

using namespace std;

const int INF = 2e9;
const int PROCESSORS = thread::hardware_concurrency();
// const int PROCESSORS = 4;

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
mutex sol_mutex;
struct search_call {
    Instance I;
    int K;
    int state;
};


void search(Instance& I, int K, int state);

class ThreadPool { 
public: 
    void start() {
        threads.clear();
        threads.resize(PROCESSORS);
        for (int i = 0; i < PROCESSORS; i++) { 
            threads[i] = thread([this, i] { 
                while (true) {
                    // cout << "Thread " << i << " spinlocking" << endl;
                    search_call task;
                    {                    
                        unique_lock<mutex> lock(stack_mutex); 
                        // unique_lock<mutex> working_lock(working_mutex);
                        // cout << "Thread " << i << " got the lock in getting task" << endl;


                        if (S.empty() && working > 0){
                            continue;
                        }
                        // cv.wait(lock, [this] { 
                        //     return !S.empty() || working == 0; 
                        // }); 

                        {
                            // unique_lock<mutex> working_lock(working_mutex);
                            if (working == 0 && S.empty()) { 
                                // cout << "Thread " << i << " stopped" << endl;
                                return; 
                            }
                        }

                        task = S.top(); 
                        
                        if(S.top().state == 0) {
                            S.top().state = 1;
                        }
                        else {
                            S.pop();
                            // cv.notify_one(); 
                        }

                        {
                            unique_lock<mutex> working_lock(working_mutex);
                            working++;
                        }
                        // working.fetch_add(1, memory_order_seq_cst);
                    }
                    // cout << "Thread " << i << " runs something" << endl;
                    search(task.I, task.K, task.state); 
                    // cout << "Thread " << i << " finished running something" << endl;

                    {
                        // unique_lock<mutex> lock(stack_mutex);
                        unique_lock<mutex> working_lock(working_mutex);
                        working--;
                        // working.fetch_add(-1, memory_order_seq_cst);
                    }

                    // cout << "Thread " << i << " updated working" << endl;
                } 
            }); 
        }
    }

    void wait() {
        for(int i = 0; i < PROCESSORS; i++) {
            // cout << "waiting for thread " << i << endl;
            threads[i].join();
        }
    }
  
    void enqueue(search_call&& call) 
    { 
        { 
            unique_lock<mutex> lock(stack_mutex);
            // unique_lock<mutex> working_lock(working_mutex);

            S.emplace(move(call)); 
        } 
        cv.notify_one(); 
    } 
  
private: 
    
    vector<thread> threads; 
  
    stack<search_call> S; 
    condition_variable cv;
  
    mutex stack_mutex; 
    mutex working_mutex;
    int working = 0;
}; 
ThreadPool thread_pool;

vector<int> solve(int K, int N, Instance I) {
    if(K < 0) {
        return vector<int>();
    }
    
    sol.crossings = 2e9;

    thread_pool.enqueue({I, K, 0});
    thread_pool.start();
    thread_pool.wait();

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
        // cout << "solve with k = " << k << " finished" << endl;
        K *= 2;
    }
    cerr << K << " " << sol.crossings << "\n";
    for(auto i : x) {
        cout << i + M + 1 << "\n";
    }
    return 0;
}

void search(Instance& I, int K, int state) {
    {
        if(I.crossings > K || I.crossings >= sol.crossings) {
            return;
        }
    }

    pair<int, int> p = I.find_choice();
    
    if(p.first == -1) {
        if(sol.crossings > I.crossings) {
            unique_lock<mutex> lock(sol_mutex);
            if(sol.crossings > I.crossings) {
                sol = I;
            }
            return;
        }
    }

    if(c[p.first][p.second] > c[p.second][p.first]) {
        swap(p.first, p.second);
    }

    if(state == 0) {
        thread_pool.enqueue({move(I.commit(p.first, p.second)), K, 0});
    } 
    else {
        thread_pool.enqueue({move(I.commit(p.second, p.first)), K, 0});
    }   

}