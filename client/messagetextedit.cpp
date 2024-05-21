#include "messagetextedit.h"

#include <QMimeData>
#include <QMessageBox>
#include <QFileIconProvider>
#include <qpainter.h>
#include <QKeyEvent>


MessageTextEdit::MessageTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    this->setMaximumHeight(60);

}

MessageTextEdit::~MessageTextEdit()
{

}

QVector<MsgInfo> MessageTextEdit::getMsgList()  //To Think
{
    _getMsgList.clear();

    QString doc = this->document()->toPlainText();
    QString text="";//存储文本信息
    int indexUrl = 0;
    int count = _msgList.size();

    for(int index=0; index<doc.size(); index++)
    {
        if(doc[index]==QChar::ObjectReplacementCharacter)
        {
            if(!text.isEmpty())
            {
                QPixmap pix;
                insertMsgList(_getMsgList,"text",text,pix);
                text.clear();
            }
            while(indexUrl<count)
            {
                MsgInfo msg =  _msgList[indexUrl];
                if(this->document()->toHtml().contains(msg.content,Qt::CaseSensitive))
                {
                    _getMsgList.append(msg);
                    indexUrl++;
                    break;
                }
                indexUrl++;
            }
        }
        else
        {
            text.append(doc[index]);
        }
    }
    if(!text.isEmpty())
    {
        QPixmap pix;
        insertMsgList(_getMsgList,"text",text,pix);
        text.clear();
    }
    _msgList.clear();
    this->clear();
    return _getMsgList;
}

void MessageTextEdit::insertFileFromUrl(const QStringList &urls)
{
    if(urls.isEmpty())
        return;

    foreach (QString url, urls){
        if(isImage(url))
            insertImages(url);
        else
            insertTextFile(url);
    }
}

void MessageTextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->source()==this)
        event->ignore();
    else
        event->accept();
}

void MessageTextEdit::dropEvent(QDropEvent *event)
{
    if(canInsertFromMimeData(event->mimeData())){
        insertFromMimeData(event->mimeData());
        event->accept();
    }
}

void MessageTextEdit::keyPressEvent(QKeyEvent *e)
{
    if((e->key()==Qt::Key_Enter||e->key()==Qt::Key_Return)&& !(e->modifiers() & Qt::ShiftModifier))
    {
        //emit send();
        qDebug("To do emit send()");
        return;
    }
    QTextEdit::keyPressEvent(e);
}

void MessageTextEdit::insertImages(const QString &url)
{
    QImage image(url);

    //按比例缩放图片
    if(image.width()>120||image.height()>80)
    {
        if(image.width()>image.height())
        {
            image =  image.scaledToWidth(120,Qt::SmoothTransformation);
        }
        else
            image = image.scaledToHeight(80,Qt::SmoothTransformation);
    }

    // 将图片转换为文档可以接受的格式并插入
    QTextCursor cursor = this->textCursor();
    // QTextDocument *document = this->document();
    // // 为图片生成一个唯一的名称
    // QString imageName = QString("image_%1").arg(QDateTime::currentMSecsSinceEpoch());
    // // 将图片添加到文档的资源中
    // document->addResource(QTextDocument::ImageResource, QUrl(imageName), image);
    cursor.insertImage("image");
    insertMsgList(_msgList,"image",url,QPixmap::fromImage(image));
}

void MessageTextEdit::insertTextFile(const QString &url)
{
    QFileInfo fileInfo(url);
    if(fileInfo.isDir())
    {
        QMessageBox::information(this,"提示","只允许拖拽单个文件");
        return;
    }

    if(fileInfo.size()>100*1024*1024)
    {
        QMessageBox::information(this,"提示","发送的文件大小不能大于100M");
        return;
    }

    QPixmap pix = getFileIconPixmap(url);
    QTextCursor cursor = this->textCursor();
    cursor.insertImage(pix.toImage(),url);
    insertMsgList(_msgList,"file",url,pix);
}

bool MessageTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
    if (source->hasImage())
        return true;
    else
        return QTextEdit::canInsertFromMimeData(source);
}

void MessageTextEdit::insertFromMimeData(const QMimeData *source)
{
    QStringList urls = getUrl(source->text());

    if(urls.isEmpty())
        return;

    foreach (QString url, urls)
    {
        if(isImage(url))
            insertImages(url);
        else
            insertTextFile(url);
    }
}

bool MessageTextEdit::isImage(QString url)
{
    QString imageFormat = "bmp,jpg,png,tif,gif,pcx,tga,exif,fpx,svg,psd,cdr,pcd,dxf,ufo,eps,ai,raw,wmf,webp";
    QStringList imageFormatList = imageFormat.split(",");

    QFileInfo fileInfo(url);
    QString suffix = fileInfo.suffix();

    if(imageFormatList.contains(suffix,Qt::CaseInsensitive)){ //Qt::CaseInsensitive忽略大小写
        return true;
    }
    return false;
}

void MessageTextEdit::insertMsgList(QVector<MsgInfo> &list, QString flag, QString text, QPixmap pix)
{
    MsgInfo msg;
    msg.msgFlag=flag;
    msg.content = text;
    msg.pixmap = pix;
    list.append(msg);
}

QStringList MessageTextEdit::getUrl(QString text)
{
    QStringList urls;
    if(text.isEmpty()) return urls;

    QStringList list = text.split("\n");
    foreach (QString url, list) {
        if(!url.isEmpty()){
            QStringList str = url.split("///");
            if(str.size()>=2)
                urls.append(str.at(1));
        }
    }
    return urls;
}

QPixmap MessageTextEdit::getFileIconPixmap(const QString &url)
{
    QFileIconProvider provider;
    QFileInfo fileInfo(url);
    QIcon icon = provider.icon(fileInfo);

    QString strFileSize = getFileSize(fileInfo.size());

    QFont font("宋体", 10, QFont::Normal, false);
    QFontMetrics fontMetrics(font);
    QSize textSize = fontMetrics.size(Qt::TextSingleLine, fileInfo.fileName());

    QSize fileSize = fontMetrics.size(Qt::TextSingleLine, strFileSize);
    int maxWidth = qMax(textSize.width(), fileSize.width());
    QPixmap pix(50 + maxWidth + 10, 50);
    pix.fill(Qt::white); // 填充白色背景以提高可见性

    QPainter painter(&pix); //等效于QPainter painter; painter.begin(&pix);
    // painter.setRenderHint(QPainter::Antialiasing); // 如果想要边缘更平滑可以取消注释
    painter.setFont(font); //应用自定义字体

    // 文件图标
    QRect rect(0, 0, 50, 50);
    painter.drawPixmap(rect, icon.pixmap(40, 40));

    painter.setPen(Qt::black); // 设置文本颜色

    // 文件名
    QRect rectText(50 + 10, 3, textSize.width(), textSize.height());
    painter.drawText(rectText, Qt::AlignLeft, fileInfo.fileName()); // 添加对齐方式以提高清晰度

    // 文件大小
    QRect rectFile(50 + 10, textSize.height() + 5, fileSize.width(), fileSize.height());
    painter.drawText(rectFile, Qt::AlignLeft, strFileSize); // 添加对齐方式以提高清晰度

    painter.end();
    return pix;
}

QString MessageTextEdit::getFileSize(qint64 size)
{
    QString Unit;
    double num;
    if(size < 1024){
        num = size;
        Unit = "B";
    }
    else if(size < 1024 * 1224){
        num = size / 1024.0;
        Unit = "KB";
    }
    else if(size <  1024 * 1024 * 1024){
        num = size / 1024.0 / 1024.0;
        Unit = "MB";
    }
    else{
        num = size / 1024.0 / 1024.0/ 1024.0;
        Unit = "GB";
    }
    return QString::number(num,'f',2) + " " + Unit;
}

