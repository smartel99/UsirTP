﻿#pragma once
#include <windows.h>      // For common windows data types and function headers
#define STRICT_TYPED_ITEMIDS
#include <shlobj.h>
#include <objbase.h>      // For COM headers
#include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#include <shlwapi.h>
#include <knownfolders.h> // for KnownFolder APIs/data types/function headers
#include <propvarutil.h>  // for PROPVAR-related functions
#include <propkey.h>      // for the Property key APIs/data types
#include <propidl.h>      // for the Property System APIs
#include <strsafe.h>      // for StringCchPrintfW
#include <shtypes.h>      // for COMDLG_FILTERSPEC
#include <new>

#include <iostream>

/**
 * @brief   All the possible strings used by the Windows dialog for file types.
 */
const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    {L"Script (*.S)", L"*.S"},
    {L"Python (python.exe)", L"*.exe"},
    {L"Interpreter (main.py)", L"*.py"},
    {L"Log File (*.log)", L"*.log"},
    {L"Comma Separated Values File", L"*.csv"},
    {L"All Documents (*.*)", L"*.*"}
};

// Indices of file types.
/**
 * @enum FileTypeEnum_t
 * @brief   Indexes of each file type in c_rgSaveType.
 */
enum class FileType
{
    INDEX_SCRIPT = 1,   /**< Index of *.S files */
    INDEX_EXE,          /**< Index of *.exe files */
    INDEX_PY,           /**< Index of *.py files */
    INDEX_LOG,          /**< Index of *.log files */
    INDEX_CSV,          /**< Index of *.csv files */
    INDEX_DEFAULT       /**< Index of *.* files */
};

// Controls.
#define CONTROL_GROUP           2000
#define CONTROL_RADIOBUTTONLIST 2
#define CONTROL_RADIOBUTTION1   1
#define CONTROL_RADIOBUTTION2   2

// IDs for the Task Dialog Buttons.
#define IDC_BASICFILEOPEN                       100
#define IDC_ADDITEMSTOCUSTOMPLACES              101
#define IDC_ADDCUSTOMCONTROLS                   102
#define IDC_SETDEFAULTVALUESFORPROPERTIES       103
#define IDC_WRITEPROPERTIESUSINGHANDLERS        104
#define IDC_WRITEPROPERTIESWITHOUTUSINGHANDLERS 105

/* File Dialog Event Handler *************************************************/
/**
 * @class   CDialogEventHandler
 * @brief   Unfortunately for you and me, I took this from an example from
 *          Microsoft without much thoughts about it. So I don't have the link
 *          to said example nor do I remember what this is for... Good luck!
 */
class CDialogEventHandler :
    public IFileDialogEvents,
    public IFileDialogControlEvents
{
public:
    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CDialogEventHandler, IFileDialogEvents),
            QITABENT(CDialogEventHandler, IFileDialogControlEvents),
            { 0 },
#pragma warning(suppress:4838)
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

    // IFileDialogEvents methods
    IFACEMETHODIMP OnFileOk(IFileDialog*)
    {
        return S_OK;
    };
    IFACEMETHODIMP OnFolderChange(IFileDialog*)
    {
        return S_OK;
    };
    IFACEMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*)
    {
        return S_OK;
    };
    IFACEMETHODIMP OnHelp(IFileDialog*)
    {
        return S_OK;
    };
    IFACEMETHODIMP OnSelectionChange(IFileDialog*)
    {
        return S_OK;
    };
// Disable C26818 for `FDS_SHAREVIOLATION_RESPONSE` enum.
#pragma warning(suppress: 26812)
    IFACEMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*)
    {
        return S_OK;
    };
    IFACEMETHODIMP OnTypeChange(IFileDialog* pfd);
// Disable C26818 for `FDE_OVERWRITE_RESPONSE` enum.
#pragma warning(suppress: 26812)
    IFACEMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*)
    {
        return S_OK;
    };

// IFileDialogControlEvents methods
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem);
    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize*, DWORD)
    {
        return S_OK;
    };
    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize*, DWORD, BOOL)
    {
        return S_OK;
    };
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize*, DWORD)
    {
        return S_OK;
    };

    CDialogEventHandler() : _cRef(1)
    {
    };
private:
    ~CDialogEventHandler() = default;;
    long _cRef;
};

HRESULT CDialogEventHandler_CreateInstance(REFIID riid, void** ppv);
