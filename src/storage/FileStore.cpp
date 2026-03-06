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
}
bool FileStore::User_tExists(const string &User_tname)
{
}

string FileStore::saveQuestion(const Question_t &q)
{
}