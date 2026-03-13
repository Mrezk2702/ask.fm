#include <iostream>
#include "storage/FileStore.hpp"
#include "filesystem"
#include "models/user.hpp"
#include "submodules/PicaSHA2/picosha2.h"
#include "auth/auth.hpp"
using namespace std;
namespace fs = std::filesystem;

int main()
{
    FileStore ffs(fs::path("data"));
   AuthManager auther(ffs);
   // auther.signup("bob","pass123","bob@email.com");
   std::optional<User_t> test=ffs.loadUser_t("bob");
   User_t tesst{};
   auther.login("bob","pass123");


   
}
