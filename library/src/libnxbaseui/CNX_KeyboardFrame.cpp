#include "CNX_KeyboardFrame.h"
#include "ui_CNX_KeyboardFrame.h"
#include <QDateTime>

CNX_KeyboardFrame::CNX_KeyboardFrame(QWidget *parent) :
    QFrame(parent),
	ui(new Ui::CNX_KeyboardFrame)
{
    ui->setupUi(this);

	Initialize();
}

CNX_KeyboardFrame::~CNX_KeyboardFrame()
{
    delete ui;
}

void CNX_KeyboardFrame::Initialize()
{
	m_pDestinationPtr = NULL;

    on_BUTTON_KEY_SHIFT_toggled(false);
    ui->BUTTON_KEY_DOT->show();
    ui->BUTTON_KEY_BAR->show();
    ui->BUTTON_KEY_COMMA->hide();
    ui->BUTTON_KEY_UNDER_BAR->hide();
}

void CNX_KeyboardFrame::SetObject(QObject* object)
{
	m_pDestinationPtr = object;
}

void CNX_KeyboardFrame::SetEnterKeyText(QString text)
{
    ui->BUTTON_ENTER->setText(text);
}

void CNX_KeyboardFrame::SetSpaceKeyPosition(int x, int y)
{
    ui->BUTTON_KEY_SPACE->move(x, y);
}

void CNX_KeyboardFrame::SetSpacekeySize(int width, int height)
{
    ui->BUTTON_KEY_SPACE->setFixedSize(width, height);
}

void CNX_KeyboardFrame::SetEnabled(int key, bool enabled)
{
    switch (key) {
    case Qt::Key_Comma:
        ui->BUTTON_KEY_COMMA->setEnabled(enabled);
        break;

    case Qt::Key_A:
        ui->BUTTON_KEY_A->setEnabled(enabled);
        break;

    case Qt::Key_B:
        ui->BUTTON_KEY_B->setEnabled(enabled);
        break;

    case Qt::Key_C:
        ui->BUTTON_KEY_C->setEnabled(enabled);
        break;

    case Qt::Key_D:
        ui->BUTTON_KEY_D->setEnabled(enabled);
        break;

    case Qt::Key_E:
        ui->BUTTON_KEY_E->setEnabled(enabled);
        break;

    case Qt::Key_F:
        ui->BUTTON_KEY_F->setEnabled(enabled);
        break;

    case Qt::Key_G:
        ui->BUTTON_KEY_G->setEnabled(enabled);
        break;

    case Qt::Key_H:
        ui->BUTTON_KEY_H->setEnabled(enabled);
        break;

    case Qt::Key_I:
        ui->BUTTON_KEY_I->setEnabled(enabled);
        break;

    case Qt::Key_J:
        ui->BUTTON_KEY_J->setEnabled(enabled);
        break;

    case Qt::Key_K:
        ui->BUTTON_KEY_K->setEnabled(enabled);
        break;

    case Qt::Key_L:
        ui->BUTTON_KEY_L->setEnabled(enabled);
        break;

    case Qt::Key_M:
        ui->BUTTON_KEY_M->setEnabled(enabled);
        break;

    case Qt::Key_N:
        ui->BUTTON_KEY_N->setEnabled(enabled);
        break;

    case Qt::Key_O:
        ui->BUTTON_KEY_O->setEnabled(enabled);
        break;

    case Qt::Key_P:
        ui->BUTTON_KEY_P->setEnabled(enabled);
        break;

    case Qt::Key_Q:
        ui->BUTTON_KEY_Q->setEnabled(enabled);
        break;

    case Qt::Key_R:
        ui->BUTTON_KEY_R->setEnabled(enabled);
        break;

    case Qt::Key_S:
        ui->BUTTON_KEY_S->setEnabled(enabled);
        break;

    case Qt::Key_T:
        ui->BUTTON_KEY_T->setEnabled(enabled);
        break;

    case Qt::Key_U:
        ui->BUTTON_KEY_U->setEnabled(enabled);
        break;

    case Qt::Key_V:
        ui->BUTTON_KEY_V->setEnabled(enabled);
        break;

    case Qt::Key_W:
        ui->BUTTON_KEY_W->setEnabled(enabled);
        break;

    case Qt::Key_X:
        ui->BUTTON_KEY_X->setEnabled(enabled);
        break;

    case Qt::Key_Y:
        ui->BUTTON_KEY_Y->setEnabled(enabled);
        break;

    case Qt::Key_Z:
        ui->BUTTON_KEY_Z->setEnabled(enabled);
        break;

    case Qt::Key_0:
        ui->BUTTON_KEY_0->setEnabled(enabled);
        break;

    case Qt::Key_1:
        ui->BUTTON_KEY_1->setEnabled(enabled);
        break;

    case Qt::Key_2:
        ui->BUTTON_KEY_2->setEnabled(enabled);
        break;

    case Qt::Key_3:
        ui->BUTTON_KEY_3->setEnabled(enabled);
        break;

    case Qt::Key_4:
        ui->BUTTON_KEY_4->setEnabled(enabled);
        break;

    case Qt::Key_5:
        ui->BUTTON_KEY_5->setEnabled(enabled);
        break;

    case Qt::Key_6:
        ui->BUTTON_KEY_6->setEnabled(enabled);
        break;

    case Qt::Key_7:
        ui->BUTTON_KEY_7->setEnabled(enabled);
        break;

    case Qt::Key_8:
        ui->BUTTON_KEY_8->setEnabled(enabled);
        break;

    case Qt::Key_9:
        ui->BUTTON_KEY_9->setEnabled(enabled);
        break;

    default:
        break;
    }
}

void CNX_KeyboardFrame::InputKey(int key)
{
	if (!m_pDestinationPtr)
		return;

    QKeySequence k(key);
    QString text = k.toString();
    Qt::KeyboardModifier modifier = ui->BUTTON_KEY_SHIFT->isChecked() ? Qt::ShiftModifier : Qt::NoModifier;

    if (modifier == Qt::ShiftModifier)
        text = text.toUpper();
    else
        text = text.toLower();

	QApplication::sendEvent(m_pDestinationPtr, new QKeyEvent(QEvent::KeyPress, key, modifier, text));
	QApplication::sendEvent(m_pDestinationPtr, new QKeyEvent(QEvent::KeyRelease, key, modifier, text));
}

void CNX_KeyboardFrame::on_BUTTON_KEY_1_clicked()
{
	InputKey(Qt::Key_1);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_2_clicked()
{
	InputKey(Qt::Key_2);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_3_clicked()
{
	InputKey(Qt::Key_3);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_4_clicked()
{
	InputKey(Qt::Key_4);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_5_clicked()
{
	InputKey(Qt::Key_5);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_6_clicked()
{
	InputKey(Qt::Key_6);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_7_clicked()
{
	InputKey(Qt::Key_7);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_8_clicked()
{
	InputKey(Qt::Key_8);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_9_clicked()
{
	InputKey(Qt::Key_9);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_0_clicked()
{
	InputKey(Qt::Key_0);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_Q_clicked()
{
	InputKey(Qt::Key_Q);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_W_clicked()
{
	InputKey(Qt::Key_W);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_E_clicked()
{
	InputKey(Qt::Key_E);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_R_clicked()
{
	InputKey(Qt::Key_R);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_T_clicked()
{
	InputKey(Qt::Key_T);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_Y_clicked()
{
	InputKey(Qt::Key_Y);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_U_clicked()
{
	InputKey(Qt::Key_U);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_I_clicked()
{
	InputKey(Qt::Key_I);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_O_clicked()
{
	InputKey(Qt::Key_O);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_P_clicked()
{
	InputKey(Qt::Key_P);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_A_clicked()
{
	InputKey(Qt::Key_A);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_S_clicked()
{
	InputKey(Qt::Key_S);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_D_clicked()
{
	InputKey(Qt::Key_D);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_F_clicked()
{
	InputKey(Qt::Key_F);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_G_clicked()
{
	InputKey(Qt::Key_G);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_H_clicked()
{
	InputKey(Qt::Key_H);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_J_clicked()
{
	InputKey(Qt::Key_J);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_K_clicked()
{
	InputKey(Qt::Key_K);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_L_clicked()
{
	InputKey(Qt::Key_L);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_Z_clicked()
{
	InputKey(Qt::Key_Z);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_X_clicked()
{
	InputKey(Qt::Key_X);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_C_clicked()
{
	InputKey(Qt::Key_C);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_V_clicked()
{
	InputKey(Qt::Key_V);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_B_clicked()
{
	InputKey(Qt::Key_B);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_N_clicked()
{
	InputKey(Qt::Key_N);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_M_clicked()
{
	InputKey(Qt::Key_M);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_COMMA_clicked()
{
	InputKey(Qt::Key_Comma);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_DOT_clicked()
{
	InputKey(Qt::Key_Period);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_BAR_clicked()
{
	InputKey(Qt::Key_Minus);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_UNDER_BAR_clicked()
{
	InputKey(Qt::Key_Underscore);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_SPACE_clicked()
{
	InputKey(' ');
}

void CNX_KeyboardFrame::on_BUTTON_KEY_BACK_SPACE_clicked()
{
	InputKey(Qt::Key_Backspace);
}

void CNX_KeyboardFrame::on_BUTTON_ENTER_clicked()
{
	InputKey(Qt::Key_Enter);
}

void CNX_KeyboardFrame::on_BUTTON_KEY_SHIFT_toggled(bool checked)
{
    if (checked) {
        if (ui->BUTTON_KEY_COMMA->isEnabled()) {
            ui->BUTTON_KEY_COMMA->show();
            ui->BUTTON_KEY_DOT->hide();
        }

        ui->BUTTON_KEY_UNDER_BAR->show();

        ui->BUTTON_KEY_BAR->hide();

        ui->BUTTON_KEY_Q->setText(QChar('Q').toUpper());
        ui->BUTTON_KEY_W->setText(QChar('W').toUpper());
        ui->BUTTON_KEY_E->setText(QChar('E').toUpper());
        ui->BUTTON_KEY_R->setText(QChar('R').toUpper());
        ui->BUTTON_KEY_T->setText(QChar('T').toUpper());
        ui->BUTTON_KEY_Y->setText(QChar('Y').toUpper());
        ui->BUTTON_KEY_U->setText(QChar('U').toUpper());
        ui->BUTTON_KEY_I->setText(QChar('I').toUpper());
        ui->BUTTON_KEY_O->setText(QChar('O').toUpper());
        ui->BUTTON_KEY_P->setText(QChar('P').toUpper());
        ui->BUTTON_KEY_A->setText(QChar('A').toUpper());
        ui->BUTTON_KEY_S->setText(QChar('S').toUpper());
        ui->BUTTON_KEY_D->setText(QChar('D').toUpper());
        ui->BUTTON_KEY_F->setText(QChar('F').toUpper());
        ui->BUTTON_KEY_G->setText(QChar('G').toUpper());
        ui->BUTTON_KEY_H->setText(QChar('H').toUpper());
        ui->BUTTON_KEY_J->setText(QChar('J').toUpper());
        ui->BUTTON_KEY_K->setText(QChar('K').toUpper());
        ui->BUTTON_KEY_L->setText(QChar('L').toUpper());
        ui->BUTTON_KEY_Z->setText(QChar('Z').toUpper());
        ui->BUTTON_KEY_X->setText(QChar('X').toUpper());
        ui->BUTTON_KEY_C->setText(QChar('C').toUpper());
        ui->BUTTON_KEY_V->setText(QChar('V').toUpper());
        ui->BUTTON_KEY_B->setText(QChar('B').toUpper());
        ui->BUTTON_KEY_N->setText(QChar('N').toUpper());
        ui->BUTTON_KEY_M->setText(QChar('M').toUpper());
    } else {
        ui->BUTTON_KEY_DOT->show();
        ui->BUTTON_KEY_BAR->show();
        ui->BUTTON_KEY_COMMA->hide();
        ui->BUTTON_KEY_UNDER_BAR->hide();

        ui->BUTTON_KEY_Q->setText(QChar('Q').toLower());
        ui->BUTTON_KEY_W->setText(QChar('W').toLower());
        ui->BUTTON_KEY_E->setText(QChar('E').toLower());
        ui->BUTTON_KEY_R->setText(QChar('R').toLower());
        ui->BUTTON_KEY_T->setText(QChar('T').toLower());
        ui->BUTTON_KEY_Y->setText(QChar('Y').toLower());
        ui->BUTTON_KEY_U->setText(QChar('U').toLower());
        ui->BUTTON_KEY_I->setText(QChar('I').toLower());
        ui->BUTTON_KEY_O->setText(QChar('O').toLower());
        ui->BUTTON_KEY_P->setText(QChar('P').toLower());
        ui->BUTTON_KEY_A->setText(QChar('A').toLower());
        ui->BUTTON_KEY_S->setText(QChar('S').toLower());
        ui->BUTTON_KEY_D->setText(QChar('D').toLower());
        ui->BUTTON_KEY_F->setText(QChar('F').toLower());
        ui->BUTTON_KEY_G->setText(QChar('G').toLower());
        ui->BUTTON_KEY_H->setText(QChar('H').toLower());
        ui->BUTTON_KEY_J->setText(QChar('J').toLower());
        ui->BUTTON_KEY_K->setText(QChar('K').toLower());
        ui->BUTTON_KEY_L->setText(QChar('L').toLower());
        ui->BUTTON_KEY_Z->setText(QChar('Z').toLower());
        ui->BUTTON_KEY_X->setText(QChar('X').toLower());
        ui->BUTTON_KEY_C->setText(QChar('C').toLower());
        ui->BUTTON_KEY_V->setText(QChar('V').toLower());
        ui->BUTTON_KEY_B->setText(QChar('B').toLower());
        ui->BUTTON_KEY_N->setText(QChar('N').toLower());
        ui->BUTTON_KEY_M->setText(QChar('M').toLower());
    }
}
