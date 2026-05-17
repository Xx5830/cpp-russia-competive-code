#include <algorithm>
#include <cmath>
#include <omp.h>
#include <vector>

struct Point {
    float x = 0.f;
    float y = 0.f;
};

class SolutionRobust {
    struct Segment {
        double x0, y0, x1, y1;
        bool isHorizontal;
    };

    std::vector<std::vector<Segment>> strips_;
    double minX_, minY_, maxX_, maxY_, invH_;
    int total_seg_;

    double crossProduct(double ax, double ay, double bx, double by, double cx, double cy) const {
        return (bx - ax) * (cy - ay) - (cx - ax) * (by - ay);
    }

    bool onSegment(double ax, double ay, double bx, double by, double px, double py) const {
        double minX = std::min(ax, bx);
        double maxX = std::max(ax, bx);
        double minY = std::min(ay, by);
        double maxY = std::max(ay, by);

        if (px < minX - 1e-4 || px > maxX + 1e-4 || py < minY - 1e-4 || py > maxY + 1e-4) {
            return false;
        }

        double dx = bx - ax;
        double dy = by - ay;
        double len2 = dx * dx + dy * dy;

        if (len2 < 1e-12) {
            double dpx = px - ax;
            double dpy = py - ay;
            return (dpx * dpx + dpy * dpy) < 1e-6;
        }

        double cp = crossProduct(ax, ay, bx, by, px, py);
        double dist_sq = (cp * cp) / len2;

        return dist_sq < 1e-6;
    }

  public:
    void TestPoints(const std::vector<Point>& polygon, const std::vector<Point>& points, std::vector<int>& result) {
        int n = (int)polygon.size();
        result.assign(points.size(), 0);
        if (n < 3)
            return;

        minX_ = minY_ = 1e30;
        maxX_ = maxY_ = -1e30;
        for (const auto& p : polygon) {
            minX_ = std::min(minX_, (double)p.x);
            maxX_ = std::max(maxX_, (double)p.x);
            minY_ = std::min(minY_, (double)p.y);
            maxY_ = std::max(maxY_, (double)p.y);
        }

        minX_ -= 1e-2;
        minY_ -= 1e-2;
        maxX_ += 1e-2;
        maxY_ += 1e-2;

        total_seg_ = std::max(8, std::min(2048, (int)(4.0 * std::sqrt((double)n))));
        strips_.assign(total_seg_, {});
        invH_ = total_seg_ / (maxY_ - minY_);

        for (int i = 0, j = n - 1; i < n; j = i++) {
            double x0 = polygon[j].x, y0 = polygon[j].y;
            double x1 = polygon[i].x, y1 = polygon[i].y;

            Segment seg{x0, y0, x1, y1, std::abs(y1 - y0) < 1e-7};

            int s0 = std::max(0, std::min(total_seg_ - 1, (int)((std::min(y0, y1) - minY_) * invH_)));
            int s1 = std::max(0, std::min(total_seg_ - 1, (int)((std::max(y0, y1) - minY_) * invH_)));

            for (int s = s0; s <= s1; ++s) {
                strips_[s].push_back(seg);
            }
        }

        int m = (int)points.size();

#pragma omp parallel for schedule(dynamic, 1024)
        for (int pi = 0; pi < m; ++pi) {
            double px = points[pi].x;
            double py = points[pi].y;

            if (px < minX_ || px > maxX_ || py < minY_ || py > maxY_) {
                result[pi] = 0;
                continue;
            }

            int s = std::max(0, std::min(total_seg_ - 1, (int)((py - minY_) * invH_)));

            bool isBoundary = false;
            int crossings = 0;

            for (const auto& seg : strips_[s]) {
                if (onSegment(seg.x0, seg.y0, seg.x1, seg.y1, px, py)) {
                    isBoundary = true;
                    break;
                }

                if (seg.isHorizontal)
                    continue;

                bool upward = (seg.y0 <= py && py < seg.y1);
                bool downward = (seg.y1 <= py && py < seg.y0);

                if (upward || downward) {
                    double cp = crossProduct(seg.x0, seg.y0, seg.x1, seg.y1, px, py);
                    if (upward && cp > 0) {
                        crossings++;
                    } else if (downward && cp < 0) {
                        crossings++;
                    }
                }
            }

            if (isBoundary) {
                result[pi] = 1;
            } else {
                result[pi] = crossings & 1;
            }
        }
    }
};