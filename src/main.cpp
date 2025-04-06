#include <fstream>
#include <filesystem>
#include <windows.h>
#include <getopt.h>

#include "pyentry.hpp"
namespace fs = std::filesystem;


int main(int argc, char* argv[]) {
    int c;
    char* dll_path = nullptr;
    bool hide_console_window = true;
    while ((c = getopt(argc, argv, "d:w") != -1)) {
        switch (c) {
            case 'd': {
                dll_path = optarg;
                break;
            }
            case 'w': {
                hide_console_window = false;
                break;
            }
        }
    }

    pyentry::PyEntry entry(dll_path);
    auto flag_file = fs::current_path().concat("/initialized");
    auto deps_file = fs::current_path().concat("/requirements.txt");
    if (!fs::exists(flag_file) && fs::exists(deps_file)) {
        entry.install_deps(deps_file.string().c_str());
        std::ofstream f;
        f.open(flag_file);
    } else {
        if (hide_console_window) {
            auto hwnd = GetConsoleWindow();
            if (hwnd) {
                ShowWindow(hwnd, SW_HIDE);
            }
        }
    }
    return entry.run();
}
