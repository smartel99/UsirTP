﻿/**
 ******************************************************************************
 * @addtogroup Popup
 * @{
 * @file    Popup
 * @author  Samuel Martel
 * @brief   Header for the Popup module.
 *
 * @date 1/14/2020 10:37:17 AM
 *
 ******************************************************************************
 */
#ifndef _Popup
#define _Popup

/*****************************************************************************/
/* Includes */
#include "vendor/imgui/imgui.h"
#include <iostream>
#include <functional>
#include <vector>

namespace Popup
{
/*****************************************************************************/
/* Exported defines */


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */


/*****************************************************************************/
/* Exported functions */
void Init(std::string name = "Popup", bool showCloseButton = true, ImVec2 size = ImVec2(400, 300));
void Init(std::string name, std::function<void()> onCloseEvent, ImVec2 size = ImVec2(400, 300));
bool IsOpen();
void Render();
void AddCall(std::function<void()> func);
void AddCall(std::function<bool()> func);
void AddCall(std::function<void(std::string&)> func, std::string arg);
void AddCall(std::function<bool(std::string&)> func, std::string arg);
void AddCall(std::function<bool(std::string&)> func, std::string arg, std::function<void()> cb, bool closeOnCb = false);
void AddCall(std::function<void(std::string&, std::string&, bool)>,
             std::string txt,
             std::string arg1,
             bool arg2 = false);

void Text(std::string& txt);
void TextCentered(std::string& txt);
void TextStylized(std::string& txt, std::string& style, bool centered = false);
bool Button(std::string& label);
void SameLine();
}
/* Have a wonderful day :) */
#endif /* _Popup */
/**
 * @}
 */
/****** END OF FILE ******/
