#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <omp.h>
#include <random>
#include <string>
#include <vector>

struct Point {
    float x = 0.f;
    float y = 0.f;
};

static inline bool onSegment(long double px, long double py, long double x0, long double y0, long double x1,
                             long double y1) {
    if (px < std::min(x0, x1) || px > std::max(x0, x1))
        return false;
    if (py < std::min(y0, y1) || py > std::max(y0, y1))
        return false;
    long double cross = (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
    long double len2 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    return cross * cross <= 1e-20L * len2;
}

static inline int pipCrossing(long double px, long double py, const std::vector<Point>& poly) {
    int n = (int)poly.size();
    int c = 0;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        long double xi = poly[i].x, yi = poly[i].y;
        long double xj = poly[j].x, yj = poly[j].y;
        if (onSegment(px, py, xi, yi, xj, yj))
            return 1;
        if ((yi > py) != (yj > py)) {
            long double xInt = (xj - xi) * (py - yi) / (yj - yi) + xi;
            if (px < xInt)
                ++c;
        }
    }
    return c & 1;
}

struct Seg {
    long double x0, y0, x1, y1, inv_slope;
};

static std::vector<Seg> buildSegs(const std::vector<Point>& poly) {
    int n = (int)poly.size();
    std::vector<Seg> out;
    out.reserve(n);
    for (int i = 0, j = n - 1; i < n; j = i++) {
        long double x0 = poly[i].x, y0 = poly[i].y;
        long double x1 = poly[j].x, y1 = poly[j].y;
        long double dy = y1 - y0;
        if (std::fabs(dy) < 1e-15L)
            continue;
        if (dy < 0) {
            std::swap(x0, x1);
            std::swap(y0, y1);
            dy = -dy;
        }
        out.push_back({x0, y0, x1, y1, (x1 - x0) / dy});
    }
    return out;
}

static inline int crossCount(const Seg& s, long double px, long double py) {
    return (px < s.x0 + s.inv_slope * (py - s.y0)) ? 1 : 0;
}

static bool onBoundary(long double px, long double py, const std::vector<Point>& poly) {
    int n = (int)poly.size();
    for (int i = 0, j = n - 1; i < n; j = i++)
        if (onSegment(px, py, poly[i].x, poly[i].y, poly[j].x, poly[j].y))
            return true;
    return false;
}

static inline int bucketOf(long double y, long double minY, long double invH, int K) {
    int b = (int)((y - minY) * invH);
    return b < 0 ? 0 : b >= K ? K - 1 : b;
}

class Solution1 {
  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        result.resize(points.size());
        for (size_t pi = 0; pi < points.size(); ++pi)
            result[pi] = pipCrossing(points[pi].x, points[pi].y, polygon);
    }
};

class Solution2 {
    std::vector<std::vector<int>> buckets_;
    std::vector<Seg> segs_;
    long double minX_, minY_, maxX_, maxY_, invH_;
    int K_;

    void build(const std::vector<Point>& poly) {
        segs_ = buildSegs(poly);
        minX_ = minY_ = 1e30L;
        maxX_ = maxY_ = -1e30L;
        for (auto& p : poly) {
            minX_ = std::min(minX_, (long double)p.x);
            maxX_ = std::max(maxX_, (long double)p.x);
            minY_ = std::min(minY_, (long double)p.y);
            maxY_ = std::max(maxY_, (long double)p.y);
        }
        minX_ -= 1e-9L;
        minY_ -= 1e-9L;
        maxX_ += 1e-9L;
        maxY_ += 1e-9L;
        K_ = std::max(8, std::min(2048, (int)(4.0L * std::sqrt((long double)poly.size()))));
        invH_ = (long double)K_ / (maxY_ - minY_);
        buckets_.assign(K_, {});
        for (int si = 0; si < (int)segs_.size(); ++si) {
            const Seg& s = segs_[si];
            int b0 = std::max(0, bucketOf(s.y0, minY_, invH_, K_) - 1);
            int b1 = std::min(K_ - 1, bucketOf(s.y1, minY_, invH_, K_) + 1);
            for (int b = b0; b <= b1; ++b)
                buckets_[b].push_back(si);
        }
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        build(polygon);
        int m = (int)points.size();
        result.resize(m);
        for (int pi = 0; pi < m; ++pi) {
            long double px = points[pi].x, py = points[pi].y;
            if (onBoundary(px, py, polygon)) {
                result[pi] = 1;
                continue;
            }
            if (px < minX_ || px > maxX_ || py < minY_ || py > maxY_) {
                result[pi] = 0;
                continue;
            }
            int b = bucketOf(py, minY_, invH_, K_);
            int c = 0;
            for (int si : buckets_[b]) {
                const Seg& s = segs_[si];
                if (py < s.y0 || py >= s.y1)
                    continue;
                c += crossCount(s, px, py);
            }
            result[pi] = c & 1;
        }
    }
};

class Solution3 {
    std::vector<Seg> segs_;
    long double minX_, minY_, maxX_, maxY_;

    void build(const std::vector<Point>& poly) {
        segs_ = buildSegs(poly);
        std::sort(segs_.begin(), segs_.end(), [](const Seg& a, const Seg& b) { return a.y0 < b.y0; });
        minX_ = minY_ = 1e30L;
        maxX_ = maxY_ = -1e30L;
        for (auto& p : poly) {
            minX_ = std::min(minX_, (long double)p.x);
            maxX_ = std::max(maxX_, (long double)p.x);
            minY_ = std::min(minY_, (long double)p.y);
            maxY_ = std::max(maxY_, (long double)p.y);
        }
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        build(polygon);
        int m = (int)points.size();
        result.resize(m);
        for (int pi = 0; pi < m; ++pi) {
            long double px = points[pi].x, py = points[pi].y;
            if (onBoundary(px, py, polygon)) {
                result[pi] = 1;
                continue;
            }
            if (px < minX_ || px > maxX_ || py < minY_ || py > maxY_) {
                result[pi] = 0;
                continue;
            }
            int c = 0;
            for (const Seg& s : segs_) {
                if (s.y0 > py)
                    break;
                if (py >= s.y1)
                    continue;
                c += crossCount(s, px, py);
            }
            result[pi] = c & 1;
        }
    }
};

class Solution4 {
    std::vector<std::vector<int>> buckets_;
    std::vector<Seg> segs_;
    long double minX_, minY_, maxX_, maxY_, invH_;
    int K_;

    void build(const std::vector<Point>& poly) {
        segs_ = buildSegs(poly);
        minX_ = minY_ = 1e30L;
        maxX_ = maxY_ = -1e30L;
        for (auto& p : poly) {
            minX_ = std::min(minX_, (long double)p.x);
            maxX_ = std::max(maxX_, (long double)p.x);
            minY_ = std::min(minY_, (long double)p.y);
            maxY_ = std::max(maxY_, (long double)p.y);
        }
        minX_ -= 1e-9L;
        minY_ -= 1e-9L;
        maxX_ += 1e-9L;
        maxY_ += 1e-9L;
        K_ = std::max(8, std::min(2048, (int)(4.0L * std::sqrt((long double)poly.size()))));
        invH_ = (long double)K_ / (maxY_ - minY_);
        buckets_.assign(K_, {});
        for (int si = 0; si < (int)segs_.size(); ++si) {
            const Seg& s = segs_[si];
            int b0 = std::max(0, bucketOf(s.y0, minY_, invH_, K_) - 1);
            int b1 = std::min(K_ - 1, bucketOf(s.y1, minY_, invH_, K_) + 1);
            for (int b = b0; b <= b1; ++b)
                buckets_[b].push_back(si);
        }
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        build(polygon);
        int m = (int)points.size();
        result.assign(m, 0);
        std::vector<int> order(m);
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int a, int b) { return points[a].y < points[b].y; });
        for (int pi : order) {
            long double px = points[pi].x, py = points[pi].y;
            if (onBoundary(px, py, polygon)) {
                result[pi] = 1;
                continue;
            }
            if (px < minX_ || px > maxX_ || py < minY_ || py > maxY_)
                continue;
            int b = bucketOf(py, minY_, invH_, K_);
            int c = 0;
            for (int si : buckets_[b]) {
                const Seg& s = segs_[si];
                if (py < s.y0 || py >= s.y1)
                    continue;
                c += crossCount(s, px, py);
            }
            result[pi] = c & 1;
        }
    }
};

class Solution5 {
    std::vector<std::vector<int>> buckets_;
    std::vector<Seg> segs_;
    long double minX_, minY_, maxX_, maxY_, invH_;
    int K_;
    std::vector<Point> poly_;

    void build(const std::vector<Point>& poly) {
        poly_ = poly;
        segs_ = buildSegs(poly);
        minX_ = minY_ = 1e30L;
        maxX_ = maxY_ = -1e30L;
        for (auto& p : poly) {
            minX_ = std::min(minX_, (long double)p.x);
            maxX_ = std::max(maxX_, (long double)p.x);
            minY_ = std::min(minY_, (long double)p.y);
            maxY_ = std::max(maxY_, (long double)p.y);
        }
        minX_ -= 1e-9L;
        minY_ -= 1e-9L;
        maxX_ += 1e-9L;
        maxY_ += 1e-9L;
        K_ = std::max(8, std::min(2048, (int)(4.0L * std::sqrt((long double)poly.size()))));
        invH_ = (long double)K_ / (maxY_ - minY_);
        buckets_.assign(K_, {});
        for (int si = 0; si < (int)segs_.size(); ++si) {
            const Seg& s = segs_[si];
            int b0 = std::max(0, bucketOf(s.y0, minY_, invH_, K_) - 1);
            int b1 = std::min(K_ - 1, bucketOf(s.y1, minY_, invH_, K_) + 1);
            for (int b = b0; b <= b1; ++b)
                buckets_[b].push_back(si);
        }
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        build(polygon);
        int m = (int)points.size();
        result.assign(m, 0);
#pragma omp parallel for schedule(dynamic, 512)
        for (int pi = 0; pi < m; ++pi) {
            long double px = points[pi].x, py = points[pi].y;
            if (onBoundary(px, py, poly_)) {
                result[pi] = 1;
                continue;
            }
            if (px < minX_ || px > maxX_ || py < minY_ || py > maxY_)
                continue;
            int b = bucketOf(py, minY_, invH_, K_);
            int c = 0;
            for (int si : buckets_[b]) {
                const Seg& s = segs_[si];
                if (py < s.y0 || py >= s.y1)
                    continue;
                c += crossCount(s, px, py);
            }
            result[pi] = c & 1;
        }
    }
};

class Solution6 {
    std::vector<std::vector<int>> buckets_;
    std::vector<Seg> segs_;
    long double minX_, minY_, maxX_, maxY_, invH_;
    int K_;
    std::vector<Point> poly_;

    void build(const std::vector<Point>& poly) {
        poly_ = poly;
        segs_ = buildSegs(poly);
        minX_ = minY_ = 1e30L;
        maxX_ = maxY_ = -1e30L;
        for (auto& p : poly) {
            minX_ = std::min(minX_, (long double)p.x);
            maxX_ = std::max(maxX_, (long double)p.x);
            minY_ = std::min(minY_, (long double)p.y);
            maxY_ = std::max(maxY_, (long double)p.y);
        }
        minX_ -= 1e-9L;
        minY_ -= 1e-9L;
        maxX_ += 1e-9L;
        maxY_ += 1e-9L;
        K_ = std::max(8, std::min(2048, (int)(4.0L * std::sqrt((long double)poly.size()))));
        invH_ = (long double)K_ / (maxY_ - minY_);
        buckets_.assign(K_, {});
        for (int si = 0; si < (int)segs_.size(); ++si) {
            const Seg& s = segs_[si];
            int b0 = std::max(0, bucketOf(s.y0, minY_, invH_, K_) - 1);
            int b1 = std::min(K_ - 1, bucketOf(s.y1, minY_, invH_, K_) + 1);
            for (int b = b0; b <= b1; ++b)
                buckets_[b].push_back(si);
        }
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        build(polygon);
        int m = (int)points.size();
        result.assign(m, 0);
#pragma omp parallel for schedule(dynamic, 512)
        for (int pi = 0; pi < m; ++pi) {
            long double px = points[pi].x, py = points[pi].y;
            if (onBoundary(px, py, poly_)) {
                result[pi] = 0;
                continue;
            }
            if (px < minX_ || px > maxX_ || py < minY_ || py > maxY_)
                continue;
            int b = bucketOf(py, minY_, invH_, K_);
            int c = 0;
            for (int si : buckets_[b]) {
                const Seg& s = segs_[si];
                if (py < s.y0 || py >= s.y1)
                    continue;
                c += crossCount(s, px, py);
            }
            result[pi] = c & 1;
        }
    }
};

using Clock = std::chrono::high_resolution_clock;
using Ms = std::chrono::duration<double, std::milli>;
static std::string failLog;

template <class S>
double runOne(S& sol, const std::vector<Point>& poly, const std::vector<Point>& pts, std::vector<int>& out) {
    out.clear();
    auto t0 = Clock::now();
    sol.TestPoints(poly, pts, out);
    return Ms(Clock::now() - t0).count();
}

static bool checkEq(const std::vector<int>& ref, const std::vector<int>& got, const std::string& sn,
                    const std::string& tn, std::ofstream& logFile) {
    if (ref.size() != got.size()) {
        std::string msg = "[SIZE MISMATCH] " + sn + " @ " + tn + "\n";
        failLog += msg;
        logFile << msg;
        return false;
    }
    int bad = 0;
    for (size_t i = 0; i < ref.size(); ++i) {
        if (ref[i] != got[i]) {
            if (bad < 10) {
                std::string msg = "[MISMATCH] " + sn + " @ " + tn + " idx=" + std::to_string(i) +
                                  " exp=" + std::to_string(ref[i]) + " got=" + std::to_string(got[i]) + "\n";
                failLog += msg;
                logFile << msg;
            }
            ++bad;
        }
    }
    if (bad) {
        std::string s = "[TOTAL] " + sn + " @ " + tn + " mismatches=" + std::to_string(bad) + "\n";
        failLog += s;
        logFile << s;
        return false;
    }
    return true;
}

static std::vector<Point> convexPoly(int n, float cx, float cy, float r, std::mt19937& rng) {
    std::uniform_real_distribution<float> jit(-0.1f / n, 0.1f / n);
    std::vector<float> a(n);
    for (int i = 0; i < n; ++i)
        a[i] = 2.f * 3.14159265f * i / n + jit(rng);
    std::sort(a.begin(), a.end());
    std::vector<Point> p(n);
    for (int i = 0; i < n; ++i)
        p[i] = {cx + r * std::cos(a[i]), cy + r * std::sin(a[i])};
    return p;
}

static std::vector<Point> star(int sp, float cx, float cy, float ro, float ri) {
    std::vector<Point> p;
    for (int i = 0; i < sp * 2; ++i) {
        float a = 3.14159265f * i / sp, r = (i & 1) ? ri : ro;
        p.push_back({cx + r * std::cos(a), cy + r * std::sin(a)});
    }
    return p;
}

static std::vector<Point> rpts(int m, float lo, float hi, std::mt19937& rng) {
    std::uniform_real_distribution<float> d(lo, hi);
    std::vector<Point> v(m);
    for (auto& p : v)
        p = {d(rng), d(rng)};
    return v;
}

struct TC {
    std::string name;
    std::vector<Point> poly, pts;
};

int main() {
    std::ofstream logFile("tests_log2");
    logFile << "=== Point-In-Polygon Benchmark ===\n\n";

    std::mt19937 rng(42);
    std::vector<TC> tests = {
        {"Triangle / 1K pts", {{0, 0}, {5, 10}, {10, 0}}, rpts(1000, -2, 12, rng)},
        {"Octagon / 50K pts", convexPoly(8, 50, 50, 40, rng), rpts(50000, 0, 100, rng)},
        {"Star-12 (concave)/200K", star(12, 500, 500, 400, 150), rpts(200000, 0, 1000, rng)},
        {"Convex-1000 / 200K pts", convexPoly(1000, 500, 500, 400, rng), rpts(200000, 0, 1000, rng)},
        {"Convex-64 / 500K dense", convexPoly(64, 500, 500, 400, rng), rpts(500000, 100, 900, rng)},
        {"Convex-4096 / 100K pts", convexPoly(4096, 500, 500, 400, rng), rpts(100000, 0, 1000, rng)},
    };

    struct Solver {
        std::string name;
        std::function<double(const std::vector<Point>&, const std::vector<Point>&, std::vector<int>&)> run;
        double tot = 0;
        int wins = 0;
    };

    Solution1 s1;
    Solution2 s2;
    Solution3 s3;
    Solution4 s4;
    Solution5 s5;
    Solution6 s6;

    std::vector<Solver> sv = {
        {"Sol1 Naive", [&](auto& p, auto& q, auto& o) { return runOne(s1, p, q, o); }},
        {"Sol2 Strip", [&](auto& p, auto& q, auto& o) { return runOne(s2, p, q, o); }},
        {"Sol3 Sorted", [&](auto& p, auto& q, auto& o) { return runOne(s3, p, q, o); }},
        {"Sol4 InvSlope", [&](auto& p, auto& q, auto& o) { return runOne(s4, p, q, o); }},
        {"Sol5 Parallel", [&](auto& p, auto& q, auto& o) { return runOne(s5, p, q, o); }},
        {"Sol6 BndOut", [&](auto& p, auto& q, auto& o) { return runOne(s6, p, q, o); }},
    };

    const int NW = 22, TW = 13;
    std::cout << "\n=== Point-In-Polygon Benchmark ===\n";
    std::cout << "OpenMP threads: " << omp_get_max_threads() << "\n";
    std::cout << "long double size: " << sizeof(long double) << " bytes\n\n";

    for (auto& tc : tests) {
        std::cout << "─── " << tc.name << "  [poly=" << tc.poly.size() << " | pts=" << tc.pts.size() << "]\n";
        std::cout << std::left << std::setw(NW) << "Solver" << std::right << std::setw(TW) << "Time (ms)" << "\n";
        std::cout << std::string(NW + TW, '-') << "\n";

        std::vector<int> ref;
        sv[0].run(tc.poly, tc.pts, ref);

        std::vector<double> times(sv.size());
        std::vector<std::vector<int>> outs(sv.size());
        for (size_t si = 0; si < sv.size(); ++si)
            times[si] = sv[si].run(tc.poly, tc.pts, outs[si]);

        for (size_t si = 1; si < sv.size() - 1; ++si)
            checkEq(ref, outs[si], sv[si].name, tc.name, logFile);

        int best = (int)(std::min_element(times.begin(), times.end()) - times.begin());
        for (size_t si = 0; si < sv.size(); ++si) {
            std::cout << std::left << std::setw(NW) << sv[si].name << std::right << std::setw(TW) << std::fixed
                      << std::setprecision(3) << times[si];
            if ((int)si == best)
                std::cout << "  ◄ BEST";
            if (si == sv.size() - 1)
                std::cout << "  [boundary=OUT]";
            std::cout << "\n";
            sv[si].tot += times[si];
        }
        sv[best].wins++;
        std::cout << "\n";
    }

    std::cout << "=== SUMMARY ===\n";
    std::cout << std::left << std::setw(NW) << "Solver" << std::right << std::setw(TW) << "Total (ms)" << std::setw(7)
              << "Wins" << "\n";
    std::cout << std::string(NW + TW + 7, '=') << "\n";
    int best = 0;
    for (size_t si = 0; si < sv.size(); ++si) {
        std::cout << std::left << std::setw(NW) << sv[si].name << std::right << std::setw(TW) << std::fixed
                  << std::setprecision(3) << sv[si].tot << std::setw(7) << sv[si].wins << "\n";
        if (sv[si].tot < sv[best].tot)
            best = (int)si;
    }
    std::cout << "\n  Overall winner: " << sv[best].name << "\n";

    if (!failLog.empty()) {
        std::cout << "\n!!! CORRECTNESS FAILURES !!!\n" << failLog;
    } else {
        std::cout << "  All results match reference. OK.\n";
    }

    logFile.close();
    std::cout << "\nLog saved to: tests_log2\n";
    return 0;
}