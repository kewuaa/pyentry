#include "pyentry.hpp"


extern "C" {
int __cdecl init_entry(const char* dll_path, const char* init_file) {
    if (dll_path) {
        pyentry::PyEntry pe(dll_path);
        return pe.run(init_file);
    } else {
        pyentry::PyEntry pe;
        return pe.run(init_file);
    }
}
}
