#include "CTRPluginFrameworkImpl/Menu/PluginMenuImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/GatewayRAMDumper.hpp"
#include <ctime>

#include <algorithm>

namespace CTRPluginFramework
{
    GatewayRAMDumper::GatewayRAMDumper(void) :
    _currentAddress(0),
    _endAddress(0),
    _regionIndex(0),
    _achievedSize(0),
    _totalSize(0)
    {   
    }

    bool    GatewayRAMDumper::operator()(void)
    {
        // Clear variables
        _fileName.clear();
        _file.Seek(0, File::SET);
        _currentAddress = 0;
        _endAddress = 0;
        _regionIndex = 0;
        _achievedSize = 0;
        _totalSize = 0;

        // Get regions list
        PluginMenuImpl::GetRegionsList(_regions);

        // Open the file
        _OpenFile();

        // Check that the file is opened
        if (!_file.IsOpen())
        {
            return (true);
        }

        // Write header
        _WriteHeader();

        // Process every region
        
        for (Region &region : _regions)
        {
            // Set variables
            _currentAddress = region.startAddress;
            _endAddress = region.endAddress;

            u8      *_pool = static_cast<u8 *>(GetPool());

            // Dump region in chunk
            while (_currentAddress < _endAddress)
            {
                u8   *pool = _pool;
                u32  *pool32 =reinterpret_cast<u32*>(_pool);
                u32  size = _endAddress - _currentAddress;
                size = size > 0x60000 ? 0x60000 : size;

                u32  ssize = size;

                // Copy memory to pool
                while (ssize >= 4)
                {
                    *pool32++ = *reinterpret_cast<u32 *>(_currentAddress);
                    _currentAddress += 4;
                    ssize -= 4;
                }
                pool = reinterpret_cast<u8 *>(pool32);
                while (ssize-- > 0)
                    *pool++ = *reinterpret_cast<u8*>(_currentAddress++);

                // Update achieved size
                _achievedSize += size;

                // Write pool to file
                _file.Write(_pool, size);                  

                // Update UI
                _DrawProgress();

                // Check abort key (B)
                Controller::Update();
                if (Controller::IsKeyPressed(B))
                    goto abort;
            }
        }

        // A little message
        MessageBox("Info\n\nDump finished !")();

        _file.Flush();
        _file.Close();
        return (true);

    abort:
        // Close file
        _file.Close();

        // Remove file
        std::string path("Dumps/" + _fileName);
        File::Remove(path);
        return (true);
    }

    void    GatewayRAMDumper::_OpenFile(void)
    {
        char buffer[100] = { 0 };
        time_t t = time(NULL);
        struct tm *timeinfo = localtime(&t);

        strftime(buffer, 100, "%x-%X", timeinfo);

        std::string     timeString(buffer);

        timeString.erase(std::remove(timeString.begin(), timeString.end(), '/'), timeString.end());
        timeString.erase(std::remove(timeString.begin(), timeString.end(), ':'), timeString.end());

        MessageBox      msgBox("Do you want to name the file ?", DialogType::DialogYesNo);

        if (msgBox())
        {
            // Custom name
            Keyboard    keyboard;

            keyboard.DisplayTopScreen = false;
            keyboard.CanAbort(false);

            keyboard.Open(_fileName);
        }
        else
        {
            // Default name
            Process::GetTitleID(_fileName);
        }

        _fileName += "-";
        _fileName += timeString + ".bin";

        std::string path("Dumps");

        // Create Dump directory if doesn't exists
        if (!Directory::IsExists(path))
            Directory::Create(path);

        // Assemble path
        path += "/";
        path += _fileName;

        // Open file        
        if (File::Open(_file, path, File::READ | File::WRITE | File::CREATE))
            MessageBox("Error\n\nCouldn't create: \n" + path)();
    }

#define REGION_INFOS_LENGTH (sizeof(u32) * 3)
    void    GatewayRAMDumper::_WriteHeader(void)
    {
        u32     *p_buffer = static_cast<u32 *>(GetPool());
        u32     *buffer = p_buffer;

        u32     fileOffset = sizeof(u32) * 2 + REGION_INFOS_LENGTH * _regions.size();

        // Write regions count
        *buffer++ = _regions.size();

        // Padding ?
        *buffer++ = 0;
        
        // Wrtie region's infos
        for (Region &region : _regions)
        {
            u32 regionSize = region.endAddress - region.startAddress;

            *buffer++ = region.startAddress;
            *buffer++ = fileOffset;
            *buffer++ = regionSize;

            fileOffset += regionSize;

            // Add region's size to totalSize
            _totalSize += regionSize;

            // Flush region
            svcFlushProcessDataCache(ProcessImpl::_processHandle, (void *)region.startAddress, regionSize);
        }
        
        // Buffer to file
        _file.Write(p_buffer, (u32)buffer - (u32)p_buffer);
    }

    void    GatewayRAMDumper::_DrawProgress()
    {
        Color    &gainsboro = Color::Gainsboro;
        Color    &limegreen = Color::LimeGreen;

        Renderer::SetTarget(TOP);

        Window::TopWindow.Draw("Gateway RAM Dumper");

        int posY = 116;

        // Progressbar
        // Draw border
        IntRect progBarBorder = IntRect(130, posY, 140, 15);
        Renderer::DrawRect(progBarBorder, gainsboro, false);

        float percent = 138.f / 100.f;
        float progress = 100.0f * (float)(_achievedSize) / (float)_totalSize;
        float prog = progress * percent;

        // Draw progress fill
        IntRect progBarFill = IntRect(131, posY + 1, (u32)prog, 13);
        Renderer::DrawRect(progBarFill, limegreen);

        Renderer::EndFrame();
    }
}
