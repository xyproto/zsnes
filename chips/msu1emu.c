/*
Copyright (C) 2023 Sneed, ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "msu1emu.h"
#include "../ui.h"
#include "../cfg.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

//Data
u1 MSU_StatusRead;
u4 MSU_Data_Seek;
u1* MSU_DATA = NULL;

//DSP
short* TRACK_DATA = NULL;
u2 MSU_Track;
u1 MSU_MusicVolume;
u1 MSU_CurrentStatus;
int MSU_Rate_Add = 0;
int MSU_Track_Position = 0;
int MSU_Loop_Point = 0;
int MSU_Track_Length = 0;
int MSU_Busy = 0;
char MSU_BasePath[260];

//Prepare read registers
void initMSU1regsRead(void) {
    REGPTR(0x2000) = msustatusread;
    REGPTR(0x2001) = msudataread;
    REGPTR(0x2002) = msuid1;
    REGPTR(0x2003) = msuid2;
    REGPTR(0x2004) = msuid3;
    REGPTR(0x2005) = msuid4;
    REGPTR(0x2006) = msuid5;
    REGPTR(0x2007) = msuid6;
}

//Prepare write registers
void initMSU1regsWrite(void) {
    REGPTW(0x2000) = msudataseek0;
    REGPTW(0x2001) = msudataseek1;
    REGPTW(0x2002) = msudataseek2;
    REGPTW(0x2003) = msudataseek3;
    REGPTW(0x2004) = msu1track0;
    REGPTW(0x2005) = msu1track1;
    REGPTW(0x2006) = msu1volume;
    REGPTW(0x2007) = msu1statecontrol;
}

//Read the MSU binary file (.msu file)
int readMSU() {
    //Cleanup
    if(MSU_DATA) {
        free(MSU_DATA);
        MSU_DATA = NULL;
    }
    MSU_Data_Seek = 0;
    MSU_StatusRead = 2;
    MSU_Busy = 0;
    MSU_MusicVolume = 0xFF;
    
    //Get Filename
    MSU_BasePath[strlen(MSU_BasePath) - 4] = 0;

    char MSUBinaryFile[260];
    sprintf(MSUBinaryFile, "%s.msu", MSU_BasePath);

    //Open binary
    FILE *MSUBinary; long filelen;
    MSUBinary = fopen(MSUBinaryFile, "rb");
    if(MSUBinary) {
#ifdef DEBUG
        printf("Found MSU-1 binary!\n");
#endif
        fseek(MSUBinary, 0, SEEK_END); filelen = ftell(MSUBinary); rewind(MSUBinary);

        MSU_DATA = (u1*)malloc(filelen);
        if(MSU_DATA) {
            fread(MSU_DATA, filelen, 1, MSUBinary);
            fclose(MSUBinary);
            return 1;
        } else {
            printf("Not enough space in memory for MSU-1.\n");
        }
    }
    return 0;
}

void MSU1HandleTrackChange() {
    while(MSU_Busy) { ;; }
    MSU_Track_Position = 0;
    MSU_Rate_Add = 0;

    //Requested a track.
    MSU_Track_Length = 0;
    MSU_StatusRead &= ~0x30; //Turn off playing bits

    //Request new track
#ifdef DEBUG
    printf("Requesting track %hu to be played..\n", MSU_Track);
#endif
    if(TRACK_DATA) {
        free(TRACK_DATA);
        TRACK_DATA = NULL;
    }

    //Print track file
    char MSUTrackFile[260];
    sprintf(MSUTrackFile, "%s-%hu.pcm", MSU_BasePath, MSU_Track);

    //Open track
    FILE *TrackFileReader; long filelen;
    TrackFileReader = fopen(MSUTrackFile, "rb");
    if(TrackFileReader) {
        fseek(TrackFileReader, 0, SEEK_END); filelen = ftell(TrackFileReader);

        //remove header
        filelen -= 8;

        //initialize track data
        TRACK_DATA = (short*)malloc(filelen);
        if(TRACK_DATA) {
            fseek(TrackFileReader, 4, SEEK_SET);
            fread(&MSU_Loop_Point, sizeof(int), 1, TrackFileReader);
            fread(TRACK_DATA, filelen, 1, TrackFileReader);
            fclose(TrackFileReader);
#ifdef DEBUG
            printf("Succesfully loaded Track %lu with length %lu\n", MSU_Track, filelen);
#endif
            MSU_Track_Length = filelen / 2;
        } else {
            printf("Not enough space in memory for MSU-1.\n");
        }
    }
}

//Read from bits
void MSU1GetStatusBitsSpecial() {
    MSU_StatusRead &= 0x37; //Leave revision and playing bits alone
    if(!MSU_Track_Length) { MSU_StatusRead |= 0x8; }
}

//Handle status
void MSU1HandleStatusBits() {
    MSU1GetStatusBitsSpecial();

    //Error reading audio
    if(MSU_StatusRead & 0x8) { return; }
	MSU_StatusRead = (MSU_StatusRead & ~0x30) | ((MSU_CurrentStatus & 0x03) << 4);
#ifdef DEBUG
    printf("Status bits new: %hhu\n", MSU_StatusRead);
#endif
}

//Mix MSU1 audio signal with DSP
void mixMSU1Audio(int* start, int* end, int rate) {
    //Play
    if((MSU_StatusRead & 0x10) && MSU_Track_Length > 0) {
        MSU_Busy = 1;
        //printf("MSU Status: Track: %d   Playing: %d     Repeat: %d    Volume: %d     Pos: %d/%d\n", (int)MSU_Track, MSU_Playing, MSU_Repeat, (int)MSU_MusicVolume, MSU_Track_Position, MSU_Track_Length);
        for (; start < end; start++) {
            //Check if the pointer of the track is valid.
            if(MSU_Track_Position < MSU_Track_Length) {
                *start += (TRACK_DATA[MSU_Track_Position] * MSU_MusicVolume) / 0x80;

                //Stereo Mixer
                if(StereoSound) {
                    *start++; *start += (TRACK_DATA[MSU_Track_Position+1] * MSU_MusicVolume) / 0x80;
                }

                //Take care of incrementing the pointer
                MSU_Rate_Add += 44100;
                while(MSU_Rate_Add >= rate) {
                    MSU_Track_Position += 2;
                    MSU_Rate_Add -= rate;
                }
            }

            //Check if we should repeat
            if(MSU_Track_Position >= MSU_Track_Length) {
                if(MSU_StatusRead & 0x20) { MSU_Track_Position = MSU_Loop_Point * 2; }
            }
        }
        MSU_Busy = 0;
    }
}