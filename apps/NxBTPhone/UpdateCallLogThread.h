#ifndef UPDATECALLLOGTHREAD_H
#define UPDATECALLLOGTHREAD_H

#include <QThread>
// for read/parse vCard (*.vcf file format)
#include <io/vCard/VCardReader.h>

struct CallLogInfo {
	QString direction;
	QString owner;
	QString type;
};

class UpdateCallLogThread : public QThread
{
	Q_OBJECT

signals:
	void signalAdd(vector<CallLogInfo> sInfoList);

	void signalCompleted();

public:
	UpdateCallLogThread();

	~UpdateCallLogThread();

	void SetFile(QString path);

	void Start();

	void Stop();

protected:
	virtual void run();

private:
	bool m_bRunning;

	QString m_File;
};

#endif // UPDATECALLLOGTHREAD_H
