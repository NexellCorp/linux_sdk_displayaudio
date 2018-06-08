#ifndef KEYBOARDDIALOG_H
#define KEYBOARDDIALOG_H

#include <CNX_BaseDialog.h>
#include "KeyboardFrame.h"

class KeyboardDialog : public CNX_BaseDialog
{
	Q_OBJECT

public:
	explicit KeyboardDialog(QWidget *parent = 0);
	~KeyboardDialog();

	void SetText(QString text);

	QString GetText();

private:
	KeyboardFrame* m_pKeyboardFrame;

};

#endif // KEYBOARDDIALOG_H
