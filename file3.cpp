#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

class Solution1 {
    std::vector<int> parent;
    std::vector<uint8_t> rank;
    std::vector<int> prev_lbl, cur_lbl;

    int find(int x) noexcept {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }
    bool unite(int a, int b) noexcept {
        a = find(a);
        b = find(b);
        if (a == b)
            return false;
        if (rank[a] < rank[b])
            std::swap(a, b);
        parent[b] = a;
        rank[a] += (rank[a] == rank[b]);
        return true;
    }

  public:
    int numIslands(std::vector<uint32_t>& grid, int m, int n) {
        if (m == 0 || n == 0)
            return 0;
        int max_labels = m * n + 2;
        if ((int)parent.size() < max_labels) {
            parent.resize(max_labels);
            rank.resize(max_labels, 0);
        }
        if ((int)prev_lbl.size() < m) {
            prev_lbl.resize(m);
            cur_lbl.resize(m);
        }
        parent[0] = 0;
        rank[0] = 0;
        std::memset(prev_lbl.data(), 0, m * sizeof(int));
        int next_label = 1, islands = 0;

        for (int r = 0; r < n; ++r) {
            std::memset(cur_lbl.data(), 0, m * sizeof(int));
            const uint32_t* row = &grid[r * m];
            for (int c = 0; c < m; ++c) {
                if (row[c] == 0)
                    continue;
                int left = (c > 0) ? cur_lbl[c - 1] : 0;
                int above = prev_lbl[c];
                if (!left && !above) {
                    parent[next_label] = next_label;
                    rank[next_label] = 0;
                    cur_lbl[c] = next_label++;
                    ++islands;
                } else if (left && !above) {
                    cur_lbl[c] = find(left);
                } else if (!left) {
                    cur_lbl[c] = find(above);
                } else {
                    islands -= unite(left, above);
                    cur_lbl[c] = find(left);
                }
            }
            std::swap(prev_lbl, cur_lbl);
        }
        return islands;
    }
};

class Solution2 {
    std::vector<uint32_t> g;
    std::vector<int> stack;

  public:
    int numIslands(std::vector<uint32_t>& grid, int m, int n) {
        if (m == 0 || n == 0)
            return 0;
        int total = m * n;
        if ((int)g.size() < total)
            g.resize(total);
        std::memcpy(g.data(), grid.data(), total * sizeof(uint32_t));
        stack.clear();
        stack.reserve(std::max(64, total / 8));
        int islands = 0;

        for (int i = 0; i < total; ++i) {
            if (g[i] == 0)
                continue;
            ++islands;
            g[i] = 0;
            stack.push_back(i);
            while (!stack.empty()) {
                int idx = stack.back();
                stack.pop_back();
                int r = idx / m, c = idx % m;
                if (r > 0 && g[idx - m]) {
                    g[idx - m] = 0;
                    stack.push_back(idx - m);
                }
                if (r < n - 1 && g[idx + m]) {
                    g[idx + m] = 0;
                    stack.push_back(idx + m);
                }
                if (c > 0 && g[idx - 1]) {
                    g[idx - 1] = 0;
                    stack.push_back(idx - 1);
                }
                if (c < m - 1 && g[idx + 1]) {
                    g[idx + 1] = 0;
                    stack.push_back(idx + 1);
                }
            }
        }
        return islands;
    }
};

class Solution3 {
    std::vector<int> parent;
    std::vector<uint8_t> rank;
    int find(int x) noexcept {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

  public:
    int numIslands(std::vector<uint32_t>& grid, int m, int n) {
        if (m == 0 || n == 0)
            return 0;
        int total = m * n;
        if ((int)parent.size() < total) {
            parent.resize(total);
            rank.resize(total, 0);
        }
        std::iota(parent.begin(), parent.begin() + total, 0);
        std::fill(rank.begin(), rank.begin() + total, 0);
        int islands = 0;

        for (int i = 0; i < total; ++i) {
            if (grid[i] == 0)
                continue;
            ++islands;
            int r = i / m, c = i % m;
            if (c > 0 && grid[i - 1]) {
                int a = find(i), b = find(i - 1);
                if (a != b) {
                    if (rank[a] < rank[b])
                        std::swap(a, b);
                    parent[b] = a;
                    rank[a] += (rank[a] == rank[b]);
                    --islands;
                }
            }
            if (r > 0 && grid[i - m]) {
                int a = find(i), b = find(i - m);
                if (a != b) {
                    if (rank[a] < rank[b])
                        std::swap(a, b);
                    parent[b] = a;
                    rank[a] += (rank[a] == rank[b]);
                    --islands;
                }
            }
        }
        return islands;
    }
};

class Solution4 {
  public:
    Solution4() { nthreads_ = std::max(1u, std::min(16u, std::thread::hardware_concurrency())); }

    int numIslands(std::vector<uint32_t>& grid, int m, int n) {
        if (m == 0 || n == 0)
            return 0;

        int T = std::min(nthreads_, n);
        if (T < 1)
            T = 1;
        int rows_per_thread = (n + T - 1) / T;

        int max_labels_per_thread = rows_per_thread * m + 2;
        int total_labels = T * max_labels_per_thread;

        if ((int)parent_.size() < total_labels) {
            parent_.resize(total_labels);
            rank_.resize(total_labels, 0);
        }

        workers_.resize(T);
        threads_.clear();

        for (int t = 0; t < T; ++t) {
            int r0 = t * rows_per_thread;
            int r1 = std::min(n, r0 + rows_per_thread);
            int label_base = t * max_labels_per_thread;
            threads_.emplace_back(&Solution4::process_strip, this, t, std::ref(grid), m, r0, r1, label_base,
                                  max_labels_per_thread);
        }
        for (auto& th : threads_)
            th.join();

        int total_islands = 0;
        for (const auto& w : workers_)
            total_islands += w.local_count;

        for (int t = 0; t + 1 < T; ++t) {
            const auto& w_curr = workers_[t];
            const auto& w_next = workers_[t + 1];
            const int* last_row = w_curr.last_row_lbl.data();
            const int* first_row = w_next.first_row_lbl.data();

            for (int c = 0; c < m; ++c) {
                int la = last_row[c];
                int lb = first_row[c];
                if (la == 0 || lb == 0)
                    continue;

                int a = find_global(la);
                int b = find_global(lb);
                if (a != b) {
                    if (rank_[a] < rank_[b])
                        std::swap(a, b);
                    parent_[b] = a;
                    if (rank_[a] == rank_[b])
                        ++rank_[a];
                    --total_islands;
                }
            }
        }

        return total_islands;
    }

  private:
    struct Worker {
        std::vector<int> first_row_lbl, last_row_lbl;
        int local_count = 0;
    };

    int nthreads_;
    std::vector<Worker> workers_;
    std::vector<std::thread> threads_;

    std::vector<int> parent_;
    std::vector<uint8_t> rank_;

    void process_strip(int id, std::vector<uint32_t>& grid, int m, int r0, int r1, int label_base, int max_labels) {
        Worker& w = workers_[id];
        std::vector<int> prev_lbl(m, 0), cur_lbl(m, 0);
        if ((int)w.first_row_lbl.size() < m) {
            w.first_row_lbl.resize(m);
            w.last_row_lbl.resize(m);
        }

        w.local_count = 0;
        int next_label = label_base + 1;
        parent_[label_base] = label_base;
        rank_[label_base] = 0;

        bool first_row = true;
        for (int r = r0; r < r1; ++r) {
            std::fill(cur_lbl.begin(), cur_lbl.end(), 0);
            const uint32_t* row = &grid[r * m];
            for (int c = 0; c < m; ++c) {
                if (row[c] == 0)
                    continue;

                int left = (c > 0) ? cur_lbl[c - 1] : 0;
                int above = prev_lbl[c];

                if (!left && !above) {
                    parent_[next_label] = next_label;
                    rank_[next_label] = 0;
                    cur_lbl[c] = next_label++;
                    ++w.local_count;
                } else if (left && !above) {
                    cur_lbl[c] = find_global(left);
                } else if (!left) {
                    cur_lbl[c] = find_global(above);
                } else {
                    int a = find_global(left);
                    int b = find_global(above);
                    if (a != b) {
                        if (rank_[a] < rank_[b])
                            std::swap(a, b);
                        parent_[b] = a;
                        if (rank_[a] == rank_[b])
                            ++rank_[a];
                        --w.local_count;
                    }
                    cur_lbl[c] = a;
                }
            }
            if (first_row) {
                std::memcpy(w.first_row_lbl.data(), cur_lbl.data(), m * sizeof(int));
                first_row = false;
            }
            prev_lbl.swap(cur_lbl);
        }
        std::memcpy(w.last_row_lbl.data(), prev_lbl.data(), m * sizeof(int));
    }

    int find_global(int x) noexcept {
        while (parent_[x] != x) {
            parent_[x] = parent_[parent_[x]];
            x = parent_[x];
        }
        return x;
    }
};

std::vector<uint32_t> make_random(int rows, int cols, double density, uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> d(0.0, 1.0);
    std::vector<uint32_t> g(rows * cols);
    for (auto& x : g)
        x = (d(rng) < density) ? 1 : 0;
    return g;
}

std::vector<uint32_t> make_all_ones(int rows, int cols) { return std::vector<uint32_t>(rows * cols, 1); }

std::vector<uint32_t> make_checkerboard(int rows, int cols) {
    std::vector<uint32_t> g(rows * cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            g[r * cols + c] = ((r + c) % 2 == 0) ? 1 : 0;
    return g;
}

std::vector<uint32_t> make_crosses(int rows, int cols) {
    std::vector<uint32_t> g(rows * cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            g[r * cols + c] = (r % 4 == 0 || c % 4 == 0) ? 1 : 0;
    return g;
}

std::vector<uint32_t> make_snake(int rows, int cols) {
    std::vector<uint32_t> g(rows * cols);
    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < cols; ++c)
                g[r * cols + c] = 1;
        } else {
            g[r * cols + ((r / 2) % 2 == 0 ? cols - 1 : 0)] = 1;
        }
    }
    return g;
}

std::vector<uint32_t> make_comb(int rows, int cols) {
    std::vector<uint32_t> g(rows * cols);
    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < cols; ++c)
                g[r * cols + c] = 1;
        } else {
            g[r * cols] = 1;
        }
    }
    return g;
}

std::vector<uint32_t> make_stripes(int rows, int cols) {
    std::vector<uint32_t> g(rows * cols);
    for (int r = 0; r < rows; r += 2)
        for (int c = 0; c < cols; ++c)
            g[r * cols + c] = 1;
    return g;
}

std::vector<uint32_t> make_vbars(int rows, int cols) {
    std::vector<uint32_t> g(rows * cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; c += 2)
            g[r * cols + c] = 1;
    return g;
}

using Clock = std::chrono::high_resolution_clock;
using Ns = std::chrono::duration<double, std::nano>;

struct Tee {
    std::ofstream file;
    explicit Tee(const char* path) : file(path) {}
    void operator()(const std::string& s) {
        std::cout << s;
        file << s;
    }
    void line(const std::string& s) { operator()(s + "\n"); }
};

static constexpr int NSOLS = 4;
static const char* SOL_NAMES[NSOLS] = {"Sol-1", "Sol-2", "Sol-3", "Sol-4"};
static const char* SOL_DESC[NSOLS] = {"Row-scan DSU (label prop)  ", "DFS with stack             ",
                                      "Single-pass flat DSU       ", "Multi-threaded strip DSU   "};

struct BenchResult {
    std::string name;
    int answer;
    double ms[NSOLS];
    int rank[NSOLS];
    int reps;
};

class Harness {
    Solution1 s1;
    Solution2 s2;
    Solution3 s3;
    Solution4 s4;

    int invoke(int sol, std::vector<uint32_t>& grid, int m, int n) {
        switch (sol) {
        case 0:
            return s1.numIslands(grid, m, n);
        case 1:
            return s2.numIslands(grid, m, n);
        case 2:
            return s3.numIslands(grid, m, n);
        case 3:
            return s4.numIslands(grid, m, n);
        }
        return -1;
    }

  public:
    bool check(const std::string& name, std::vector<uint32_t>& grid, int m, int n, int expected, int answers[NSOLS]) {
        for (int s = 0; s < NSOLS; ++s)
            answers[s] = invoke(s, grid, m, n);
        int ref = (expected >= 0) ? expected : answers[0];
        bool ok = true;
        for (int s = 0; s < NSOLS; ++s)
            if (answers[s] != ref)
                ok = false;
        return ok;
    }

    BenchResult bench(const std::string& name, std::vector<uint32_t>& grid, int m, int n, int reps) {
        BenchResult r;
        r.name = name;
        r.reps = reps;
        r.answer = invoke(0, grid, m, n);

        int warmup = std::max(3, reps / 10);
        for (int s = 0; s < NSOLS; ++s) {
            volatile int sink = 0;
            for (int i = 0; i < warmup; ++i)
                sink += invoke(s, grid, m, n);
            auto t0 = Clock::now();
            int acc = 0;
            for (int i = 0; i < reps; ++i)
                acc += invoke(s, grid, m, n);
            r.ms[s] = Ns(Clock::now() - t0).count() * 1e-6;
            (void)acc;
        }

        int order[NSOLS];
        std::iota(order, order + NSOLS, 0);
        std::sort(order, order + NSOLS, [&](int a, int b) { return r.ms[a] < r.ms[b]; });
        for (int i = 0; i < NSOLS; ++i)
            r.rank[order[i]] = i + 1;
        return r;
    }
};

static const char* MEDAL[5] = {"", "[1]", "[2]", "[3]", "[4]"};

static void print_result(Tee& out, const BenchResult& br) {
    char buf[512];
    std::snprintf(buf, sizeof(buf), "  %-46s  ans=%-5d  reps=%d\n", br.name.c_str(), br.answer, br.reps);
    out(buf);
    for (int s = 0; s < NSOLS; ++s) {
        double ns_per = br.ms[s] * 1e6 / br.reps;
        std::snprintf(buf, sizeof(buf), "    %s %s  %8.2f ms  %7.1f ns/call%s\n", MEDAL[br.rank[s]], SOL_NAMES[s],
                      br.ms[s], ns_per, (br.rank[s] == 1) ? "  <-- fastest" : "");
        out(buf);
    }
    out("\n");
}

static void print_summary(Tee& out, const std::vector<BenchResult>& results) {
    double totals[NSOLS] = {};
    int wins[NSOLS] = {};
    for (auto& r : results) {
        for (int s = 0; s < NSOLS; ++s)
            totals[s] += r.ms[s];
        for (int s = 0; s < NSOLS; ++s)
            if (r.rank[s] == 1)
                ++wins[s];
    }
    int order[NSOLS];
    std::iota(order, order + NSOLS, 0);
    std::sort(order, order + NSOLS, [&](int a, int b) { return totals[a] < totals[b]; });
    out.line(std::string(70, '='));
    out.line("  SUMMARY — total wall time across all benchmarks");
    out.line(std::string(70, '-'));
    char buf[256];
    std::snprintf(buf, sizeof(buf), "  %-6s  %-42s  %9s  %5s\n", "Rank", "Solution", "Total ms", "Wins");
    out(buf);
    out.line(std::string(70, '-'));
    for (int i = 0; i < NSOLS; ++i) {
        int s = order[i];
        std::snprintf(buf, sizeof(buf), "  %s  %s  %9.1f  %5d\n", MEDAL[i + 1], SOL_DESC[s], totals[s], wins[s]);
        out(buf);
    }
    out.line(std::string(70, '='));
}

int main() {
    Tee out("tests_log.txt");
    out.line("ISLAND COUNT — FLAT GRID BENCHMARK (4 solutions)\n");

    Harness H;

    out.line("--- CORRECTNESS ---");
    struct CSpec {
        std::string name;
        std::function<std::vector<uint32_t>()> make;
        int rows, cols, expected;
    };
    std::vector<CSpec> cspecs = {
        {"Empty 0x0", [] { return std::vector<uint32_t>{}; }, 0, 0, 0},
        {"1x1 zero", [] { return std::vector<uint32_t>{0}; }, 1, 1, 0},
        {"1x1 one", [] { return std::vector<uint32_t>{1}; }, 1, 1, 1},
        {"3x3 checker", [] { return make_checkerboard(3, 3); }, 3, 3, 5},
        {"3x3 all ones", [] { return make_all_ones(3, 3); }, 3, 3, 1},
        {"10x10 random d=50%", [] { return make_random(10, 10, 0.5, 42); }, 10, 10, -1},
    };
    bool all_ok = true;
    for (auto& cs : cspecs) {
        auto g = cs.make();
        int ans[NSOLS];
        bool ok = H.check(cs.name, g, cs.cols, cs.rows, cs.expected, ans);
        all_ok &= ok;
        if (!ok) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "  FAIL %s  -> %d %d %d %d\n", cs.name.c_str(), ans[0], ans[1], ans[2],
                          ans[3]);
            out(buf);
        }
    }
    out(all_ok ? "All correct.\n\n" : "SOME FAILURES!\n\n");

    out.line("--- BENCHMARKS ---");
    std::vector<BenchResult> results;
    auto run = [&](const std::string& name, std::vector<uint32_t> grid, int cols, int rows, int reps) {
        auto br = H.bench(name, grid, cols, rows, reps);
        print_result(out, br);
        results.push_back(br);
    };

    run("8x8   random d=50%", make_random(8, 8, 0.5, 1), 8, 8, 3000000);
    run("8x8   checkerboard", make_checkerboard(8, 8), 8, 8, 3000000);
    run("32x32 random d=50%", make_random(32, 32, 0.5, 3), 32, 32, 500000);
    run("32x32 all-ones", make_all_ones(32, 32), 32, 32, 500000);
    run("256x32 random d=50%", make_random(256, 32, 0.5, 10), 32, 256, 50000);
    run("1024x32 random d=50%", make_random(1024, 32, 0.5, 11), 32, 1024, 12000);
    run("4096x32 random d=50%", make_random(4096, 32, 0.5, 12), 32, 4096, 3000);
    run("4096x32 all-ones", make_all_ones(4096, 32), 32, 4096, 3000);

    print_summary(out, results);
    out.line("Results saved to tests_log.txt");
    return all_ok ? 0 : 1;
}