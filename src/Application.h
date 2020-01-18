#ifndef _WINDOW_H
#define _WINDOW_H


#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLFW/glfw3.h"

#include "glErrors.h"

#include <vector>
#include <functional>

#define CEP_COLOR_LIGHT_GRAY ImVec4(0.33984375f, 0.33984375f, 0.33984375f, 1.0f)

class Application
{
public:
    Application(void);
    ~Application(void);

    void AddWidget(std::function<void()> widgetFunction);
    void Run(void);
    inline float GetWidth(void)
    {
        return m_width;
    }
    inline float GetHeigth(void)
    {
        return m_heigth;
    }
    inline bool GetHasError(void)
    {
        return m_error;
    }

private:
    float m_width = 800.0f;
    float m_heigth = 600.0f;
    bool m_error = true;

    GLFWwindow* m_window = NULL;

    std::vector<std::function<void()>> m_widgets;

    void windowSizeCallback(GLFWwindow* win, int w, int h);

};

#endif
