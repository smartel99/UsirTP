#include "Login.h"
#include "utils/db/MongoCore.h"
#include "widgets/Popup.h"


static char tmpNameBuf[100] = { 0 };
static char tmpPwdBuf[100] = { 0 };

namespace Login
{
static void HandleUsernameInput();
static void HandlePasswordInput();
static void TryLogin();
static void Cancel();
static void LoginGood();
static void LoginBad();

void Login::OpenPopup()
{
    // Clear buffers.
    Cancel();

    Popup::Init("Login", false);
    Popup::AddCall(HandleUsernameInput);
    Popup::AddCall(HandlePasswordInput);
    Popup::AddCall(Popup::Button, "Login", TryLogin, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "Cancel", Cancel, true);
}


void HandleUsernameInput()
{
    ImGui::InputText("Username", tmpNameBuf, sizeof(tmpNameBuf));
}

void HandlePasswordInput()
{
    ImGui::InputText("Password", tmpPwdBuf, sizeof(tmpPwdBuf), ImGuiInputTextFlags_Password);
}

void TryLogin()
{
    std::string name = tmpNameBuf;
    std::string pwd = tmpPwdBuf;

    memset(tmpNameBuf, 0, sizeof(tmpNameBuf));
    memset(tmpPwdBuf, 0, sizeof(tmpPwdBuf));
    if (!name.empty() && !pwd.empty())
    {
        if (DB::Login(name, pwd) == true)
        {
            LoginGood();
        }
        else
        {
            LoginBad();
        }
    }
}

void Cancel()
{
    memset(tmpNameBuf, 0, sizeof(tmpNameBuf));
    memset(tmpPwdBuf, 0, sizeof(tmpPwdBuf));
}

void LoginGood()
{
    Popup::Init("Login");
    Popup::AddCall(ImGui::Spacing);
    Popup::AddCall(Popup::TextStylized, "Logged in", "Bold", true);
}

void LoginBad()
{
    Popup::Init("Login", std::function<void()>(OpenPopup));
    Popup::AddCall(ImGui::Spacing);
    Popup::AddCall(Popup::TextStylized, "Invalid username and/or password!", "Bold", true);
}
}
