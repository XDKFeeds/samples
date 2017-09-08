/*

!==============================================================
!
! SAMPLE SOURCE CODE - SUBJECT TO THE TERMS OF SAMPLE CODE LICENSE AGREEMENT,
! http://software.intel.com/en-us/articles/intel-sample-source-code-license-agreement/
!
! Copyright (C) Intel Corporation
!
! THIS FILE IS PROVIDED "AS IS" WITH NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT
! NOT LIMITED TO ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
! PURPOSE, NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS.
!
! =============================================================
*/

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <cassert>
#if _WIN32 || _WIN64
#include <windows.h>
//This works around an issue that prevents this Sample from compiling with specific Microsoft SDKs
//Note: The best Solution would be to use the latest Visual Studio headers from Microsoft
//Note: This fixes A problem with MS Vista SDK - but breaks standard VS2008 x64 builds.
//#define _interlockedbittestandset(x, y) _interlockedbittestandset(LONG *, y)
//#define _interlockedbittestandreset(x, y) _interlockedbittestandreset(LONG *, y)
//#define _interlockedbittestandset64(x, y) _interlockedbittestandset64(LONG64 *, y)
//#define _interlockedbittestandreset64(x, y) _interlockedbittestandreset64(LONG64 *, y)
#else
#include <unistd.h>
#endif

typedef unsigned int color_t;
typedef unsigned char colorcomp_t;

//! Sipmle proxy class for managing of different video systems
class video
{
    //! colorspace information
    char depth, red_shift, green_shift, blue_shift;
    color_t red_mask, green_mask, blue_mask;
    friend class drawing_area;

public:
    //! Constructor
    video();
    //! Destructor
    ~video();
    //! member to set window name
    const char *title;
    //! true is enable to show fps
    bool calc_fps;
    //! if true: on windows fork processing thread for on_process(), on non-windows note that next_frame() is called concurrently.
    bool threaded;
    //! true while running within main_loop()
    bool running;
    //! if true, do gui updating
    bool updating;
    //! initialize graphical video system
    bool init_window(int sizex, int sizey);
    //! initialize console. returns true if console is available
    bool init_console();
    //! terminate video system
    void terminate();
    //! Do standard event & processing loop. Use threaded = true to separate event/updating loop from frame processing
    void main_loop();
    //! Process next frame
    bool next_frame();
    //! Change window title
    void show_title();
    //! translate RGB components into packed type
    inline color_t get_color(colorcomp_t red, colorcomp_t green, colorcomp_t blue) const;

    //! Mouse events handler.
    virtual void on_mouse(int x, int y, int key) { }
    //! Mouse events handler.
    virtual void on_key(int key) { }
    //! Main processing loop. Redefine with your own
    virtual void on_process() { while(next_frame()); }

#ifdef _WINDOWS
    //! Windows specific members
    //! if VIDEO_WINMAIN isn't defined then set this just before init() by arguments of WinMain
    static HINSTANCE win_hInstance; static int win_iCmdShow;
    //! optionally call it just before init() to set own. Use ascii strings convention
    void win_set_class(WNDCLASSEX &);
    //! load and set accelerator table from resources
    void win_load_accelerators(int idc);
#endif
};

//! Drawing class
class drawing_area
{
    const size_t base_index, max_index, index_stride;
    const char pixel_depth;
    unsigned int * const ptr32;
    size_t index;
public:
    const int start_x, start_y, size_x, size_y;
    //! constructor
    drawing_area(int x, int y, int sizex, int sizey);
    //! destructor
    ~drawing_area();
    //! set current position. local_x could be bigger then size_x
    inline void set_pos(int local_x, int local_y);
    //! put pixel in current position with incremental address calculating to next right pixel
    inline void put_pixel(color_t color);
    //! draw pixel at position by packed color
    void set_pixel(int localx, int localy, color_t color)
        { set_pos(localx, localy); put_pixel(color); }
};

inline color_t video::get_color(colorcomp_t red, colorcomp_t green, colorcomp_t blue) const
{
    if(red_shift == 16) // only for depth == 24 && red_shift > blue_shift
        return (red<<16) | (green<<8) | blue;
    else if(depth >= 24)
        return (red<<red_shift) | (green<<green_shift) | (blue<<blue_shift);
    else if(depth > 0) {
        register char bs = blue_shift, rs = red_shift;
        if(blue_shift < 0) blue >>= -bs, bs = 0;
        else /*red_shift < 0*/ red >>= -rs, rs = 0;
        return (red<<rs)&red_mask | (green<<green_shift)&green_mask | (blue<<bs)&blue_mask;
    } else { // UYVY colorspace
        register unsigned y, u, v;
        y = red * 77 + green * 150 + blue * 29; // sum(77+150+29=256) * max(=255):  limit->2^16
        u = (2048 + (blue << 3) - (y >> 5)) >> 4; // (limit->2^12)>>4
        v = (2048 + (red << 3) - (y >> 5)) >> 4;
        y = y >> 8;
        return u | (y << 8) | (v << 16) | (y << 24);
    }
}

inline void drawing_area::set_pos(int local_x, int local_y)
{
    index = base_index + local_x + local_y*index_stride;
}

inline void drawing_area::put_pixel(color_t color)
{
    assert(index < max_index);
    if(pixel_depth > 16) ptr32[index++] = color;
    else if(pixel_depth > 0)
        ((unsigned short*)ptr32)[index++] = (unsigned short)color;
    else { // UYVY colorspace
        if(index&1) color >>= 16;
        ((unsigned short*)ptr32)[index++] = (unsigned short)color;
    }
}

#if defined(_WINDOWS) && (defined(VIDEO_WINMAIN) || defined(VIDEO_WINMAIN_ARGS) )
#include <cstdlib>
//! define WinMain for subsystem:windows.
#ifdef VIDEO_WINMAIN_ARGS
int main(int, char *[]);
#else
int main();
#endif
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR szCmdLine, int iCmdShow)
{
    video::win_hInstance = hInstance;  video::win_iCmdShow = iCmdShow;
#ifdef VIDEO_WINMAIN_ARGS
    return main(__argc, __argv);
#else
    return main();
#endif
}
#endif

#endif// __VIDEO_H__
