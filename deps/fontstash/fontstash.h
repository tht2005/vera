//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
// Copyright (c) 2014 Mapzen karim@mapzen.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef FONS_H
#define FONS_H

#include <cstring>
#include <stdbool.h>

#define FONS_INVALID -1

struct FONSscript;
struct FONSlanguage;
struct FONSdirection;

enum FONSflags {
    FONS_ZERO_TOPLEFT = 1,
    FONS_ZERO_BOTTOMLEFT = 2,
    FONS_NORMALIZE_TEX_COORDS = 4,
};

enum FONSalign {
    // Horizontal align
    FONS_ALIGN_LEFT     = 1<<0, // Default
    FONS_ALIGN_CENTER   = 1<<1,
    FONS_ALIGN_RIGHT    = 1<<2,

    // Vertical align
    FONS_ALIGN_TOP      = 1<<3,
    FONS_ALIGN_MIDDLE   = 1<<4,
    FONS_ALIGN_BOTTOM   = 1<<5,
    FONS_ALIGN_BASELINE = 1<<6, // Default
};

enum FONSeffectType {
    FONS_EFFECT_NONE = 0,
    FONS_EFFECT_BLUR = 1,
    FONS_EFFECT_GROW = 2,
    FONS_EFFECT_DISTANCE_FIELD = 3,
    FONS_EFFECT_DISTANCE_FIELD_FAST = 4,
};

enum FONSerrorCode {
    // Font atlas is full.
    FONS_ATLAS_FULL = 1,
    // Scratch memory used to render glyphs is full, requested size reported in 'val', you may need to bump up FONS_SCRATCH_BUF_SIZE.
    FONS_SCRATCH_FULL = 2,
    // Calls to fonsPushState has craeted too large stack, if you need deep state stack bump up FONS_MAX_STATES.
    FONS_STATES_OVERFLOW = 3,
    // Trying to pop too many states fonsPopState().
    FONS_STATES_UNDERFLOW = 4,
    FONS_HB_SCRIPT_DETECTION_FAILED = 5,
};

struct FONSquad {
    float x0,y0,s0,t0;
    float x1,y1,s1,t1;
};
typedef struct FONSquad FONSquad;

struct FONSparams {
    int             width, height;
    unsigned char   flags;
    void*           userPtr;
    int (*renderCreate)(void* uptr, int width, int height);
    int (*renderResize)(void* uptr, int width, int height);
    void (*renderUpdate)(void* uptr, int* rect, const unsigned char* data);
    void (*renderDraw)(void* uptr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts);
    void (*renderDelete)(void* uptr);
    void (*pushQuad)(void* uptr, const FONSquad* quad);
};
typedef struct FONSparams FONSparams;

struct FONStextIter {
    float x, y, nextx, nexty, scale, spacing;
    unsigned int codepoint;
    int blurType;
    short isize, iblur;
    struct FONSfont* font;
    int prevGlyphIndex;
    const char* str;
    const char* next;
    const char* end;
    unsigned int utf8state;
};
typedef struct FONStextIter FONStextIter;
typedef struct FONScontext FONScontext;
typedef struct FONSshaping FONSshaping;

// Contructor and destructor.
FONScontext* fonsCreateInternal(FONSparams* params);
void fonsDeleteInternal(FONScontext* s);

void fonsSetErrorCallback(FONScontext* s, void (*callback)(void* uptr, int error, int val), void* uptr);
// Returns current atlas size.
void fonsGetAtlasSize(FONScontext* s, int* width, int* height);
// Expands the atlas size.
int fonsExpandAtlas(FONScontext* s, int width, int height, const char);
// Reseta the whole stash.
int fonsResetAtlas(FONScontext* s, int width, int height, const char);

// Add fonts
int fonsAddFont(FONScontext* s, const char* name, const char* path);
int fonsAddFont(FONScontext* s, const char* name, unsigned char* data, size_t dataSize);
int fonsAddFontMem(FONScontext* s, const char* name, unsigned char* data, size_t dataSize, unsigned char freeData);
int fonsGetFontByName(FONScontext* s, const char* name);
int fonsAddFallbackFont(FONScontext* stash, int base, int fallback);

// State handling
void fonsPushState(FONScontext* s);
void fonsPopState(FONScontext* s);
void fonsClearState(FONScontext* s);

// State setting
void fonsSetSize(FONScontext* s, float size);
void fonsSetColor(FONScontext* s, unsigned int color);
void fonsSetSpacing(FONScontext* s, float spacing);
void fonsSetBlur(FONScontext* s, float blur);
void fonsSetBlurType(FONScontext* s, int blurType);
void fonsSetAlign(FONScontext* s, int align);
void fonsSetFont(FONScontext* s, int font);

// Draw text
float fonsDrawText(FONScontext* s, float x, float y, const char* string, const char* end, const char c);

bool fonsTextDrawable(FONScontext* stash, const char* string, const char* end, char cacheshaping);

// Measure text

// /!\ this would give unexpected results when using harfbuzz shaping
float fonsTextBounds(FONScontext* s, float x, float y, const char* string, const char* end, float* bounds);
void fonsLineBounds(FONScontext* s, float y, float* miny, float* maxy);
void fonsVertMetrics(FONScontext* s, float* ascender, float* descender, float* lineh);

// Text iterator

// /!\ this would give unexpected results when using harfbuzz shaping
int fonsTextIterInit(FONScontext* stash, FONStextIter* iter, float x, float y, const char* str, const char* end);
int fonsTextIterNext(FONScontext* stash, FONStextIter* iter, struct FONSquad* quad);

// Pull texture changes
const unsigned char* fonsGetTextureData(FONScontext* stash, int* width, int* height);
int fonsValidateTexture(FONScontext* s, int* dirty);

// Font shaping
void fonsSetShaping(FONScontext* stash);
void fonsSetShaping(FONScontext* stash, FONSscript script, FONSdirection direction, FONSlanguage language);
unsigned int fonsDecUTF8(unsigned int* state, unsigned int byte);

#endif // FONTSTASH_H

#ifdef FONTSTASH_IMPLEMENTATION

#ifdef FONS_USE_HARFBUZZ

#define FONSlanguage hb_language_t
#define FONSdirection hb_direction_t
#define FONSscript hb_script_t

#else

typedef struct FONSlanguage {} FONSlanguage;
typedef struct FONSdirection {} FONSdirection;
typedef struct FONSscript {} FONSscript;

#endif

typedef struct FONSttFontImpl FONSttFontImpl;

int fons__tt_initShaper(FONSttFontImpl* font);
void fons__tt_freeShaper(FONSttFontImpl* font);

#define SDF_IMPLEMENTATION
#include "sdf.h"

#define FONS_NOTUSED(v)  (void)sizeof(v)

#ifdef FONS_USE_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
#include <math.h>

struct FONSttFontImpl {
    FT_Face font;
    void* shaper;
};
typedef struct FONSttFontImpl FONSttFontImpl;

static FT_Library ftLibrary;

int fons__tt_init(FONScontext *stash) {
    FONS_NOTUSED(stash);
    FT_Error ftError;
    ftError = FT_Init_FreeType(&ftLibrary);
    return ftError == 0;
}

int fons__tt_loadFont(FONScontext *context, FONSttFontImpl *font, unsigned char *data, int dataSize) {
    FT_Error ftError;
    FONS_NOTUSED(context);

    //font->font.userdata = stash;
    ftError = FT_New_Memory_Face(ftLibrary, (const FT_Byte*)data, dataSize, 0, &font->font);

    bool setcharmap = false;
    // force USC-2
    for(int i = 0; i < font->font->num_charmaps; i++) {
        if (((  font->font->charmaps[i]->platform_id == 0)
            && (font->font->charmaps[i]->encoding_id == 3))
           || ((font->font->charmaps[i]->platform_id == 3)
            && (font->font->charmaps[i]->encoding_id == 1))) {
                ftError = FT_Set_Charmap(font->font, font->font->charmaps[i]);
                setcharmap = true;
                break;
        }
    }

    fons__tt_initShaper(font);
    return ftError == 0;
}

void fons__tt_getFontVMetrics(FONSttFontImpl *font, int *ascent, int *descent, int *lineGap) {
    *ascent = font->font->ascender;
    *descent = font->font->descender;
    *lineGap = font->font->height - (*ascent - *descent);
}

float fons__tt_getPixelHeightScale(FONSttFontImpl *font, float size) {
    return size / (float)font->font->units_per_EM;
}

int fons__tt_getGlyphIndex(FONSttFontImpl *font, int codepoint, int useShaping) {
    if(useShaping) {
        return codepoint;
    } else {
        return FT_Get_Char_Index(font->font, codepoint);
    }
}

int fons__tt_setPixelSize(FONSttFontImpl* font, float size) {
    FT_Error ftError;
    ftError = FT_Set_Pixel_Sizes(font->font, 0, size);
    return ftError;
}

int fons__tt_buildGlyphBitmap(FONSttFontImpl *font, int glyph, float size, float scale,
                              int *advance, int *lsb, int *x0, int *y0, int *x1, int *y1) {
    FT_Error ftError;
    FT_GlyphSlot ftGlyph;
    FONS_NOTUSED(scale);

    ftError = fons__tt_setPixelSize(font, size);
    if (ftError) return 0;
    ftError = FT_Load_Glyph(font->font, glyph, FT_LOAD_RENDER);
    if (ftError) return 0;
    ftError = FT_Get_Advance(font->font, glyph, FT_LOAD_NO_SCALE, (FT_Fixed*)advance);
    if (ftError) return 0;
    ftGlyph = font->font->glyph;
    *lsb = (int)ftGlyph->metrics.horiBearingX;
    *x0 = ftGlyph->bitmap_left;
    *x1 = *x0 + ftGlyph->bitmap.width;
    *y0 = -ftGlyph->bitmap_top;
    *y1 = *y0 + ftGlyph->bitmap.rows;
    return 1;
}

void fons__tt_renderGlyphBitmap(FONSttFontImpl *font, unsigned char *output, int outWidth, int outHeight, int outStride,
                                float scaleX, float scaleY, int glyph) {
    FT_GlyphSlot ftGlyph = font->font->glyph;
    int ftGlyphOffset = 0;
    int x, y;
    FONS_NOTUSED(outWidth);
    FONS_NOTUSED(outHeight);
    FONS_NOTUSED(scaleX);
    FONS_NOTUSED(scaleY);
    FONS_NOTUSED(glyph);    // glyph has already been loaded by fons__tt_buildGlyphBitmap

    for ( y = 0; y < ftGlyph->bitmap.rows; y++ ) {
        for ( x = 0; x < ftGlyph->bitmap.width; x++ ) {
            output[(y * outStride) + x] = ftGlyph->bitmap.buffer[ftGlyphOffset++];
        }
    }
}

int fons__tt_getGlyphKernAdvance(FONSttFontImpl *font, int glyph1, int glyph2) {
    FT_Vector ftKerning;
    FT_Get_Kerning(font->font, glyph1, glyph2, FT_KERNING_DEFAULT, &ftKerning);
    return (int)ftKerning.x;
}

float fons__tt_getUnitScale() {
    // 26.6 freetype pixel format, need to perform scaling, 1 unit = 1/64 pixel.
    return 1.0 / 64;
}

#else

#define STB_TRUETYPE_IMPLEMENTATION
static void* fons__tmpalloc(size_t size, void* up);
static void fons__tmpfree(void* ptr, void* up);
#define STBTT_malloc(x,u)    fons__tmpalloc(x,u)
#define STBTT_free(x,u)      fons__tmpfree(x,u)
#include "stb_truetype.h"

struct FONSttFontImpl {
    stbtt_fontinfo  font;
    void*           shaper;
};
typedef struct FONSttFontImpl FONSttFontImpl;

int fons__tt_init(FONScontext *context) {
    FONS_NOTUSED(context);
    return 1;
}

int fons__tt_loadFont(FONScontext *context, FONSttFontImpl *font, unsigned char *data, int dataSize) {
    int stbError;
    FONS_NOTUSED(dataSize);

    font->font.userdata = context;
    stbError = stbtt_InitFont(&font->font, data, 0);
    return stbError;
}

void fons__tt_getFontVMetrics(FONSttFontImpl *font, int *ascent, int *descent, int *lineGap) {
    stbtt_GetFontVMetrics(&font->font, ascent, descent, lineGap);
}

float fons__tt_getPixelHeightScale(FONSttFontImpl *font, float size) {
    return stbtt_ScaleForMappingEmToPixels(&font->font, size);
}

int fons__tt_getGlyphIndex(FONSttFontImpl *font, int codepoint, int useShaping) {
    (void) useShaping;
    return stbtt_FindGlyphIndex(&font->font, codepoint);
}

int fons__tt_setPixelSize(FONSttFontImpl* font, float pxSize) {
    FONS_NOTUSED(font);
    FONS_NOTUSED(pxSize);
    return 0;
}

int fons__tt_buildGlyphBitmap(FONSttFontImpl *font, int glyph, float size, float scale,
                              int *advance, int *lsb, int *x0, int *y0, int *x1, int *y1) {
    FONS_NOTUSED(size);
    stbtt_GetGlyphHMetrics(&font->font, glyph, advance, lsb);
    stbtt_GetGlyphBitmapBox(&font->font, glyph, scale, scale, x0, y0, x1, y1);
    return 1;
}

void fons__tt_renderGlyphBitmap(FONSttFontImpl *font, unsigned char *output, int outWidth, int outHeight, int outStride,
                                float scaleX, float scaleY, int glyph) {
    stbtt_MakeGlyphBitmap(&font->font, output, outWidth, outHeight, outStride, scaleX, scaleY, glyph);
}

int fons__tt_getGlyphKernAdvance(FONSttFontImpl *font, int glyph1, int glyph2) {
    return stbtt_GetGlyphKernAdvance(&font->font, glyph1, glyph2);
}

float fons__tt_getUnitScale() { return 1.0; }

#endif // STBTT

#ifndef FONS_SCRATCH_BUF_SIZE
#define FONS_SCRATCH_BUF_SIZE 160000
#endif
#ifndef FONS_HASH_LUT_SIZE
#define FONS_HASH_LUT_SIZE 256
#endif
#ifndef FONS_INIT_FONTS
#define FONS_INIT_FONTS 4
#endif
#ifndef FONS_INIT_GLYPHS
#define FONS_INIT_GLYPHS 256
#endif
#ifndef FONS_INIT_ATLAS_NODES
#define FONS_INIT_ATLAS_NODES 256
#endif
#ifndef FONS_VERTEX_COUNT
#define FONS_VERTEX_COUNT 1024
#endif
#ifndef FONS_MAX_STATES
#define FONS_MAX_STATES 20
#endif
#ifndef FONS_MAX_FALLBACKS
#	define FONS_MAX_FALLBACKS 20
#endif


static unsigned int fons__hashint(unsigned int a) {
    a += ~(a<<15);
    a ^=  (a>>10);
    a +=  (a<<3);
    a ^=  (a>>6);
    a += ~(a<<11);
    a ^=  (a>>16);
    return a;
}

static int fons__mini(int a, int b) { return a < b ? a : b; }
static int fons__maxi(int a, int b) { return a > b ? a : b; }

struct FONSglyph {
    unsigned int    codepoint;
    int             index;
    int             next;
    int             blurType;
    short           size, blur;
    short           x0,y0,x1,y1;
    short           xadv,xoff,yoff;
};
typedef struct FONSglyph FONSglyph;

struct FONSfont {
    FONSttFontImpl  font;
    char            name[64];
    unsigned char*  data;
    int             dataSize;
    unsigned char   freeData;
    float           ascender;
    float           descender;
    float           lineh;
    FONSglyph*      glyphs;
    int             cglyphs;
    int             nglyphs;
    int             lut[FONS_HASH_LUT_SIZE];
    int             fallbacks[FONS_MAX_FALLBACKS];
    int             nfallbacks;
};
typedef struct FONSfont FONSfont;

struct FONSstate {
    int             font;
    int             align;
    float           size;
    int             blurType;
    unsigned int    color;
    float           blur;
    float           spacing;
    int             useShaping;
};
typedef struct FONSstate FONSstate;

struct FONSatlasNode {
    short x, y, width;
};
typedef struct FONSatlasNode FONSatlasNode;

struct FONSatlas
{
    int width, height;
    FONSatlasNode* nodes;
    int nnodes;
    int cnodes;
};
typedef struct FONSatlas FONSatlas;

struct FONSshapingRes
{
    unsigned int glyphCount;
    uint32_t* codepoints;
    float* advance;
    float* offset;
};
typedef struct FONSshapingRes FONSshapingRes;

struct FONSshaping {
    FONSshapingRes* result;
    FONSscript script;
    FONSdirection direction;
    FONSlanguage language;
    bool customConfig;
    int it;
};

struct FONScontext
{
    FONSparams params;
    float itw,ith;
    unsigned char* texData;
    int dirtyRect[4];
    FONSfont** fonts;
    FONSatlas* atlas;
    int cfonts;
    int nfonts;
    float verts[FONS_VERTEX_COUNT*2];
    float tcoords[FONS_VERTEX_COUNT*2];
    unsigned int colors[FONS_VERTEX_COUNT];
    int nverts;
    unsigned char* scratch;
    int nscratch;
    FONSstate states[FONS_MAX_STATES];
    int nstates;
    void (*handleError)(void* uptr, int error, int val);
    void* errorUptr;
    FONSshaping* shaping;
};

#ifdef FONS_USE_HARFBUZZ

struct FONShbFontShaper
{
    hb_font_t* font;
    hb_buffer_t* buffer;
};

typedef struct FONShbFontShaper FONShbFontShaper;

int fons__tt_initShaper(FONSttFontImpl* font) {
    FONShbFontShaper* shaper = (FONShbFontShaper *) malloc(sizeof(FONShbFontShaper));

    shaper->font = hb_ft_font_create(font->font, NULL);
    shaper->buffer = hb_buffer_create();

    font->shaper = shaper;

    return hb_buffer_allocation_successful(shaper->buffer);
}

void fons__tt_freeShaper(FONSttFontImpl* font) {
    FONShbFontShaper* shaper = (FONShbFontShaper*)font->shaper;
    if(shaper) {
        hb_buffer_destroy(shaper->buffer);
        hb_font_destroy(shaper->font);
        free(shaper);
    }
}

void fons__hb_shape(FONScontext* stash, const char* text, FONSfont* font) {
    FONSshaping* shaping;
    FONSshapingRes* res;
    FONShbFontShaper* shaper;
    hb_buffer_t* buffer;
    hb_font_t* ft;
    int i,j;

    shaper = (FONShbFontShaper *) font->font.shaper;
    shaping = stash->shaping;
    res = shaping->result;
    buffer = shaper->buffer;
    ft = shaper->font;

    hb_buffer_reset(buffer);
    hb_buffer_add_utf8(buffer, text, (int)strlen(text), 0, (int)strlen(text));

    if (shaping->customConfig) {
        hb_buffer_set_direction(buffer, shaping->direction);
        hb_buffer_set_script(buffer, shaping->script);
        hb_buffer_set_language(buffer, shaping->language);
    } else {
        hb_buffer_guess_segment_properties(buffer);
    }

    hb_shape(ft, buffer, NULL, 0);

    hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(buffer, &res->glyphCount);
    hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(buffer, &res->glyphCount);

    res->codepoints = (uint32_t *) malloc(sizeof(uint32_t) * res->glyphCount);

    res->offset = (float *) malloc(sizeof(float) * res->glyphCount * 2);
    res->advance = (float *) malloc(sizeof(float) * res->glyphCount * 2);

    for(i = 0, j = 0; i < res->glyphCount; i++, j+=2) {
        res->advance[j] = glyphPos[i].x_advance;
        res->advance[j+1] = glyphPos[i].y_advance;
        res->offset[j] = glyphPos[i].x_offset;
        res->offset[j+1] = glyphPos[i].y_offset;
        res->codepoints[i] = glyphInfo[i].codepoint;
    }
}

void fons__hb_freeShapingResult(FONSshaping* shaping) {
    if(shaping) {
        shaping->customConfig = false;
        free(shaping->result->advance);
        free(shaping->result->offset);
        free(shaping->result->codepoints);
        free(shaping->result);
    }
}

#else

void fons__hb_shape(FONScontext* stash, const char* text, FONSfont* font) {
    (void) font;
    FONS_NOTUSED(stash);
    FONS_NOTUSED(text);
}

void fons__hb_freeShapingResult(FONSshaping* shaping) {
    FONS_NOTUSED(shaping);
}

int fons__tt_initShaper(FONSttFontImpl* font) {
    font->shaper = NULL;
    FONS_NOTUSED(font);
    return -1;
}

void fons__tt_freeShaper(FONSttFontImpl* font) {
    FONS_NOTUSED(font);
}

#endif // FONS_USE_HARFBUZZ

static void* fons__tmpalloc(size_t size, void* up) {
    unsigned char* ptr;
    FONScontext* stash = (FONScontext*)up;

    // 16-byte align the returned pointer
    size = (size + 0xf) & ~0xf;

    if (stash->nscratch+(int)size > FONS_SCRATCH_BUF_SIZE) {
        if (stash->handleError)
            stash->handleError(stash->errorUptr, FONS_SCRATCH_FULL, stash->nscratch+(int)size);
        return NULL;
    }
    ptr = stash->scratch + stash->nscratch;
    stash->nscratch += (int)size;
    return ptr;
}

static void fons__tmpfree(void* ptr, void* up) {
    (void)ptr;
    (void)up;
    // empty
}

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define FONS_UTF8_ACCEPT 0
#define FONS_UTF8_REJECT 12

static unsigned int fons__decutf8(unsigned int* state, unsigned int* codep, unsigned int byte) {
    static const unsigned char utf8d[] = {
        // The first part of the table maps bytes to character classes that
        // to reduce the size of the transition table and create bitmasks.
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

        // The second part is a transition table that maps a combination
        // of a state of the automaton and a character class to a state.
        0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
        12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
        12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
        12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
        12,36,12,12,12,12,12,12,12,12,12,12,
    };

    unsigned int type = utf8d[byte];

    *codep = (*state != FONS_UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

    *state = utf8d[256 + *state + type];
    return *state;
}

// Atlas based on Skyline Bin Packer by Jukka Jylänki

static void fons__deleteAtlas(FONSatlas* atlas) {
    if (atlas == NULL) return;
    if (atlas->nodes != NULL) free(atlas->nodes);
    free(atlas);
}

static FONSatlas* fons__allocAtlas(int w, int h, int nnodes) {
    FONSatlas* atlas = NULL;

    // Allocate memory for the font stash.
    atlas = (FONSatlas*)malloc(sizeof(FONSatlas));
    if (atlas == NULL) goto error;
    memset(atlas, 0, sizeof(FONSatlas));

    atlas->width = w;
    atlas->height = h;

    // Allocate space for skyline nodes
    atlas->nodes = (FONSatlasNode*)malloc(sizeof(FONSatlasNode) * nnodes);
    if (atlas->nodes == NULL) goto error;
    memset(atlas->nodes, 0, sizeof(FONSatlasNode) * nnodes);
    atlas->nnodes = 0;
    atlas->cnodes = nnodes;

    // Init root node.
    atlas->nodes[0].x = 0;
    atlas->nodes[0].y = 0;
    atlas->nodes[0].width = (short)w;
    atlas->nnodes++;

    return atlas;

error:
    if (atlas) fons__deleteAtlas(atlas);
    return NULL;
}

static int fons__atlasInsertNode(FONSatlas* atlas, int idx, int x, int y, int w) {
    int i;
    // Insert node
    if (atlas->nnodes+1 > atlas->cnodes) {
        atlas->cnodes = atlas->cnodes == 0 ? 8 : atlas->cnodes * 2;
        atlas->nodes = (FONSatlasNode*)realloc(atlas->nodes, sizeof(FONSatlasNode) * atlas->cnodes);
        if (atlas->nodes == NULL)
            return 0;
    }
    for (i = atlas->nnodes; i > idx; i--)
        atlas->nodes[i] = atlas->nodes[i-1];
    atlas->nodes[idx].x = (short)x;
    atlas->nodes[idx].y = (short)y;
    atlas->nodes[idx].width = (short)w;
    atlas->nnodes++;

    return 1;
}

static void fons__atlasRemoveNode(FONSatlas* atlas, int idx) {
    int i;
    if (atlas->nnodes == 0) return;
    for (i = idx; i < atlas->nnodes-1; i++)
        atlas->nodes[i] = atlas->nodes[i+1];
    atlas->nnodes--;
}

static void fons__atlasExpand(FONSatlas* atlas, int w, int h) {
    // Insert node for empty space
    if (w > atlas->width)
        fons__atlasInsertNode(atlas, atlas->nnodes, atlas->width, 0, w - atlas->width);
    atlas->width = w;
    atlas->height = h;
}

static void fons__atlasReset(FONSatlas* atlas, int w, int h) {
    atlas->width = w;
    atlas->height = h;
    atlas->nnodes = 0;

    // Init root node.
    atlas->nodes[0].x = 0;
    atlas->nodes[0].y = 0;
    atlas->nodes[0].width = (short)w;
    atlas->nnodes++;
}

static int fons__atlasAddSkylineLevel(FONSatlas* atlas, int idx, int x, int y, int w, int h) {
    int i;

    // Insert new node
    if (fons__atlasInsertNode(atlas, idx, x, y+h, w) == 0)
        return 0;

    // Delete skyline segments that fall under the shaodw of the new segment.
    for (i = idx+1; i < atlas->nnodes; i++) {
        if (atlas->nodes[i].x < atlas->nodes[i-1].x + atlas->nodes[i-1].width) {
            int shrink = atlas->nodes[i-1].x + atlas->nodes[i-1].width - atlas->nodes[i].x;
            atlas->nodes[i].x += (short)shrink;
            atlas->nodes[i].width -= (short)shrink;
            if (atlas->nodes[i].width <= 0) {
                fons__atlasRemoveNode(atlas, i);
                i--;
            } else {
                break;
            }
        } else {
            break;
        }
    }

    // Merge same height skyline segments that are next to each other.
    for (i = 0; i < atlas->nnodes-1; i++) {
        if (atlas->nodes[i].y == atlas->nodes[i+1].y) {
            atlas->nodes[i].width += atlas->nodes[i+1].width;
            fons__atlasRemoveNode(atlas, i+1);
            i--;
        }
    }

    return 1;
}

static int fons__atlasRectFits(FONSatlas* atlas, int i, int w, int h) {
    // Checks if there is enough space at the location of skyline span 'i',
    // and return the max height of all skyline spans under that at that location,
    // (think tetris block being dropped at that position). Or -1 if no space found.
    int x = atlas->nodes[i].x;
    int y = atlas->nodes[i].y;
    int spaceLeft;
    if (x + w > atlas->width)
        return -1;
    spaceLeft = w;
    while (spaceLeft > 0) {
        if (i == atlas->nnodes) return -1;
        y = fons__maxi(y, atlas->nodes[i].y);
        if (y + h > atlas->height) return -1;
        spaceLeft -= atlas->nodes[i].width;
        ++i;
    }
    return y;
}

static int fons__atlasAddRect(FONSatlas* atlas, int rw, int rh, int* rx, int* ry) {
    int besth = atlas->height, bestw = atlas->width, besti = -1;
    int bestx = -1, besty = -1, i;

    // Bottom left fit heuristic.
    for (i = 0; i < atlas->nnodes; i++) {
        int y = fons__atlasRectFits(atlas, i, rw, rh);
        if (y != -1) {
            if (y + rh < besth || (y + rh == besth && atlas->nodes[i].width < bestw)) {
                besti = i;
                bestw = atlas->nodes[i].width;
                besth = y + rh;
                bestx = atlas->nodes[i].x;
                besty = y;
            }
        }
    }

    if (besti == -1)
        return 0;

    // Perform the actual packing.
    if (fons__atlasAddSkylineLevel(atlas, besti, bestx, besty, rw, rh) == 0)
        return 0;

    *rx = bestx;
    *ry = besty;

    return 1;
}

static void fons__addWhiteRect(FONScontext* stash, int w, int h) {
    int x, y, gx, gy;
    unsigned char* dst;
    if (fons__atlasAddRect(stash->atlas, w, h, &gx, &gy) == 0)
        return;

    // Rasterize
    dst = &stash->texData[gx + gy * stash->params.width];
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++)
            dst[x] = 0xff;
        dst += stash->params.width;
    }

    stash->dirtyRect[0] = fons__mini(stash->dirtyRect[0], gx);
    stash->dirtyRect[1] = fons__mini(stash->dirtyRect[1], gy);
    stash->dirtyRect[2] = fons__maxi(stash->dirtyRect[2], gx+w);
    stash->dirtyRect[3] = fons__maxi(stash->dirtyRect[3], gy+h);
}

void fons__allocShaping(FONScontext* stash) {
    FONSshaping* shaping = (FONSshaping *) malloc(sizeof(FONSshaping));
    stash->shaping = shaping;
    stash->shaping->it = -1;
}

void fons__deleteShaping(FONScontext* stash) {
    if(stash->shaping) {
        free(stash->shaping);
    }
}

FONScontext* fonsCreateInternal(FONSparams* params) {
    FONScontext* stash = NULL;

    // Allocate memory for the font stash.
    stash = (FONScontext*)malloc(sizeof(FONScontext));
    if (stash == NULL) goto error;
    memset(stash, 0, sizeof(FONScontext));

    stash->params = *params;

    // Allocate scratch buffer.
    stash->scratch = (unsigned char*)malloc(FONS_SCRATCH_BUF_SIZE);
    if (stash->scratch == NULL) goto error;

    // Initialize implementation library
    if (!fons__tt_init(stash)) goto error;

    if (stash->params.renderCreate != NULL) {
        if (stash->params.renderCreate(stash->params.userPtr, stash->params.width, stash->params.height) == 0)
            goto error;
    }

    stash->atlas = fons__allocAtlas(stash->params.width, stash->params.height, FONS_INIT_ATLAS_NODES);
    if (stash->atlas == NULL) goto error;

    // Allocate space for fonts.
    stash->fonts = (FONSfont**)malloc(sizeof(FONSfont*) * FONS_INIT_FONTS);
    if (stash->fonts == NULL) goto error;
    memset(stash->fonts, 0, sizeof(FONSfont*) * FONS_INIT_FONTS);
    stash->cfonts = FONS_INIT_FONTS;
    stash->nfonts = 0;

    // Create texture for the cache.
    stash->itw = 1.0f/stash->params.width;
    stash->ith = 1.0f/stash->params.height;
    stash->texData = (unsigned char*)malloc(stash->params.width * stash->params.height);
    if (stash->texData == NULL) goto error;
    memset(stash->texData, 0, stash->params.width * stash->params.height);

    stash->dirtyRect[0] = stash->params.width;
    stash->dirtyRect[1] = stash->params.height;
    stash->dirtyRect[2] = 0;
    stash->dirtyRect[3] = 0;

    // Add white rect at 0,0 for debug drawing.
    fons__addWhiteRect(stash, 2,2);

    fons__allocShaping(stash);

    fonsPushState(stash);
    fonsClearState(stash);

    return stash;

error:
    fonsDeleteInternal(stash);
    return NULL;
}

static FONSstate* fons__getState(FONScontext* stash) {
    return &stash->states[stash->nstates-1];
}

int fonsAddFallbackFont(FONScontext* stash, int base, int fallback)
{
	FONSfont* baseFont = stash->fonts[base];
	if (baseFont->nfallbacks < FONS_MAX_FALLBACKS) {
		baseFont->fallbacks[baseFont->nfallbacks++] = fallback;
		return 1;
	}
	return 0;
}

void fonsSetSize(FONScontext* stash, float size) {
    fons__getState(stash)->size = size;
}

void fonsSetColor(FONScontext* stash, unsigned int color) {
    fons__getState(stash)->color = color;
}

void fonsSetSpacing(FONScontext* stash, float spacing) {
    fons__getState(stash)->spacing = spacing;
}

void fonsSetBlur(FONScontext* stash, float blur) {
    fons__getState(stash)->blur = blur;
}

void fonsSetAlign(FONScontext* stash, int align) {
    fons__getState(stash)->align = align;
}

void fonsSetBlurType(FONScontext* stash, int blurType) {
    fons__getState(stash)->blurType = blurType;
}

void fonsSetFont(FONScontext* stash, int font) {
    fons__getState(stash)->font = font;
}

void fons__clearShaping(FONScontext* stash) {
    fons__hb_freeShapingResult(stash->shaping);
    stash->shaping->it = -1;
    fons__getState(stash)->useShaping = 0;
}

void fonsPushState(FONScontext* stash) {
    if (stash->nstates >= FONS_MAX_STATES) {
        if (stash->handleError)
            stash->handleError(stash->errorUptr, FONS_STATES_OVERFLOW, 0);
        return;
    }
    if (stash->nstates > 0)
        memcpy(&stash->states[stash->nstates], &stash->states[stash->nstates-1], sizeof(FONSstate));
    stash->nstates++;
}

void fonsPopState(FONScontext* stash) {
    if (stash->nstates <= 1) {
        if (stash->handleError)
            stash->handleError(stash->errorUptr, FONS_STATES_UNDERFLOW, 0);
        return;
    }
    stash->nstates--;
}

void fonsClearState(FONScontext* stash) {
    FONSstate* state = fons__getState(stash);
    state->size = 12.0f;
    state->color = 0xffffffff;
    state->font = 0;
    state->blur = 0;
    state->blurType = FONS_EFFECT_NONE;
    state->spacing = 0;
    state->align = FONS_ALIGN_LEFT | FONS_ALIGN_BASELINE;
    state->useShaping = 0;
}

static void fons__freeFont(FONSfont* font) {
    if (font == NULL) return;
    if (font->glyphs) free(font->glyphs);
    if (font->freeData && font->data) free(font->data);
    fons__tt_freeShaper(&font->font);
    free(font);
}

static int fons__allocFont(FONScontext* stash) {
    FONSfont* font = NULL;
    if (stash->nfonts+1 > stash->cfonts) {
        stash->cfonts = stash->cfonts == 0 ? 8 : stash->cfonts * 2;
        stash->fonts = (FONSfont**)realloc(stash->fonts, sizeof(FONSfont*) * stash->cfonts);
        if (stash->fonts == NULL)
            return -1;
    }
    font = (FONSfont*)malloc(sizeof(FONSfont));
    if (font == NULL) goto error;
    memset(font, 0, sizeof(FONSfont));

    font->glyphs = (FONSglyph*)malloc(sizeof(FONSglyph) * FONS_INIT_GLYPHS);
    if (font->glyphs == NULL) goto error;
    font->cglyphs = FONS_INIT_GLYPHS;
    font->nglyphs = 0;

    stash->fonts[stash->nfonts++] = font;
    return stash->nfonts-1;

error:
    fons__freeFont(font);

    return FONS_INVALID;
}

unsigned int fonsDecUTF8(unsigned int* state, unsigned int byte) {
    unsigned int codep;
    return fons__decutf8(state, &codep, byte);
}


int fonsAddFont(FONScontext* stash, const char* name, unsigned char* data, size_t dataSize) {
    return fonsAddFontMem(stash, name, data, dataSize, 1);
}

int fonsAddFont(FONScontext* stash, const char* name, const char* path) {
    FILE* fp = 0;
    size_t dataSize = 0;
    size_t nRead = 0;
    unsigned char* data = NULL;

    // Read in the font data.
    fp = fopen(path, "rb");
    if (fp == NULL) goto error;
    fseek(fp,0,SEEK_END);
    dataSize = (size_t)ftell(fp);
    fseek(fp,0,SEEK_SET);
    data = (unsigned char*)malloc(dataSize);
    if (data == NULL) goto error;
    nRead = fread(data, 1, dataSize, fp);
    if (nRead != dataSize) goto error;
    fclose(fp);
    fp = 0;

    return fonsAddFontMem(stash, name, data, dataSize, 1);

error:
    if (data) free(data);
    if (fp) fclose(fp);
    return FONS_INVALID;
}

int fonsAddFontMem(FONScontext* stash, const char* name, unsigned char* data, size_t dataSize, unsigned char freeData) {
    int i, ascent, descent, fh, lineGap;
    FONSfont* font;

    int idx = fons__allocFont(stash);
    if (idx == FONS_INVALID)
        return FONS_INVALID;

    font = stash->fonts[idx];

    strncpy(font->name, name, sizeof(font->name));
    font->name[sizeof(font->name)-1] = '\0';

    // Init hash lookup.
    for (i = 0; i < FONS_HASH_LUT_SIZE; ++i)
        font->lut[i] = -1;

    // Read in the font data.
    font->dataSize = dataSize;
    font->data = data;
    font->freeData = freeData;

    // Init font
    stash->nscratch = 0;
    if (!fons__tt_loadFont(stash, &font->font, data, dataSize)) goto error;

    // Store normalized line height. The real line height is got
    // by multiplying the lineh by font size.
    fons__tt_getFontVMetrics( &font->font, &ascent, &descent, &lineGap);
    fh = ascent - descent;
    font->ascender = (float)ascent / (float)fh;
    font->descender = (float)descent / (float)fh;
    font->lineh = (float)(fh + lineGap) / (float)fh;

    return idx;

error:
    fons__freeFont(font);
    stash->nfonts--;
    return FONS_INVALID;
}

int fonsGetFontByName(FONScontext* s, const char* name) {
    int i;
    for (i = 0; i < s->nfonts; i++) {
        if (strcmp(s->fonts[i]->name, name) == 0)
            return i;
    }
    return FONS_INVALID;
}


static FONSglyph* fons__allocGlyph(FONSfont* font) {
    if (font->nglyphs+1 > font->cglyphs) {
        font->cglyphs = font->cglyphs == 0 ? 8 : font->cglyphs * 2;
        font->glyphs = (FONSglyph*)realloc(font->glyphs, sizeof(FONSglyph) * font->cglyphs);
        if (font->glyphs == NULL) return NULL;
    }
    font->nglyphs++;
    return &font->glyphs[font->nglyphs-1];
}


// Based on Exponential blur, Jani Huhtanen, 2006

#define APREC 16
#define ZPREC 7

static void fons__blurCols(unsigned char* dst, int w, int h, int dstStride, int alpha) {
    int x, y;
    for (y = 0; y < h; y++) {
        int z = 0; // force zero border
        for (x = 1; x < w; x++) {
            z += (alpha * (((int)(dst[x]) << ZPREC) - z)) >> APREC;
            dst[x] = (unsigned char)(z >> ZPREC);
        }
        dst[w-1] = 0; // force zero border
        z = 0;
        for (x = w-2; x >= 0; x--) {
            z += (alpha * (((int)(dst[x]) << ZPREC) - z)) >> APREC;
            dst[x] = (unsigned char)(z >> ZPREC);
        }
        dst[0] = 0; // force zero border
        dst += dstStride;
    }
}

static void fons__blurRows(unsigned char* dst, int w, int h, int dstStride, int alpha) {
    int x, y;
    for (x = 0; x < w; x++) {
        int z = 0; // force zero border
        for (y = dstStride; y < h*dstStride; y += dstStride) {
            z += (alpha * (((int)(dst[y]) << ZPREC) - z)) >> APREC;
            dst[y] = (unsigned char)(z >> ZPREC);
        }
        dst[(h-1)*dstStride] = 0; // force zero border
        z = 0;
        for (y = (h-2)*dstStride; y >= 0; y -= dstStride) {
            z += (alpha * (((int)(dst[y]) << ZPREC) - z)) >> APREC;
            dst[y] = (unsigned char)(z >> ZPREC);
        }
        dst[0] = 0; // force zero border
        dst++;
    }
}


static void fons__blur(FONScontext* stash, unsigned char* dst, int w, int h, int dstStride, int blur) {
    int alpha;
    float sigma;
    (void)stash;

    if (blur < 1)
        return;
    // Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends to infinity)
    sigma = (float)blur * 0.57735f; // 1 / sqrt(3)
    alpha = (int)((1<<APREC) * (1.0f - expf(-2.3f / (sigma+1.0f))));
    fons__blurRows(dst, w, h, dstStride, alpha);
    fons__blurCols(dst, w, h, dstStride, alpha);
    fons__blurRows(dst, w, h, dstStride, alpha);
    fons__blurCols(dst, w, h, dstStride, alpha);
    //  fons__blurrows(dst, w, h, dstStride, alpha);
    //  fons__blurcols(dst, w, h, dstStride, alpha);
}

static FONSglyph* fons__getGlyph(FONScontext* stash, FONSfont* font, unsigned int codepoint,
                                 short isize, short iblur, int blurType) {
    int i, g, advance, lsb, x0, y0, x1, y1, gw, gh, gx, gy, x, y;
    float scale;
    FONSglyph* glyph = NULL;
    unsigned int h;
    float size = isize/10.0f;
    int pad, added;
    unsigned char* bdst;
    unsigned char* dst;

    FONSfont* renderFont = font;

    if (isize < 2) return NULL;
    if (iblur > 20) iblur = 20;
    pad = iblur+2;

    // Reset allocator.
    stash->nscratch = 0;

    // Find code point and size.
    h = fons__hashint(codepoint) & (FONS_HASH_LUT_SIZE-1);
    i = font->lut[h];
    while (i != -1) {
        if (font->glyphs[i].codepoint == codepoint && font->glyphs[i].size == isize
                && font->glyphs[i].blur == iblur && font->glyphs[i].blurType == blurType)
            return &font->glyphs[i];
        i = font->glyphs[i].next;
    }

    // Could not find glyph, create it.
    
    g = fons__tt_getGlyphIndex(&font->font, codepoint, fons__getState(stash)->useShaping && font->font.shaper != NULL);
	// Try to find the glyph in fallback fonts.
	if (g == 0) {
        for (i = 0; i < font->nfallbacks; ++i) {
			FONSfont* fallbackFont = stash->fonts[font->fallbacks[i]];
			int fallbackIndex = fons__tt_getGlyphIndex(&fallbackFont->font, codepoint, fons__getState(stash)->useShaping && font->font.shaper != NULL);
			if (fallbackIndex != 0) {
				g = fallbackIndex;
				renderFont = fallbackFont;
				break;
			}
		}
		// It is possible that we did not find a fallback glyph.
		// In that case the glyph index 'g' is 0, and we'll proceed below and cache empty glyph.
	}
    scale = fons__tt_getPixelHeightScale(&renderFont->font, size);
    fons__tt_buildGlyphBitmap(&renderFont->font, g, size, scale, &advance, &lsb, &x0, &y0, &x1, &y1);
    gw = x1-x0 + pad*2;
    gh = y1-y0 + pad*2;

    // Find free spot for the rect in the atlas
    added = fons__atlasAddRect(stash->atlas, gw, gh, &gx, &gy);
    if (added == 0 && stash->handleError != NULL) {
        // Atlas is full, let the user to resize the atlas (or not), and try again.
        stash->handleError(stash->errorUptr, FONS_ATLAS_FULL, 0);
        added = fons__atlasAddRect(stash->atlas, gw, gh, &gx, &gy);
    }

    if (added == 0) return NULL;

    // Init glyph.
    glyph = fons__allocGlyph(font);
    glyph->codepoint = codepoint;
    glyph->size = isize;
    glyph->blur = iblur;
    glyph->blurType = blurType;
    glyph->index = g;
    glyph->x0 = (short)gx;
    glyph->y0 = (short)gy;
    glyph->x1 = (short)(glyph->x0+gw);
    glyph->y1 = (short)(glyph->y0+gh);
    glyph->xadv = (short)(scale * advance * 10.0f);
    glyph->xoff = (short)(x0 - pad);
    glyph->yoff = (short)(y0 - pad);
    glyph->next = 0;

    // Insert char to hash lookup.
    glyph->next = font->lut[h];
    font->lut[h] = font->nglyphs-1;

    // Rasterize
    dst = &stash->texData[(glyph->x0+pad) + (glyph->y0+pad) * stash->params.width];
    fons__tt_renderGlyphBitmap(&renderFont->font, dst, gw-pad*2,gh-pad*2, stash->params.width, scale,scale, g);

    // Make sure there is one pixel empty border.
    dst = &stash->texData[glyph->x0 + glyph->y0 * stash->params.width];
    for (y = 0; y < gh; y++) {
        dst[y*stash->params.width] = 0;
        dst[gw-1 + y*stash->params.width] = 0;
    }
    for (x = 0; x < gw; x++) {
        dst[x] = 0;
        dst[x + (gh-1)*stash->params.width] = 0;
    }

    // Blur
    if (iblur > 0) {
        stash->nscratch = 0;
        bdst = &stash->texData[glyph->x0 + glyph->y0 * stash->params.width];

        if (blurType == FONS_EFFECT_BLUR) {
            fons__blur(stash, bdst, gw,gh, stash->params.width, iblur);
        } else if (blurType == FONS_EFFECT_GROW) {
            unsigned char* sdfTemp = (unsigned char*)fons__tmpalloc(gw * gh * sizeof(float) * 3, stash);
            if (sdfTemp) {
                sdfBuildDistanceFieldNoAlloc(bdst, stash->params.width, iblur, bdst, gw, gh, stash->params.width, sdfTemp);
                fons__tmpfree(sdfTemp, stash);

                for (y = 0; y < gh; y++) {
                    int yw = y * stash->params.width;
                    for (x = 0; x < gw; x++) {
                        int a = (int) bdst[x+ yw];
                        int smoothingLimit = 255 / (iblur);
                        if (a < smoothingLimit)
                            a = a * 255 / smoothingLimit;
                        else
                            a = 255;

                        bdst[x + yw] = a;
                    }
                }
            }

        } else if (blurType == FONS_EFFECT_DISTANCE_FIELD) {
            // The required temp array must fit width * height * sizeof(float) * 3 bytes.
            unsigned char* sdfTemp = (unsigned char*)fons__tmpalloc(gw * gh * sizeof(float) * 3, stash);
            if (sdfTemp) {
                sdfBuildDistanceFieldNoAlloc(bdst, stash->params.width, iblur, bdst, gw, gh, stash->params.width, sdfTemp);
                fons__tmpfree(sdfTemp, stash);
            }

        } else if (blurType == FONS_EFFECT_DISTANCE_FIELD_FAST) {
            // When using sdfCoverageToDistanceField input and output must be separate arrays. Allocate temp array for output.
            unsigned char* sdfOut = (unsigned char*)fons__tmpalloc(gw * gh, stash);
            if (sdfOut) {
                sdfCoverageToDistanceField(sdfOut, gw, bdst, gw, gh, stash->params.width);

                // Copy SDF output back to the strided texture data.
                i = 0;
                for (y = 0; y < gh; y++) {
                    int yw = y * stash->params.width;
                    for (x = 0; x < gw; x++) {
                        bdst[x + yw] = sdfOut[i];
                        i++;
                    }
                }
                fons__tmpfree(sdfOut, stash);
            }
        }

        stash->nscratch = 0;
    }

    stash->dirtyRect[0] = fons__mini(stash->dirtyRect[0], glyph->x0);
    stash->dirtyRect[1] = fons__mini(stash->dirtyRect[1], glyph->y0);
    stash->dirtyRect[2] = fons__maxi(stash->dirtyRect[2], glyph->x1);
    stash->dirtyRect[3] = fons__maxi(stash->dirtyRect[3], glyph->y1);

    return glyph;
}

static void fons__getQuad(FONScontext* stash, FONSfont* font,
                          int prevGlyphIndex, FONSglyph* glyph,
                          float scale, float spacing, float* x, float* y, FONSquad* q,
                          int useShaping) {
    float rx,ry,xoff,yoff,x0,y0,x1,y1,xadv,yadv;

    if(!useShaping) {
        if (prevGlyphIndex != -1) {
            float adv = fons__tt_getGlyphKernAdvance(&font->font, prevGlyphIndex, glyph->index) * scale;
            *x += (int)(adv + spacing + 0.5f);
        }

        // Each glyph has 2px border to allow good interpolation,
        // one pixel to prevent leaking, and one to allow good interpolation for rendering.
        // Inset the texture region by one pixel for corret interpolation.
        xoff = (short)(glyph->xoff+1);
        yoff = (short)(glyph->yoff+1);
        q->s0 = x0 = (float)(glyph->x0+1);
        q->t0 = y0 = (float)(glyph->y0+1);
        q->s1 = x1 = (float)(glyph->x1-1);
        q->t1 = y1 = (float)(glyph->y1-1);

        if (stash->params.flags & FONS_NORMALIZE_TEX_COORDS) {
          q->s0 *= stash->itw;
          q->t0 *= stash->ith;
          q->s1 *= stash->itw;
          q->t1 *= stash->ith;
        }

        if (stash->params.flags & FONS_ZERO_TOPLEFT) {
            rx = (float)(int)(*x + xoff);
            ry = (float)(int)(*y + yoff);

            q->x0 = rx;
            q->y0 = ry;
            q->x1 = rx + x1 - x0;
            q->y1 = ry + y1 - y0;

        } else {
            rx = (float)(int)(*x + xoff);
            ry = (float)(int)(*y - yoff);

            q->x0 = rx;
            q->y0 = ry;
            q->x1 = rx + x1 - x0;
            q->y1 = ry - y1 + y0;

        }

        *x += (int)(glyph->xadv / 10.0f + 0.5f);
    } else {
        // TODO : kerning
        FONSshapingRes* shaping = stash->shaping->result;
        int it = stash->shaping->it;
        float unitFontScale = fons__tt_getUnitScale();

        xadv = (float)shaping->advance[it] * unitFontScale;
        yadv = (float)shaping->advance[it+1] * unitFontScale;
        xoff = (float)shaping->offset[it] * unitFontScale + 1;
        yoff = (float)shaping->offset[it+1] * unitFontScale + 1;
        q->s0 = x0 = (float)(glyph->x0+1);
        q->t0 = y0 = (float)(glyph->y0+1);
        q->s1 = x1 = (float)(glyph->x1-1);
        q->t1 = y1 = (float)(glyph->y1-1);

        if (stash->params.flags & FONS_NORMALIZE_TEX_COORDS) {
            q->s0 *= stash->itw;
            q->t0 *= stash->ith;
            q->s1 *= stash->itw;
            q->t1 *= stash->ith;
        }

        rx = *x + xoff;
        ry = *y + yoff;

        q->x0 = rx + glyph->xoff;
        q->y0 = ry + glyph->yoff;
        q->x1 = q->x0 + x1 - x0;
        q->y1 = q->y0 + y1 - y0;

        *x += (int)(xadv + 0.5f);
        *y += (int)(yadv + 0.5f);
    }
}

static void fons__flush(FONScontext* stash, const char clear) {
    // Flush texture
    if (stash->dirtyRect[0] < stash->dirtyRect[2] && stash->dirtyRect[1] < stash->dirtyRect[3]) {
        if (stash->params.renderUpdate != NULL)
            stash->params.renderUpdate(stash->params.userPtr, stash->dirtyRect, stash->texData);
        // Reset dirty rect
        stash->dirtyRect[0] = stash->params.width;
        stash->dirtyRect[1] = stash->params.height;
        stash->dirtyRect[2] = 0;
        stash->dirtyRect[3] = 0;
    }

    // Flush triangles
    if (stash->nverts > 0) {
        if (stash->params.renderDraw != NULL)
            stash->params.renderDraw(stash->params.userPtr, stash->verts, stash->tcoords, stash->colors, stash->nverts);
        if(clear)
            stash->nverts = 0;
    }
}

static __inline void fons__vertex(FONScontext* stash, float x, float y, float s, float t, unsigned int c) {
    stash->verts[stash->nverts*2+0] = x;
    stash->verts[stash->nverts*2+1] = y;
    stash->tcoords[stash->nverts*2+0] = s;
    stash->tcoords[stash->nverts*2+1] = t;
    stash->colors[stash->nverts] = c;
    stash->nverts++;
}

static float fons__getVertAlign(FONScontext* stash, FONSfont* font, int align, short isize) {
    if (stash->params.flags & FONS_ZERO_TOPLEFT) {
        if (align & FONS_ALIGN_TOP) {
            return font->ascender * (float)isize/10.0f;
        } else if (align & FONS_ALIGN_MIDDLE) {
            return (font->ascender + font->descender) / 2.0f * (float)isize/10.0f;
        } else if (align & FONS_ALIGN_BASELINE) {
            return 0.0f;
        } else if (align & FONS_ALIGN_BOTTOM) {
            return font->descender * (float)isize/10.0f;
        }
    } else {
        if (align & FONS_ALIGN_TOP) {
            return -font->ascender * (float)isize/10.0f;
        } else if (align & FONS_ALIGN_MIDDLE) {
            return -(font->ascender + font->descender) / 2.0f * (float)isize/10.0f;
        } else if (align & FONS_ALIGN_BASELINE) {
            return 0.0f;
        } else if (align & FONS_ALIGN_BOTTOM) {
            return -font->descender * (float)isize/10.0f;
        }
    }
    return 0.0;
}

static __inline void fons__vertices(FONScontext* stash, FONSquad q, FONSstate* state) {
    if (stash->params.pushQuad) {
        stash->params.pushQuad(stash->params.userPtr, &q);
        return;
    }
    fons__vertex(stash, q.x0, q.y0, q.s0, q.t0, state->color);
    fons__vertex(stash, q.x1, q.y1, q.s1, q.t1, state->color);
    fons__vertex(stash, q.x1, q.y0, q.s1, q.t0, state->color);

    fons__vertex(stash, q.x0, q.y0, q.s0, q.t0, state->color);
    fons__vertex(stash, q.x0, q.y1, q.s0, q.t1, state->color);
    fons__vertex(stash, q.x1, q.y1, q.s1, q.t1, state->color);
}

void fonsSetShaping(FONScontext* stash) {
    stash->shaping->customConfig = false;
    fons__getState(stash)->useShaping = true;
}

void fonsSetShaping(FONScontext* stash, FONSscript script, FONSdirection direction, FONSlanguage language) {
    stash->shaping->script = script;
    stash->shaping->direction = direction;
    stash->shaping->language = language;
    stash->shaping->customConfig = true;
    fons__getState(stash)->useShaping = true;
}

bool fonsTextDrawable(FONScontext* stash, const char* str, const char* end, char cacheshaping) {
    if (stash == NULL) return -1;

    FONSstate* state = fons__getState(stash);
    unsigned int codepoint;
    FONSfont* font;
    int useShaping;

    if (state->font < 0 || state->font >= stash->nfonts) {
        return false;
    }

    font = stash->fonts[state->font];
    if (font->data == NULL) {
        return false;
    }

    short isize = (short)(state->size * 10.0f);
    useShaping = font->font.shaper != NULL && fons__getState(stash)->useShaping;

    if(useShaping) {
        FONSshaping* shaping = stash->shaping;

        if(shaping) {
            unsigned int i, j;
            shaping->result = (FONSshapingRes*) malloc(sizeof(FONSshapingRes));

            fons__tt_setPixelSize(&font->font, (float)isize / 10.0f);
            fons__hb_shape(stash, str, font);

            for (i = 0, j = 0; i < shaping->result->glyphCount; i++, j+=2) {
                shaping->it = j;
                codepoint = shaping->result->codepoints[i];
                if (codepoint == 0) {
                    fons__clearShaping(stash);
                    return false;
                }
            }

            if(!cacheshaping) {
                fons__clearShaping(stash);
            }
        } else {
            return false;
        }
    } else {
        int g;
        unsigned int utf8state = 0;

        if (end == NULL)
            end = str + strlen(str);

        for (; str != end; ++str) {
            if (fons__decutf8(&utf8state, &codepoint, *(const unsigned char*)str))
                continue;

            if (codepoint == 0) {
                return false;
            }

            g = fons__tt_getGlyphIndex(&font->font, codepoint, 0);
            if (g == 0) {
                return false;
            }
        }
    }

    return true;
}

float fonsDrawText(FONScontext* stash,
                   float x, float y,
                   const char* str, const char* end,
                   const char clear) {
    if (stash == NULL) return x;

    FONSstate* state = fons__getState(stash);
    unsigned int codepoint;
    unsigned int utf8state = 0;
    FONSglyph* glyph = NULL;
    FONSquad q;
    int prevGlyphIndex = -1;
    short isize = (short)(state->size*10.0f);
    short iblur = (short)state->blur;
    float scale;
    FONSfont* font;
    int useShaping;
    bool invalid = false;

    if (state->font < 0 || state->font >= stash->nfonts) return x;
    font = stash->fonts[state->font];
    if (font->data == NULL) return x;

    scale = fons__tt_getPixelHeightScale(&font->font, (float)isize/10.0f);

    useShaping = font->font.shaper != NULL && fons__getState(stash)->useShaping;

    y += fons__getVertAlign(stash, font, state->align, isize);

    if (useShaping) {
        FONSshaping* shaping = stash->shaping;

        if(shaping) {
            unsigned int i, j;

            // shaping result hasn't been cached
            if (!shaping->result) {
                shaping->result = (FONSshapingRes*) malloc(sizeof(FONSshapingRes));

                // harfbuzz needs this to be called to be able to perform shaping
                fons__tt_setPixelSize(&font->font, (float)isize/10.0f);

                fons__hb_shape(stash, str, font);
            }

            for (i = 0, j = 0; i < shaping->result->glyphCount; i++, j+=2) {
                shaping->it = j;
                codepoint = shaping->result->codepoints[i];
                if (codepoint == 0) {
                    continue;
                }
                glyph = fons__getGlyph(stash, font, codepoint, isize, iblur, state->blurType);

                if (glyph != NULL) {
                    fons__getQuad(stash, font, prevGlyphIndex, glyph, scale, state->spacing, &x, &y, &q, useShaping);

                    if (stash->nverts+6 > FONS_VERTEX_COUNT)
                        fons__flush(stash, clear);

                    fons__vertices(stash, q, state);
                } else {
                    invalid += true;
                }
                prevGlyphIndex = glyph != NULL ? glyph->index : -1;
            }

            fons__flush(stash, clear);

            if(clear) {
                fons__clearShaping(stash);
            }
        }
    } else {
        float width = 0.0f;

        if (end == NULL)
            end = str + strlen(str);

        // Align horizontally
        if (state->align & FONS_ALIGN_LEFT) {
            // empty
        } else if (state->align & FONS_ALIGN_RIGHT) {
            width = fonsTextBounds(stash, x,y, str, end, NULL);
            x -= width;
        } else if (state->align & FONS_ALIGN_CENTER) {
            width = fonsTextBounds(stash, x,y, str, end, NULL);
            x -= width * 0.5f;
        }

        // Align vertically.
        // y += fons__getVertAlign(stash, font, state->align, isize);

        for (; str != end; ++str) {
            if (fons__decutf8(&utf8state, &codepoint, *(const unsigned char*)str))
                continue;

            glyph = fons__getGlyph(stash, font, codepoint, isize, iblur, state->blurType);

            if (glyph != NULL) {
                fons__getQuad(stash, font, prevGlyphIndex, glyph, scale, state->spacing, &x, &y, &q, useShaping);

                if (stash->nverts+6 > FONS_VERTEX_COUNT)
                    fons__flush(stash, clear);

                fons__vertices(stash, q, state);
            } 
            else {
                std::cout << "Warning: Glyph not found for codepoint: " << char(codepoint) << std::endl;
                invalid += true;
            }
            prevGlyphIndex = glyph != NULL ? glyph->index : -1;
        }
        fons__flush(stash, clear);
    }

    if (invalid) {
        fons__flush(stash, 1);
        return -1.f;
    }
    return x;
}

int fonsTextIterInit(FONScontext* stash, FONStextIter* iter,
                     float x, float y, const char* str, const char* end) {
    if (stash == NULL) return 0;

    FONSstate* state = fons__getState(stash);
    float width;

    memset(iter, 0, sizeof(*iter));

    if (state->font < 0 || state->font >= stash->nfonts) return 0;
    iter->font = stash->fonts[state->font];
    if (iter->font->data == NULL) return 0;

    iter->isize = (short)(state->size*10.0f);
    iter->iblur = (short)state->blur;
    iter->blurType = (short)state->blurType;
    iter->scale = fons__tt_getPixelHeightScale(&iter->font->font, (float)iter->isize/10.0f);

    // Align horizontally
    if (state->align & FONS_ALIGN_LEFT) {
        // empty
    } else if (state->align & FONS_ALIGN_RIGHT) {
        width = fonsTextBounds(stash, x,y, str, end, NULL);
        x -= width;
    } else if (state->align & FONS_ALIGN_CENTER) {
        width = fonsTextBounds(stash, x,y, str, end, NULL);
        x -= width * 0.5f;
    }
    // Align vertically.
    y += fons__getVertAlign(stash, iter->font, state->align, iter->isize);

    if (end == NULL)
        end = str + strlen(str);

    iter->x = iter->nextx = x;
    iter->y = iter->nexty = y;
    iter->spacing = state->spacing;
    iter->str = str;
    iter->next = str;
    iter->end = end;
    iter->codepoint = 0;
    iter->prevGlyphIndex = -1;

    return 1;
}

int fonsTextIterNext(FONScontext* stash, FONStextIter* iter, FONSquad* quad) {
    FONSglyph* glyph = NULL;
    const char* str = iter->next;
    iter->str = iter->next;

    if (str == iter->end)
        return 0;

    for (; str != iter->end; str++) {
        if (fons__decutf8(&iter->utf8state, &iter->codepoint, *(const unsigned char*)str))
            continue;
        str++;
        // Get glyph and quad
        iter->x = iter->nextx;
        iter->y = iter->nexty;
        glyph = fons__getGlyph(stash, iter->font, iter->codepoint, iter->isize, iter->iblur, iter->blurType);
        if (glyph != NULL)
            fons__getQuad(stash, iter->font, iter->prevGlyphIndex, glyph, iter->scale, iter->spacing, &iter->nextx, &iter->nexty, quad, 0 /* TODO */);
        iter->prevGlyphIndex = glyph != NULL ? glyph->index : -1;
        break;
    }
    iter->next = str;

    return 1;
}

float fonsTextBounds(FONScontext* stash,
                     float x, float y,
                     const char* str, const char* end,
                     float* bounds) {
    if (stash == NULL) return 0;

    FONSstate* state = fons__getState(stash);
    unsigned int codepoint;
    unsigned int utf8state = 0;
    FONSquad q;
    FONSglyph* glyph = NULL;
    int prevGlyphIndex = -1;
    short isize = (short)(state->size*10.0f);
    short iblur = (short)state->blur;
    int blurType = state->blurType;
    float scale;
    FONSfont* font;
    float startx, advance;
    float minx, miny, maxx, maxy;

    if (state->font < 0 || state->font >= stash->nfonts) return 0;
    font = stash->fonts[state->font];
    if (font->data == NULL) return 0;

    scale = fons__tt_getPixelHeightScale(&font->font, (float)isize/10.0f);

    // Align vertically.
    y += fons__getVertAlign(stash, font, state->align, isize);

    minx = maxx = x;
    miny = maxy = y;
    startx = x;

    if (end == NULL)
        end = str + strlen(str);

    for (; str != end; ++str) {
        if (fons__decutf8(&utf8state, &codepoint, *(const unsigned char*)str))
            continue;
        glyph = fons__getGlyph(stash, font, codepoint, isize, iblur, blurType);
        if (glyph != NULL) {
            fons__getQuad(stash, font, prevGlyphIndex, glyph, scale, state->spacing, &x, &y, &q, 0 /* TODO */);
            if (q.x0 < minx) minx = q.x0;
            if (q.x1 > maxx) maxx = q.x1;
            if (stash->params.flags & FONS_ZERO_TOPLEFT) {
                if (q.y0 < miny) miny = q.y0;
                if (q.y1 > maxy) maxy = q.y1;
            } else {
                if (q.y1 < miny) miny = q.y1;
                if (q.y0 > maxy) maxy = q.y0;
            }
        }
        prevGlyphIndex = glyph != NULL ? glyph->index : -1;
    }

    advance = x - startx;

    // Align horizontally
    if (state->align & FONS_ALIGN_LEFT) {
        // empty
    } else if (state->align & FONS_ALIGN_RIGHT) {
        minx -= advance;
        maxx -= advance;
    } else if (state->align & FONS_ALIGN_CENTER) {
        minx -= advance * 0.5f;
        maxx -= advance * 0.5f;
    }

    if (bounds) {
        bounds[0] = minx;
        bounds[1] = miny;
        bounds[2] = maxx - minx;
        bounds[3] = maxy - miny;
    }

    return advance;
}

void fonsVertMetrics(FONScontext* stash,
                     float* ascender, float* descender, float* lineh) {
    if (stash == NULL) return;

    FONSfont* font;
    FONSstate* state = fons__getState(stash);
    short isize;

    if (state->font < 0 || state->font >= stash->nfonts) return;
    font = stash->fonts[state->font];
    isize = (short)(state->size*10.0f);
    if (font->data == NULL) return;

    if (ascender)
        *ascender = font->ascender*isize/10.0f;
    if (descender)
        *descender = font->descender*isize/10.0f;
    if (lineh)
        *lineh = font->lineh*isize/10.0f;
}

void fonsLineBounds(FONScontext* stash, float y, float* miny, float* maxy) {
    if (stash == NULL) return;

    FONSfont* font;
    FONSstate* state = fons__getState(stash);
    short isize;

    if (state->font < 0 || state->font >= stash->nfonts) return;
    font = stash->fonts[state->font];
    isize = (short)(state->size*10.0f);
    if (font->data == NULL) return;

    y += fons__getVertAlign(stash, font, state->align, isize);

    if (stash->params.flags & FONS_ZERO_TOPLEFT) {
        *miny = y - font->ascender * (float)isize/10.0f;
        *maxy = *miny + font->lineh*isize/10.0f;
    } else {
        *maxy = y + font->descender * (float)isize/10.0f;
        *miny = *maxy - font->lineh*isize/10.0f;
    }
}

const unsigned char* fonsGetTextureData(FONScontext* stash, int* width, int* height) {
    if (width != NULL)
        *width = stash->params.width;
    if (height != NULL)
        *height = stash->params.height;
    return stash->texData;
}

int fonsValidateTexture(FONScontext* stash, int* dirty) {
    if (stash->dirtyRect[0] < stash->dirtyRect[2] && stash->dirtyRect[1] < stash->dirtyRect[3]) {
        dirty[0] = stash->dirtyRect[0];
        dirty[1] = stash->dirtyRect[1];
        dirty[2] = stash->dirtyRect[2];
        dirty[3] = stash->dirtyRect[3];
        // Reset dirty rect
        stash->dirtyRect[0] = stash->params.width;
        stash->dirtyRect[1] = stash->params.height;
        stash->dirtyRect[2] = 0;
        stash->dirtyRect[3] = 0;
        return 1;
    }
    return 0;
}

void fonsDeleteInternal(FONScontext* stash) {
    int i;
    if (stash == NULL) return;

    if (stash->params.renderDelete)
        stash->params.renderDelete(stash->params.userPtr);

    fons__deleteShaping(stash);
    for (i = 0; i < stash->nfonts; ++i)
        fons__freeFont(stash->fonts[i]);

    if (stash->atlas) fons__deleteAtlas(stash->atlas);
    if (stash->fonts) free(stash->fonts);
    if (stash->texData) free(stash->texData);
    if (stash->scratch) free(stash->scratch);
    free(stash);
}

void fonsSetErrorCallback(FONScontext* stash, void (*callback)(void* uptr, int error, int val), void* uptr) {
    if (stash == NULL) return;
    stash->handleError = callback;
    stash->errorUptr = uptr;
}

void fonsGetAtlasSize(FONScontext* stash, int* width, int* height) {
    if (stash == NULL) return;
    *width = stash->params.width;
    *height = stash->params.height;
}

int fonsExpandAtlas(FONScontext* stash, int width, int height, const char clear) {
    int i, maxy = 0;
    unsigned char* data = NULL;
    if (stash == NULL) return 0;

    width = fons__maxi(width, stash->params.width);
    height = fons__maxi(height, stash->params.height);

    if (width == stash->params.width && height == stash->params.height)
        return 1;

    // Flush pending glyphs.
    fons__flush(stash, clear);

    // Create new texture
    if (stash->params.renderResize != NULL) {
        if (stash->params.renderResize(stash->params.userPtr, width, height) == 0)
            return 0;
    }
    // Copy old texture data over.
    data = (unsigned char*)malloc(width * height);
    if (data == NULL)
        return 0;
    for (i = 0; i < stash->params.height; i++) {
        unsigned char* dst = &data[i*width];
        unsigned char* src = &stash->texData[i*stash->params.width];
        memcpy(dst, src, stash->params.width);
        if (width > stash->params.width)
            memset(dst+stash->params.width, 0, width - stash->params.width);
    }
    if (height > stash->params.height)
        memset(&data[stash->params.height * width], 0, (height - stash->params.height) * width);

    free(stash->texData);
    stash->texData = data;

    // Increase atlas size
    fons__atlasExpand(stash->atlas, width, height);

    // Add axisting data as dirty.
    for (i = 0; i < stash->atlas->nnodes; i++)
        maxy = fons__maxi(maxy, stash->atlas->nodes[i].y);
    stash->dirtyRect[0] = 0;
    stash->dirtyRect[1] = 0;
    stash->dirtyRect[2] = stash->params.width;
    stash->dirtyRect[3] = maxy;

    stash->params.width = width;
    stash->params.height = height;
    stash->itw = 1.0f/stash->params.width;
    stash->ith = 1.0f/stash->params.height;

    return 1;
}

int fonsResetAtlas(FONScontext* stash, int width, int height, const char clear) {
    int i, j;
    if (stash == NULL) return 0;

    // Flush pending glyphs.
    fons__flush(stash, clear);

    // Create new texture
    if (stash->params.renderResize != NULL) {
        if (stash->params.renderResize(stash->params.userPtr, width, height) == 0)
            return 0;
    }

    // Reset atlas
    fons__atlasReset(stash->atlas, width, height);

    // Clear texture data.
    stash->texData = (unsigned char*)realloc(stash->texData, width * height);
    if (stash->texData == NULL) return 0;
    memset(stash->texData, 0, width * height);

    // Reset dirty rect
    stash->dirtyRect[0] = width;
    stash->dirtyRect[1] = height;
    stash->dirtyRect[2] = 0;
    stash->dirtyRect[3] = 0;

    // Reset cached glyphs
    for (i = 0; i < stash->nfonts; i++) {
        FONSfont* font = stash->fonts[i];
        font->nglyphs = 0;
        for (j = 0; j < FONS_HASH_LUT_SIZE; j++)
            font->lut[j] = -1;
    }

    stash->params.width = width;
    stash->params.height = height;
    stash->itw = 1.0f/stash->params.width;
    stash->ith = 1.0f/stash->params.height;

    // Add white rect at 0,0 for debug drawing.
    fons__addWhiteRect(stash, 2,2);

    return 1;
}


#endif