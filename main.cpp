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
    int time_to_index; // to index
    int news_prob;  // probability of publishing news, in percents
    vector<int> links;
};

template <class RandomEngine>
double compute_score(int num_sites, int num_topics, int num_robots, int simu_duration, int obso_coeff, vector<site_t> const & sites, vector<int> const & freq, vector<vector<int> > const & sites_with_topic, vector<vector<int> > const & paths, RandomEngine & gen) {
    double score = 0;
    vector<pair<int, int> > events;  // (topic, time) ; topic is -1 if already consumed
    vector<queue<int> > news_on_site(num_sites);
    reversed_priority_queue<tuple<int, int, int> > index_finishes;  // (time, index of robot, index in path)
    REP (robot, num_robots) {
        if (paths[robot].empty()) continue;
        int site = paths[robot][0];
        index_finishes.emplace(sites[site].time_to_index, robot, 0);
    }
    if (index_finishes.empty()) return 0;
    REP (cur_time, simu_duration + 1) {
        // generate news
        REP (topic, num_topics) if (cur_time % freq[topic] == 0) {
            events.emplace_back(topic, cur_time);
            bool news_appeared = false;
            while (not news_appeared) {
                for (int site : sites_with_topic[topic]) {
                    if (uniform_int_distribution<int>(1, 100)(gen) <= sites[site].news_prob) {
                        news_on_site[site].push(events.size() - 1);
                        news_appeared = true;
                    }
                }
            }
        }
        // indexing
        while (get<0>(index_finishes.top()) == cur_time) {
            int robot = get<1>(index_finishes.top());
            int index = get<2>(index_finishes.top());
            index_finishes.pop();
            int site = paths[robot][index];
            int index_begin_time = cur_time - sites[site].time_to_index;
            while (not news_on_site[site].empty()) {
                int ev = news_on_site[site].front();
                int & ev_topic = events[ev].first;  // reference
                int ev_time = events[ev].second;
                if (index_begin_time < ev_time) {
                    break;  // ignore newer events than index_begin_time
                }
                news_on_site[site].pop();
                if (ev_topic != -1) {
                    score += max(0, obso_coeff * freq[ev_topic] - (cur_time - ev_time));
                    ev_topic = -1;  // set consumed
                }
            }
            int next_index = (index + 1) % paths[robot].size();
            int next_site = paths[robot][next_index];
            index_finishes.emplace(cur_time + sites[next_site].time_to_index, robot, next_index);
        }
    }
    return 1e4 * score / obso_coeff / num_topics / simu_duration;
}

vector<vector<int> > solve(int num_sites, int num_topics, int num_robots, int simu_duration, int obso_coeff, vector<site_t> const & sites, vector<int> const & freq) {
    vector<vector<int> > sites_with_topic(num_topics);
    REP (i, num_sites) {
        for (int j : sites[i].topics) {
            sites_with_topic[j].push_back(i);
        }
    }

    mt19937_64 gen((random_device()()));
    vector<vector<int> > paths(num_robots);
    auto get_score = [&]() {
        return compute_score(num_sites, num_topics, num_robots, simu_duration, obso_coeff, sites, freq, sites_with_topic, paths, gen);
    };

    REP (i, num_robots) {
        while (true) {
            int front = uniform_int_distribution<int>(0, num_sites - 1)(gen);
            paths[i].push_back(front);
            while (true) {
                int back = paths[i].back();
                int j = uniform_int_distribution<int>(0, sites[back].links.size() - 1)(gen);
                int next = sites[back].links[j];
                if (next == front) break;
                paths[i].push_back(next);
            }
            if (paths[i].size() < num_sites) break;
            paths[i].clear();
        }
    }

    cerr << "estimated score: " << get_score() << endl;
    return paths;
}

int main() {
    // input
    int num_sites; scanf("%d", &num_sites);
    int num_topics; scanf("%d", &num_topics);
    int num_robots; scanf("%d", &num_robots);
    int simu_duration; scanf("%d", &simu_duration);
    int obso_coeff; scanf("%d", &obso_coeff);
    vector<site_t> sites(num_sites);
    REP (i, num_sites) {
        int q_i; scanf("%d", &q_i);
        sites[i].topics.resize(q_i);
        REP (j, q_i) {
            scanf("%d", &sites[i].topics[j]);
            -- sites[i].topics[j];
        }
        scanf("%d", &sites[i].time_to_index);
        scanf("%d", &sites[i].news_prob);
        int o_i; scanf("%d", &o_i);
        sites[i].links.resize(o_i);
        REP (j, o_i) {
            scanf("%d", &sites[i].links[j]);
            -- sites[i].links[j];
        }
    }
    vector<int> freq(num_topics);
    REP (i, num_topics) {
        scanf("%d", &freq[i]);
    }

    // debug info
    cerr << "number of sites:  " << num_sites << endl;
    cerr << "number of topics: " << num_topics << endl;
    cerr << "number of robots: " << num_robots << endl;
    cerr << "duration of simulation: " << simu_duration << endl;
    cerr << "coefficient of obsolescence: " << obso_coeff << endl;

    // solve
    vector<vector<int> > answer = solve(num_sites, num_topics, num_robots, simu_duration, obso_coeff, sites, freq);

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
