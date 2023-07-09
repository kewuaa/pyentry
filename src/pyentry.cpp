#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "pyentry.hpp"
#include "Python.h"
#ifdef _DEBUG
#define DEBUG(format, ...) std::printf(format, ##__VA_ARGS__);
#else
#define DEBUG(format, ...)
#endif


namespace pyentry {
void PyEntry::_init_pyfunction() {
    if (_pydll != nullptr) {
        GetProcAddress(_pydll, "Py_Initialize")();
        ((void (*)(const wchar_t*))GetProcAddress(_pydll, "Py_SetPythonHome"))(L"./");
    } else {
        throw std::runtime_error("load dll failed");
    }
};

int PyEntry::_run_string(const char* s) {
    static auto run = 
        (int (*)(const char*))GetProcAddress(_pydll, "PyRun_SimpleString");
    return run(s);
}

PyEntry::PyEntry():
    _pydll(LoadLibraryA("runtime/python38.dll")),
    _selfpy(true)
{
    DEBUG("try to load dll -> runtime/python38.dll\n");
    _init_pyfunction();
}

PyEntry::PyEntry(const char* dll_path): _selfpy(false) {
    DEBUG("try to load dll -> %s\n", dll_path);
    _pydll = LoadLibraryA(dll_path);
    _init_pyfunction();
    _run_string(
        "from sys import path\n"
        "path.append('./runtime/Lib')"
    );
}

PyEntry::~PyEntry() {
    GetProcAddress(_pydll, "Py_Finalize")();
    FreeLibrary(_pydll);
}

int PyEntry::run(const char* init_file) {
    if (init_file) {
        if (_selfpy) {
            system(".\\runtime\\python.exe .\\runtime\\get-pip.py");
        }
        char buffer[1000];
        int len = sprintf(
            buffer,
            "from pip._internal.cli.main import main as _main\n"
            "_main(['install', '--no-warn-script-location', '-r', r'%s'])\n",
            init_file
        );
        std::unique_ptr<char> cmd(new char[len + 1]);
        memcpy(cmd.get(), buffer, sizeof(char) * len);
        cmd.get()[len] = 0;
        _run_string(cmd.get());
        _run_string(
            "import site, importlib\n"
            "importlib.reload(site)\n"
        );
    }
    return _run_string(
        "from pathlib import Path\n"
        "import sys\n"
        "entry_name = Path(sys.executable).stem\n"
        "__import__(entry_name).run()\n"
    );
}
}
