#include "simple3d.hpp"
#include <sstream>

class Wnd : public Window {
public:
    Mesh map1;
    std::vector<float> colors;
    Texture map1Texture;
    Particle player;
    RVec playerSize = RVec(0.6f, 1.6f, 0.6f);
    std::vector<RVec> shortestPoints;
    std::vector<RVec> shortestPointsNormals;
    bool physicsSwitch = false;

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

    RVec PointOnPlane(RVec pos, RVec p1, RVec p2, RVec p3) {
        RVec normal = RVec::PlaneNormal(p1, p2, p3);
        RVec point = ((p1 - pos).Cross(normal)).Cross(normal) + p1;
        RVec maxP = RVec(std::max(p1.x, std::max(p2.x, p3.x)), std::max(p1.y, std::max(p2.y, p3.y)), std::max(p1.z, std::max(p2.z, p3.z)));
        RVec minP = RVec(std::min(p1.x, std::min(p2.x, p3.x)), std::min(p1.y, std::min(p2.y, p3.y)), std::min(p1.z, std::min(p2.z, p3.z)));

        if(point.x >= minP.x && point.x <= maxP.x && point.y >= minP.y && point.y <= maxP.y && point.z >= minP.z && point.z <= maxP.z) {
            point.w = 1.0f;
        }
        else {
            point.w = 0.0f;
        }

        return point;
    }

    RVec front, right;

    Framebuffer fb;
    Renderbuffer rb;
    RMat mapTransform = RMat::Identity();

    virtual void Update() override {
        //SetRotation(RVec(Window::sTime));
        SetProjection(ToRadians(90.0f));
        SetView(player.mPosition + front + RVec(0.0f, 0.8f), player.mPosition + RVec(0.0f, 0.8f));
        //UseTexture(&gWhite);
        //RenderCube(RVec(), RVec(0.3f), RVec(1.0f));

        UseTexture(&map1Texture);
        SetRotation(RVec());
        SetScale(RVec(10.0f));
        mapTransform = _gTransform;
        Render(map1.mVertices, colors, map1.mTextureCoords);

        SetRotation(RVec());
        SetScale(RVec(1.0f));
        //UseTexture(fb());
        RenderSquare(RVec(3.0), RVec(10.0), RVec(1.0f));

        glLineWidth(4.0f);
        UseTexture(&gWhite);
        for(uint32_t i = 0; i < shortestPoints.size(); i++) {
            RenderLine(player.mPosition - RVec(0.2f), shortestPoints[i], RVec(0.0f, 1.0f, 0.0f, 1.0f));
            RenderLine(shortestPoints[i], shortestPoints[i] + shortestPointsNormals[i], RVec(1.0f, 1.0f, 0.0f, 1.0f));
            RenderCube(shortestPoints[i], RVec(0.08f), RVec(0.0f, 0.0f, 1.0f, 1.0f));
        }

        RenderLine(RVec(0.0f), RVec::Cross(RVec(-1.0f, 0.0f, 1.0f), RVec(cos(Window::sTime), sin(Window::sTime), 1.0f)).Normalize(), RVec(1.0f));
        RenderLine(RVec(0.0f), RVec(-1.0f, 0.0f, 1.0f), RVec(1.0f));
        RenderLine(RVec(0.0f), RVec(cos(Window::sTime), sin(Window::sTime), 1.0f), RVec(1.0f));

        _gView = RMat::Identity();
        _gTransform = RMat::Identity();
        _gProjection = RMat::Identity();
        RenderText("Pos X:" + std::to_string(player.mPosition.x) + " Y:" + std::to_string(player.mPosition.y) + " Z:" + std::to_string(player.mPosition.z), RVec(-0.9f, 0.9f), 2.0f);
        RenderText("\x1b[00fPhysics: " + (physicsSwitch == true ? std::string("1") : std::string("0")), RVec(0.7f, 0.9f), 2.0f);

        std::stringstream ss;
        for(uint32_t i = 0; i < shortestPoints.size(); i++) {
            if(AABBPointCollider(player.mPosition, playerSize, shortestPoints[i])) {
                ss << shortestPointsNormals[i] << "\n";
            }
        }
        RenderText(ss.str(), RVec(), 2.0f);

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

        shortestPoints.clear();

        RVec normal;

        for(uint32_t i = 0; i < map1.mVertices.size() / 9; i++) {
            RVec pt1 = mapTransform * RVec(map1.mVertices[i * 9 + 0], map1.mVertices[i * 9 + 1], map1.mVertices[i * 9 + 2]);
            RVec pt2 = mapTransform * RVec(map1.mVertices[i * 9 + 3], map1.mVertices[i * 9 + 4], map1.mVertices[i * 9 + 5]);
            RVec pt3 = mapTransform * RVec(map1.mVertices[i * 9 + 6], map1.mVertices[i * 9 + 7], map1.mVertices[i * 9 + 8]);

            /*RVec normal = RVec::PlaneNormal(p1, p2, p3) / RVec::PlaneNormal(p1, p2, p3).Length();
            real distance = abs(player.mPosition.Dot(normal));

            if(distance < shortestDistance) {
                shortestDistance = distance;

                shortestPoint = player.mPosition - (RVec::PlaneNormal(p1, p2, p3) * distance);
            }*/


            RVec point = PointOnPlane(player.mPosition, pt1, pt2, pt3);
            normal = RVec::Cross(pt1 - pt2, pt1 - pt3).Normalize();

            if(point.w == 1.0f) {
                shortestPoints.push_back(point);
                shortestPointsNormals.push_back(normal);
            }
        }

        if(physicsSwitch) {
            for(auto point : shortestPoints) {
                if(AABBPointCollider(player.mPosition, playerSize, point)) {
                    player.ResolveCollision(normal.Negate() * Window::sDeltaTime);
                }
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