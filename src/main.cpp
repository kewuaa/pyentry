#include "pyentry.hpp"


extern "C" {
int init_entry(const char* dll_dir, const char* init_file) {
    if (dll_dir) {
        pyentry::PyEntry pe(dll_dir);
        return pe.run(init_file);
    } else {
        pyentry::PyEntry pe;
        return pe.run(init_file);
    }
}
}
