#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

//------------------------------------------------------------------------------
enum {
	NX_CAPTURE_TYPE_NONE,
	NX_CAPTURE_TYPE_BMP,
	NX_CAPTURE_TYPE_PNG,
};

#define NX_DEV_FRAMEBUFFER		"/dev/fb"
#define	NX_MMAP_DEVICE			"/dev/mem"
#define	NX_MMAP_ALIGN			4096		// 0x1000

#define NX_MLC_REG_BASE			0xC0102000
#define NX_MLC_RGB0_REG_OFFSET	0x38
#define NX_MLC_RGB1_REG_OFFSET	0x6c

//	For Bitmap
#pragma pack(push,1)
typedef struct{
	uint8_t signature[2];
	uint32_t filesize;
	uint32_t reserved;
	uint32_t fileoffset_to_pixelarray;
} FILEHEADER;

typedef struct{
	uint32_t dibheadersize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bitsperpixel;
	uint32_t compression;
	uint32_t imagesize;
	uint32_t ypixelpermeter;
	uint32_t xpixelpermeter;
	uint32_t numcolorspallette;
	uint32_t mostimpcolor;
} BITMAPINFOHEADER;

typedef struct {
	FILEHEADER fileheader;
	BITMAPINFOHEADER bitmapinfoheader;
} BITMAP;
#pragma pack(pop)

//	For PNG
#define PNG_DEBUG 3
#include <png.h>

//------------------------------------------------------------------------------
static unsigned int iomem_map(unsigned int phys, unsigned int len)
{
	unsigned int virt = 0;
	int fd;

	fd = open(NX_MMAP_DEVICE, O_RDWR|O_SYNC);
	if (0 > fd) {
		printf("Fail, open %s, %s\n", NX_MMAP_DEVICE, strerror(errno));
		return 0;
	}

	if (len & (NX_MMAP_ALIGN-1))
		len = (len & ~(NX_MMAP_ALIGN-1)) + NX_MMAP_ALIGN;

	virt = (unsigned int)mmap((void*)0, len,
				PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)phys);
	if (-1 == (int)virt) {
		printf("Fail: map phys=0x%08x, len=%d, %s \n", phys, len, strerror(errno));
		goto _err;
	}

_err:
	close(fd);
	return virt;
}

//------------------------------------------------------------------------------
static void iomem_free(unsigned int virt, unsigned int len)
{
	if (virt && len)
		munmap((void*)virt, len);
}

//------------------------------------------------------------------------------
static char* GetExtension( char *pFile )
{
	if( pFile == NULL )
	{
		return NULL;
	}

	char *pPtr = pFile + strlen(pFile) - 1;
	while( pPtr != pFile )
	{
		if( *pPtr == '.' ) break;
		pPtr--;
	}

	return (pPtr != pFile) ? (pPtr + 1) : NULL;
}

//------------------------------------------------------------------------------
static int32_t GetDisplayInfo( const char *pDev, int32_t *iWidth, int32_t *iHeight, int32_t *iBpp )
{
	int32_t fd, iRet = -1;
	if( 0 > (fd = open(pDev, O_RDWR)) )
	{
		return -1;
	}

	struct fb_var_screeninfo fbvar;
	iRet = ioctl( fd, FBIOGET_VSCREENINFO, &fbvar );
	if( !iRet )
	{
		if( iWidth )	*iWidth  = fbvar.xres;
		if( iHeight )	*iHeight = fbvar.yres;
		if( iBpp )		*iBpp    = fbvar.bits_per_pixel;
	}

	close( fd );
	return iRet;
}

//------------------------------------------------------------------------------
static int32_t SaveBitmap( const char *pFile, int32_t iWidth, int32_t iHeight, int32_t iBpp, uint32_t iVirAddr )
{
	FILE *pResult = fopen( pFile, "wb");
	if( pResult == NULL )
		return -1;

	BITMAP bm;
	memset( &bm, 0, sizeof(bm) );
	bm.fileheader.signature[0]             = 'B';
	bm.fileheader.signature[1]             = 'M';
	bm.fileheader.filesize                 = iWidth*iHeight*iBpp/8 + sizeof(bm);
	bm.fileheader.fileoffset_to_pixelarray = sizeof(bm);
	bm.bitmapinfoheader.dibheadersize      = sizeof(BITMAPINFOHEADER);  //specifies the number of bytes required by the struct
	bm.bitmapinfoheader.width              = iWidth;  //specifies width in pixels
	bm.bitmapinfoheader.height             = iHeight;  //species height in pixels
	bm.bitmapinfoheader.planes             = 1; //specifies the number of color planes, must be 1
	bm.bitmapinfoheader.bitsperpixel       = iBpp; //specifies the number of bit per pixel
	bm.bitmapinfoheader.compression        = 0;//spcifies the type of compression
	bm.bitmapinfoheader.imagesize          = iWidth*iHeight*iBpp/8;  //size of image in bytes
	bm.bitmapinfoheader.ypixelpermeter     = 0x130B ;  //number of pixels per meter in x axis
	bm.bitmapinfoheader.xpixelpermeter     = 0x130B ;  //number of pixels per meter in y axis
	bm.bitmapinfoheader.numcolorspallette  = 0;

	uint32_t *lineBuf = (uint32_t*)malloc(iWidth*iBpp/8);
	uint32_t *buf = (uint32_t *)malloc(iWidth*iHeight*iBpp/8);
	fwrite(&bm, 1, sizeof(bm), pResult);

	//  Reverse order
	for( int32_t i=0 ; i<iWidth*iHeight; i++ )
	{
		buf[iWidth*iHeight-1-i] = ((uint32_t*)iVirAddr)[i];
	}

	//  flip horizental
	for( int32_t i=0 ; i<iHeight ; i++ )
	{
		for( int32_t j=0 ; j<iWidth ; j++ )
		{
			lineBuf[j] = buf[ i*iWidth + iWidth-j-1 ];
		}
		memcpy( &buf[i*iWidth], lineBuf, iWidth*iBpp/8 );
	}

	//fwrite( (void*)iVirAddr, 1, iWidth*iHeight*iBpp/8, fd );
	fwrite( buf, 1, iWidth*iHeight*iBpp/8, pResult );
	fclose( pResult );

	free( buf );
	return 0;
}

//------------------------------------------------------------------------------
static int32_t SavePng( const char *pFile, int32_t iWidth, int32_t iHeight, int32_t iBpp, uint32_t iVirAddr )
{
	int32_t iRet = -1;
	uint8_t *pInData  = (uint8_t*)iVirAddr;
	uint8_t *pTmpData = NULL;

	png_structp	hPng = NULL;
	png_infop	hPngInfo = NULL;
	png_bytep*	pPngData = NULL;

	FILE *pResult = fopen( pFile, "wb" );
	if( pResult == NULL )
		goto ERROR;

	hPng  = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if( hPng == NULL )
		goto ERROR;

	hPngInfo = png_create_info_struct( hPng );
	if( hPngInfo == NULL )
		goto ERROR;

	png_set_IHDR(
		hPng, hPngInfo,
		iWidth, iHeight, 8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE
	);

	pTmpData = (uint8_t*)malloc( iWidth * iHeight * iBpp/8 );
	for( int32_t i = 0; i < iHeight; i++ )
	{
		for( int32_t j = 0; j < iWidth; j++ )
		{
			// BGRA to RGBA
			pTmpData[(i * iWidth * 4) +  (j * 4 + 0)] = pInData[(i * iWidth * 4) +  (j * 4 + 2)];
			pTmpData[(i * iWidth * 4) +  (j * 4 + 1)] = pInData[(i * iWidth * 4) +  (j * 4 + 1)];
			pTmpData[(i * iWidth * 4) +  (j * 4 + 2)] = pInData[(i * iWidth * 4) +  (j * 4 + 0)];
			pTmpData[(i * iWidth * 4) +  (j * 4 + 3)] = pInData[(i * iWidth * 4) +  (j * 4 + 3)];
		}
	}

	pPngData = (png_bytep*)malloc( sizeof(png_bytep) * iHeight );
	for( int32_t i = 0; i < iHeight; i++ )
	{
		pPngData[i] = pTmpData + i * 4 * iWidth;
	}

	png_init_io( hPng, pResult );
	png_set_rows( hPng, hPngInfo, pPngData );
	png_write_png( hPng, hPngInfo, PNG_TRANSFORM_IDENTITY, NULL );

	iRet = 0;

ERROR:
	if( pResult )
		fclose( pResult );

	if( hPng )
		png_destroy_write_struct( &hPng, &hPngInfo );

	if( pTmpData )
		free( pTmpData );

	if( pPngData )
		free( pPngData );

	return iRet;
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char **argv )
{
	int32_t iCaptureType = NX_CAPTURE_TYPE_NONE;

	if( argc != 2 )
	{
		printf("Usage: %s [OutputFile]               \n", argv[0]);
		printf("       (supported format is bmp, png)\n");
		return -1;
	}

	char *pExtension = GetExtension( argv[1] );
	if( !strcasecmp( pExtension, "png" ) ) iCaptureType = NX_CAPTURE_TYPE_PNG;
	if( !strcasecmp( pExtension, "bmp" ) ) iCaptureType = NX_CAPTURE_TYPE_BMP;

	if( iCaptureType == NX_CAPTURE_TYPE_NONE )
	{
		printf("Not support output format. ( name: %s, ext: %s )\n",
			argv[1], pExtension );
		return -1;
	}

	int32_t iWidth=0, iHeight=0, iBpp=0;
	if( 0 > GetDisplayInfo( NX_DEV_FRAMEBUFFER, &iWidth, &iHeight, &iBpp ) )
	{
		printf("Not support display device. ( %s )\n", NX_DEV_FRAMEBUFFER);
		return -1;
	}

	if( iWidth == 0 || iHeight == 0 || iBpp == 0 )
	{
		printf("Not support display device. ( width: %d, height: %d, bpp: %d )\n",
			iWidth, iHeight, iBpp );
		return -1;
	}

	uint32_t iBaseReg = iomem_map( NX_MLC_REG_BASE, 4096 );
	printf("RGB0 = 0x%08X, RGB1 = 0x%08X, Width = %d, Height = %d, BitPerPiexel = %d\n",
		*((unsigned int*)(iBaseReg + NX_MLC_RGB0_REG_OFFSET)),
		*((unsigned int*)(iBaseReg + NX_MLC_RGB1_REG_OFFSET)),
		iWidth, iHeight, iBpp );

	uint32_t iBaseCaptureReg = iomem_map(*((unsigned int*)(iBaseReg + NX_MLC_RGB1_REG_OFFSET)), iWidth*iHeight*iBpp/8);
	if( iBaseCaptureReg != 0xFFFFFFFF )
	{
		switch( iCaptureType )
		{
			case NX_CAPTURE_TYPE_BMP:	SaveBitmap( argv[1], iWidth, iHeight, iBpp, iBaseCaptureReg );	break;
			case NX_CAPTURE_TYPE_PNG:	SavePng( argv[1], iWidth, iHeight, iBpp, iBaseCaptureReg );		break;
			default:					break;
		}
	}

	iomem_free( iBaseCaptureReg, iWidth*iHeight*iBpp/8 );

	return 0;
}

