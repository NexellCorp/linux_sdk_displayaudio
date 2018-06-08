#ifndef NXPROCESSMANAGER_H
#define NXPROCESSMANAGER_H

#include <QObject>
#include <QTime>

#include <NX_Type.h>
#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>

class NxProcessManager : public QObject
{
	Q_OBJECT

public:
	explicit NxProcessManager(QObject *parent = 0);
	~NxProcessManager();

public slots:
	void execute( QString exec, int32_t bDirect = false );

	bool check( QString exec );
	void show( QString exec );
	void hide( QString exec );

private:
	QObject* m_pParent;
};

#endif // NXPROCESSMANAGER_H
