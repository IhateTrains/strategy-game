#include <string>
#include "../world.hpp"

void start_client(int argc, char** argv);

#ifdef windows
const char* gettext(const char* str) {
    return str;
}
#endif

#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <atomic>
std::atomic<bool> run;

#ifdef unix
#	include <libintl.h>
#	include <locale.h>
#endif
#include "../path.hpp"
#include "../network.hpp"
#include "../io_impl.hpp"
#include "../actions.hpp"
std::mutex world_lock;

std::string async_get_input(void) {
    std::cout << "server> ";

    std::string cmd;
    std::cin >> cmd;
    return cmd;
}

#include <iostream>
#include <future>

#define _XOPEN_SOURCE 700
#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
int main(int argc, char** argv) {
#ifdef unix
    setlocale(LC_ALL, "");
    bindtextdomain("main", Path::get("locale").c_str());
    textdomain("main");
#endif
    DIR *dir = opendir(Path::get_full().c_str());
    if(dir != NULL) {
        struct dirent *de;
        while(de = readdir(dir)) {
            if(de->d_name[0] == '.')
                continue;
            
            if(de->d_type == DT_DIR) {
                Path::add_path(de->d_name);
            }
        }
    }
    
#ifndef UNIT_TEST
    start_client(argc, argv);
    exit(EXIT_SUCCESS);
#endif
    return 0;
}

#ifdef windows
#include <windows.h>
#include <cstring>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszArgument, int iShow) {
    char* argv[1];
    argv[0] = new char[2];
    strcpy((char*)argv[0], "/");
    
    main(1, argv);
    
    free(argv[0]);
}
#endif