#include <string>
#include "world.hpp"
#include "lua_api.hpp"

void client_main(int argc, char** argv);

#ifdef windows
const char* gettext(const char* str) {
    return str;
}
#endif

#include <chrono>
#include <thread>

#include <atomic>
std::atomic<bool> run;

#ifdef unix
#	include <libintl.h>
#	include <locale.h>
#endif
#include "path.hpp"
#include "network.hpp"

#include <iostream>
#include <fstream>

#include "io_impl.hpp"

#include "actions.hpp"
std::mutex world_lock;

std::string async_get_input(void) {
    std::cout << "server> ";

    std::string cmd;
    std::cin >> cmd;
    return cmd;
}

#include <iostream>
#include <future>
int main(int argc, char** argv) {
#ifdef unix
    setlocale(LC_ALL, "");
    bindtextdomain("main", Path::get("locale").c_str());
    textdomain("main");
#endif

    // Path::add_path("test");
    Path::add_path("base_game");

#ifndef UNIT_TEST
    // Run as a server for servicing multiple clients
    World* world = new World();
    world->load_mod();
    Server* server = new Server(1836);

    std::future<std::string> future = std::async(std::launch::async, async_get_input);
    run = true;
    while(run) {
        std::unique_lock<std::mutex> lock(world_lock);
        world->do_tick();

        if(future.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
            std::string r = future.get();

            if(r == "brick") {
                std::cout << "Bricking! :D" << std::endl;
                for(auto& province: world->provinces) {
                    for(auto& pop: province->pops) {
                        pop.militancy = 1.f;
                        pop.life_needs_met = 0.f;
                        pop.budget = 0.f;
                    }
                }
                break;
            } else if(r == "exit") {
                std::cout << "Quitting..." << std::endl;
                run = false;
                break;
            } else {
                int ret = luaL_loadstring(world->lua, r.c_str());
                if (ret == 0) {
                    lua_pcall(world->lua, 0, 0, 0);
                } else {
                    print_error("Failed to load lua from input string.");
                }
            }
            future = std::async(std::launch::async, async_get_input);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
        
    print_info("Destroying world");
    delete world;
    print_info("Destroying server");
    delete server;
#else
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