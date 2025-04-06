#pragma once
#include <windows.h>


namespace pyentry {
class PyEntry {
private:
    HMODULE _pydll;
    bool _selfpy;
    void _init_pyfunction();
    int _run_string(const char* s);
public:
    PyEntry(const char* dll_path);
    ~PyEntry();
    void install_deps(const char* dep_file);
    int run();
};
}
