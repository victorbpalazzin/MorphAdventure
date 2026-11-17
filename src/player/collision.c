#include "collision.h"
#include <math.h>

static bool FinitePoint(GameVec2 p) { return isfinite(p.x) && isfinite(p.y); }
static double Clamp(double v, double lo, double hi) { return fmax(lo, fmin(v, hi)); }

bool CollisionCircleRect(GameVec2 center, float radius, GameRect rect)
{
    if (!FinitePoint(center) || !isfinite(radius) || radius <= 0 ||
        !isfinite(rect.x) || !isfinite(rect.y) || !isfinite(rect.width) ||
        !isfinite(rect.height) || rect.width <= 0 || rect.height <= 0) return false;
    double dx = center.x - Clamp(center.x, rect.x, (double)rect.x + rect.width);
    double dy = center.y - Clamp(center.y, rect.y, (double)rect.y + rect.height);
    return dx*dx + dy*dy < (double)radius * radius;
}

bool CollisionCirclePolygon(GameVec2 center, float radius, const GameVec2 *vertices, size_t count)
{
    if (!vertices || count < 3 || !FinitePoint(center) || !isfinite(radius) || radius <= 0) return false;
    for (size_t i = 0; i < count; i++) if (!FinitePoint(vertices[i])) return false;
    bool inside = false;
    double x = center.x, y = center.y, radius2 = (double)radius * radius;
    for (size_t i = 0, j = count - 1; i < count; j = i++) {
        double ax = vertices[i].x, ay = vertices[i].y;
        double bx = vertices[j].x, by = vertices[j].y;
        double dx = bx-ax, dy = by-ay, length2 = dx*dx + dy*dy;
        if ((ay > y) != (by > y) && x < (bx-ax)*(y-ay)/(by-ay)+ax) inside = !inside;
        double t = length2 > 0 ? Clamp(((x-ax)*dx+(y-ay)*dy)/length2, 0, 1) : 0;
        double ex = x-ax-t*dx, ey = y-ay-t*dy;
        if (ex*ex + ey*ey < radius2) return true;
    }
    return inside;
}
