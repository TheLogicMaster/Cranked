#include "Bump.hpp"
#include "Cranked.hpp"

using namespace cranked;

bool Bump::getSegmentIntersectionIndices(Rect rect, LineSeg seg, bool huge1, bool huge2, float &ti1, float &ti2, Vec2 &n1, Vec2 &n2)  {
    ti1 = huge1 ? numeric_limits<float>::lowest() : 0, ti2 = huge2 ? numeric_limits<float>::max() : 1;
    float dx = seg.end.x - seg.start.x;
    float dy = seg.end.y - seg.start.y;
    Vec2 n;
    n1 = {}, n2 = {};
    float p, q, r;
    for (int i = 0; i < 4; i++) {
        if (i == 0)
            n = { -1, 0 }, p = -dx, q = seg.start.x - rect.pos.x;
        else if (i == 1)
            n = { 1, 0 }, p = dx, q = rect.pos.x + rect.size.x - seg.start.x;
        else if (i == 2)
            n = { 0, -1 }, p = -dy, q = seg.start.y - rect.pos.y;
        else
            n = { 0, 1 }, p = dy, q = rect.pos.y + rect.size.y - seg.start.y;
        if (p == 0.0f) {
            if (q <= 0)
                return false;
        } else {
            r = q / p;
            if (p < 0) {
                if (r > ti2)
                    return false;
                else if (r > ti1)
                    ti1 = r, n1 = n;
            } else {
                if (r < ti1)
                    return false;
                else if (r < ti2)
                    ti2 = r, n2 = n;
            }
        }
    }
    return true;
}

optional<Bump::Collision> Bump::detectCollision(Rect entity, Rect other, Vec2 goal)  {
    Vec2 delta{goal.x - entity.pos.x, goal.y - entity.pos.y};
    Rect diff = rectDiff(entity, other);
    bool overlaps = false;
    float ti;
    Vec2 normal;

    if (containsPoint(diff, Vec2(0, 0))) {
        Vec2 corner = diff.nearestCorner(Vec2{0, 0});
        Vec2 size = { min(entity.size.x, abs(corner.x)), min(entity.size.y, abs(corner.y)) };
        ti = -size.x * size.y;
        overlaps = true;
    } else {
        float ti1, ti2;
        Vec2 n1, n2;
        if (getSegmentIntersectionIndices(diff, { { 0, 0 }, delta }, true, true, ti1, ti2, n1, n2)
                and ti1 < 1 and abs(ti1 - ti2) > DELTA and (0 < ti1 + DELTA or 0.0f == ti1 and ti2 > 0)) {
            ti = ti1;
            normal = n1;
        } else
            return {};
    }

    auto sign = [](float val){
        return val > 0 ? 1.0f : val < 0 ? -1.0f : 0.0f;
    };

    Vec2 touch;
    if (overlaps) {
        if (delta.x == 0.0f and delta.y == 0.0f) {
            Vec2 p = diff.nearestCorner(Vec2{0, 0});
            if (abs(p.x) < abs(p.y))
                p.y = 0;
            else
                p.x = 0;
            normal = { sign(p.x), sign(p.y) };
            touch = { entity.pos.x + p.x, entity.pos.y + p.y };
        } else {
            float ti1, ti2;
            Vec2 n1, n2;
            if (!getSegmentIntersectionIndices(diff, { { 0, 0 }, delta }, true, false, ti1, ti2, n1, n2))
                return {};
            touch = entity.pos + delta * ti1;
        }
    } else
        touch = entity.pos + delta * ti;

    return Collision {
            .overlaps = overlaps,
            .ti = ti,
            .move = delta,
            .normal = normal,
            .touch = touch,
            .entityRect = entity,
            .otherRect = other
    };
}

void Bump::traverseGrid(LineSeg seg, const function<void(IntVec2)> &func) {
    auto init = [&](float ct, float t1, float t2){
        float v = t2 - t1;
        if (v > 0)
            return make_tuple(1.0f, (float)CELL_SIZE / v, ((ct + v) * (float)CELL_SIZE - t1) / v);
        else if (v < 0)
            return make_tuple(-1.0f, -(float)CELL_SIZE / v, ((ct + v - 1) * (float)CELL_SIZE - t1) / v);
        else
            return make_tuple(0.0f, numeric_limits<float>::max(), numeric_limits<float>::max());
    };

    Vec2 c1 = worldToCell(seg.start).as<float>();
    Vec2 c2 = worldToCell(seg.end).as<float>();
    auto [stepX, dx, tx] = init(c1.x, seg.start.x, seg.end.x);
    auto [stepY, dy, ty] = init(c1.y, seg.start.y, seg.end.y);
    Vec2 c = c1;

    func(c.as<int32 >());

    while (abs(c.x - c2.x) + abs(c.y - c2.y) > 1) {
        if (tx < ty) {
            tx += dx;
            c.x += stepX;
            func(c.as<int32 >());
        } else {
            if (tx == ty)
                func((c + Vec2{ stepX, 0 }).as<int32 >());
            ty += dy;
            c.y += stepY;
            func(c.as<int32 >());
        }
    }

    if (c.x != c2.x or c.y != c2.y)
        func(c2.as<int32 >());
}

vector<Bump::Collision> Bump::project(Sprite sprite, Rect rect, Vec2 goal, const Bump::ResponseFilter &filter) {
    vector<Collision> collisions;
    unordered_set<Sprite> visited;
    float tl = min(goal.x, rect.pos.x);
    float tt = min(goal.y, rect.pos.y);
    float tr = max(goal.x + rect.size.x, rect.pos.x + rect.size.x);
    float tb = max(goal.y + rect.size.y, rect.pos.y + rect.size.y);
    float tw = tr - tl;
    float th = tb - tt;
    auto cellRect = worldToCellRect({ tl, tt, tw, th });

    for (auto other : getSpritesInCellRect(cellRect)) {
        if (!visited.emplace(other).second)
            continue;
        auto response = filter(sprite, other);
        if (response == ResponseType::Ignore)
            continue;
        auto detected = detectCollision(rect, other->getWorldCollideRect(), goal);
        if (!detected)
            continue;
        auto &col = collisions.emplace_back(*detected);
        col.sprite = sprite;
        col.other = other;
        col.type = response;
    }

    sort(collisions.begin(), collisions.end(), [&](Collision &a, Collision &b){
        if (a.ti != b.ti)
            return a.ti < b.ti;
        float ad = squareDistance(a.entityRect, a.otherRect);
        float bd = squareDistance(a.entityRect, b.otherRect);
        return ad < bd;
    });

    return collisions;
}

vector<Bump::SpriteSegmentIntersectionInfo> Bump::querySegmentInfo(LineSeg seg, const std::optional<SpriteFilter> &filter) {
    vector<SpriteSegmentIntersectionInfo> info;
    unordered_set<Sprite> visited;
    Vec2 delta = seg.end - seg.start;
    for (auto cell : getCellsTouchedBySegment(seg)) {
        for (auto &sprite : cell->sprites) {
            if (!visited.emplace(sprite.get()).second)
                continue;
            if (filter and !filter->operator()(sprite.get()))
                continue;
            float ti1, ti2;
            Vec2 n1, n2;
            if (!getSegmentIntersectionIndices(sprite->getWorldCollideRect(), seg, false, false, ti1, ti2, n1, n2))
                continue;
            float tii1, tii2;
            getSegmentIntersectionIndices(sprite->getWorldCollideRect(), seg, true, true, tii1, tii2, n1, n2);
            info.emplace_back(sprite.get(), ti1, ti2, LineSeg{ seg.start + delta * ti1, seg.start + delta * ti2 }, min(tii1, tii2));
        }
    }
    sort(info.begin(), info.end(), [](SpriteSegmentIntersectionInfo &a, SpriteSegmentIntersectionInfo &b){ return a.weight < b.weight; });
    return info;
}

tuple<Vec2, vector<Bump::Collision>> Bump::check(Sprite sprite, Vec2 goal, const Bump::ResponseFilter &filter) {
    unordered_set<Sprite> visited{ sprite };
    std::vector<Collision> collisions;

    auto visitedFilter = [&](Sprite s, Sprite other){
        if (visited.find(other) != visited.end())
            return ResponseType::Ignore;
        return filter(s, other);
    };

    Rect rect = sprite->getWorldCollideRect();
    vector<Collision> projected = project(sprite, rect, goal, visitedFilter);

    while (!projected.empty()) {
        auto &col = collisions.emplace_back(projected.front());
        tie(goal, projected) = invoke(responseFunctions[(int)col.type], this, col, rect, goal, visitedFilter);
    }

    return { goal, collisions };
}

tuple<Vec2, vector<Bump::Collision>> Bump::move(Sprite sprite, Vec2 goal, bool simulate, const Bump::ResponseFilter &filter) {
    auto [actual, collisions] = check(sprite, goal, filter);
    Rect actualRect = { actual, sprite->collideRect.size };
    Rect startRect = sprite->getWorldCollideRect();

    if (!simulate and actualRect != startRect) {
        IntRect startCellRect = worldToCellRect(startRect);
        IntRect actualCellRect = worldToCellRect(actualRect);

        if (startCellRect != actualCellRect) {
            auto [ startCellPos, startCellSize ] = startCellRect;
            auto [ actualCellPos, actualCellSize ] = actualCellRect;
            auto [ cl1, ct1 ] = startCellPos;
            auto [ cw1, ch1 ] = startCellSize;
            auto [ cl2, ct2 ] = actualCellPos;
            auto [ cw2, ch2 ] = actualCellSize;
            int cr1 = cl1 + cw1 - 1, cb1 = ct1 + ch1 - 1;
            int cr2 = cl2 + cw2 - 1, cb2 = ct2 + ch2 - 1;

            for (int y = ct1; y <= cb1; y++) {
                bool yOut = y < ct2 or y > cb2;
                for (int x = cl1; x <= cr1; x++) {
                    if (!yOut and !(x < cl2 or x > cr2))
                        continue;
                    removeSpriteFromCell(sprite, IntVec2(x, y));
                }
            }

            for (int y = ct2; y <= cb2; y++) {
                bool yOut = y < ct1 or y > cb1;
                for (int x = cl2; x <= cr2; x++) {
                    if (!yOut and !(x < cl2 or x > cr2))
                        continue;
                    addSpriteToCell(sprite, IntVec2(x, y));
                }
            }
        }

        sprite->bounds.pos = actual - sprite->collideRect.pos;
    }

    return { actual, collisions };
}
