#include "simple3d.hpp"
#include <thread>
#include <sstream>
#include <chrono>

class Portal {
public:
    Transform mTransform;

    Framebuffer mPortalFb;
    Renderbuffer mPortalRb;
    RMat lastView;

    void Bind() {
        lastView = _gView;
        mPortalFb.Bind();
        SetView(mTransform.GetPosition(), mTransform.GetPosition() + (RMat::RotateX(mTransform.GetRotation().x) * RMat::RotateY(mTransform.GetRotation().y) * RMat::RotateZ(mTransform.GetRotation().z) * RVec(0.0f, 0.0f, 1.0f)));
    }

    void MakeTexture(int width, int height) {
        mPortalFb.Unbind();
        mPortalRb.CreateStorage(&mPortalFb, width, height);
        _gView = lastView;
    }

    void Render() {
        RMat lastTf = _gTransform;
        _gTransform = mTransform.GetTransform();

        UseTexture(mPortalFb());
        RenderSquare(RVec(), RVec(1.0f), RVec(1.0f));
        UseTexture(&gWhite);

        _gTransform = lastTf;
    }

    void Teleport(Portal* other, Particle* particle) {
        if(AABBPointCollider(mTransform.GetPosition(), mTransform.GetScale(), particle->mPosition)) {
            particle->mPosition = other->mTransform.GetPosition();
        }
    }
};

class Wnd : public Window {
public:
    Mesh map1;
    std::vector<float> colors;
    Texture map1Texture;
    Particle player;
    RVec playerSize = RVec(1.0f, 2.0f, 1.0f);
    std::vector<RVec> shortestPoints;
    std::vector<RVec> shortestPointsNormals;
    bool physicsSwitch = false;

    bool onGround = false;
    const real playerWalk = 0.5;
    const real playerSprint = 0.75;
    const real playerJumpHeight = 9.8;

    int fixedPerSec = 256;

    void FixedUpdate() {
        while(1) {
            std::chrono::time_point start = std::chrono::high_resolution_clock::now();

            double fixed_dt = 1.0 / (double)fixedPerSec;
            if(physicsSwitch) {
                player.Update(fixed_dt);
            }

            shortestPoints.clear();
            shortestPointsNormals.clear();

            RVec normal;

            for(uint32_t i = 0; i < map1.mVertices.size() / 9; i++) {
                RVec pt1 = mapTransform * RVec(map1.mVertices[i * 9 + 0], map1.mVertices[i * 9 + 1], map1.mVertices[i * 9 + 2]);
                RVec pt2 = mapTransform * RVec(map1.mVertices[i * 9 + 3], map1.mVertices[i * 9 + 4], map1.mVertices[i * 9 + 5]);
                RVec pt3 = mapTransform * RVec(map1.mVertices[i * 9 + 6], map1.mVertices[i * 9 + 7], map1.mVertices[i * 9 + 8]);

                normal = RVec::PlaneNormal(pt1, pt2, pt3);
                RVec point = PointOnPlane(player.mPosition, pt1, pt2, pt3);

                if(point.w == 1.0f) {
                    shortestPoints.push_back(point);
                    shortestPointsNormals.push_back(normal);
                }
            }

            onGround = false;

            if(physicsSwitch) {
                for(uint32_t i = 0; i < shortestPoints.size(); i++) {
                    if(AABBPointCollider(player.mPosition, playerSize, shortestPoints[i])) {
                        if(player.mPosition.y - (playerSize.y / 2.0) > shortestPoints[i].y) {
                            onGround = true;
                        }
                        else {
                            player.mVelocity -= shortestPointsNormals[i];
                        }
                        
                        real px = shortestPoints[i].Distance(player.mPosition + RVec(playerSize.x));
                        real py = shortestPoints[i].Distance(player.mPosition + RVec(playerSize.y));
                        real pz = shortestPoints[i].Distance(player.mPosition + RVec(playerSize.z));
                        real mx = shortestPoints[i].Distance(player.mPosition - RVec(playerSize.x));
                        real my = shortestPoints[i].Distance(player.mPosition - RVec(playerSize.y));
                        real mz = shortestPoints[i].Distance(player.mPosition - RVec(playerSize.z));

                        real min = std::min(px, std::min(py, std::min(pz, std::min(mx, std::min(my, mz)))));

                        player.ResolveCollision(shortestPointsNormals[i].Negate() * RVec(min) * fixed_dt);
                    }
                }
            }

            while(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() < fixed_dt) continue;
        }
    }

    virtual void Start() override {
        player.mFriction = 0.1f;
        player.mHighFriction = 0.2f;

        std::jthread fixedThread(&Wnd::FixedUpdate, this);
        fixedThread.detach();

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

    Portal portal1, portal2;

    virtual void Update() override {
        SetProjection(ToRadians(90.0f));
        SetView(player.mPosition + front + RVec(0.0f, 0.8f), player.mPosition + RVec(0.0f, 0.8f));

        UseTexture(&map1Texture);
        SetRotation(RVec());
        SetScale(RVec(30.0f));
        mapTransform = _gTransform;
        Render(map1.mVertices, colors, map1.mTextureCoords);

        SetScale(RVec(1.0f));


        glLineWidth(4.0f);
        UseTexture(&gWhite);
        for(uint32_t i = 0; i < shortestPoints.size(); i++) {
            //RenderLine(player.mPosition - RVec(0.2f), shortestPoints[i], RVec(0.0f, 1.0f, 0.0f, 1.0f));
            RenderLine(shortestPoints[i], shortestPoints[i] + shortestPointsNormals[i].Negate(), RVec(1.0f, 1.0f, 0.0f, 1.0f));
            RenderCube(shortestPoints[i], RVec(0.08f), RVec(0.0f, 0.0f, 1.0f, 1.0f));
        }

        //_gTransform = RMat::Identity();

        //portal1.mTransform.SetPosition(RVec(-1.0f, 1.0f, -4.0f));
        //portal1.mTransform.SetScale(RVec(2.0f, 2.0f, 0.0f));
        //portal2.mTransform.SetPosition(RVec(-1.0f, 1.0f, 4.0f));
        //portal2.mTransform.SetScale(RVec(2.0f, 2.0f, 0.0f));

        //portal1.Render();
        //portal2.Render();

        //portal1.Teleport(&portal2, &player);
        //portal2.Teleport(&portal1, &player);

        _gView = RMat::Identity();
        _gTransform = RMat::Identity();
        _gProjection = RMat::Identity();
        RenderText("Pos X:" + std::to_string(player.mPosition.x) + " Y:" + std::to_string(player.mPosition.y) + " Z:" + std::to_string(player.mPosition.z), RVec(-0.9f, 0.9f), 2.0f);
        RenderText("\x1b[00fPhysics: " + (physicsSwitch == true ? std::string("1") : std::string("0")), RVec(0.7f, 0.9f), 2.0f);
        RenderText("\x1b[0f0Velocity: " + std::to_string(player.mVelocity.Length()), RVec(-0.9f, 0.8f), 2.0f);
        glPointSize(4.0f);
        RenderPoint(RVec(), RVec(1.0f));
        glPointSize(1.0f);

        /*portal1.Bind();

        glClear(0x4100);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

        SetProjection(ToRadians(90.0f));

        UseTexture(&map1Texture);
        SetRotation(RVec());
        SetScale(RVec(30.0f));
        mapTransform = _gTransform;
        Render(map1.mVertices, colors, map1.mTextureCoords);

        SetScale(RVec(1.0f));

        portal1.MakeTexture(Window::sWidth, Window::sHeight);*/

        /*portal2.Bind();

        UseTexture(&map1Texture);
        SetRotation(RVec());
        SetScale(RVec(30.0f));
        mapTransform = _gTransform;
        Render(map1.mVertices, colors, map1.mTextureCoords);

        SetScale(RVec(1.0f));

        portal2.MakeTexture(Window::sWidth, Window::sHeight);*/
    }

    int lastOffX = 400, lastOffY = 300;
    float camX, camY;

    virtual void LateUpdate() override {
        RVec calc_front = right.Cross(RVec(0.0f, 1.0f)).Normalize().Negate();
        if(physicsSwitch) {
            real speed = playerWalk;

            if(GetKey(GLFW_KEY_LEFT_SHIFT)) {
                speed = playerSprint;
            }

            if(GetKey('W')) {
                player.mVelocity += calc_front * speed;
            }
            else if(GetKey('S')) {
                player.mVelocity -= calc_front * speed;
            }

            if(GetKey('A')) {
                player.mVelocity.x -= right.x * speed;
                player.mVelocity.z -= right.z * speed;
            }
            else if(GetKey('D')) {
                player.mVelocity.x += right.x * speed;
                player.mVelocity.z += right.z * speed;
            }

            if(GetKey(' ') && onGround) {
                player.mVelocity.y += playerJumpHeight;
            }
        }
        else {
            player.mVelocity = RVec();
            if(GetKey('W')) {
                player.mPosition += calc_front * 0.1;
            }
            else if(GetKey('S')) {
                player.mPosition -= calc_front * 0.1;
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