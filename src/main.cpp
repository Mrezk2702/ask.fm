#include <iostream>
#include "storage/FileStore.hpp"
#include "filesystem"
#include "models/user.hpp"
using namespace std;
namespace fs = std::filesystem;

int main()
{
    FileStore ffs(fs::path("data"));
    User_t usr{};
    usr.username = "mohamed";
    std::optional<User_t> test = ffs.loadUser_t("bob");
    if (test != std::nullopt)
    {
        User_t loaded = test.value();

        std::cout << "User loaded successfully!\n";
        std::cout << "Username: " << loaded.username << "\n";
        std::cout << "Email: " << loaded.email << "\n";
        std::cout << "Bio: " << loaded.bio << "\n";
        std::cout << "Allow anonymous: " << loaded.allow_anonymous << "\n";
        std::cout << "Created at: " << loaded.created_at << "\n";
    }
    else
    {
         std::cout << "User not found\n";
    }
}