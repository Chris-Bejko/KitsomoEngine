// PlayerPrefs.h
#pragma once
#include <string>
#include <map>
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"

class PlayerPrefs
{
public:
    static PlayerPrefs& get()
    {
        static PlayerPrefs instance;
        return instance;
    }

    void SetInt(const std::string& key, int val)       { data[key] = val; Save();}
    void SetFloat(const std::string& key, float val)   { data[key] = val; Save();}
    void SetString(const std::string& key, std::string val) { data[key] = val; Save();}

    int         GetInt(const std::string& key, int def = 0)
    { return data.count(key) ? data[key].get<int>() : def; }
    
    float       GetFloat(const std::string& key, float def = 0.f)
    { return data.count(key) ? data[key].get<float>() : def; }
    
    std::string GetString(const std::string& key, std::string def = "")
    { return data.count(key) ? data[key].get<std::string>() : def; }

    bool HasKey(const std::string& key) { return data.count(key) > 0; }
    void DeleteKey(const std::string& key) { data.erase(key); Save(); }
    void DeleteAll() { data.clear(); Save(); }

    void Save()
    {
        std::filesystem::create_directories(GetSavePath().parent_path());
        std::ofstream f(GetSavePath());
        f << data.dump(4);
    }

    void Load()
    {
        auto path = GetSavePath();
        if (!std::filesystem::exists(path)) return;
        std::ifstream f(path);
        try { data = nlohmann::json::parse(f); }
        catch (...) {}
    }

private:
    nlohmann::json data;

    std::filesystem::path GetSavePath()
    {
        #ifdef _WIN32
            // %APPDATA%/ECSEngine/prefs.json
            char* appdata = nullptr;
            size_t len = 0;
            _dupenv_s(&appdata, &len, "APPDATA");
            std::filesystem::path path = appdata;
            free(appdata);
            return path / "ECSEngine" / "prefs.json";

        #elif __APPLE__
            return std::filesystem::path(getenv("HOME")) / 
                   "Library" / "Preferences" / "ECSEngine" / "prefs.json";

        #else // Linux/Android
            return std::filesystem::path(getenv("HOME")) / 
                   ".config" / "ECSEngine" / "prefs.json";
        #endif
    }
};