#include <QObject>

#ifndef CAUDIOPLAYERSIGNALS_H
#define CAUDIOPLAYERSIGNALS_H

class CAudioPlayerSignals: public QObject
{
	Q_OBJECT

public:
	CAudioPlayerSignals() {}
	virtual ~CAudioPlayerSignals() {}

public slots:
	//	Media Player Callback
	void PlayerCallback( int eventType )
	{
		emit MediaPlayerCallback( eventType );
	}

signals:
	void MediaPlayerCallback(int eventType);

};

#endif // CAUDIOPLAYERSIGNALS_H
