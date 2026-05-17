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

class Solution1 {
  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        const int n = (int)polygon.size();
        result.resize(points.size());
        for (size_t pi = 0; pi < points.size(); ++pi) {
            float px = points[pi].x, py = points[pi].y;
            int c = 0;
            for (int i = 0, j = n - 1; i < n; j = i++) {
                float xi = polygon[i].x, yi = polygon[i].y;
                float xj = polygon[j].x, yj = polygon[j].y;
                if (((yi > py) != (yj > py)) && (px < (xj - xi) * (py - yi) / (yj - yi) + xi))
                    ++c;
            }
            result[pi] = c & 1;
        }
    }
};

class Solution2 {
    struct Segment {
        float x0, y0, x1, y1;
    };
    std::vector<std::vector<Segment>> seg_;
    float minX_, minY_, maxX_, maxY_, count_blocks_h;
    int total_seg_;

    void build(const std::vector<Point>& poly) {
        int n = (int)poly.size();
        minX_ = minY_ = 1e30f;
        maxX_ = maxY_ = -1e30f;
        for (auto& p : poly) {
            minX_ = std::min(minX_, p.x);
            maxX_ = std::max(maxX_, p.x);
            minY_ = std::min(minY_, p.y);
            maxY_ = std::max(maxY_, p.y);
        }
        minX_ -= 1e-4f;
        minY_ -= 1e-4f;
        maxX_ += 1e-4f;
        maxY_ += 1e-4f;
        total_seg_ = std::max(8, std::min(2048, (int)(4.f * std::sqrt((float)n))));
        seg_.assign(total_seg_, {});
        count_blocks_h = total_seg_ / (maxY_ - minY_);
        for (int i = 0, j = n - 1; i < n; j = i++) {
            float y0 = poly[i].y, y1 = poly[j].y;
            int r0 = std::max(0, std::min(total_seg_ - 1, (int)((std::min(y0, y1) - minY_) * count_blocks_h)));
            int r1 = std::max(0, std::min(total_seg_ - 1, (int)((std::max(y0, y1) - minY_) * count_blocks_h)));
            Segment seg{poly[i].x, y0, poly[j].x, y1};
            for (int r = r0; r <= r1; ++r) {
                seg_[r].push_back(seg);
            }
        }
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        build(polygon);
        result.resize(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            float x = points[i].x, y = points[i].y;
            if (x <= minX_ || x >= maxX_ || y <= minY_ || y >= maxY_) {
                result[i] = 0;
                continue;
            }

            int r = std::max(0, std::min(total_seg_ - 1, (int)((y - minY_) * count_blocks_h)));
            int xr = 0;

            for (auto& seg : seg_[r]) {
                if (((seg.y0 > y) != (seg.y1 > y)) &&
                    (x < (seg.x1 - seg.x0) * (y - seg.y0) / (seg.y1 - seg.y0) + seg.x0)) {
                    ++xr;
                }
            }

            result[i] = xr & 1;
        }
    }
};

class Solution3 {
    struct Seg {
        float x0, y0, x1, y1, lo, hi;
    };
    std::vector<Seg> segs_;
    float minX_, minY_, maxX_, maxY_;

    void build(const std::vector<Point>& poly) {
        int n = (int)poly.size();
        minX_ = minY_ = 1e30f;
        maxX_ = maxY_ = -1e30f;
        segs_.clear();
        segs_.reserve(n);
        for (int i = 0, j = n - 1; i < n; j = i++) {
            float x0 = poly[i].x, y0 = poly[i].y, x1 = poly[j].x, y1 = poly[j].y;
            minX_ = std::min(minX_, x0);
            maxX_ = std::max(maxX_, x0);
            minY_ = std::min(minY_, y0);
            maxY_ = std::max(maxY_, y0);
            segs_.push_back({x0, y0, x1, y1, std::min(y0, y1), std::max(y0, y1)});
        }
        std::sort(segs_.begin(), segs_.end(), [](auto& a, auto& b) { return a.lo < b.lo; });
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        build(polygon);
        result.resize(points.size());
        for (size_t pi = 0; pi < points.size(); ++pi) {
            float px = points[pi].x, py = points[pi].y;
            if (px < minX_ || px > maxX_ || py < minY_ || py > maxY_) {
                result[pi] = 0;
                continue;
            }
            int c = 0;
            for (auto& seg : segs_) {
                if (seg.lo > py) {
                    break;
                }
                if (seg.hi <= py) {
                    continue;
                }
                if (((seg.y0 > py) != (seg.y1 > py)) &&
                    (px < (seg.x1 - seg.x0) * (py - seg.y0) / (seg.y1 - seg.y0) + seg.x0)) {
                    ++c;
                }
            }
            result[pi] = c & 1;
        }
    }
};

class Solution4 {
    struct Segment {
        float x0, y0, inv_slope, maxY;
    };
    std::vector<std::vector<Segment>> segments_;
    float minX_, minY_, maxX_, maxY_, invH_;
    int total_seg_;

    void build(const std::vector<Point>& poly) {
        int n = (int)poly.size();
        minX_ = minY_ = 1e30f;
        maxX_ = maxY_ = -1e30f;
        for (auto& p : poly) {
            minX_ = std::min(minX_, p.x);
            maxX_ = std::max(maxX_, p.x);
            minY_ = std::min(minY_, p.y);
            maxY_ = std::max(maxY_, p.y);
        }
        minX_ -= 1e-4f;
        minY_ -= 1e-4f;
        maxX_ += 1e-4f;
        maxY_ += 1e-4f;
        total_seg_ = std::max(8, std::min(2048, (int)(4.f * std::sqrt((float)n))));
        segments_.assign(total_seg_, {});
        invH_ = total_seg_ / (maxY_ - minY_);
        for (int i = 0, j = n - 1; i < n; j = i++) {
            float x0 = poly[i].x, y0 = poly[i].y, x1 = poly[j].x, y1 = poly[j].y;
            float dy = y1 - y0;
            if (std::fabs(dy) < 1e-9f) {
                continue;
            }
            if (dy < 0) {
                std::swap(x0, x1);
                std::swap(y0, y1);
                dy = -dy;
            }
            Segment seg{x0, y0, (x1 - x0) / dy, y1};
            int s0 = std::max(0, std::min(total_seg_ - 1, (int)((y0 - minY_) * invH_)));
            int s1 = std::max(0, std::min(total_seg_ - 1, (int)((y1 - minY_) * invH_)));
            for (int s = s0; s <= s1; ++s) {
                segments_[s].push_back(seg);
            }
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
            float px = points[pi].x, py = points[pi].y;
            if (px <= minX_ || px >= maxX_ || py <= minY_ || py >= maxY_) {
                continue;
            }
            int s = std::max(0, std::min(total_seg_ - 1, (int)((py - minY_) * invH_)));
            int c = 0;
            for (auto& seg : segments_[s]) {
                if (seg.y0 >= py || seg.maxY < py) {
                    continue;
                }
                c += (px < seg.x0 + seg.inv_slope * (py - seg.y0)) ? 1 : 0;
            }
            result[pi] = c & 1;
        }
    }
};

class Solution5 {
    struct Segment {
        float x0, y0, inv_slope, maxY;
    };
    std::vector<std::vector<Segment>> segments_;
    float minX_, minY_, maxX_, maxY_, invH_;
    int total_seg_;

    void build(const std::vector<Point>& poly) {
        int n = (int)poly.size();
        minX_ = minY_ = 1e30f;
        maxX_ = maxY_ = -1e30f;
        for (auto& p : poly) {
            minX_ = std::min(minX_, p.x);
            maxX_ = std::max(maxX_, p.x);
            minY_ = std::min(minY_, p.y);
            maxY_ = std::max(maxY_, p.y);
        }
        minX_ -= 1e-4f;
        minY_ -= 1e-4f;
        maxX_ += 1e-4f;
        maxY_ += 1e-4f;
        total_seg_ = std::max(8, std::min(2048, (int)(4.f * std::sqrt((float)n))));
        segments_.assign(total_seg_, {});
        invH_ = total_seg_ / (maxY_ - minY_);
        for (int i = 0, j = n - 1; i < n; j = i++) {
            float x0 = poly[i].x, y0 = poly[i].y, x1 = poly[j].x, y1 = poly[j].y;
            float dy = y1 - y0;
            if (std::fabs(dy) < 1e-9f) {
                continue;
            }
            if (dy < 0) {
                std::swap(x0, x1);
                std::swap(y0, y1);
                dy = -dy;
            }
            Segment seg{x0, y0, (x1 - x0) / dy, y1};
            int s0 = std::max(0, std::min(total_seg_ - 1, (int)((y0 - minY_) * invH_)));
            int s1 = std::max(0, std::min(total_seg_ - 1, (int)((y1 - minY_) * invH_)));
            for (int s = s0; s <= s1; ++s) {
                segments_[s].push_back(seg);
            }
        }
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        build(polygon);
        int m = (int)points.size();
        result.assign(m, 0);

#pragma omp parallel for schedule(dynamic, 1024)
        for (int pi = 0; pi < m; ++pi) {
            float px = points[pi].x, py = points[pi].y;
            if (px <= minX_ || px >= maxX_ || py <= minY_ || py >= maxY_) {
                continue;
            }
            int s = std::max(0, std::min(total_seg_ - 1, (int)((py - minY_) * invH_)));
            int c = 0;
            for (auto& seg : segments_[s]) {
                if (seg.y0 >= py || seg.maxY < py) {
                    continue;
                }
                c += (px < seg.x0 + seg.inv_slope * (py - seg.y0)) ? 1 : 0;
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
        std::string msg = "[SIZE MISMATCH] " + sn + " @ " + tn + " expected_size=" + std::to_string(ref.size()) +
                          " got_size=" + std::to_string(got.size()) + "\n";
        failLog += msg;
        logFile << msg;
        return false;
    }
    int mismatchCount = 0;
    for (size_t i = 0; i < ref.size(); ++i) {
        if (ref[i] != got[i]) {
            if (mismatchCount < 10) {
                std::string msg = "[VALUE MISMATCH] " + sn + " @ " + tn + " point_idx=" + std::to_string(i) +
                                  " expected=" + std::to_string(ref[i]) + " got=" + std::to_string(got[i]) + "\n";
                failLog += msg;
                logFile << msg;
            }
            mismatchCount++;
        }
    }
    if (mismatchCount > 0) {
        std::string summary = "[TOTAL MISMATCHES] " + sn + " @ " + tn + " count=" + std::to_string(mismatchCount) +
                              " out_of=" + std::to_string(ref.size()) + "\n";
        failLog += summary;
        logFile << summary;
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
    logFile << "=== Point-In-Polygon Benchmark with Logging ===\n\n";

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

    std::vector<Solver> sv = {
        {"Sol1 Naive RayCast", [&](auto& p, auto& q, auto& o) { return runOne(s1, p, q, o); }},
        {"Sol2 Strip Grid", [&](auto& p, auto& q, auto& o) { return runOne(s2, p, q, o); }},
        {"Sol3 Sorted Edges", [&](auto& p, auto& q, auto& o) { return runOne(s3, p, q, o); }},
        {"Sol4 Strip+InvSlope", [&](auto& p, auto& q, auto& o) { return runOne(s4, p, q, o); }},
        {"Sol5 Multithreaded", [&](auto& p, auto& q, auto& o) { return runOne(s5, p, q, o); }},
    };

    const int NW = 24, TW = 13;
    std::cout << "\n=== Point-In-Polygon Benchmark ===\n";
    std::cout << "OpenMP threads available: " << omp_get_max_threads() << "\n\n";
    logFile << "OpenMP threads available: " << omp_get_max_threads() << "\n\n";

    for (auto& tc : tests) {
        std::cout << "─── " << tc.name << "  [poly=" << tc.poly.size() << " | pts=" << tc.pts.size() << "]\n";
        std::cout << std::left << std::setw(NW) << "Solver" << std::right << std::setw(TW) << "Time (ms)" << "\n";
        std::cout << std::string(NW + TW, '-') << "\n";

        logFile << "─── " << tc.name << "  [poly=" << tc.poly.size() << " | pts=" << tc.pts.size() << "]\n";

        std::vector<int> ref;
        sv[0].run(tc.poly, tc.pts, ref);
        std::vector<double> times(sv.size());
        std::vector<std::vector<int>> outs(sv.size());

        for (size_t si = 0; si < sv.size(); ++si)
            times[si] = sv[si].run(tc.poly, tc.pts, outs[si]);

        for (size_t si = 1; si < sv.size(); ++si)
            checkEq(ref, outs[si], sv[si].name, tc.name, logFile);

        int best = (int)(std::min_element(times.begin(), times.end()) - times.begin());
        for (size_t si = 0; si < sv.size(); ++si) {
            std::cout << std::left << std::setw(NW) << sv[si].name << std::right << std::setw(TW) << std::fixed
                      << std::setprecision(3) << times[si];
            if ((int)si == best)
                std::cout << "  ◄ BEST";
            std::cout << "\n";
            sv[si].tot += times[si];

            logFile << sv[si].name << ": " << std::fixed << std::setprecision(3) << times[si] << " ms\n";
        }
        sv[best].wins++;
        std::cout << "\n";
        logFile << "\n";
    }

    std::cout << "=== SUMMARY ===\n";
    std::cout << std::left << std::setw(NW) << "Solver" << std::right << std::setw(TW) << "Total (ms)" << std::setw(7)
              << "Wins" << "\n";
    std::cout << std::string(NW + TW + 7, '=') << "\n";

    logFile << "=== SUMMARY ===\n";

    int best = 0;
    for (size_t si = 0; si < sv.size(); ++si) {
        std::cout << std::left << std::setw(NW) << sv[si].name << std::right << std::setw(TW) << std::fixed
                  << std::setprecision(3) << sv[si].tot << std::setw(7) << sv[si].wins << "\n";
        logFile << sv[si].name << ": Total=" << std::fixed << std::setprecision(3) << sv[si].tot
                << " ms, Wins=" << sv[si].wins << "\n";
        if (sv[si].tot < sv[best].tot)
            best = (int)si;
    }

    std::cout << "\n  Overall winner: " << sv[best].name << "\n";
    logFile << "\nOverall winner: " << sv[best].name << "\n";

    if (!failLog.empty()) {
        std::cout << "\n!!! CORRECTNESS FAILURES !!!\n" << failLog;
        logFile << "\n!!! CORRECTNESS FAILURES !!!\n" << failLog;
    } else {
        std::cout << "  All results match reference (Solution1). OK.\n";
        logFile << "All results match reference (Solution1). OK.\n";
    }

    logFile.close();
    std::cout << "\nDetailed log saved to: tests_log2\n";

    return 0;
}
