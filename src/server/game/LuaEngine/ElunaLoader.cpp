/*
* Copyright (C) 2010 - 2022 Eluna Lua Engine <https://elunaluaengine.github.io/>
* Copyright (C) 2022 - 2022 Hour of Twilight <https://www.houroftwilight.net/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ElunaLoader.h"
#include "ElunaUtility.h"
#include "ElunaIncludes.h"
#include <filesystem>
#include <boost/filesystem.hpp>

void ElunaLoader::LoadScripts()
{
    lua_folderpath = eConfigMgr->GetStringDefault("Eluna.ScriptPath", "lua_scripts");

    uint32 oldMSTime = ElunaUtil::GetCurrTime();

    // clear script storage
    Scripts.clear();
#ifndef ELUNA_WINDOWS
    if (lua_folderpath[0] == '~')
        if (const char* home = getenv("HOME"))
            lua_folderpath.replace(0, 1, home);
#endif
    ELUNA_LOG_INFO("[Eluna]: Searching scripts from `%s`", lua_folderpath.c_str());
    lua_requirepath.clear();
    ReadFiles(lua_folderpath);
    // Erase last ;
    if (!lua_requirepath.empty())
        lua_requirepath.erase(lua_requirepath.end() - 1);

    ELUNA_LOG_DEBUG("[Eluna]: Loaded %u scripts in %u ms", uint32(Scripts.size()), ElunaUtil::GetTimeDiff(oldMSTime));
}

// Finds lua script files from given path (including subdirectories) and pushes them to scripts
void ElunaLoader::ReadFiles(std::string path)
{
    ELUNA_LOG_DEBUG("[Eluna]: Load script cache from path `%s`", path.c_str());

    boost::filesystem::path someDir(path);
    boost::filesystem::directory_iterator end_iter;

    if (boost::filesystem::exists(someDir) && boost::filesystem::is_directory(someDir))
    {
        lua_requirepath +=
            path + "/?.lua;" +
            path + "/?.ext;";

        for (boost::filesystem::directory_iterator dir_iter(someDir); dir_iter != end_iter; ++dir_iter)
        {
            std::string fullpath = dir_iter->path().generic_string();

            // Check if file is hidden
#ifdef ELUNA_WINDOWS
            DWORD dwAttrib = GetFileAttributes(fullpath.c_str());
            if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_HIDDEN))
                continue;
#else
            std::string name = dir_iter->path().filename().generic_string().c_str();
            if (name[0] == '.')
                continue;
#endif

            // load subfolder
            if (boost::filesystem::is_directory(dir_iter->status()))
            {
                ReadFiles(fullpath);
                continue;
            }

            if (boost::filesystem::is_regular_file(dir_iter->status()))
            {
                // get script name and extension
                std::string scriptname = dir_iter->path().filename().generic_string();
                std::string extension = dir_iter->path().extension().generic_string();

                // open file
                std::ifstream file(fullpath, std::ios::in | std::ios::binary);
                if (!file.is_open())
                    continue;

                // read contents
                std::string content{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

                // push contents to script struct
                Scripts.push_back(lua_info(content.c_str(), scriptname.c_str(), extension.c_str(), fullpath.c_str()));

                // close file
                file.close();
            }
        }
    }
}
