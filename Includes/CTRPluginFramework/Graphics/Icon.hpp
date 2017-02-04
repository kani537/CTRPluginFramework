#ifndef CTRPLUGINFRAMEWORK_ICON_HPP
#define CTRPLUGINFRAMEWORK_ICON_HPP

namespace CTRPluginFramework
{
    class Icon
    {
    public:

        /*
        ** CheckBox 
        ** 15px * 15px
        ************/
        static int      DrawCheckBox(IntVector &pos, bool isChecked);
        static int      DrawCheckBox(int posX, int posY, bool isChecked);

        /*
        ** Close
        ** 20px * 20px
        ************/
        static int      DrawClose(IntVector &pos, bool filled);
        static int      DrawClose(int posX, int posY, bool filled);

        /*
        ** Controller
        ** 15px * 15px
        *************/
        static int      DrawController(int posX, int posY);

        /*
        ** Folder
        ** 15px * 15px
        ************/
        static int      DrawFolder(IntVector &pos);
        static int      DrawFolder(int posX, int posY);

        /*
        ** Favorite
        ** 25px * 25px
        ************/
        static int      DrawAddFavorite(IntVector &pos, bool filled);
        static int      DrawAddFavorite(int posX, int posY, bool filled);
        static int      DrawFavorite(IntVector &pos);
        static int      DrawFavorite(int posX, int posY);

        /*
        ** Info
        ** 25px * 25px
        ***********/
        static int      DrawInfo(IntVector &pos, bool filled);
        static int      DrawInfo(int posX, int posY, bool filled);

        /*
        ** Guide
        ** 15px * 15px
        ***********/
        static int      DrawGuide(IntVector &pos);
        static int      DrawGuide(int posX, int posY);

        /*
        ** Hand Cursor
        ** 15 px * 15 px
        ****************/
        static int      DrawHandCursor(int posX, int posY);
        /*
        ** Search
        ** 15px * 15 px
        **************/
        static int      DrawSearch(int posX, int posY);

        /*
        ** Tools
        ** 15px * 15 px
        *************/
        static int      DrawTools(int posX, int posY);

    private:
        static int      DrawImg(u8 *img, int posX, int posY, int sizeX, int sizeY);
    };
}

#endif