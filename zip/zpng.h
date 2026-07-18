#ifndef ZPNG_H
#define ZPNG_H

#ifndef NO_PNG
void Grab_PNG_Data(void);
#endif
void Grab_BMP_Data(void);
void Grab_ASCII_Data_Path(const char* path);
void Grab_Frame_Hash_Path(const char* path);
void Grab_BMP_Data_8(void);

#endif
