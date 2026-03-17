#include "FileStore.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "FileLock.hpp"
#include "id_generator.hpp"
#include <ctime>
namespace fs = std::filesystem;
using namespace std;

FileStore::FileStore(const fs::path &data_dir)
    : data_dir(data_dir),
      id_gen(data_dir / "meta.txt") // ← IDGenerator gets its path here
{
    // create base directories on startup
    fs::create_directories(data_dir / user_dir);
    fs::create_directories(data_dir / question_dir);
    fs::create_directories(data_dir / sessions_dir);
}
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
    outfile << "salt=" << u.salt << "\n";
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
    if (!fs::exists(fullpath))
    {
        return std::nullopt;
    }
    ifstream inFile(fullpath);
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
        else if (key == "salt")
        {
            user_data.salt = value;
        }
    }

    return user_data;
}
bool FileStore::userExists(const string &User_tname)
{
    fs::path fullpath = fs::path(this->data_dir) / this->user_dir;
    fullpath = fullpath / (User_tname + ".txt");
    return fs::exists(fullpath);
}

string FileStore::saveQuestion(const Question_t &q)
{
    fs::path fullpath = fs::path(this->data_dir) / this->question_dir;
    if (!fs::create_directories(fullpath) && !fs::exists(fullpath))
    {
        cerr << "Failed to create directory: " << fullpath << "\n";
        return "";
    }
    fullpath = fullpath / ("q_" + q.id + ".txt");
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

    Question_t q_to_save = q;
    q_to_save.id = id_gen.nextQuestionID();
    outfile << "id=" << q.id << "\n";
    outfile << "from=" << q.from_user << "\n";
    outfile << "to=" << q.to_user << "\n";
    outfile << "anonymous=" << q.is_anonymous << "\n";
    outfile << "body=" << q.body << "\n";
    outfile << "answer=" << q.answer << "\n";
    outfile << "parent_id=" << q.parent_id << "\n";

    outfile << "children=";
    for (auto child_it = q.children_ids.begin(); child_it < q.children_ids.end(); child_it++)
    {
        outfile << *child_it;
        if (child_it != q.children_ids.end() - 1)
        {
            outfile << ",";
        }
    }
    outfile << "\n";
    outfile << "created_at=" << q.created_at << "\n";
    outfile << "answered_at=" << q.answered_at << "\n";
    return q.id;
}

bool FileStore::saveSession(const string &token, const string &username)
{
    fs::path fullPath = fs::path(this->data_dir) / this->sessions_dir;
    if (!fs::create_directories(fullPath) && !fs::exists(fullPath))
    {
        cerr << "Failed to create directory: " << fullPath << "\n";
        return false;
    }
    fullPath = fullPath / ("sess_" + token + ".txt");
    FileLock flock(fullPath);

    if (!flock.is_locked())
    {
        return false;
    }
    ofstream outfile(fullPath);
    if (!outfile.is_open())
    {
        cerr << "failed to open " << fullPath << " for writing" << "\n";
        return false;
    }
    time_t time_val = time(nullptr);
    outfile << "token=" << token << "\n";
    outfile << "username=" << username << "\n";
    outfile << "created_at=" << time_val << "\n";
    outfile << "expires_at=" << time_val + day_in_seconds << "\n"; // value of a 24h in seconds
    return outfile.good();
}

optional<Session_t> FileStore::loadSession(const string &token)
{
    fs::path fullpath = fs::path(this->data_dir) / this->sessions_dir;
    fullpath = fullpath / ("sess_" + token + ".txt");
    if (!fs::exists(fullpath))
    {
        return std::nullopt;
    }
    ifstream inFile(fullpath);
    if (!inFile.is_open())
    {
        std::cerr << "Failed to open " << fullpath << " for reading\n";
        return std::nullopt;
    }
    Session_t session_data;
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
        if (key == "token")
            session_data.token = value;
        else if (key == "username")
            session_data.username = value;
        else if (key == "created_at")
        {
            try
            {

                session_data.created_at = stol(value);
            }
            catch (const std::exception &e)
            {
                std::cerr << "loadSession: malformed created_at\n";
            }
        }
        else if (key == "expires_at")
        {
            try
            {
                session_data.expires_at = stol(value);
            }
            catch (const std::exception &e)
            {
                std::cerr << "loadSession: malformed expired_at\n";
            }
        }
    }

    return session_data;
}

bool FileStore::deleteSession(const string &token)
{
    fs::path fullpath = fs::path(this->data_dir) / this->sessions_dir;
    fullpath = fullpath / ("sess_" + token + ".txt");
    if (!fs::exists(fullpath))
    {
        cerr<<"token invalid\n";
        return false;
    }
    return  fs::remove(fullpath);

}