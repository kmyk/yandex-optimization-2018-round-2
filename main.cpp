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

chrono::high_resolution_clock::time_point clock_begin;

class xor_shift_128 {
public:
    typedef uint32_t result_type;
    xor_shift_128(uint32_t seed) {
        set_seed(seed);
    }
    void set_seed(uint32_t seed) {
        a = seed = 1812433253u * (seed ^ (seed >> 30));
        b = seed = 1812433253u * (seed ^ (seed >> 30)) + 1;
        c = seed = 1812433253u * (seed ^ (seed >> 30)) + 2;
        d = seed = 1812433253u * (seed ^ (seed >> 30)) + 3;
    }
    uint32_t operator() () {
        uint32_t t = (a ^ (a << 11));
        a = b; b = c; c = d;
        return d = (d ^ (d >> 19)) ^ (t ^ (t >> 8));
    }
    static constexpr uint32_t max() { return numeric_limits<result_type>::max(); }
    static constexpr uint32_t min() { return numeric_limits<result_type>::min(); }
private:
    uint32_t a, b, c, d;
};

template <class IntType>
class fast_uniform_int_distribution {
public:
    fast_uniform_int_distribution(IntType l, IntType r) : l(l), r(r) {}
    template <class RandomEngine> IntType operator() (RandomEngine & gen) { return (gen() & 0x7fffffff) % (r - l + 1) + l; }
private:
    IntType l, r;
};

struct site_t {
    vector<int> topics;
    int time_to_index; // to index
    int news_prob;  // probability of publishing news, in percents
    vector<int> links;
};

template <class RandomEngine>
double compute_score(int num_sites, int num_topics, int num_robots, int simu_duration, int obso_coeff, vector<site_t> const & sites, vector<int> const & freq, vector<vector<int> > const & sites_with_topic, vector<vector<int> > const & paths, RandomEngine & gen) {
    reversed_priority_queue<pair<int, int> > news_generation;  // (time, topic)
    REP (topic, num_topics) {
        news_generation.emplace(0, topic);
    }

    reversed_priority_queue<tuple<int, int, int> > index_finishes;  // (time, index of robot, index in path)
    REP (robot, num_robots) {
        if (paths[robot].empty()) continue;
        int site = paths[robot][0];
        index_finishes.emplace(sites[site].time_to_index, robot, 0);
    }
    if (index_finishes.empty()) return 0;

    double score = 0;
    vector<pair<int, int> > events;  // (topic, time) ; topic is -1 if already consumed
    vector<queue<int> > news_on_site(num_sites);

    for (int cur_time = 0; cur_time <= simu_duration; ) {
        // generate news
        while (news_generation.top().first == cur_time) {
            int topic = news_generation.top().second;
            news_generation.pop();
            events.emplace_back(topic, cur_time);
            bool news_appeared = false;
            while (not news_appeared) {
                for (int site : sites_with_topic[topic]) {
                    if (fast_uniform_int_distribution<int>(1, 100)(gen) <= sites[site].news_prob) {
                        news_on_site[site].push(events.size() - 1);
                        news_appeared = true;
                    }
                }
            }
            news_generation.emplace(cur_time + freq[topic], topic);
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

        // update time
        cur_time = min(news_generation.top().first, get<0>(index_finishes.top()));
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

    xor_shift_128 gen((random_device()()));
    auto get_score = [&](vector<vector<int> > const & paths, int simu_duration) {
        return compute_score(num_sites, num_topics, num_robots, simu_duration, obso_coeff, sites, freq, sites_with_topic, paths, gen);
    };

    constexpr int MAX_TIME_TO_INDEX = 1000;
    auto make_chain = [&](vector<bool> used_by_others) {
        for (int iteration = 0; ; ++ iteration) {
            vector<int> path;
            vector<bool> used = iteration < 100 ? used_by_others : vector<bool>(num_sites);
            while (true) {
                int i = fast_uniform_int_distribution<int>(0, num_sites - 1)(gen);
                if (used[i]) continue;
                path.push_back(i);
                break;
            }
            while (true) {
                int i = path.back();
                int sum_next_time = 0;
                for (int j : sites[i].links) if (not used[j]) {
                    sum_next_time += MAX_TIME_TO_INDEX / sites[j].time_to_index;
                }
                if (sum_next_time == 0) {
                    path.clear();
                    break;
                }
                int acc = fast_uniform_int_distribution<int>(0, sum_next_time - 1)(gen);
                int j = -1;
                for (int k : sites[i].links) if (not used[k]) {
                    acc -= MAX_TIME_TO_INDEX / sites[k].time_to_index;
                    if (acc < 0) {
                        j = k;
                        break;
                    }
                }
                if (j == path.front()) break;
                path.push_back(j);
                used[j] = true;
                if (path.size() > min(30, num_sites / num_robots)) {
                    path.clear();
                    break;
                }
            }
            if (not path.empty()) return path;
        }
    };
    auto get_used = [&](vector<vector<int> > paths) {
        vector<bool> used(num_sites);
        for (auto const & path : paths) {
            for (int i : path) {
                used[i] = true;
            }
        }
        return used;
    };

    vector<vector<int> > paths(num_robots);
    REP (robot, num_robots) {
        paths[robot] = make_chain(get_used(paths));
    }
    constexpr int MIN_SIMU_DURATION = 10000;
    double highscore = get_score(paths, MIN_SIMU_DURATION);
    int iteration = 0;
    for (; ; ++ iteration) {
        chrono::high_resolution_clock::time_point clock_end = chrono::high_resolution_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(clock_end - clock_begin).count() >= 1800) break;

        int robot = fast_uniform_int_distribution<int>(0, num_robots - 1)(gen);
        vector<int> path;
        paths[robot].swap(path);
        paths[robot] = make_chain(get_used(paths));
        double score = get_score(paths, MIN_SIMU_DURATION);
        if (highscore < score) {
            highscore = score;
        } else {
            paths[robot].swap(path);
        }
    }

    cerr << "iteration: " << iteration << endl;
#ifdef LOCAL
    cerr << "estimated score: " << get_score(paths, simu_duration) << endl;
#endif
    return paths;
}


int main() {
    clock_begin = chrono::high_resolution_clock::now();

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
