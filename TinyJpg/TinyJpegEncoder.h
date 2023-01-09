/**
 * tiny_jpeg.h
 *
 * Tiny JPEG Encoder
 *  - Sergio Gonzalez
 *
 * This is a readable and simple single-header JPEG encoder.
 *
 * Features
 *  - Implements Baseline DCT JPEG compression.
 *  - No dynamic allocations.
 *
 * This library is coded in the spirit of the stb libraries and mostly follows
 * the stb guidelines.
 *
 * It is written in C99. And depends on the C standard library.
 * Works with C++11
 *
 *
 * ==== Thanks ====
 *
 *  AssociationSirius (Bug reports)
 *  Bernard van Gastel (Thread-safe defaults, BSD compilation)
 *
 *
 * ==== License ====
 *
 * This software is in the public domain. Where that dedication is not
 * recognized, you are granted a perpetual, irrevocable license to copy and
 * modify this file as you see fit.
 *
 */
#ifndef _TINY_JPEG_H_
#define _TINY_JPEG_H_

#ifdef __cplusplus
extern "C"
{
#endif

//Only use zero for debugging and/or inspection.
#define TJE_USE_FAST_DCT 1
	
// C std lib
#include <assert.h>
#include <inttypes.h>
#include <math.h>   // floorf, ceilf
#include <stdio.h>  // FILE, puts
#include <string.h> // memcpy


#define TJEI_BUFFER_SIZE 1024
typedef void tje_write_func(void* context, void* data, int size);

typedef struct
{
    void*           context;
    tje_write_func* func;
} TJEWriteContext;

typedef struct
{
    // Huffman data.
    uint8_t         ehuffsize[4][257];
    uint16_t        ehuffcode[4][256];
    uint8_t const * ht_bits[4];
    uint8_t const * ht_vals[4];

    // Cuantization tables.
    uint8_t         qt_luma[64];
    uint8_t         qt_chroma[64];

    // fwrite by default. User-defined when using tje_encode_with_func.
    TJEWriteContext write_context;

    // Buffered output. Big performance win when using the usual stdlib implementations.
    size_t          output_buffer_count;
    uint8_t         output_buffer[TJEI_BUFFER_SIZE];
} TJEState;

// TODO: Get rid of packed structs!
#pragma pack(push)
#pragma pack(1)
typedef struct
{
    uint16_t SOI;
    // JFIF header.
    uint16_t APP0;
    uint16_t jfif_len;
    uint8_t  jfif_id[5];
    uint16_t version;
    uint8_t  units;
    uint16_t x_density;
    uint16_t y_density;
    uint8_t  x_thumb;
    uint8_t  y_thumb;
} TJEJPEGHeader;

typedef struct
{
    uint16_t com;
    uint16_t com_len;
    char     com_str[32];
} TJEJPEGComment;

// Helper struct for TJEFrameHeader (below).
typedef struct
{
    uint8_t  component_id;
    uint8_t  sampling_factors;    // most significant 4 bits: horizontal. 4 LSB: vertical (A.1.1)
    uint8_t  qt;                  // Quantization table selector.
} TJEComponentSpec;

typedef struct
{
    uint16_t         SOF;
    uint16_t         len;                   // 8 + 3 * frame.num_components
    uint8_t          precision;             // Sample precision (bits per sample).
    uint16_t         height;
    uint16_t         width;
    uint8_t          num_components;        // For this implementation, will be equal to 3.
    TJEComponentSpec component_spec[3];
} TJEFrameHeader;

typedef struct
{
    uint8_t component_id;                 // Just as with TJEComponentSpec
    uint8_t dc_ac;                        // (dc|ac)
} TJEFrameComponentSpec;

typedef struct
{
    uint16_t              SOS;
    uint16_t              len;
    uint8_t               num_components;  // 3.
    TJEFrameComponentSpec component_spec[3];
    uint8_t               first;  // 0
    uint8_t               last;  // 63
    uint8_t               ah_al;  // o
} TJEScanHeader;
#pragma pack(pop)


typedef enum
{
    TJEI_DC = 0,
    TJEI_AC = 1
} TJEHuffmanTableClass;

enum {
    TJEI_LUMA_DC,
    TJEI_LUMA_AC,
    TJEI_CHROMA_DC,
    TJEI_CHROMA_AC,
};

struct TJEProcessedQT
{
    float chroma[64];
    float luma[64];
};



// - tje_encode_with_func -
//
// Usage
//  Same as tje_encode_to_file_at_quality, but it takes a callback that knows
//  how to handle (or ignore) `context`. The callback receives an array `data`
//  of `size` bytes, which can be written directly to a file. There is no need
//  to free the data.

int tje_encode_with_func(tje_write_func* func,
                         void* context,
                         const int quality,
                         const int width,
                         const int height,
                         const int num_components,
                         const unsigned char* src_data);

// - tje_encode_to_file_at_quality -
//
// Usage:
//  Takes bitmap data and writes a JPEG-encoded image to disk.
//
//  PARAMETERS
//      dest_path:          filename to which we will write. e.g. "out.jpg"
//      quality:            3: Highest. Compression varies wildly (between 1/3 and 1/20).
//                          2: Very good quality. About 1/2 the size of 3.
//                          1: Noticeable. About 1/6 the size of 3, or 1/3 the size of 2.
//      width, height:      image size in pixels
//      num_components:     3 is RGB. 4 is RGBA. Those are the only supported values
//      src_data:           pointer to the pixel data.
//
//  RETURN:
//      0 on error. 1 on success.

int tje_encode_to_file_at_quality(const char* dest_path,
    const int quality,
    const int width,
    const int height,
    const int num_components,
    const unsigned char* src_data);
											 
#ifdef __cplusplus
}  // extern C
#endif

#endif

