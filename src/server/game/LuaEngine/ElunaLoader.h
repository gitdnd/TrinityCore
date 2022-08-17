/*
* Copyright (C) 2010 - 2022 Eluna Lua Engine <https://elunaluaengine.github.io/>
* Copyright (C) 2022 - 2022 Hour of Twilight <https://www.houroftwilight.net/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ELUNALOADER_H
#define _ELUNALOADER_H

class ElunaLoader
{
private:
    ElunaLoader();
    ~ElunaLoader();

public:
    ElunaLoader(ElunaLoader const&) = delete;
    ElunaLoader(ElunaLoader&&) = delete;

    ElunaLoader& operator= (ElunaLoader const&) = delete;
    ElunaLoader& operator= (ElunaLoader&&) = delete;
    static ElunaLoader* instance();
    void LoadScripts();
    void ReadFiles(std::string path);

    // Lua script folder path
    std::string lua_folderpath;
    // lua path variable for require() function
    std::string lua_requirepath;

    struct lua_info
    {
        lua_info(const char* content, const char* name, const char* extension, const char* path) {
            this->script_content = content;
            this->script_name = name;
            this->script_extension = extension;
            this->script_path = path;
        }

        const char* script_content;
        const char* script_name;
        const char* script_extension;
        const char* script_path;
    };

    std::vector<lua_info> Scripts;
};

#define sElunaLoader ElunaLoader::instance()

#endif
