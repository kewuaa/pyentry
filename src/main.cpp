#include "pyentry.hpp"


extern "C" {
int __cdecl init_entry(const char* dll_dir, unsigned int py_version, const char* init_file) {
    if (dll_dir) {
        pyentry::PyEntry pe(dll_dir, py_version);
        return pe.run(init_file);
    } else {
        pyentry::PyEntry pe;
        return pe.run(init_file);
    }
}
}
