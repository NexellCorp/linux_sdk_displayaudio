#include "KeyboardDialog.h"
#include <QEvent>
#include <QMouseEvent>

KeyboardDialog::KeyboardDialog(QWidget *parent) :
	CNX_BaseDialog(parent)
{
	m_pKeyboardFrame = new KeyboardFrame(this);
	connect(m_pKeyboardFrame, SIGNAL(signalAccepted()), this, SLOT(accept()));
	connect(m_pKeyboardFrame, SIGNAL(signalRejected()), this, SLOT(reject()));

	SetWidget((QWidget*)m_pKeyboardFrame);
}

KeyboardDialog::~KeyboardDialog()
{

}

void KeyboardDialog::SetText(QString text)
{
	m_pKeyboardFrame->setText(text);
}

QString KeyboardDialog::GetText()
{
	return m_pKeyboardFrame->text();
}
