#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define SCREEN_W 1280
#define SCREEN_H 800
#define MAIN_W   320
#define MAIN_H   200
#define SKY_W    480
#define SKY_H    300

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "raylib-scenes");
    SetTargetFPS(60);

    RenderTexture2D mainTarget = LoadRenderTexture(MAIN_W, MAIN_H);
    RenderTexture2D skyTarget  = LoadRenderTexture(SKY_W, SKY_H);
    SetTextureFilter(mainTarget.texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(skyTarget.texture, TEXTURE_FILTER_POINT);

    const char *vsCode =
        "#version 330 core\n"
        "layout(location = 0) in vec2 aPos;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "}\n";

    const char *fsCode =
        "#version 330 core\n"
        "out vec4 fragColor;\n"
        "\n"
        "uniform vec3 uCamPos;\n"
        "uniform vec3 uCamForward;\n"
        "uniform vec3 uCamRight;\n"
        "uniform vec3 uCamUp;\n"
        "uniform vec2 uResolution;\n"
        "uniform float uTime;\n"
        "uniform float uPassType;\n"
        "\n"
        "float sdBox(vec3 p, vec3 b) {\n"
        "    vec3 q = abs(p) - b;\n"
        "    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);\n"
        "}\n"
        "\n"
        "float map(vec3 p) {\n"
        "    float d = 1e9;\n"
        "    if (uPassType < 0.5) {\n"
        "        d = p.y;\n"
        "        d = min(d, sdBox(p - vec3(-5.0, 2.0,  2.0), vec3(1.5, 2.0, 2.0)));\n"
        "        d = min(d, sdBox(p - vec3(-5.5, 3.0, -4.0), vec3(2.0, 3.0, 2.0)));\n"
        "        d = min(d, sdBox(p - vec3(-4.5, 1.5,  7.0), vec3(1.5, 1.5, 2.5)));\n"
        "        d = min(d, sdBox(p - vec3(-5.0, 2.5, -8.0), vec3(2.0, 2.5, 1.5)));\n"
        "        d = min(d, sdBox(p - vec3( 5.0, 1.8, -2.0), vec3(1.5, 1.8, 2.0)));\n"
        "        d = min(d, sdBox(p - vec3( 5.5, 3.5,  4.0), vec3(2.0, 3.5, 2.0)));\n"
        "        d = min(d, sdBox(p - vec3( 4.5, 2.0, -6.0), vec3(1.5, 2.0, 2.5)));\n"
        "        d = min(d, sdBox(p - vec3( 5.0, 1.5,  7.5), vec3(2.0, 1.5, 1.5)));\n"
        "    }\n"
        "    return d;\n"
        "}\n"
        "\n"
        "vec3 calcNormal(vec3 p) {\n"
        "    vec2 e = vec2(0.001, 0.0);\n"
        "    return normalize(vec3(\n"
        "        map(p + e.xyy) - map(p - e.xyy),\n"
        "        map(p + e.yxy) - map(p - e.yxy),\n"
        "        map(p + e.yyx) - map(p - e.yyx)\n"
        "    ));\n"
        "}\n"
        "\n"
        "float hsh(vec2 p) {\n"
        "    p = fract(p * vec2(0.3183, 0.618));\n"
        "    return fract(p.x * p.y * (p.x + p.y) * 17.0);\n"
        "}\n"
        "\n"
        "void main() {\n"
        "    vec2 uv = (gl_FragCoord.xy - 0.5 * uResolution) / (uResolution.y * 0.65);\n"
        "    vec3 ro = uCamPos;\n"
        "\n"
        "    float jx = sin(uTime * 0.7 + gl_FragCoord.y * 0.1) * 0.001;\n"
        "    float jy = cos(uTime * 0.5 + gl_FragCoord.x * 0.1) * 0.001;\n"
        "    vec3 rd = normalize(uCamForward + (uv.x + jx) * uCamRight + (uv.y + jy) * uCamUp);\n"
        "\n"
        "    vec3 col = vec3(0.0);\n"
        "\n"
        "    if (uPassType > 1.5) {\n"
        "        float skyGrad = 0.5 + 0.5 * rd.y;\n"
        "        col = mix(vec3(0.12, 0.09, 0.25), vec3(0.05, 0.03, 0.12), skyGrad);\n"
        "        vec2 sv = floor(rd.xy * 300.0);\n"
        "        float sh = hsh(sv);\n"
        "        float tp = hsh(sv + 100.0);\n"
        "        float twinkle = 0.5 + 0.5 * sin(uTime * 0.5 + tp * 6.283);\n"
        "        twinkle = 0.6 + 0.4 * twinkle;\n"
        "        float star = smoothstep(0.997, 0.998, sh) * twinkle;\n"
        "        vec3 starCol = mix(vec3(0.79, 0.75, 0.80), vec3(1.0), fract(sh * 7.0));\n"
        "        col = mix(col, starCol, star);\n"
        "        vec3 moonDir = normalize(vec3(0.0, 0.2, -1.0));\n"
        "        float core = step(0.997, dot(rd, moonDir));\n"
        "        float pulse = 0.85 + 0.15 * sin(uTime * 0.3);\n"
        "        vec3 sc = vec3(0.95, 0.9, 0.85) * pulse;\n"
        "        col = mix(col, sc, core);\n"
        "        fragColor = vec4(col, 1.0);\n"
        "        return;\n"
        "    }\n"
        "\n"
        "    float t = 0.0;\n"
        "    for (int i = 0; i < 128; i++) {\n"
        "        vec3 p = ro + rd * t;\n"
        "        float d = map(p);\n"
        "        if (d < 0.001) {\n"
        "            vec3 n = calcNormal(p);\n"
        "            vec3 light = normalize(vec3(1.0, 1.5, -1.0));\n"
        "            float dif = max(dot(n, light), 0.0);\n"
        "            float amb = 0.15 + 0.85 * max(dot(n, vec3(0.0, 1.0, 0.0)), 0.0);\n"
        "            if (p.y < 0.005) {\n"
        "                if (abs(p.x) < 2.5) {\n"
        "                    col = vec3(0.12, 0.12, 0.14);\n"
        "                } else {\n"
        "                    vec2 c = floor(p.xz * 2.0);\n"
        "                    float pat = mod(c.x + c.y, 2.0);\n"
        "                    col = mix(vec3(0.25, 0.23, 0.28), vec3(0.15, 0.13, 0.18), pat);\n"
        "                }\n"
        "                col *= dif * 0.7 + amb * 0.5;\n"
        "            } else {\n"
        "                float bh = hsh(floor(p.xz * 4.0 + 0.5));\n"
        "                col = mix(vec3(0.7, 0.5, 0.3), vec3(0.4, 0.5, 0.6), bh);\n"
        "                col *= dif * 0.6 + 0.4;\n"
        "            }\n"
        "            fragColor = vec4(col, 1.0);\n"
        "            return;\n"
        "        }\n"
        "        t += d;\n"
        "        if (t > 200.0) break;\n"
        "    }\n"
        "\n"
        "    fragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
        "}\n";

    Shader shader = LoadShaderFromMemory(vsCode, fsCode);
    int passTypeLoc    = GetShaderLocation(shader, "uPassType");
    int camPosLoc      = GetShaderLocation(shader, "uCamPos");
    int camForwardLoc  = GetShaderLocation(shader, "uCamForward");
    int camRightLoc    = GetShaderLocation(shader, "uCamRight");
    int camUpLoc       = GetShaderLocation(shader, "uCamUp");
    int resolutionLoc  = GetShaderLocation(shader, "uResolution");
    int timeLoc        = GetShaderLocation(shader, "uTime");

    float verts[] = { -1, -1,  1, -1,  1, 1,  -1, -1,  1, 1,  -1, 1 };
    unsigned int vao = rlLoadVertexArray();
    rlEnableVertexArray(vao);
    unsigned int vbo = rlLoadVertexBuffer(verts, sizeof(verts), false);
    rlEnableVertexBuffer(vbo);
    rlSetVertexAttribute(0, 2, RL_FLOAT, false, 0, 0);
    rlEnableVertexAttribute(0);
    rlDisableVertexArray();
    rlDisableVertexBuffer();

    Vector3 playerPos = { 0.0f, 1.6f, 0.0f };
    float speed = 3.0f;
    float yaw = PI;
    float pitch = -0.05f;
    float mouseSens = 0.0006f;

    DisableCursor();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 md = GetMouseDelta();
        yaw -= md.x * mouseSens;
        pitch -= md.y * mouseSens;
        if (pitch > PI * 0.49f) pitch = PI * 0.49f;
        if (pitch < -PI * 0.49f) pitch = -PI * 0.49f;

        Vector3 fwd = { sinf(yaw), 0.0f, cosf(yaw) };
        Vector3 sid = { cosf(yaw), 0.0f, -sinf(yaw) };
        Vector3 move = { 0 };
        if (IsKeyDown(KEY_W)) { move.x += fwd.x; move.z += fwd.z; }
        if (IsKeyDown(KEY_S)) { move.x -= fwd.x; move.z -= fwd.z; }
        if (IsKeyDown(KEY_A)) { move.x += sid.x; move.z += sid.z; }
        if (IsKeyDown(KEY_D)) { move.x -= sid.x; move.z -= sid.z; }
        float len = sqrtf(move.x * move.x + move.z * move.z);
        if (len > 0.0f) { move.x /= len; move.z /= len; }
        playerPos.x += move.x * speed * dt;
        playerPos.z += move.z * speed * dt;

        Vector3 camPos = playerPos;
        Vector3 camTarget = {
            playerPos.x + sinf(yaw) * cosf(pitch),
            playerPos.y + sinf(pitch),
            playerPos.z + cosf(yaw) * cosf(pitch)
        };
        Vector3 forward = Vector3Normalize(Vector3Subtract(camTarget, camPos));
        Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, worldUp));
        Vector3 up = Vector3CrossProduct(right, forward);

        float time = (float)GetTime();

        #define RENDER_PASS(target, type, w, h) do { \
            float res[2] = { w, h }; \
            float pt = (float)(type); \
            BeginTextureMode(target); \
            ClearBackground((Color){ 0, 0, 0, 0 }); \
            rlEnableShader(shader.id); \
            rlSetUniform(passTypeLoc,   &pt,      RL_SHADER_UNIFORM_FLOAT, 1); \
            rlSetUniform(camPosLoc,     &camPos,  RL_SHADER_UNIFORM_VEC3, 1); \
            rlSetUniform(camForwardLoc, &forward, RL_SHADER_UNIFORM_VEC3, 1); \
            rlSetUniform(camRightLoc,   &right,   RL_SHADER_UNIFORM_VEC3, 1); \
            rlSetUniform(camUpLoc,      &up,      RL_SHADER_UNIFORM_VEC3, 1); \
            rlSetUniform(resolutionLoc, res,      RL_SHADER_UNIFORM_VEC2, 1); \
            rlSetUniform(timeLoc,       &time,    RL_SHADER_UNIFORM_FLOAT, 1); \
            rlEnableVertexArray(vao); \
            rlDrawVertexArray(0, 6); \
            rlDisableVertexArray(); \
            rlDisableShader(); \
            EndTextureMode(); \
        } while(0)

        RENDER_PASS(skyTarget,  2, SKY_W, SKY_H);
        RENDER_PASS(mainTarget, 0, MAIN_W, MAIN_H);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(skyTarget.texture,
            (Rectangle){ 0, 0, SKY_W, -SKY_H },
            (Rectangle){ 0, 0, SCREEN_W, SCREEN_H },
            (Vector2){ 0 }, 0.0f, WHITE);
        DrawTexturePro(mainTarget.texture,
            (Rectangle){ 0, 0, MAIN_W, -MAIN_H },
            (Rectangle){ 0, 0, SCREEN_W, SCREEN_H },
            (Vector2){ 0 }, 0.0f, WHITE);
        DrawFPS(10, 10);
        EndDrawing();
    }

    rlUnloadVertexArray(vao);
    rlUnloadVertexBuffer(vbo);
    UnloadShader(shader);
    UnloadRenderTexture(mainTarget);
    UnloadRenderTexture(skyTarget);
    CloseWindow();

    return 0;
}
