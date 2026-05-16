#include <algorithm>
#include <bit>
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
#include <vector>

class Solution1 {
    std::vector<int> parent_;
    std::vector<uint8_t> rank_;
    std::vector<int> prev_line_, line_;

    int find(int x) noexcept {
        while (parent_[x] != x) {
            parent_[x] = parent_[parent_[x]];
            x = parent_[x];
        }
        return x;
    }

    bool unite(int a, int b) noexcept {
        a = find(a);
        b = find(b);
        if (a == b)
            return false;
        if (rank_[a] < rank_[b])
            std::swap(a, b);
        parent_[b] = a;
        rank_[a] += (rank_[a] == rank_[b]);
        return true;
    }

  public:
    int numIslands(std::vector<uint32_t>& grid, int m, int n) {
        if (m == 0 || n == 0) {
            return 0;
        }

        int mx = m * n + 2;

        if ((int)parent_.size() < mx) {
            parent_.resize(mx);
            rank_.resize(mx, 0);
        }
        if ((int)prev_line_.size() < m) {
            prev_line_.resize(m);
            line_.resize(m);
        }

        parent_[0] = 0;
        rank_[0] = 0;
        std::memset(prev_line_.data(), 0, m * sizeof(int));
        int t = 1, result = 0;

        for (int i = 0; i < n; ++i) {
            std::memset(line_.data(), 0, m * sizeof(int));
            const uint32_t* row = &grid[i * m];
            for (int j = 0; j < m; ++j) {
                if (row[j] == 0) {
                    continue;
                }

                int left = (j > 0) ? line_[j - 1] : 0;
                int up = prev_line_[j];

                if (!left && !up) {
                    parent_[t] = t;
                    rank_[t] = 0;
                    line_[j] = t++;
                    ++result;
                } else if (left && !up) {
                    line_[j] = find(left);
                } else if (!left) {
                    line_[j] = find(up);
                } else {
                    result -= unite(left, up);
                    line_[j] = find(left);
                }
            }
            std::swap(prev_line_, line_);
        }
        return result;
    }
};

class Solution2 {
    std::vector<uint32_t> g_;
    std::vector<int> stack_;

  public:
    int numIslands(std::vector<uint32_t>& grid, int m, int n) {
        if (m == 0 || n == 0) {
            return 0;
        }

        int mx = m * n;
        if ((int)g_.size() < mx) {
            g_.resize(mx);
        }

        std::memcpy(g_.data(), grid.data(), mx * sizeof(uint32_t));

        stack_.clear();
        stack_.reserve(std::max(64, mx / 8));
        int result = 0;

        for (int i = 0; i < mx; ++i) {
            if (g_[i] == 0) {
                continue;
            }

            ++result;
            g_[i] = 0;
            stack_.push_back(i);

            while (!stack_.empty()) {
                int v = stack_.back();
                stack_.pop_back();
                int i = v / m;
                int j = v % m;

                if (i > 0 && g_[v - m]) {
                    g_[v - m] = 0;
                    stack_.push_back(v - m);
                }
                if (i < n - 1 && g_[v + m]) {
                    g_[v + m] = 0;
                    stack_.push_back(v + m);
                }
                if (j > 0 && g_[v - 1]) {
                    g_[v - 1] = 0;
                    stack_.push_back(v - 1);
                }
                if (j < m - 1 && g_[v + 1]) {
                    g_[v + 1] = 0;
                    stack_.push_back(v + 1);
                }
            }
        }
        return result;
    }
};

class Solution3 {
    std::vector<int> parent_;
    std::vector<uint8_t> rank_;

    int find(int x) noexcept {
        while (parent_[x] != x) {
            parent_[x] = parent_[parent_[x]];
            x = parent_[x];
        }
        return x;
    }

  public:
    int numIslands(std::vector<uint32_t>& grid, int m, int n) {
        if (m == 0 || n == 0) {
            return 0;
        }

        int mx = m * n;
        if ((int)parent_.size() < mx) {
            parent_.resize(mx);
            rank_.resize(mx, 0);
        }
        std::iota(parent_.begin(), parent_.begin() + mx, 0);
        std::fill(rank_.begin(), rank_.begin() + mx, uint8_t(0));

        int result = 0;
        for (int v = 0; v < mx; ++v) {
            if (grid[v] == 0) {
                continue;
            }

            ++result;
            int i = v / m, j = v % m;

            if (j > 0 && grid[v - 1]) {
                int a = find(v), b = find(v - 1);
                if (a != b) {
                    if (rank_[a] < rank_[b]) {
                        std::swap(a, b);
                    }

                    parent_[b] = a;
                    rank_[a] += (rank_[a] == rank_[b]);
                    --result;
                }
            }

            if (i > 0 && grid[v - m]) {
                int a = find(v), b = find(v - m);
                if (a != b) {
                    if (rank_[a] < rank_[b]) {
                        std::swap(a, b);
                    }

                    parent_[b] = a;
                    rank_[a] += (rank_[a] == rank_[b]);
                    --result;
                }
            }
        }
        return result;
    }
};

std::vector<uint32_t> make_random(int rows, int cols, double density, uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> d(0.0, 1.0);
    std::vector<uint32_t> g(rows * cols);

    for (auto& x : g) {
        x = (d(rng) < density) ? 1 : 0;
    }

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

static constexpr int NSOLS = 3;
static const char* SOL_NAMES[NSOLS] = {"Sol-1", "Sol-2", "Sol-3"};
static const char* SOL_DESC[NSOLS] = {"Row-scan DSU (label prop)  ", "DFS with stack             ",
                                      "Single-pass flat DSU       "};

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

    int invoke(int sol, std::vector<uint32_t>& grid, int m, int n) {
        switch (sol) {
        case 0:
            return s1.numIslands(grid, m, n);
        case 1:
            return s2.numIslands(grid, m, n);
        case 2:
            return s3.numIslands(grid, m, n);
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

        int order[NSOLS] = {0, 1, 2};
        std::sort(order, order + NSOLS, [&](int a, int b) { return r.ms[a] < r.ms[b]; });
        for (int i = 0; i < NSOLS; ++i)
            r.rank[order[i]] = i + 1;
        return r;
    }
};

static const char* MEDAL[4] = {"", "[1]", "[2]", "[3]"};

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
    int order[NSOLS] = {0, 1, 2};
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
    out.line("ISLAND COUNT — FLAT GRID BENCHMARK (3 solutions)\n");

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
            std::snprintf(buf, sizeof(buf), "  FAIL %s  -> %d %d %d\n", cs.name.c_str(), ans[0], ans[1], ans[2]);
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

    run("8x8   random d=50%", make_random(8, 8, 0.5, 1), 8, 8, 30000);
    run("8x8   checkerboard", make_checkerboard(8, 8), 8, 8, 30000);
    run("32x32 random d=50%", make_random(32, 32, 0.5, 3), 32, 32, 5000);
    run("32x32 all-ones", make_all_ones(32, 32), 32, 32, 5000);
    run("256x32 random d=50%", make_random(256, 32, 0.5, 10), 32, 256, 5000);
    run("1024x32 random d=50%", make_random(1024, 32, 0.5, 11), 32, 1024, 1200);
    run("4096x32 random d=50%", make_random(4096, 32, 0.5, 12), 32, 4096, 3000);
    run("4096x32 all-ones", make_all_ones(4096, 32), 32, 4096, 3000);

    print_summary(out, results);
    out.line("Results saved to tests_log.txt");
    return all_ok ? 0 : 1;
}
