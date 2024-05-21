#include "chatuserwid.h"
#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

void ChatUserWid::SetInfo(QString name, QString head, QString msg)
{
    _name = name;
    _head = head;
    _msg = msg;
    // 加载图片
    QPixmap pixmap(_head);

    // 设置图片自动缩放
    ui->icon_lab->setPixmap(pixmap.scaled(ui->icon_lab->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lab->setScaledContents(true);

    ui->user_name_lab->setText(_name);
    ui->user_chat_lab->setText(_msg);
}
