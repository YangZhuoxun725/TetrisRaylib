// Demo: click to drop boxes into a Box2D world.
//   raylib  -> window, input, drawing
//   raygui  -> sliders + reset button
//   Box2D   -> physics (v3 C API)
//   EnTT    -> entities/components
//   glm     -> vector math
//   fmt     -> logging + text formatting
//   json    -> persisted settings (settings.json)

#include "pch.hpp"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

// ---------------------------------------------------------------- settings ---
struct Settings {
    float gravity = 10.0f;  // m/s^2 (positive = down, since screen Y points down)
    float boxSize = 0.4f;   // half-extent in meters
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, gravity, boxSize)

static Settings LoadSettings(const std::string& path) {
    Settings s;
    std::ifstream file(path);
    if (!file) {
        fmt::print("[settings] {} not found, using defaults\n", path);
        return s;
    }
    try {
        s = nlohmann::json::parse(file).get<Settings>();
        fmt::print("[settings] loaded {} (gravity={:.1f}, boxSize={:.2f})\n",
            path, s.gravity, s.boxSize);
    }
    catch (const std::exception& e) {
        fmt::print(stderr, "[settings] failed to parse {}: {}\n", path, e.what());
    }
    return s;
}

static void SaveSettings(const std::string& path, const Settings& s) {
    nlohmann::json j = s;
    std::ofstream(path) << j.dump(2);
    fmt::print("[settings] saved {}\n", path);
}

// -------------------------------------------------------------- components ---
struct PhysicsBody { b2BodyId id; };
struct BoxShape { glm::vec2 halfSize; };  // meters
struct Tint { Color color; };
struct Dynamic {};                       // tag: removed on reset

// -------------------------------------------------------------------- main ---
int main() {
    constexpr int   kWidth = 1280;
    constexpr int   kHeight = 720;
    constexpr float kPPM = 50.0f;  // pixels per meter
    constexpr float kWorldW = kWidth / kPPM;
    constexpr float kWorldH = kHeight / kPPM;

    InitWindow(kWidth, kHeight, "MyGame - raylib + Box2D + EnTT");
    SetTargetFPS(60);

    Settings settings = LoadSettings("settings.json");

    // Physics world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{ 0.0f, settings.gravity };
    b2WorldId world = b2CreateWorld(&worldDef);
    float appliedGravity = settings.gravity;

    // ECS registry
    entt::registry registry;

    // Helper: create a physics box + the ECS entity that owns it
    auto makeBox = [&](glm::vec2 pos, glm::vec2 half, Color color, bool dynamic) {
        b2BodyDef bd = b2DefaultBodyDef();
        bd.type = dynamic ? b2_dynamicBody : b2_staticBody;
        bd.position = b2Vec2{ pos.x, pos.y };
        b2BodyId body = b2CreateBody(world, &bd);

        b2Polygon poly = b2MakeBox(half.x, half.y);
        b2ShapeDef sd = b2DefaultShapeDef();
        sd.density = 1.0f;
        b2CreatePolygonShape(body, &sd, &poly);

        entt::entity e = registry.create();
        registry.emplace<PhysicsBody>(e, PhysicsBody{ body });
        registry.emplace<BoxShape>(e, BoxShape{ half });
        registry.emplace<Tint>(e, Tint{ color });
        if (dynamic) registry.emplace<Dynamic>(e);
        return e;
        };

    // Static level geometry: floor + two walls
    const Color wallColor{ 90, 90, 110, 255 };
    makeBox({ kWorldW * 0.5f, kWorldH - 0.25f }, { kWorldW * 0.5f, 0.25f }, wallColor, false);
    makeBox({ 0.25f, kWorldH * 0.5f }, { 0.25f, kWorldH * 0.5f }, wallColor, false);
    makeBox({ kWorldW - 0.25f, kWorldH * 0.5f }, { 0.25f, kWorldH * 0.5f }, wallColor, false);

    const Rectangle panel{ 10, 10, 260, 140 };

    auto resetBoxes = [&] {
        std::vector<entt::entity> doomed;
        for (auto [e, body] : registry.view<PhysicsBody, Dynamic>().each()) {
            b2DestroyBody(body.id);
            doomed.push_back(e);
        }
        for (auto e : doomed) registry.destroy(e);
        fmt::print("[game] reset, removed {} boxes\n", doomed.size());
        };

    while (!WindowShouldClose()) {
        // ---- input ----
        const Vector2 mouse = GetMousePosition();
        const glm::vec2 mouseWorld = glm::vec2(mouse.x, mouse.y) / kPPM;
        const bool overGui = CheckCollisionPointRec(mouse, panel);

        if (!overGui && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Color c{ static_cast<unsigned char>(GetRandomValue(80, 255)),
                    static_cast<unsigned char>(GetRandomValue(80, 255)),
                    static_cast<unsigned char>(GetRandomValue(80, 255)), 255 };
            makeBox(mouseWorld, glm::vec2(settings.boxSize), c, true);
            fmt::print("[game] spawned box at ({:.2f}, {:.2f}) m\n", mouseWorld.x, mouseWorld.y);
        }
        if (IsKeyPressed(KEY_R)) resetBoxes();

        // ---- update ----
        if (settings.gravity != appliedGravity) {
            b2World_SetGravity(world, b2Vec2{ 0.0f, settings.gravity });
            appliedGravity = settings.gravity;
        }
        b2World_Step(world, 1.0f / 60.0f, 4);

        // ---- draw ----
        BeginDrawing();
        ClearBackground(Color{ 30, 30, 40, 255 });

        for (auto [e, body, shape, tint] : registry.view<PhysicsBody, BoxShape, Tint>().each()) {
            const b2Vec2 p = b2Body_GetPosition(body.id);
            const float  angle = b2Rot_GetAngle(b2Body_GetRotation(body.id)) * RAD2DEG;
            const float  w = shape.halfSize.x * 2.0f * kPPM;
            const float  h = shape.halfSize.y * 2.0f * kPPM;
            DrawRectanglePro(Rectangle{ p.x * kPPM, p.y * kPPM, w, h },
                Vector2{ w * 0.5f, h * 0.5f }, angle, tint.color);
        }

        // raygui controls
        GuiPanel(panel, "Controls");
        GuiSliderBar(Rectangle{ 90, 40, 150, 20 }, "Gravity", nullptr, &settings.gravity, 0.0f, 30.0f);
        GuiSliderBar(Rectangle{ 90, 70, 150, 20 }, "Box size", nullptr, &settings.boxSize, 0.1f, 1.0f);
        if (GuiButton(Rectangle{ 20, 105, 220, 28 }, "Reset (R)")) resetBoxes();

        // fmt-formatted HUD
        const std::size_t boxCount = registry.storage<Dynamic>().size();
        const std::string hud = fmt::format("Boxes: {}   FPS: {}   Mouse: ({:.1f}, {:.1f}) m",
            boxCount, GetFPS(), mouseWorld.x, mouseWorld.y);
        DrawText(hud.c_str(), 290, 14, 20, RAYWHITE);
        DrawText("Left-click to drop a box", 290, 40, 16, GRAY);

        EndDrawing();
    }

    SaveSettings("settings.json", settings);
    b2DestroyWorld(world);
    CloseWindow();
    return 0;
}