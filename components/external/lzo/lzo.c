/* lzo.c - location for general purpose minilzo functions
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 * Change Logs:
 * Date           Author       Notes
 */
#include <rtthread.h>
#include "minilzo.h"
#include <rtgui/filerw.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#endif
#define RT_USING_LZO
#if defined(RT_USING_LZO)

/* the worst of allocation */
#define LZO1X_WORST(x) ( (x) + ((x)/16) + 64 + 3 ) 

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

char* parse_lzo_error_code(int error_code)
{
	switch(error_code)
	{
	case LZO_E_ERROR:                 return "error";
	case LZO_E_OUT_OF_MEMORY:         return "out of memory";
	case LZO_E_NOT_COMPRESSIBLE:      return "not compressible";
	case LZO_E_INPUT_OVERRUN:         return "input overrun";
	case LZO_E_OUTPUT_OVERRUN:        return "output overrun";
	case LZO_E_LOOKBEHIND_OVERRUN:    return "lookbehind overrun";
	case LZO_E_EOF_NOT_FOUND:         return "eof not found";
	case LZO_E_INPUT_NOT_CONSUMED:    return "input not consumed";
	case LZO_E_NOT_YET_IMPLEMENTED:   return "not yet implemented";    /* [not used right now] */
	case LZO_E_INVALID_ARGUMENT:      return "invalid argument";
	default: return "none";
	}
}

int lzo(char *srcfile, char *destfile)
{
	int result;
	struct rtgui_filerw *file;
	struct stat s;
	lzo_bytep in;
	lzo_bytep out;
	lzo_uint in_len, out_len;

	rt_memset(&s, 0, sizeof(struct stat));
	stat(srcfile, &s);
	in_len = s.st_size; 
	
	in = rt_malloc(in_len); 
	if (in == RT_NULL) return -1;
	out = rt_malloc(LZO1X_WORST(in_len));
	if (out == RT_NULL)	return -1;

	file = rtgui_filerw_create_file(srcfile, "rb");
	if(file == RT_NULL) 
	{
		result = -1;
		goto _exit;
	}
	rtgui_filerw_read(file, in, in_len, 1); 
	rtgui_filerw_close(file);

	result = lzo1x_1_compress(in, in_len, out, &out_len, wrkmem);
	if(result != LZO_E_OK)
	{
		rt_kprintf("internal error - compression failed: \nerr_code:(%d) %s, %s.\n", 
			result, parse_lzo_error_code(result), "Please use the binary access");
		goto _exit;
	}

	file = rtgui_filerw_create_file(destfile, "wb");
	if(file == RT_NULL)
	{
		result = -1;
		goto _exit;
	}
	
	rtgui_filerw_write(file, &in_len, sizeof(lzo_uint), 1);	/* source file len */
	rtgui_filerw_write(file, out, out_len, 1); 
	rtgui_filerw_close(file);
	rt_kprintf("compress lzo ok!\n");
	result = 0;

_exit:
	rt_free(in);
	rt_free(out);
	return result;
}
#ifdef RT_USING_FINSH
FINSH_FUNCTION_EXPORT(lzo, compress a file. usage:lzo(src, dest));
#endif

int lzode(char *srcfile, char *destfile)
{
	int result;
	struct rtgui_filerw *file;
	struct stat s;
	lzo_bytep in=RT_NULL;
	lzo_bytep out=RT_NULL;
	lzo_uint in_len, out_len;

	rt_memset(&s, 0, sizeof(struct stat));
	stat(srcfile, &s);
	in_len = s.st_size; 
	
	file = rtgui_filerw_create_file(srcfile, "rb");
	if(file == RT_NULL) return 0;

	rtgui_filerw_read(file, &out_len, sizeof(lzo_uint), 1); /* source file len */
	in_len -= sizeof(lzo_uint);
	in = rt_malloc(in_len); 
	if (in == RT_NULL) return -1;
	out = rt_malloc(out_len); 
	if (out == RT_NULL)	return -1;

	rtgui_filerw_read(file, in, in_len, 1); 
	rtgui_filerw_close(file);

	result = lzo1x_decompress(in, in_len, out, &out_len, RT_NULL);
	if(result != LZO_E_OK)
	{
		rt_kprintf("internal error - decompression failed: \nerr_code:(%d) %s, %s.\n", 
			result, parse_lzo_error_code(result), "Please use the binary access");
		goto _exit;
	}

	file = rtgui_filerw_create_file(destfile, "wb");
	if(file == RT_NULL)
	{
		result = -1;
		goto _exit;
	}
	rtgui_filerw_write(file, out, out_len, 1);
	rtgui_filerw_close(file);

	rt_kprintf("decompress lzo ok!\n");
	result = 0;

_exit:
	rt_free(in);
	rt_free(out);
	return result;
}
#ifdef RT_USING_FINSH
FINSH_FUNCTION_EXPORT(lzode, decompress a file. usage:lzode(src, dest));
#endif

#endif
