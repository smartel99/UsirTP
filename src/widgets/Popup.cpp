#include "Popup.h"
#include "utils/Fonts.h"
#include "widgets/Logger.h"
#include <windows.h>
#include <algorithm>

#pragma region Classes Definitions
enum class FunctionVectorEnum_t
{
    FUNCTION_VOID_VOID,
    FUNCTION_BOOL_VOID,
    FUNCTION_VOID_STRING,
    FUNCTION_BOOL_STRING,
    FUNCTION_BOOL_STRING_CALLBACK,
    FUNCTION_VOID_STRING_STRING_BOOL,
};


class FunctionCallObj
{
public:
    FunctionCallObj(FunctionVectorEnum_t v, size_t i) :funcVec(v), functionIndex(i)
    {
    }

    bool operator==(const FunctionCallObj& other)
    {
        return (funcVec == other.funcVec &&
                functionIndex == other.functionIndex);
    }

    FunctionVectorEnum_t funcVec;
    size_t functionIndex;
};

class PopupCallStack
{
public:
    PopupCallStack(std::string name = "Popup",
                   bool shouldShowCloseButton = true,
                   std::function<void()> onCloseCallback = nullptr) :
        m_name(name + "##"), m_shouldShowCloseButton(shouldShowCloseButton),
        m_onCloseCallback(onCloseCallback), m_callStack(std::vector<FunctionCallObj>())
    {
        m_name += StringUtils::NumToString(ImGui::GetID("SuperGoodSeedForImGuiId"));
    }

    bool operator==(const PopupCallStack& other)
    {
        return (m_callStack[0] == other.m_callStack[0] &&
                m_name == other.m_name &&
                m_shouldShowCloseButton == other.m_shouldShowCloseButton);
    }
    std::vector<FunctionCallObj> m_callStack = std::vector<FunctionCallObj>();
    std::string m_name = "";
    bool m_shouldShowCloseButton = true;
    std::function<void()> m_onCloseCallback = nullptr;
    bool m_shouldBeClosed = false;
};

class VoidStringFunctionObj
{
public:
    VoidStringFunctionObj(std::function<void(std::string&)> f, std::string l) :
        func(f), arg(l)
    {
    }

    std::function<void(std::string&)> func;
    std::string arg;
};

class BoolStringFunctionObj
{
public:
    BoolStringFunctionObj(std::function<bool(std::string&)> f, std::string l) :
        func(f), arg(l)
    {
    }

    std::function<bool(std::string&)> func;
    std::string arg;
};

class BoolStringCallbackFunctionObj
{
public:
    BoolStringCallbackFunctionObj(std::function<bool(std::string&)> f,
                                  std::string l,
                                  std::function<void()> cb,
                                  bool closeOnCallback = true)
        :func(f), label(l), callbackFunc(cb), shouldCloseOnCallback(closeOnCallback)
    {
    }

    std::function <bool(std::string&)> func;
    std::string label;
    std::function<void()> callbackFunc;
    bool shouldCloseOnCallback = true;
};

class VoidStringStringBoolFunctionObj
{
public:
    VoidStringStringBoolFunctionObj(std::function<void(std::string&, std::string&, bool)> f,
                                    std::string s1,
                                    std::string s2,
                                    bool b) :func(f), label(s1), arg(s2), arg2(b)
    {
    }

    std::function<void(std::string&, std::string&, bool)> func;
    std::string label;
    std::string arg;
    bool arg2;
};
#pragma endregion

static void SetTextColor(const std::string& col);


static bool isInit = false;
static bool showCloseButton = true;
static std::string popupName = "";
static std::function<void()> onCloseCallback = nullptr;

static std::vector<std::function<void()>>   functionsVoidVoid;
static std::vector<std::function<bool()>>   functionsBoolVoid;
static std::vector<VoidStringFunctionObj>   functionsVoidString;
static std::vector<BoolStringFunctionObj>   functionsBoolString;
static std::vector<BoolStringCallbackFunctionObj>  functionsBoolStringCallback;
static std::vector<VoidStringStringBoolFunctionObj> functionsVoidStringStringBool;

static std::vector<PopupCallStack>    functionCalls;

static float width = 0.f;
static float height = 0.f;

void Popup::Init(std::string name, bool showButton)
{
    functionCalls.emplace_back(PopupCallStack(name, showButton));
    isInit = true;
}

void Popup::Init(std::string name, std::function<void()> onCloseEvent)
{
    functionCalls.emplace_back(PopupCallStack(name, true, onCloseEvent));
    isInit = true;
}

bool Popup::IsOpen()
{
    return isInit;
}

void Popup::Render()
{
    if (functionCalls.empty())
    {
        return;
    }

    width = float(GetSystemMetrics(SM_CXSCREEN));
    height = float(GetSystemMetrics(SM_CYSCREEN));
    static ImVec2 size = ImVec2(width, height);
    static ImVec2 childPos = ImVec2(size.x / 2.0f, size.y / 2.0f); // Center of window.

    std::vector<PopupCallStack> popups = functionCalls;
    for (auto& popup : popups)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::SetNextWindowSize(size, ImGuiCond_Once);

        ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.5f));
        ImGui::Begin(std::string("popupBg##" + StringUtils::NumToString(ImGui::GetID("SuperGoodSeedForImGuiId"))).c_str(),
                     nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        {
            ImGui::SetNextWindowPos(childPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

            ImGui::PushStyleColor(ImGuiCol_ChildBg, col);
            ImGui::BeginChild(std::string("popupChildBg##" +
                                          StringUtils::NumToString(ImGui::GetID("SuperGoodSeedForImGuiId"))).c_str(),
                              ImVec2(400.f, 300.f),
                              true, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
            {
#pragma region Popup Title
                Fonts::Push("Bold");
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());
                ImGui::Button(popup.m_name.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f));
                ImGui::PopStyleColor(3);
                Fonts::Pop();
                ImGui::Separator();
                ImGui::Spacing();
                ImVec2 separatorLoc = ImGui::GetCursorScreenPos();
                ImVec2 okButtonLoc = ImVec2(separatorLoc.x,
                    (ImGui::GetContentRegionAvail().y + separatorLoc.y) - ImGui::GetFrameHeightWithSpacing());

                ImGui::BeginChildFrame(ImGui::GetID("##PopupChildContent"),
                                       ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() * 1.33f)));
#pragma endregion

#pragma region Popup Content
                for (FunctionCallObj func : popup.m_callStack)
                {
                    switch (func.funcVec)
                    {
                        case FunctionVectorEnum_t::FUNCTION_VOID_VOID:
                            if (functionsVoidVoid[func.functionIndex])
                            {
                                functionsVoidVoid[func.functionIndex]();
                            }
                            break;
                        case FunctionVectorEnum_t::FUNCTION_BOOL_VOID:
                            if (functionsBoolVoid[func.functionIndex])
                            {
                                functionsBoolVoid[func.functionIndex]();
                            }
                            break;
                        case FunctionVectorEnum_t::FUNCTION_VOID_STRING:  // In a scope due to its usage of variables.
                        {
                            if (functionsVoidString[func.functionIndex].func)
                            {
                                functionsVoidString[func.functionIndex].func(functionsVoidString[func.functionIndex].arg);
                            }
                            break;
                        }
                        case FunctionVectorEnum_t::FUNCTION_BOOL_STRING:  // In a scope due to its usage of variables.
                        {
                            std::function<bool(std::string&)> f = functionsBoolString[func.functionIndex].func;
                            std::string arg = functionsBoolString[func.functionIndex].arg;
                            if (f)
                            {
                                f(arg);
                            }
                            break;
                        }
                        case FunctionVectorEnum_t::FUNCTION_BOOL_STRING_CALLBACK:
                        {
                            std::function<bool(std::string&)> f =
                                functionsBoolStringCallback[func.functionIndex].func;
                            std::function<void()> cb =
                                functionsBoolStringCallback[func.functionIndex].callbackFunc;
                            std::string label =
                                functionsBoolStringCallback[func.functionIndex].label;
                            bool shouldClose = functionsBoolStringCallback[func.functionIndex].shouldCloseOnCallback;
                            if (f)
                            {
                                if (f(label) == true)
                                {

                                    cb();
                                    if (shouldClose)
                                    {
                                        popup.m_shouldBeClosed = true;
                                    }
                                }
                            }
                            break;
                        }
                        case FunctionVectorEnum_t::FUNCTION_VOID_STRING_STRING_BOOL:
                        {
                            std::function<void(std::string&, std::string&, bool)> f =
                                functionsVoidStringStringBool[func.functionIndex].func;
                            std::string s1 = functionsVoidStringStringBool[func.functionIndex].label;
                            std::string s2 = functionsVoidStringStringBool[func.functionIndex].arg;
                            bool b1 = functionsVoidStringStringBool[func.functionIndex].arg2;
                            if (f)
                            {
                                f(s1, s2, b1);
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
#pragma endregion

#pragma region Popup Close Button
                ImGui::EndChildFrame();

                ImGui::Separator();
                ImGui::Spacing();

                if (popup.m_shouldShowCloseButton == true)
                {
                    ImGui::Columns(3, "##PopupCloseButtonColumns", false);
                    ImGui::NextColumn();
                    float colWidth = ImGui::GetColumnWidth();
                    if (ImGui::Button("Close##Button", ImVec2(115.f, 0)))
                    {
                        if (popup.m_onCloseCallback)
                        {
                            popup.m_onCloseCallback();
                        }
//                         functionCalls.erase(popup);
                        popup.m_shouldBeClosed = true;
                    }
                    ImGui::NextColumn();
                    ImGui::Columns(1);
                    ImVec2 winSize = ImGui::GetWindowSize();
                    winSize.y += ImGui::GetScrollMaxY();
                }
#pragma endregion
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    // Delete requested pop ups.
    for (auto& popup : popups)
    {
        if (popup.m_shouldBeClosed)
        {
            // Find popup in vector.
            for (auto p = functionCalls.begin(); p != functionCalls.end();)
            {
                if (*p == popup)
                {
                    functionCalls.erase(p);
                }
                else
                {
                    p++;
                }
            }
        }
    }
}

void Popup::AddCall(std::function<void()> func)
{
    functionsVoidVoid.emplace_back(func);
    functionCalls[functionCalls.size() - 1].m_callStack.emplace_back(
        FunctionCallObj(FunctionVectorEnum_t::FUNCTION_VOID_VOID,
                        functionsVoidVoid.size() - 1));
}

void Popup::AddCall(std::function<bool()> func)
{
    functionsBoolVoid.emplace_back(func);
    functionCalls[functionCalls.size() - 1].m_callStack.emplace_back(
        FunctionCallObj(FunctionVectorEnum_t::FUNCTION_BOOL_VOID,
                        functionsBoolVoid.size() - 1));
}

void Popup::AddCall(std::function<void(std::string&)> func, std::string arg)
{
    functionsVoidString.emplace_back(VoidStringFunctionObj(func, arg));
    functionCalls[functionCalls.size() - 1].m_callStack.emplace_back(
        FunctionCallObj(FunctionVectorEnum_t::FUNCTION_VOID_STRING,
                        functionsVoidString.size() - 1));
}

void Popup::AddCall(std::function<bool(std::string&)> func, std::string arg)
{
    functionsBoolString.emplace_back(BoolStringFunctionObj(func, arg));
    functionCalls[functionCalls.size() - 1].m_callStack.emplace_back(
        FunctionCallObj(FunctionVectorEnum_t::FUNCTION_BOOL_STRING,
                        functionsBoolString.size() - 1));
}

void Popup::AddCall(std::function<bool(std::string&)> func, std::string arg, std::function<void()> cb, bool closeOnCb)
{
    functionsBoolStringCallback.emplace_back(BoolStringCallbackFunctionObj(func, arg, cb, closeOnCb));
    functionCalls[functionCalls.size() - 1].m_callStack.emplace_back(
        FunctionCallObj(FunctionVectorEnum_t::FUNCTION_BOOL_STRING_CALLBACK,
                        functionsBoolStringCallback.size() - 1));
}

void Popup::AddCall(std::function<void(std::string&, std::string&, bool)> f,
                    std::string s1,
                    std::string s2,
                    bool b)
{
    functionsVoidStringStringBool.emplace_back(VoidStringStringBoolFunctionObj(f, s1, s2, b));
    functionCalls[functionCalls.size() - 1].m_callStack.emplace_back(
        FunctionCallObj(FunctionVectorEnum_t::FUNCTION_VOID_STRING_STRING_BOOL,
                        functionsVoidStringStringBool.size() - 1));
}

void Popup::Text(std::string& txt)
{
    ImGui::TextWrapped(txt.c_str());
}

void Popup::TextCentered(std::string& txt)
{
    // Render strings with '\n' in them as individual buttons in a way
    // that makes them look like one.
    size_t crPos = txt.find_first_of('\n');
    if (crPos != std::string::npos)
    {
        std::string firstPart = std::string(txt.substr(0, crPos));
        std::string rest = std::string(txt.substr(crPos + 1));
        std::string padding = std::string(" ");
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, -10));
        Popup::TextCentered(firstPart);     // Render the first part.
        Popup::TextCentered(rest);          // Render the rest.
        Popup::TextCentered(padding);
        ImGui::PopStyleVar();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());
        ImGui::PushTextWrapPos();

        ImGui::Button(txt.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f));

        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor(3);
    }
}

void Popup::TextStylized(std::string& txt, std::string& style, bool centered)
{
    std::string font = style;
    std::string color = "";
    if (style.find('/') != std::string::npos)
    {
        font = style.substr(0, style.find('/') - 1);
        color = style.substr(style.find('/') + 1);
    }
    SetTextColor(color);

    Fonts::Push(font);
    if (centered == false)
    {
        Popup::Text(txt);
    }
    else
    {
        Popup::TextCentered(txt);
    }
    Fonts::Pop();
    ImGui::PopStyleColor();
}

bool Popup::Button(std::string& label)
{
    return ImGui::Button(label.c_str());
}

void Popup::SameLine()
{
    ImGui::SameLine();
}

void SetTextColor(const std::string& col)
{
    ImU32 c = 0;
    if (col.empty())
    {
        c = size_t(ImGui::GetColorU32(ImGuiCol_Text));
    }
    else
    {
//         char* end;
//         c = std::strtoul(col.c_str(), &end, 10);
        c = StringUtils::StringToNum<unsigned long>(col);
    }
    ImGui::PushStyleColor(ImGuiCol_Text, ImU32(c));
}
