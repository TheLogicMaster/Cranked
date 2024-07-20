#pragma once

#include "Graphics.hpp"

namespace cranked {

    /// Bump.lua ported to C++ (https://github.com/kikito/bump.lua)
    class Bump {
    public:
        /// How the physics engine should reconcile a collision between two sprites
        enum class ResponseType {
            Slide, // Slide over surface like a platformer
            Freeze, // Stop moving after collision
            Overlap, // Don't respond to collision
            Bounce, // Bounce away from other sprite
            Ignore // Ignore collision completely (For filtering)
        };

        struct Collision {
            bool overlaps;
            float ti;
            Vec2 move;
            Vec2 normal;
            Vec2 touch;
            Vec2 slide;
            Vec2 bounce;
            Rect entityRect;
            Rect otherRect;
            Sprite sprite;
            Sprite other;
            ResponseType type;
        };

        struct SpriteSegmentIntersectionInfo {
            Sprite sprite; // Intersected sprite
            float ti1; // Percentage along segment for intersection start
            float ti2; // Percentage along segment for intersection end
            LineSeg seg; // Segment from intersection start to end
            float weight; // Entry point along infinite line (Only used for sorting)
        };

        /// Return how to respond to a collision between two sprites (Ordering matters)
        typedef function<ResponseType(Sprite sprite, Sprite other)> ResponseFilter;

        /// Returns true when a sprite should be included in the result
        typedef function<bool(Sprite)> SpriteFilter;

        explicit Bump(Cranked &cranked) : cranked(cranked) {}

        /// Call to reset this system without worrying about memory exceptions
        void reset() {
            for (auto &row : cells) {
                for (auto &cell: row.second) {
                    auto &sprites = cell.second.sprites;
                    for (SpriteRef &ref: vector(sprites.begin(), sprites.end()))
                        ref.reset();
                }
            }
            cells.clear();
        }

        /// Call to reset the world without swallowing memory exceptions
        void resetWorld() {
            cells.clear();
        }

        /// Call to update the world with the specified sprite after changing sprite collision state externally
        void updateSprite(Sprite sprite) {
            bool enabled = sprite->areCollisionsEnabled() and sprite->collideRect;
            for (auto &row : cells)
                for (auto &cell : row.second)
                    eraseByEquivalentKey(cell.second.sprites, sprite);
            if (enabled) {
                auto rect = worldToCellRect(sprite->getWorldCollideRect());
                for (int y = rect.pos.y; y < rect.pos.y + rect.size.y; y++)
                    for (int x = rect.pos.x; x < rect.pos.x + rect.size.x; x++)
                        addSpriteToCell(sprite, { x, y });
            }
        }

        void removeSprite(Sprite sprite) {
            for (auto &row : cells)
                for (auto &cell : row.second)
                    eraseByEquivalentKey(cell.second.sprites, sprite);
        }

        vector<Sprite> getOverlappingSprites(Sprite sprite) {
            return queryRect(sprite->getWorldCollideRect(), [sprite](Sprite s){ return s != sprite; });
        }

        [[nodiscard]] vector<Sprite> getAllOverlappingSprites();

        /// Moves (Or just simulates) a sprite towards the given goal position after filtering for other sprites to collide with
        tuple<Vec2, vector<Collision>> move(Sprite sprite, Vec2 goal, bool simulate, const ResponseFilter &filter);

        /// Simulates a sprite moving towards the given goal position after filtering for other sprites to collide with
        tuple<Vec2, vector<Collision>> check(Sprite sprite, Vec2 goal, const ResponseFilter &filter);

        /// The default response filter which simply returns `Freeze`
        static ResponseType defaultResponseFilter(Sprite a, Sprite b) {
            return ResponseType::Freeze;
        }

        /// Finds all sprites along the given world line segment matching the given filter along with additional information
        vector<SpriteSegmentIntersectionInfo> querySegmentInfo(LineSeg seg, const std::optional<SpriteFilter> &filter = {});

        /// Finds all sprites in the given world rect matching the given filter
        vector<Sprite> queryRect(Rect rect, const std::optional<SpriteFilter> &filter = {}) {
            vector<Sprite> sprites;
            for (Sprite sprite : getSpritesInCellRect(worldToCellRect(rect))) {
                if ((filter and !filter->operator()(sprite)) or !sprite->getWorldCollideRect().intersection(rect))
                    continue;
                sprites.emplace_back(sprite);
            }
            return sprites;
        }

        /// Finds all sprites in the given world point cell matching the given filter
        vector<Sprite> queryPoint(Vec2 point, const std::optional<SpriteFilter> &filter = {}) {
            vector<Sprite> sprites;
            for (Sprite sprite : getSpritesInCellRect({ worldToCell(point), { 1, 1 } })) {
                if ((filter and !filter->operator()(sprite)) or !containsPoint(sprite->getWorldCollideRect(), point))
                    continue;
                sprites.emplace_back(sprite);
            }
            return sprites;
        }

        /// Finds all sprites along the given world line segment matching the given filter
        vector<Sprite> querySegment(LineSeg seg, const std::optional<SpriteFilter> &filter = {}) {
            vector<Sprite> sprites;
            for (SpriteSegmentIntersectionInfo info : querySegmentInfo(seg, filter))
                sprites.push_back(info.sprite);
            return sprites;
        }

    private:
        static inline const float DELTA = 1e-10;
        static inline const int CELL_SIZE = 64; // Todo: Correct value? Should probably be smaller to be effective?

        typedef tuple<Vec2, vector<Collision>>(Bump::*ResponseFunc)(Collision &col, Rect rect, Vec2 goal, const ResponseFilter &filter);

        struct Cell {
            IntVec2 pos{};
            unordered_resource_set<Sprite> sprites;
        };

        static bool getSegmentIntersectionIndices(Rect rect, LineSeg seg, bool huge1, bool huge2, float &ti1, float &ti2, Vec2 &n1, Vec2 &n2);

        static Rect rectDiff(Rect a, Rect b) {
            return {
                b.pos.x - a.pos.x - a.size.x,
                b.pos.y - a.pos.y - a.size.y,
                a.size.x + b.size.x,
                a.size.y + b.size.y,
            };
        }

        static bool containsPoint(Rect r, Vec2 p) {
            return p.x - r.pos.x > DELTA and p.y - r.pos.y > DELTA and r.pos.x + r.size.x - p.x > DELTA and r.pos.y + r.size.y - p.y > DELTA;
        }

        static float squareDistance(Rect a, Rect b) {
            float dx = a.pos.x - b.pos.x + (a.size.x + b.size.x) / 2;
            float dy = a.pos.y - b.pos.y + (a.size.y + b.size.y) / 2;
            return dx * dx + dy * dy;
        }

        static optional<Collision> detectCollision(Rect entity, Rect other, Vec2 goal);

        [[nodiscard]] static Vec2 cellToWorld(IntVec2 pos) {
            return (pos.as<float>() - Vec2(1, 1)) * CELL_SIZE;
        }

        [[nodiscard]] static IntVec2 worldToCell(Vec2 pos) {
            pos = pos / CELL_SIZE;
            return { (int32)floor(pos.x) + 1, (int32)floor(pos.y) + 1 };
        }

        static void traverseGrid(LineSeg seg, const function<void(IntVec2)> &func) ;

        static IntRect worldToCellRect(Rect rect) {
            IntVec2 c = worldToCell(rect.pos);
            int cr = (int)ceil((rect.pos.x + rect.size.x) / (float)CELL_SIZE);
            int cb = (int)ceil((rect.pos.y + rect.size.y) / (float)CELL_SIZE);
            return { c, { cr - c.x + 1, cb - c.y + 1 } };
        }

        tuple<Vec2, vector<Collision>> touch(Collision &col, [[maybe_unused]] Rect rect, [[maybe_unused]] Vec2 goal, [[maybe_unused]] const ResponseFilter &filter) { // NOLINT(*-convert-member-functions-to-static)
            return { col.touch, {} };
        }

        tuple<Vec2, vector<Collision>> cross(Collision &col, Rect rect, Vec2 goal, const ResponseFilter &filter) {
            auto collisions = project(col.sprite, rect, goal, filter);
            return { goal, collisions };
        }

        tuple<Vec2, vector<Collision>> slide(Collision &col, Rect rect, Vec2 goal, const ResponseFilter &filter) {
            if (col.move.x != 0.0f or col.move.y != 0.0f) {
                if (col.normal.x != 0.0f)
                    goal.x = col.touch.x;
                else
                    goal.y = col.touch.y;
            }
            col.slide = goal;
            rect.pos = col.touch;
            auto collisions = project(col.sprite, rect, goal, filter);
            return { goal, collisions };
        }

        tuple<Vec2, vector<Collision>> bounce(Collision &col, Rect rect, Vec2 goal, const ResponseFilter &filter) {
            Vec2 bounce = col.touch;
            if (col.move.x != 0.0f or col.move.y != 0.0f) {
                Vec2 bn = goal - col.touch;
                if (col.normal.x == 0.0f)
                    bn.y *= -1;
                else
                    bn.x *= -1;
                bounce = col.touch + bn;
            }
            col.bounce = bounce;
            rect.pos = col.touch;
            goal = bounce;
            auto collisions = project(col.sprite, rect, goal, filter);
            return { goal, collisions };
        }

        void addSpriteToCell(Sprite sprite, IntVec2 pos) {
            auto &cell = cells[pos.y][pos.x];
            cell.pos = pos;
            cell.sprites.emplace(sprite);
        }

        void removeSpriteFromCell(Sprite sprite, IntVec2 pos) {
            eraseByEquivalentKey(cells[pos.y][pos.x].sprites, sprite);
        }

        vector<Sprite> getSpritesInCellRect(IntRect rect) {
            vector<Sprite> sprites;
            for (int y = rect.pos.y; y <= rect.pos.y + rect.size.y - 1; y++)
                for (int x = rect.pos.x; x <= rect.pos.x + rect.size.x - 1; x++)
                    for (auto &cell : cells[y][x].sprites)
                        sprites.push_back(cell.get());
            return sprites;
        }

        vector<Cell *> getCellsTouchedBySegment(LineSeg seg) {
            vector<Cell *> touched;
            traverseGrid(seg, [&](IntVec2 cellPos){
                auto pos = cellPos;
                if (auto row = cells.find(pos.y); row != cells.end())
                    if (auto cell = row->second.find(pos.x); cell != row->second.end())
                        touched.push_back(&cell->second);
            });
            return touched;
        }

        vector<Collision> project(Sprite sprite, Rect rect, Vec2 goal, const ResponseFilter &filter);

        Cranked &cranked;
        unordered_map<int, unordered_map<int, Cell>> cells;
        const array<ResponseFunc, int(ResponseType::Ignore)> responseFunctions{ &Bump::slide, &Bump::touch, &Bump::cross, &Bump::bounce };
    };

}
