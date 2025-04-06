#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "pyentry.hpp"
#include "Python.h"
#include "config.hpp"
#ifdef _DEBUG
#define DEBUG(format, ...) std::printf(format, ##__VA_ARGS__);
#else
#define DEBUG(format, ...)
#endif


namespace pyentry {
void PyEntry::_init_pyfunction() {
    if (_pydll) {
        GetProcAddress(_pydll, "Py_Initialize")();
        ((void (*)(const wchar_t*))GetProcAddress(_pydll, "Py_SetPythonHome"))(L"./");
    } else {
        fprintf(stderr, "load dll failed");
        exit(EXIT_FAILURE);
    }
};

int PyEntry::_run_string(const char* s) {
    static auto run = 
        (int (*)(const char*))GetProcAddress(_pydll, "PyRun_SimpleString");
    return run(s);
}

PyEntry::PyEntry(const char* dll_path): _pydll(LoadLibraryA(dll_path ? dll_path : PYTHON_DLL_PATH)), _selfpy(!dll_path) {
    DEBUG("try to load dll -> %s\n", dll_path ? dll_path : PYTHON_DLL_PATH);
    _init_pyfunction();
    if (!_selfpy) {
        _run_string(
            "from sys import path\n"
            "path.append('./runtime/Lib')"
        );
    }
}

PyEntry::~PyEntry() {
    if (_pydll) {
        GetProcAddress(_pydll, "Py_Finalize")();
        FreeLibrary(_pydll);
    }
}

void PyEntry::install_deps(const char* dep_file) {
    if (!dep_file) {
        return;
    }
    if (_selfpy) {
        system(".\\runtime\\python.exe .\\runtime\\get-pip.py");
    }
    char cmd[1000];
    int len = sprintf(
        cmd,
        "from pip._internal.cli.main import main as _main\n"
        "_main(['install', '--no-warn-script-location', '-r', r'%s'])\n",
        dep_file
    );
    assert(len <= 1000 && "command length too long");
    _run_string(cmd);
    _run_string(
        "import site, importlib\n"
        "importlib.reload(site)\n"
    );
}

int PyEntry::run() {
    return _run_string(
        "from pathlib import Path\n"
        "import sys\n"
        "entry_name = Path(sys.executable).stem\n"
        "__import__(entry_name).run()\n"
    );
}
}
