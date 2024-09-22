#include "simple3d.hpp"

class Wnd : public Window {
public:
    Mesh map1;
    std::vector<float> colors;
    Texture map1Texture;
    Particle player;
    RVec playerSize = RVec(0.2f, 0.5f, 0.2f);
    std::vector<RVec> shortestPoints;
    bool physicsSwitch = false;
    real shortestDistance = 100000.0f;

    virtual void Start() override {
        map1 = LoadPLYMesh("map1.ply");

        colors.resize((map1.mVertices.size() / 3) * 4);
        std::fill(colors.begin(), colors.end(), 1.0f);

        uint32_t map1TexWidth, map1TexHeight;
        std::vector<uint8_t> devTexture = LoadBitmap("dev128x128.bmp", &map1TexWidth, &map1TexHeight);
        std::vector<uint8_t> map1Pixels;        

        for(int i = 0; i < 32; i++) {
            for(int k = 0; k < 128; k++) {
                for(int j = 0; j < 32; j++) {
                    map1Pixels.insert(map1Pixels.end(), devTexture.begin() + (512 * k), devTexture.begin() + (512 * (k + 1)));
                }
            }
        }

        printf("%ld\n", (long)map1Pixels.size());

        map1TexWidth *= 32;
        map1TexHeight *= 32;
        map1Texture.LoadTextureFromMemory(map1Pixels, map1TexWidth, map1TexHeight);
    }

    RVec PointOnPlane(RVec pos, RVec size, RVec p1, RVec p2, RVec p3) {
        RVec normal = RVec::PlaneNormal(p1, p2, p3);

        RVec distances = RVec(p1.Distance(pos), p2.Distance(pos), p3.Distance(pos));
        RVec triDistances = RVec(p1.Distance(p2), p1.Distance(p3), p2.Distance(p3));

        RVec final = RVec(distances.x / triDistances.x, distances.y / triDistances.y, distances.z / triDistances.z);

        return pos - (final *  normal);
    }

    RVec front, right;

    Framebuffer fb;
    Renderbuffer rb;

    virtual void Update() override {
        SetRotation(RVec(Window::sTime));
        SetProjection(ToRadians(90.0f));
        SetView(player.mPosition + front, player.mPosition);
        UseTexture(&gWhite);
        RenderCube(RVec(), RVec(0.3f), RVec(1.0f));

        UseTexture(&map1Texture);
        SetRotation(RVec());
        SetScale(RVec(10.0f));
        Render(map1.mVertices, colors, map1.mTextureCoords);

        SetRotation(RVec());
        SetScale(RVec(1.0f));
        //UseTexture(fb());
        RenderSquare(RVec(3.0), RVec(10.0), RVec(1.0f));

        glLineWidth(4.0f);
        UseTexture(&gWhite);
        for(auto vec : shortestPoints) {
            RenderLine(player.mPosition - RVec(0.2f), vec, RVec(0.0f, 1.0f, 0.0f, 1.0f));
            RenderCube(vec, RVec(0.02f), RVec(1.0f, 0.0f, 1.0f, 1.0f));
        }

        _gView = RMat::Identity();
        _gTransform = RMat::Identity();
        _gProjection = RMat::Identity();
        RenderText("Pos X:" + std::to_string(player.mPosition.x) + " Y:" + std::to_string(player.mPosition.y) + " Z:" + std::to_string(player.mPosition.z), RVec(-0.9f, 0.9f), 2.0f);
        RenderText("\x1b[00fPhysics: " + (physicsSwitch == true ? std::string("1") : std::string("0")), RVec(0.7f, 0.9f), 2.0f);
        RenderText("\x1b[0f0Distance: " + std::to_string(shortestDistance), RVec(-0.9f, 0.8f), 2.0f);

        /*fb.Bind();

        glViewport(0, 0, 0x800, 0x800);

        glClear(0x4100);

        SetRotation(RVec(Window::sTime));
        SetProjection(ToRadians(90.0f));
        SetView(player.mPosition + front, player.mPosition);
        UseTexture(&gWhite);
        RenderCube(RVec(), RVec(0.3f), RVec(1.0f));

        UseTexture(&map1Texture);
        SetRotation(RVec());
        SetScale(RVec(10.0f));
        Render(map1.mVertices, colors, map1.mTextureCoords);

        SetRotation(RVec());
        SetScale(RVec(1.0f));
        UseTexture(fb());
        RenderSquare(RVec(3.0), RVec(10.0), RVec(1.0f));
        glClearColor(0.0f, 0.0f, 0.3f, 1.0f);

        fb.Unbind();

        rb.CreateStorage(&fb, 0x800, 0x800);
        //fb.GetColorTexture(16384, 16384);
        glViewport(0, 0, Window::sWidth, Window::sHeight);*/
    }

    int lastOffX = 400, lastOffY = 300;
    float camX, camY;

    /*RVec PointOnPlane(RVec pv1, RVec pv2, RVec pv3, RVec point) {
        RVec plane_normal = (pv2 - pv1).Cross(pv3 - pv1).Normalize();
        real point_length = (plane_normal.Dot(point - pv1));
        RVec point_on_plane = point - (plane_normal * point_length);

        return point_on_plane;
    }*/

    virtual void LateUpdate() override {
        if(physicsSwitch) {
            player.Update(Window::sDeltaTime);
        }

        shortestDistance = 100000.0f;

        shortestPoints.clear();

        RVec normal, v1, v2, v3;

        for(uint32_t i = 0; i < map1.mVertices.size() / 9; i++) {
            RVec p1 = _gTransform * RVec(map1.mVertices[i * 9 + 0], map1.mVertices[i * 9 + 1], map1.mVertices[i * 9 + 2]);
            RVec p2 = _gTransform * RVec(map1.mVertices[i * 9 + 3], map1.mVertices[i * 9 + 4], map1.mVertices[i * 9 + 5]);
            RVec p3 = _gTransform * RVec(map1.mVertices[i * 9 + 6], map1.mVertices[i * 9 + 7], map1.mVertices[i * 9 + 8]);

            /*RVec normal = RVec::PlaneNormal(p1, p2, p3) / RVec::PlaneNormal(p1, p2, p3).Length();
            real distance = abs(player.mPosition.Dot(normal));

            if(distance < shortestDistance) {
                shortestDistance = distance;

                shortestPoint = player.mPosition - (RVec::PlaneNormal(p1, p2, p3) * distance);
            }*/

            real distance = abs(RVec::PlaneNormal(p1, p2, p3).Dot(player.mPosition - p1));
            RVec point = PointOnPlane(player.mPosition, playerSize, p1, p2, p3);

            shortestPoints.push_back(point);

            if(distance < shortestDistance) {
                shortestDistance = distance;

                normal = RVec::PlaneNormal(p1, p2, p3);
                v1 = p1;
                v2 = p2;
                v3 = p3;
            }
        }

        if(physicsSwitch) {
            RVec bestPoint = RVec(100000.0f);
            for(auto point : shortestPoints) {
                if(abs(point.Distance(player.mPosition)) < abs(bestPoint.Distance(player.mPosition))) {
                    bestPoint = point;
                }
            }

            if(AABBCollider(player.mPosition, playerSize, bestPoint, RVec(0.05f))) {
                player.ResolveCollision(player.mVelocity.Negate());
            }
        }

        if(physicsSwitch) {
            if(GetKey('W')) {
                player.mVelocity += front * 0.1;
            }
            else if(GetKey('S')) {
                player.mVelocity -= front * 0.1;
            }

            if(GetKey('A')) {
                player.mVelocity.x -= right.x * 0.1;
                player.mVelocity.z -= right.z * 0.1;
            }
            else if(GetKey('D')) {
                player.mVelocity.x += right.x * 0.1;
                player.mVelocity.z += right.z * 0.1;
            }
        }
        else {
            player.mVelocity = RVec();
            if(GetKey('W')) {
                player.mPosition += front * 0.1;
            }
            else if(GetKey('S')) {
                player.mPosition -= front * 0.1;
            }

            if(GetKey('A')) {
                player.mPosition.x -= right.x * 0.1;
                player.mPosition.z -= right.z * 0.1;
            }
            else if(GetKey('D')) {
                player.mPosition.x += right.x * 0.1;
                player.mPosition.z += right.z * 0.1;
            }
        }
        
        if(GetKey('P')) {
            physicsSwitch = !physicsSwitch;
        }

        if(GetMouse(0)) {
            float offX = Window::sMouseX - lastOffX;
            float offY = lastOffY - Window::sMouseY;

            lastOffX = Window::sMouseX;
            lastOffY = Window::sMouseY;

            offX *= 0.005;
            offY *= 0.005;

            camX += offX;
            camY += offY;

            camY = std::clamp(camY, ToRadians(-89.9f), ToRadians(89.9f));

            front.x = cos(camY) * cos(camX);
            front.y = sin(camY);
            front.z = cos(camY) * sin(camX);

            right = front.Normalize().Cross(RVec(0.0f, 1.0f, 0.0f)).Normalize();
            front.Normalize();
            right.w = 0.0f;
            DisableCursor();
        }
        else {
            EnableCursor();
        }
    }

} wnd;

int main() {
    wnd.Run("Window", 800, 600, true);

    return 0;
}