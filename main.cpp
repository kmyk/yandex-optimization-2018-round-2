#include <bits/stdc++.h>
#define REP(i, n) for (int i = 0; (i) < int(n); ++ (i))
#define REP3(i, m, n) for (int i = (m); (i) < int(n); ++ (i))
#define REP_R(i, n) for (int i = int(n) - 1; (i) >= 0; -- (i))
#define REP3R(i, m, n) for (int i = int(n) - 1; (i) >= int(m); -- (i))
#define ALL(x) begin(x), end(x)
#define dump(x) cerr << #x " = " << x << endl
#define unittest_name_helper(counter) unittest_ ## counter
#define unittest_name(counter) unittest_name_helper(counter)
#define unittest __attribute__((constructor)) void unittest_name(__COUNTER__) ()
using ll = long long;
using namespace std;
template <class T> using reversed_priority_queue = priority_queue<T, vector<T>, greater<T> >;
template <class T> inline void chmax(T & a, T const & b) { a = max(a, b); }
template <class T> inline void chmin(T & a, T const & b) { a = min(a, b); }
template <typename X, typename T> auto vectors(X x, T a) { return vector<T>(x, a); }
template <typename X, typename Y, typename Z, typename... Zs> auto vectors(X x, Y y, Z z, Zs... zs) { auto cont = vectors(y, z, zs...); return vector<decltype(cont)>(x, cont); }
template <typename T> ostream & operator << (ostream & out, vector<T> const & xs) { REP (i, int(xs.size()) - 1) out << xs[i] << ' '; if (not xs.empty()) out << xs.back(); return out; }

struct site_t {
    vector<int> topics;
    int time; // to index
    int prob;  // probability of publishing news, in percents
    vector<int> links;
};

vector<vector<int> > solve(int num_sites, int num_topics, int num_robots, int duration, int obso_coeff, vector<site_t> const & sites, vector<int> const & frequency) {
    mt19937_64 gen((random_device()()));
    vector<vector<int> > path(num_robots);
    REP (i, num_robots) {
        while (true) {
            int front = uniform_int_distribution<int>(0, num_sites - 1)(gen);
            path[i].push_back(front);
            while (true) {
                int back = path[i].back();
                int j = uniform_int_distribution<int>(0, sites[back].links.size() - 1)(gen);
                int next = sites[back].links[j];
                if (next == front) break;
                path[i].push_back(next);
            }
            if (path[i].size() < num_sites) break;
            path[i].clear();
        }
    }
    return path;
}

int main() {
    // input
    int num_sites; scanf("%d", &num_sites);
    int num_topics; scanf("%d", &num_topics);
    int num_robots; scanf("%d", &num_robots);
    int duration; scanf("%d", &duration);
    int obso_coeff; scanf("%d", &obso_coeff);
    vector<site_t> sites(num_sites);
    REP (i, num_sites) {
        int q_i; scanf("%d", &q_i);
        sites[i].topics.resize(q_i);
        REP (j, q_i) {
            scanf("%d", &sites[i].topics[j]);
            -- sites[i].topics[j];
        }
        scanf("%d", &sites[i].time);
        scanf("%d", &sites[i].prob);
        int o_i; scanf("%d", &o_i);
        sites[i].links.resize(o_i);
        REP (j, o_i) {
            scanf("%d", &sites[i].links[j]);
            -- sites[i].links[j];
        }
    }
    vector<int> frequency(num_topics);
    REP (i, num_topics) {
        scanf("%d", &frequency[i]);
    }

    // debug info
    cerr << "number of sites:  " << num_sites << endl;
    cerr << "number of topics: " << num_topics << endl;
    cerr << "number of robots: " << num_robots << endl;
    cerr << "duration of simulation: " << duration << endl;
    cerr << "coefficient of obsolescence: " << obso_coeff << endl;

    // solve
    vector<vector<int> > answer = solve(num_sites, num_topics, num_robots, duration, obso_coeff, sites, frequency);

    // output
    assert (answer.size() == num_robots);
    REP (i, num_robots) {
        cout << answer[i].size();
        REP (j, answer[i].size()) {
            cout << ' ' << answer[i][j] + 1;
        }
        cout << endl;
    }
    return 0;
}
