#include "CTRPluginFramework/GuideReader.hpp"
#include "CTRPluginFramework/Rect.hpp"
#include "CTRPluginFramework/Preferences.hpp"

namespace CTRPluginFramework
{
    MenuFolder *CreateFolder(std::string path)
    {
        u32                         pos = path.rfind("/");
        std::string                 name = pos != std::string::npos ? path.substr(pos + 1) : path;
        MenuFolder                  *mFolder = new MenuFolder(name);
        Directory                   folder;
        std::vector<std::string>    directories;
        std::vector<std::string>    files;

        if (Directory::Open(folder, path) != 0)
            return (nullptr);

        // List all directories
        folder.ListFolders(directories);
        if (!directories.empty())
        {
            for (int i = 0; i < directories.size(); i++)
            {
                MenuFolder *subMFolder = CreateFolder(path + "/" + directories[i]);
                if (subMFolder != nullptr)
                    mFolder->Append(subMFolder);
            }
        }

        // List all files
        folder.ListFiles(files, ".txt");
        if (!files.empty())
        {
            for (int i = 0; i < files.size(); i++)
            {
                u32 fpos = files[i].rfind(".txt");
                std::string fname = fpos != std::string::npos ? files[i].substr(0, fpos) : files[i];
                MenuEntry *entry = new MenuEntry(fname, path);
                mFolder->Append(entry);
            }
        }
        return (mFolder);
    }

    GuideReader::GuideReader(void) :
    _text(""), _isOpen(false), _guideTB(nullptr), _last(nullptr), _menu(CreateFolder("Guide"))
    {
        _isOpen = false;
    }


    bool    GuideReader::Draw(void)
    {
        if (!_isOpen)
            return (false);

        /*
        ** Top Screen
        **************************************************/

        // If a textbox exist
        if (_guideTB != nullptr && _guideTB->IsOpen())
        {
            _guideTB->Draw();
        }
        else
        {
            _menu.Draw();
        }
        return (true);

        /*
        ** Bottom Screen
        **************************************************/

        // TODO close button
    }

    bool    GuideReader::ProcessEvent(Event &event)
    {
        if (!_isOpen)
            return (false);

        if (_guideTB != nullptr && _guideTB->IsOpen())
        {
            _guideTB->ProcessEvent(event);
        }
        else
        {
            MenuItem *item;
            int ret = _menu.ProcessEvent(event, &item);

            if (ret == -2)
            {
                Close();
                return (false);
            }

            if (ret >= 0)
            {
                MenuEntry *entry = (MenuEntry *)item;
                if (entry != _last)
                {
                    _last = entry;
                    if (_guideTB != nullptr)
                    {
                        delete _guideTB;
                        _guideTB = nullptr;
                    }

                    File file;
                    if (File::Open(file, entry->note + "/" + entry->name + ".txt") != 0)
                        return (false);

                    u64 size = file.GetSize();
                    _text.clear();

                    char *data = new char[size + 2];
                    memset(data, 0, size + 2);
                    

                    file.Rewind();
                    if (file.Read(data, size) != 0)
                        return (false);

                    _text = data;
                    _text[size] = '\0';

                    delete[] data;
                    IntRect tb = IntRect(30, 20, 340, 200);

                    _guideTB = new TextBox(entry->name, _text, tb);
                    Color limegreen(50, 205, 50);
                    _guideTB->titleColor = limegreen;
                    _guideTB->borderColor = limegreen;
                    _guideTB->Open();
                }
                else
                    _guideTB->Open();
            }
        }
        return (true);
    }

    void    GuideReader::Open(void)
    {
        _isOpen = true;
    }

    void    GuideReader::Close(void)
    {
        _isOpen = false;
    }

    bool    GuideReader::IsOpen(void)
    {
        return (_isOpen);
    }
}