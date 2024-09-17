#include "simple3d.hpp"

class Wnd : public Window {
public:
    virtual void Start() override {

    }

    RVec front, right, pos;

    Framebuffer fb;

    virtual void Update() override {
        SetRotation(RVec(Window::sTime));
        SetProjection(ToRadians(90.0f));
        SetView(pos + front, pos);
        UseTexture(&gWhite);
        RenderCube(RVec(), RVec(0.3f), RVec(1.0f));

        SetRotation(RVec());
        UseTexture(fb());
        RenderSquare(RVec(3.0), RVec(10.0), RVec(1.0f));

        RenderText("Pos X:" + std::to_string(pos.x) + " Y:" + std::to_string(pos.y) + " Z:" + std::to_string(pos.z), RVec(-0.9f, 0.9f), 2.0f);
        RenderText("Fr X:" + std::to_string(front.x) + " Y:" + std::to_string(front.y) + " Z:" + std::to_string(front.z), RVec(-0.9f, 0.9f - ((1.0f / Window::sHeight) * 40.0)), 2.0f);
        RenderText("Rt X:" + std::to_string(right.x) + " Y:" + std::to_string(right.y) + " Z:" + std::to_string(right.z), RVec(-0.9f, 0.9f - ((1.0f / Window::sHeight) * 80.0)), 2.0f);

        fb.Bind();

        glViewport(0, 0, 16384, 16384);

        glClear(0x4100);
        glClearColor(0.0f, 0.0f, 0.3f, 1.0f);

        SetRotation(RVec(Window::sTime));
        SetProjection(ToRadians(90.0f));
        SetView(pos + front, pos);
        UseTexture(&gWhite);
        RenderCube(RVec(), RVec(0.3f), RVec(1.0f));

        SetRotation(RVec());
        UseTexture(fb());
        RenderSquare(RVec(3.0), RVec(10.0), RVec(1.0f));

        fb.Unbind();

        fb.GetColorTexture(16384, 16384);
        glViewport(0, 0, Window::sWidth, Window::sHeight);
    }

    int lastOffX = 400, lastOffY = 300;
    float camX, camY;

    virtual void LateUpdate() override {
        if(GetKey('W')) {
            pos += front * 0.1;
        }
        else if(GetKey('S')) {
            pos -= front * 0.1;
        }

        if(GetKey('A')) {
            pos.x -= right.x * 0.1;
            pos.z -= right.z * 0.1;
        }
        else if(GetKey('D')) {
            pos.x += right.x * 0.1;
            pos.z += right.z * 0.1;
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
    wnd.Run("Window", 800, 600);

    return 0;
}