#include "NxLauncher.h"
#include <QApplication>
#include <QShowEvent>
#include <QHideEvent>

#include <unistd.h>
#include <sys/time.h>

#include <NX_Type.h>
#include <DAudioKeyDef.h>
#include <NX_KeyReceiver.h>

#define SCREEN_WIDTH		1024
#define SCREEN_HEIGHT		600

static void cbKeyReceiver( void *pObj, int32_t iKey, int32_t iValue )
{
	Q_UNUSED( iKey );

	if( iValue == KEY_RELEASED )
		QCoreApplication::postEvent((NxLauncher*)pObj, new NxKeyEvent(iKey));
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	NxLauncher w;

	QFont font = qApp->font();
	font.setFamily("Sans Serif");
	qApp->setFont(font);

	NXDA_StartKeyProcessing( (void*)&w, cbKeyReceiver );
	w.show();

	// bootanimation exit
	system("killall bootanimation");

	return a.exec();
}
