#ifndef MENU_H
#define MENU_H

#include "../types.h"

extern void breakatsignb();
extern void menudrawbox16b();
extern void menudrawcursor16b();
extern void saveimage();

extern char menudrawbox_string[];
extern char menudrawbox_stringa[];
extern char menudrawbox_stringb[];
extern char menudrawbox_stringc[];
extern char menudrawbox_stringd[];
extern char menudrawbox_stringe[];
extern char menudrawbox_stringf[];
extern char menudrawbox_stringg[];
extern char menudrawbox_stringh[];
extern char menudrawbox_stringi[];
extern u1   SPCSave;
extern u1   menu16btrans;
extern u4   MenuDisplace16;
extern u4   menucloc;

#endif
