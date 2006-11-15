#include <stdlib.h>
#include <string.h>

/*
Ripped from an Allegro example (exflame.c). :)
Should be fire, but looks more like smoke in ZSNES.
*/

#define MIN(x, y)    \
    (((x) < (y)) ? (x) : (y))

#define ABS(x)      \
    (((x) >= 0) ? (x) : ( - (x)))

#define FIRE_HOTSPOTS   80

static int fire_hotspot [FIRE_HOTSPOTS];
extern char * vidbuffer;

#define SCRW    288
#define SCRH    224

static unsigned char fire_line [SCRW];
static unsigned char fire_buffer [SCRW * SCRH];

static int fire_init_flag;

static void draw_bottom_line_of_fire (void)
{
    int count, count2;

    memset ((& fire_line), 0, SCRW);

    for (count = 0; count < FIRE_HOTSPOTS; count ++)
    {
        for (count2 = (fire_hotspot [count] - 20);
             count2 < (fire_hotspot [count] + 20); count2 ++)
        {
            if ((count2 >= 0) && (count2 < SCRW))
            {
                fire_line [count2] =
                    MIN ((fire_line [count2] + 20) -
                    ABS (fire_hotspot [count] - count2), 256);
            }
        }

        fire_hotspot [count] += ((rand () & 7) - 3);

        if (fire_hotspot [count] < 0)
        {
            fire_hotspot [count] += SCRW;
        }
        else if (fire_hotspot [count] >= SCRW)
        {
            fire_hotspot [count] -= SCRW;
        }
    }

    for (count = 0; count < SCRW; count ++)
    {
        fire_buffer [((SCRH - 1) *
            (SCRW)) + count] = fire_line [count];
    }
}


static void init_fire (void)
{
    int x, y, pixel, count;

    for (count = 0; count < FIRE_HOTSPOTS; count ++)
    {
        fire_hotspot [count] = (rand () % SCRW);
    }


    for (count = 0; count < SCRH; count ++)
    {
        draw_bottom_line_of_fire ();

        for (y = 0; y < (SCRH - 1); y ++)
        {
            for (x = 0; x < SCRW; x ++)
            {
                pixel = fire_buffer [((y + 1) * SCRW) + x];

                if (pixel > 0)
                {
                    pixel --;
                }

                fire_buffer [(y * SCRW) + x] = pixel;
            }
        }
    }

    fire_init_flag = 1;
}


/* void DrawFire (void) */

void DrawSmoke (void)
{
    int x, y, pixel, pixel2;

    if (! fire_init_flag)
    {
        init_fire ();
    }

    draw_bottom_line_of_fire ();

    for (y = 0; y < (SCRH - 1); y ++)
    {
        for (x = 0; x < SCRW; x ++)
        {
            pixel = fire_buffer [((y + 1) * SCRW) + x];

            if (pixel > 0)
            {
                pixel --;
            }

            fire_buffer [(y * SCRW) + x] = pixel;
        }
    }

    for (y = 0; y < SCRH; y ++)
    {
        for (x = 0; x < SCRW; x ++)
        {
            pixel = vidbuffer [(y * SCRW) + x];
            pixel2 = (fire_buffer [(y * SCRW) + x] / 8);

            if (pixel2 > pixel)
            {
                vidbuffer [(y * SCRW) + x] = pixel2;
            }
            else
            {
                vidbuffer [(y * SCRW) + x] =
                    (((pixel + pixel2) / 2) + 1);
            }
        }
    }
}
