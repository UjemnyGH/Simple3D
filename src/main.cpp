#include "simple3d.hpp"

class Wnd : public Window {
public:
    virtual void Start() override {

    }

    RVec front, right, pos;

    virtual void Update() override {
        SetRotation(RVec(Window::sTime));
        SetProjection(ToRadians(90.0f));
        SetView(pos + front, pos);
        UseTexture(&gWhite);
        RenderCube(RVec(), RVec(0.3f), RVec(1.0f));

        SetRotation(RVec());
        RenderSquare(RVec(3.0), RVec(1.0), RVec(1.0f, 0.0f, 0.0f, 1.0f));
    }

    int lastOffX = 400, lastOffY = 300;
    float camX, camY;

    virtual void LateUpdate() override {
        if(glfwGetKey(this->operator()(), GLFW_KEY_W) == GLFW_PRESS) {
            pos += front * 0.1;
        }
        else if(glfwGetKey(this->operator()(), GLFW_KEY_S) == GLFW_PRESS) {
            pos -= front * 0.1;
        }

        if(glfwGetMouseButton(this->operator()(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
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
            front.Normalize();
            front.w = 0.0f;

            right = front.Cross(RVec(0.0f, 1.0f)).Normalize();
            right.w = 0.0f;
        }
    }

} wnd;

int main() {
    wnd.Run("Window", 800, 600);

    return 0;
}