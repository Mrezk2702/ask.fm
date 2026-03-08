#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <map>
#include "FileLock.hpp"
#include <sstream>   // ostringstream
#include <iomanip>   // setfill, setw
namespace fs = std::filesystem;

class IDGenerator
{
public:
    explicit IDGenerator(const std::string &meta_filepath)
        : meta_path(meta_filepath) {}

    // returns "0043", "0044", ... or empty string on failure
    std::string nextQuestionID(){return nextID("last_question_id");};
    std::string nextSessionID(){return nextID("last_session_id");};

private:
    fs::path meta_path;

    // core logic — reads, increments, writes, returns new value
    std::string nextID(const std::string &key)
    {
        FileLock lock(meta_path);

        if (!lock.is_locked())
        {
            std::cerr << "IDGenerator: failed to lock meta file\n";
            return "";
        }
        auto data = readMeta();
        data[key] += 1;

        if (!writeMeta(data))
        {
            return "";
        }
        return formatID(data.at(key));
    }

    // reads all key=value pairs from meta file into a map
    std::map<std::string, int> readMeta()
    {
        std::map<std::string, int> ret_map;
        if (!fs::exists(this->meta_path))
        {
            return ret_map;
        }

       std:: ifstream inFile(this->meta_path);

        if (!inFile.is_open())
        {
            std::cerr << "Failed to open " << this->meta_path << " for reading" << endl;
            return ret_map;
        }
        string line;
        while (getline(inFile, line))
        {
            auto pos = line.find('=');
            if (pos == string::npos)
            {
                continue;
            }
            string key = line.substr(0, pos);
            try
            {
                int value = std::stoi(line.substr(pos + 1));
                ret_map[key] = value;
            }
            catch (const std::exception &e)
            {
                std::cerr << "IDGenerator: malformed value for key '" << key << "': " << e.what() << "\n";
            }
        }

        return ret_map;
    }

    // writes the map back to the meta file
    bool writeMeta(const std::map<std::string, int> &data)
    {
        fs::create_directories(this->meta_path.parent_path());

        std::ofstream outfile(this->meta_path);
        if (!outfile.is_open())
        {
            std::cerr << "failed to open " << this->meta_path << " for writing" << "\n";
            return false;
        }
        for (const auto &[key, value] : data)
        {
            outfile << key << "=" << value << "\n";
        }
        return outfile.good();
    }

    // formats int as zero-padded string: 42 → "0042"
    std::string formatID(int id, int width = 4)
    {
        std::ostringstream oss;
        oss  << std::setfill('0') << std::setw(width) << id;
        return oss.str();
    }
};