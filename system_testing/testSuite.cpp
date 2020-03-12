#include <iostream>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

int main(int argc, char *argv[]) {
    DIR* verzeichnis=opendir(getenv("UG4_ROOT")+ '/apps');
    struct dirent *files;
    while(files = readdir(verzeichnis))
    {
      cout<<files->d_name<<endl;
    }
    closedir(verzeichnis);
    return 0;
}
