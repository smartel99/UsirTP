#pragma once
#include <iostream>
#include <vector>
#include <fstream>

#include "utils/CDialogEventHandler.h"

class Document
{
public:
    Document(const std::wstring path);
    ~Document(void);

    Document DoOpen(void);
    void DoClose(void);
    void DoForceClose(void);
    void DoSave(void);
    void DisplayContents(void);
    void DisplayContextMenu(void);

    inline void SetOpen(bool state)
    {
        Open = state;
    }

    inline void SetWantClose(bool state)
    {
        m_WantClose = state;
    }

    inline bool IsOpen(void)
    {
        return Open;
    }

    inline bool* GetIsOpen(void)
    {
        return &Open;
    }

    inline bool IsDirty(void)
    {
        return m_Dirty;
    }

    inline bool WantsToClose(void)
    {
        return m_WantClose;
    }

    inline std::string Name(void)
    {
        return m_FileName;
    }

    bool            Open = false;         // Set when the document is open.

private:
    std::wstring    m_FilePath = L"";       // The document's file path.
    std::string     m_FileName = "";        // The document's file name.
    std::vector<std::string > m_FileData;
    bool            m_OpenPrev = false;     // Copy of Open from last update.
    bool            m_Dirty = false;        // Set when the document has been modified.
    bool            m_WantClose = false;    // Set when a request to close the document
                                            // has been made.
};

namespace File
{
    HRESULT OpenFile(std::wstring& filePath, FileTypeEnum_t type = INDEX_DEFAULT, LPCWSTR ext = L".*");
    DWORD GetCurrentPath(std::string& path);
    std::string GetCurrentPath(void);
    std::string GetPathOfFile(const std::string& file);
    bool IsEmpty(std::fstream& pFile);
    std::vector<std::string> GetFilesInDir(const std::string& dirPath);
    void GetFilesInDir(const std::string& dirPath, std::vector<std::string>& files);
}

