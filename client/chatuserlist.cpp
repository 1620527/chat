#include "chatuserlist.h"


ChatUserList::ChatUserList(QWidget *parent)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this->viewport()) {
        // 检查事件是否是鼠标悬浮进入或离开
        if (event->type() == QEvent::Enter) {
            // 鼠标悬浮，显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开，隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }

        // 检查事件是否是鼠标滚轮事件
        if(event->type() == QEvent::Wheel){
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            int numDegrees = wheelEvent->angleDelta().y() / 8;
            int numSteps = numDegrees / 15; // 计算滚动步数

            // 设置滚动幅度
            this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

            // 检查是否滚动到底部
            QScrollBar *scrollBar = this->verticalScrollBar();
            int maxScrollValue = scrollBar->maximum();
            int currentValue = scrollBar->value();
            if (maxScrollValue - currentValue <= 0) {
                // 滚动到底部，加载新的联系人
                qDebug()<<"load more chat user";
                //发送信号通知聊天界面加载更多聊天内容
                emit sig_loading_chat_user();
            }

            return true;
        }
    }

    return QListWidget::eventFilter(watched, event);
}
