#include "FileStore.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "FileLock.hpp"
namespace fs = std::filesystem;
using namespace std;

bool FileStore::saveUser_t(const User_t &u)
{
    fs::path fullpath = fs::path(this->data_dir) / this->user_dir;
    if (!fs::create_directories(fullpath) && !fs::exists(fullpath))
    {
        cerr << "Failed to create directory: " << fullpath << "\n";
        return false;
    }
    fullpath = fullpath / (u.username + ".txt");
    FileLock fLock(fullpath);

    if (!fLock.is_locked())
    {
        return false;
    }

    ofstream outfile(fullpath);

    if (!outfile.is_open())
    {
        cerr << "failed to open " << fullpath << " for writing" << endl;
        return false;
    }
    outfile << "username=" << u.username << "\n";
    outfile << "password_hash=" << u.password_hash << "\n";
    outfile << "email=" << u.email << "\n";
    outfile << "bio=" << u.bio << "\n";
    outfile << "allow_anonymous=" << u.allow_anonymous << "\n";
    outfile << "created_at=" << u.created_at << "\n";
    return outfile.good();
}

optional<User_t> FileStore::loadUser_t(const string &User_tname)
{
    fs::path fullpath = fs::path(this->data_dir) / this->user_dir;
    fullpath = fullpath / (User_tname + ".txt");
    if(!fs::exists(fullpath))
    {
        return std::nullopt;
    }
    ifstream(inFile);
    if (!inFile.is_open())
    {
        std::cerr << "Failed to open " << fullpath << " for reading" << endl;
        return std::nullopt;
    }
    User_t user_data;
    string line;
    while (getline(inFile, line))
    {
        auto pos = line.find('=');
        if (pos == string::npos)
        {
            continue; // no = found skip
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        if (key == "username")
            user_data.username = value;
        else if (key == "password_hash")
            user_data.password_hash = value;
        else if (key == "email")
            user_data.email = value;
        else if (key == "bio")
            user_data.bio = value;
        else if (key == "allow_anonymous")
            user_data.allow_anonymous = (value == "1");
        else if (key == "created_at")
            user_data.created_at = stol(value);
    }

    return user_data;
}
bool FileStore::User_tExists(const string &User_tname)
{
        fs::path fullpath = fs::path(this->data_dir) / this->user_dir;
        fullpath=fullpath/(User_tname+".txt");
        return fs::exists(fullpath);

}

string FileStore::saveQuestion(const Question_t &q)
{
        fs::path fullpath = fs::path(this->data_dir) / this->user_dir;
    if (!fs::create_directories(fullpath) && !fs::exists(fullpath))
    {
        cerr << "Failed to create directory: " << fullpath << "\n";
        return "";
    }
    fullpath = fullpath / ("q_"+q.id + ".txt");
    FileLock fLock(fullpath);

    if (!fLock.is_locked())
    {
        return "";
    }

    ofstream outfile(fullpath);
    if (!outfile.is_open())
    {
        cerr << "failed to open " << fullpath << " for writing" << endl;
        return "";
    }

    outfile<<"id="<<q.id<<"\n";
  


}