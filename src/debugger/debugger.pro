CONFIG -= link_prl copy_dir_files debug_and_release debug_and_release_target precompile_header
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

win32 {
  QMAKE_EXT_OBJ= .obj
}

# Input
HEADERS += load.h ui.h
FORMS += debugger.ui
SOURCES += load.cpp ui.cpp
