#include "KeyboardFrame.h"
#include "ui_KeyboardFrame.h"

KeyboardFrame::KeyboardFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::KeyboardFrame)
{
	ui->setupUi(this);

	initialize();
}

KeyboardFrame::~KeyboardFrame()
{
	delete ui;
}
void KeyboardFrame::initialize()
{
	// 다이얼로그를 이동시킬 수 있는지 판단하는 플래그
	m_bMoveable = false;

	ui->TITLE->installEventFilter(this);

	ui->KEYBOARD->SetObject(this);

	ui->KEYBOARD->SetEnterKeyText("ENTER");
}

bool KeyboardFrame::eventFilter(QObject *object, QEvent *event)
{
	if (object == ui->TITLE && event->type() == QEvent::MouseButtonPress) {
		m_bMoveable = true;
		return false;
	}

	if (object == ui->TITLE && event->type() == QEvent::MouseButtonRelease) {
		m_bMoveable = false;
		return false;
	}

	return QFrame::eventFilter(object, event);
}

void KeyboardFrame::keyPressEvent(QKeyEvent *e)
{
	QKeySequence k(e->key());
	QString text;

	if (Qt::Key_0 <= e->key() && e->key() <= Qt::Key_9) {
		text = k.toString();
	} else if (Qt::Key_A <= e->key() && e->key() <= Qt::Key_Z) {
		text = k.toString();
		if (e->modifiers() & Qt::ShiftModifier)
			text = text.toUpper();
		else
			text = text.toLower();
	} else if (Qt::Key_Space == e->key()) {
		text = " ";
	} else if (Qt::Key_Underscore == e->key()) {
		text = "_";
	} else if (Qt::Key_Minus == e->key()) {
		text = "-";
	} else if (Qt::Key_Comma == e->key()) {
		text = ",";
	} else if (Qt::Key_Period == e->key()) {
		text = ".";
	} else if (Qt::Key_Enter == e->key() || Qt::Key_Return == e->key()) {
		emit signalAccepted();
		return;
	}

	QApplication::sendEvent(ui->EDIT, new QKeyEvent(QEvent::KeyPress, e->key(), e->modifiers(), text));
	QApplication::sendEvent(ui->EDIT, new QKeyEvent(QEvent::KeyRelease, e->key(), e->modifiers(), text));
}

void KeyboardFrame::mousePressEvent(QMouseEvent *event)
{
	if (!m_bMoveable)
		return;

	m_StartPt = event->globalPos();
}

void KeyboardFrame::mouseMoveEvent(QMouseEvent *event)
{
	if (!m_bMoveable)
		return;

	move(pos() + (event->globalPos() - m_StartPt));
	m_StartPt = event->globalPos();

	parentWidget()->update();
}

void KeyboardFrame::setText(QString text)
{
	ui->EDIT->setText(text);
}

QString KeyboardFrame::text()
{
	return ui->EDIT->text();
}

void KeyboardFrame::on_BUTTON_CLOSE_clicked()
{
	emit signalRejected();
}
