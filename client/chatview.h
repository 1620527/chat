#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>


class ChatView : public QWidget
{
    Q_OBJECT
public:
    ChatView(QWidget *parent = nullptr);

    void appendChatItem(QWidget *item);                 //底部插入
    void prependChatItem(QWidget *item);                //顶部插入
    void insertChatItem(QWidget *before, QWidget *item);//中间插入

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onVScrollBarMoved(int min, int max);

private:
    void initStyleSheet();

private:
    QVBoxLayout* _pVl;
    QScrollArea* _pScrollArea;
    bool _isAppended;

signals:
};

#endif // CHATVIEW_H
