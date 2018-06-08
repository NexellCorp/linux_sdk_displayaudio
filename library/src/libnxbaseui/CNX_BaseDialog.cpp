#include "CNX_BaseDialog.h"
#include <QApplication>
#include <QDesktopWidget>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

//------------------------------------------------------------------------------
#define NX_DEV_FRAMEBUFFER		"/dev/fb"
#define NX_MMAP_DEVICE			"/dev/mem"
#define NX_MMAP_ALIGN			4096		// 0x1000

#define NX_MLC_REG_BASE         0xC0102000
#define NX_MLC_RGB0_REG_OFFSET  0x38
#define NX_MLC_RGB1_REG_OFFSET  0x6c

//------------------------------------------------------------------------------
static unsigned int iomem_map(unsigned int phys, unsigned int len)
{
	unsigned int virt = 0;
	int fd;

	fd = ::open(NX_MMAP_DEVICE, O_RDWR|O_SYNC);
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
		if( iWidth )    *iWidth  = fbvar.xres;
		if( iHeight )   *iHeight = fbvar.yres;
		if( iBpp )      *iBpp    = fbvar.bits_per_pixel;
	}

	close( fd );
	return iRet;
}

CNX_BaseDialog::CNX_BaseDialog(QWidget *parent) :
	QDialog(parent, Qt::FramelessWindowHint)
{
	m_pWidget = NULL;

	int32_t iWidth = 0, iHeight = 0, iBpp = 0, iSize = 0;
	if (0 > GetDisplayInfo(NX_DEV_FRAMEBUFFER, &iWidth, &iHeight, &iBpp) || iWidth == 0 || iHeight == 0 || iBpp == 0)
	{
		setAttribute(Qt::WA_TranslucentBackground, true); // background no update!!
	}

	iSize = iWidth * iHeight * iBpp / 8;

	m_Background = QImage(iWidth, iHeight, QImage::Format_RGBA8888);
	uchar* bits = m_Background.bits();

	uint32_t iBaseReg = iomem_map(NX_MLC_REG_BASE, NX_MMAP_ALIGN);
	uint32_t iBaseCaptureReg = iomem_map(*((unsigned int*)(iBaseReg + NX_MLC_RGB1_REG_OFFSET)), iSize);
	if (iBaseCaptureReg != 0xFFFFFFFF)
	{
		uint8_t* image = (uint8_t*)iBaseCaptureReg;
		uint8_t* temp = new uint8_t[iSize];

		for (int32_t y = 0; y < iHeight; y++)
		{
			// convert BGRA to RGBA
			for (int32_t x = 0; x < iWidth; x++)
			{
				temp[(y*iWidth*4)+(x*4+0)] = image[(y*iWidth*4)+(x*4+2)]; // B -> R
				temp[(y*iWidth*4)+(x*4+1)] = image[(y*iWidth*4)+(x*4+1)]; // G -> G
				temp[(y*iWidth*4)+(x*4+2)] = image[(y*iWidth*4)+(x*4+0)]; // R -> B
				temp[(y*iWidth*4)+(x*4+3)] = image[(y*iWidth*4)+(x*4+3)]; // A -> A
			}
		}
		memcpy(bits, (void*)temp, iSize);
		delete[] temp;
	}

	iomem_free(iBaseCaptureReg, iSize);

	m_ScreenGeometry = QApplication::desktop()->screenGeometry();
	setFixedSize(m_ScreenGeometry.size());
}

CNX_BaseDialog::~CNX_BaseDialog()
{

}

void CNX_BaseDialog::SetWidget(QWidget* widget, int x, int y)
{
	if (m_pWidget == widget)
		return;

	m_pWidget = widget;

	if (m_pWidget)
	{
		if (m_pWidget->width() <= 0 || m_pWidget->height() <= 0)
			return;

		int cx = x < 0 ? m_ScreenGeometry.width() / 2 - m_pWidget->width() / 2 : x;
		int cy = y < 0 ? m_ScreenGeometry.height() / 2 - m_pWidget->height() / 2 : y;

		m_pWidget->move(cx, cy);
	}
}

void CNX_BaseDialog::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.drawImage(0, 0, m_Background);
}
