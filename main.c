#include "TinyJpegEncoder.h"
#include "PicoJpegDecoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//------------------------------------------------------------------------------
static void get_pixel(int* pDst, const uint8_t* pSrc, int luma_only, int num_comps)
{
    int r, g, b;
    if (num_comps == 1)
    {
        r = g = b = pSrc[0];
    }
    else if (luma_only)
    {
        const int YR = 19595, YG = 38470, YB = 7471;
        r = g = b = (pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + 32768) / 65536;
    }
    else
    {
        r = pSrc[0]; g = pSrc[1]; b = pSrc[2];
    }
    pDst[0] = r; pDst[1] = g; pDst[2] = b;
}


static void jpg_write_func(void* context, void* data, int size)
{
    FILE* fd = (FILE*)context;
    fwrite(data, size, 1, fd);
}
//------------------------------------------------------------------------------

#define TEST_DECODE_FROM_FILE 0
#define TEST_DECODE_FROM_MEMORY 1

#if (((TEST_DECODE_FROM_FILE == 1) && (TEST_DECODE_FROM_MEMORY == 1)) || ((TEST_DECODE_FROM_FILE == 0)&&(TEST_DECODE_FROM_MEMORY == 0)))
#error "Must choose only one decode method from file or memory"
#endif

int main(int arg_c, char* arg_v[])
{
    FILE* destFd = NULL;
    char* pSrc_filename = NULL;
    char* pDst_Drawfilename = NULL;
    uint8_t* pDecodeImageData = NULL;
    int width, height, comps, file_size;

    pSrc_filename = "DSC_0010.JPG";
    pDst_Drawfilename = "DSC_0010_out.JPG";

    printf("Source file:      \"%s\"\n", pSrc_filename);
    printf("Destination file: \"%s\"\n", pDst_Drawfilename);

    //读JPG文件并解析为RGB
    pjpeg_scan_type_t scan_type;
#if (TEST_DECODE_FROM_FILE == 1)
    pDecodeImageData = pjpeg_decode_from_file((char*)pSrc_filename, &width, &height, &comps, &scan_type, 0);
#endif

#if (TEST_DECODE_FROM_MEMORY == 1)
    FILE* srcFd = NULL;
    uint8_t* pSrcImageData = NULL;
    uint32_t srcImageSize = 0;

    srcFd = fopen(pSrc_filename, "rb");
    if (NULL == srcFd)
    {
        printf("fopen %s error!\n", pSrc_filename);
        return EXIT_FAILURE;
    }
    fseek(srcFd, 0, SEEK_END);
    srcImageSize = ftell(srcFd);
    fseek(srcFd, 0, SEEK_SET);

    pSrcImageData = (uint8_t*)malloc(srcImageSize);
    if (NULL == pSrcImageData)
    {
        printf("malloc error!\n");
        fclose(srcFd);
        return EXIT_FAILURE;
    }

    fread(pSrcImageData, 1, srcImageSize, srcFd);
    fclose(srcFd);
    pDecodeImageData = pjpeg_decode_from_memory((char*)pSrcImageData, srcImageSize, &width, &height, &comps, &scan_type, 0); 
#endif

    if (!pDecodeImageData) 
    {
        printf("Could not find file\n");
#if (TEST_DECODE_FROM_MEMORY == 1)
        free(pSrcImageData);
#endif
        return EXIT_FAILURE;
    }

    //写入JPG文件

    tje_encode_to_file_at_quality(pDst_Drawfilename, 1, width, height, comps, pDecodeImageData);

#if (TEST_DECODE_FROM_MEMORY == 1)
    if (NULL != pSrcImageData) free(pSrcImageData);
#endif

    if (NULL != pDecodeImageData) free(pDecodeImageData);
}
//------------------------------------------------------------------------------
