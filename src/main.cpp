#include <iostream>
#include "storage/FileStore.hpp"
using namespace std;





int main()
{
    FileStore ffs;
    User_t usr{};
    usr.username="mohamed";
    ffs.saveUser_t(usr);

}