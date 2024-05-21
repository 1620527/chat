#include "customizeedit.h"



CustomizeEdit::CustomizeEdit(QWidget *parent)
    :QLineEdit(parent)
{

}

void CustomizeEdit::focusOutEvent(QFocusEvent *event)
{
    // 执行失去焦点时的处理逻辑
    qDebug() << "CustomizeEdit focusout";

    //发送失去焦点得信号
    emit this->sig_focus_out(); // 发射信号

    // 调用基类的focusOutEvent()方法，保证基类的行为得到执行
    QLineEdit::focusOutEvent(event);

}
