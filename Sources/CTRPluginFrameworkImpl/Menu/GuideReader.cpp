#include "CTRPluginFrameworkImpl/Menu/GuideReader.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include <math.h>
namespace CTRPluginFramework
{
    MenuFolderImpl *CreateFolder(std::string path)
    {
        u32                         pos = path.rfind("/");
        std::string                 name = pos != std::string::npos ? path.substr(pos + 1) : path;
        MenuFolderImpl              *mFolder = new MenuFolderImpl(name);
        Directory                   folder;
        std::vector<std::string>    directories;
        std::vector<std::string>    files;

        if (mFolder == nullptr || Directory::Open(folder, path) != 0)
        {
            delete mFolder;
            return (nullptr);
        }

        mFolder->note = path;

        // List all directories
        folder.ListDirectories(directories);
        if (!directories.empty())
        {
            for (int i = 0; i < directories.size(); i++)
            {
                MenuFolderImpl *subMFolder = CreateFolder(path + "/" + directories[i]);
                if (subMFolder != nullptr)
                    mFolder->Append(subMFolder);
            }
        }
        directories.clear();
        // List all files
        folder.ListFiles(files, ".txt");
        if (!files.empty())
        {
            for (int i = 0; i < files.size(); i++)
            {
                u32 fpos = files[i].rfind(".txt");
                std::string fname = fpos != std::string::npos ? files[i].substr(0, fpos) : files[i];
                MenuEntryImpl *entry = new MenuEntryImpl(fname, path);
                mFolder->Append(entry);
            }
        }
        folder.Close();
        return (mFolder);
    }

    #define ABS(x) x >= 0 ? 0 : -x

    float GetRatio(int width, int height)
    {
        if (width >= height)
            return ((float)width / 280.f);
        return ((float)height / 200.f);
    }

    BMPImage *PostProcess(BMPImage *img)
    {
        int width = img->Width();
        int height = img->Height();

        /*if (width <= maxX && height <= maxY)
            return (img);*/

        float ratio = GetRatio(width, height);

        int newWidth = (int)(ceilf((float)width / ratio));
        int newHeight = (int)(ceilf((float)height / ratio));

        BMPImage *temp = new BMPImage(newWidth, newHeight);

        img->Resample(*temp, newWidth, newHeight);
        delete img;

        if (newWidth != 280 || newHeight != 240)
        {
            BMPImage *res = new BMPImage(*temp, 280, 240);
            delete temp;
            return res;
        }        

        return (temp);
    }

    GuideReader::GuideReader(void):
    _isOpen(false), _menu(CreateFolder("Guide"), Icon::DrawFile), _guideTB(nullptr), _text(""), _last(nullptr),
    _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose)
    {
        _isOpen = false;
        _image = nullptr;
        if (Directory::Open(_currentDirectory, "Guide") == 0)
        {
            _currentDirectory.ListFiles(_bmpList, ".bmp");
            if (!_bmpList.empty())
            {
                _currentBMP = 0;
                _image = new BMPImage("Guide/" + _bmpList[0]);
                _image = PostProcess(_image);
            }
            else
                _currentBMP = -1;
        }
        else
        {
            _currentBMP = -1;
        }

    }

    /*
    ** Operator ()
    *********************/
    bool    GuideReader::operator()(EventList &eventList, Time &delta)
    {
        _isOpen = true;
        // Process event
        for (int i = 0; i < eventList.size(); i++)
            _ProcessEvent(eventList[i]);

        // Draw
        Draw();
        return (_closeBtn() || !_isOpen);
    }

    bool    GuideReader::Draw(void)
    {
        if (!_isOpen)
            return (false);

        /*
        ** Top Screen
        **************************************************/
        Renderer::SetTarget(TOP);
        // If a textbox exist
        if (_guideTB != nullptr && _guideTB->IsOpen())
        {
            _guideTB->Draw();
        }
        else
        {
            _menu.Draw();
        }

        /*
        ** Bottom Screen
        **************************************************/
        Renderer::SetTarget(BOTTOM);

        static IntRect  background(20, 20, 280, 200);
        const Color     &black = Color::Black;
        const Color     &blank = Color::Blank;
        const Color     &dimGrey = Color::BlackGrey;

        if (_image != nullptr && _image->IsLoaded())
        {
           /* if (_image->Height() < 200 || _image->Width() < 280)
            {
                Renderer::DrawRect2(background, black, dimGrey);
                Renderer::DrawRect(22, 22, 276, 196, blank, false);                
            }*/
            _image->Draw(background);
        }
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(22, 22, 276, 196, blank, false);
        }

        bool isTouchDown = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());
        _closeBtn.Update(isTouchDown, touchPos);
        _closeBtn.Draw();

        return (true);

    }

    bool    GuideReader::_ProcessEvent(Event &event)
    {
        if (!_isOpen)
            return (false);

        // Process TextBox Event
        if (_guideTB != nullptr && _guideTB->IsOpen())
        {
            _guideTB->ProcessEvent(event);
        }
        else
        {
            MenuItem *item = nullptr;
            // Process Menu Event
            int ret = _menu.ProcessEvent(event, &item);

            // If menu ask for close
            if (ret == MenuEvent::MenuClose)
            {
                Close();
                return (false);
            }

            // If folder changed
            else if (ret == MenuEvent::FolderChanged)
            {
                delete _image;
                _image = nullptr;
                _currentBMP = -1;
                _bmpList.clear();
                _currentDirectory.Close();
                if (item != nullptr && Directory::Open(_currentDirectory, item->note) == 0)
                {
                    _currentDirectory.ListFiles(_bmpList, ".bmp");
                    if (!_bmpList.empty())
                    {
                        _currentBMP = 0;
                        _image = new BMPImage(item->note + "/" + _bmpList[0]);                        
                        _image = PostProcess(_image);
                    }
                }
            }
            // If an file entry was selected by user
            else if (ret == MenuEvent::EntrySelected)
            {
                MenuEntryImpl *entry = (MenuEntryImpl *)item;
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

        // Change image on touch swip
        if (_currentBMP != -1)
        {
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Key::L && _currentBMP > 0)
                {
                    _currentBMP--;
                    delete _image;
                    _image = new BMPImage(_currentDirectory.GetName() + "/" + _bmpList[_currentBMP]);
                    _image = PostProcess(_image);
                }
                else if (event.key.code == Key::R && _currentBMP < _bmpList.size() -1)
                {
                    _currentBMP++;
                    delete _image;
                    _image = new BMPImage(_currentDirectory.GetName() + "/" + _bmpList[_currentBMP]);
                    _image = PostProcess(_image);            
                }
            }
            else if (event.type == Event::TouchSwipped)
            {
                if (event.swip.direction == Event::SwipDirection::Left && _currentBMP > 0)
                {
                    _currentBMP--;
                    delete _image;
                    _image = new BMPImage(_currentDirectory.GetName() + "/" + _bmpList[_currentBMP]);
                    _image = PostProcess(_image);
                }
                else if (event.swip.direction == Event::SwipDirection::Right && _currentBMP < _bmpList.size() -1)
                {
                    _currentBMP++;
                    delete _image;
                    _image = new BMPImage(_currentDirectory.GetName() + "/" + _bmpList[_currentBMP]);
                    _image = PostProcess(_image);           
                }  
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