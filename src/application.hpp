#pragma once

#include <string>
#include <memory>
#include "gl/vao.hpp"

class GLFWwindow;

class Application {

private:

    void SetupOpenGL();
    void DoMainLoop();
    void Cleanup();

    void Update_internal(const double delta);

    GLFWwindow* window;
    bool running;

    VAO* vao;


public:

    Application() {}
    ~Application() {}

    void Start();

    int GetWindowWidth();
    int GetWindowHeight();


protected:
    virtual void Render() = 0;
    virtual void Init() = 0;
    virtual void Update(const double delta) = 0;

    void SetViewport();

    int GetKey(int key);

    void SetWindowTitle(const std::string& title);


};
