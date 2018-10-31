#ifndef ELIDELABEL_H
#define ELIDELABEL_H

#include <QLabel>

class ElideLabel : public QLabel
{
	Q_OBJECT

public:
	explicit ElideLabel(QWidget *parent = 0);

	void setText(const QString &s);

	void setElideMode(Qt::TextElideMode eMode);

protected:
	void resizeEvent(QResizeEvent *event);

private:
	void setText();

private:
	Qt::TextElideMode m_ElideMode;

	QString m_Text;
};

#endif // ELIDELABEL_H
