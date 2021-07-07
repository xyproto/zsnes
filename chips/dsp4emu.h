#ifndef DSP4EMU_H
#define DSP4EMU_H

#include <stdint.h>
#include <stdbool.h>

struct DSP4_t
{
  bool waiting4command;
  bool half_command;
  uint16_t command;
  uint32_t in_count;
  uint32_t in_index;
  uint32_t out_count;
  uint32_t out_index;
  uint8_t parameters[512];
  uint8_t output[512];
};

extern struct DSP4_t DSP4;

struct DSP4_vars_t
{
  // op control
  int8_t DSP4_Logic;            // controls op flow


  // projection format
  int16_t lcv;                  // loop-control variable
  int16_t distance;             // z-position into virtual world
  int16_t raster;               // current raster line
  int16_t segments;             // number of raster lines drawn

  // 1.15.16 or 1.15.0 [sign, integer, fraction]
  int32_t world_x;              // line of x-projection in world
  int32_t world_y;              // line of y-projection in world
  int32_t world_dx;             // projection line x-delta
  int32_t world_dy;             // projection line y-delta
  int16_t world_ddx;            // x-delta increment
  int16_t world_ddy;            // y-delta increment
  int32_t world_xenv;           // world x-shaping factor
  int16_t world_yofs;           // world y-vertical scroll

  int16_t view_x1;              // current viewer-x
  int16_t view_y1;              // current viewer-y
  int16_t view_x2;              // future viewer-x
  int16_t view_y2;              // future viewer-y
  int16_t view_dx;              // view x-delta factor
  int16_t view_dy;              // view y-delta factor
  int16_t view_xofs1;           // current viewer x-vertical scroll
  int16_t view_yofs1;           // current viewer y-vertical scroll
  int16_t view_xofs2;           // future viewer x-vertical scroll
  int16_t view_yofs2;           // future viewer y-vertical scroll
  int16_t view_yofsenv;         // y-scroll shaping factor
  int16_t view_turnoff_x;       // road turnoff data
  int16_t view_turnoff_dx;      // road turnoff delta factor


  // drawing area

  int16_t viewport_cx;          // x-center of viewport window
  int16_t viewport_cy;          // y-center of render window
  int16_t viewport_left;        // x-left of viewport
  int16_t viewport_right;       // x-right of viewport
  int16_t viewport_top;         // y-top of viewport
  int16_t viewport_bottom;      // y-bottom of viewport


  // sprite structure

  int16_t sprite_x;             // projected x-pos of sprite
  int16_t sprite_y;             // projected y-pos of sprite
  int16_t sprite_attr;          // obj attributes
  bool sprite_size;          // sprite size: 8x8 or 16x16
  int16_t sprite_clipy;         // visible line to clip pixels off
  int16_t sprite_count;

  // generic projection variables designed for
  // two solid polygons + two polygon sides

  int16_t poly_clipLf[2][2];    // left clip boundary
  int16_t poly_clipRt[2][2];    // right clip boundary
  int16_t poly_ptr[2][2];       // HDMA structure pointers
  int16_t poly_raster[2][2];    // current raster line below horizon
  int16_t poly_top[2][2];       // top clip boundary
  int16_t poly_bottom[2][2];    // bottom clip boundary
  int16_t poly_cx[2][2];        // center for left/right points
  int16_t poly_start[2];        // current projection points
  int16_t poly_plane[2];        // previous z-plane distance


  // OAM
  int16_t OAM_attr[16];         // OAM (size,MSB) data
  int16_t OAM_index;            // index into OAM table
  int16_t OAM_bits;             // offset into OAM table

  int16_t OAM_RowMax;           // maximum number of tiles per 8 aligned pixels (row)
  int16_t OAM_Row[32];          // current number of tiles per row
};

extern struct DSP4_vars_t DSP4_vars;

#endif
