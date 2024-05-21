#ifndef GLOBAL_H
#define GLOBAL_H

#include <QWidget>
#include <functional>
#include <QStyle>
#include <QRegularExpression>
#include <memory>
#include <mutex>
#include "myprint.h"
#include "singleton.h"
#include <QByteArray>
#include <QnetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QSettings>
#include "usermgr.h"



/*用来刷新qss*/
extern std::function<void(QWidget*)> repolish;

extern std::function<QString(QString)> xorString;

enum ReqId{
    ID_GET_VARIFY_CODE = 1001, //获取验证码
    ID_REG_USER = 1002, //用户注册
    ID_RESET_PWD = 1003, //用户重置密码
    ID_LOGIN_USER = 1004, //用户登陆
    ID_CHAT_LOGIN = 1005, //登陆聊天服务器
    ID_CHAT_LOGIN_RSP= 1006, //登陆聊天服务器回包
};

enum Modules{
    REGISTER_MOD = 0,
    RESET_MOD = 1,
    LOGIN_MOD,
};

enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1,
    ERR_NETWORK = 2,
};

extern QString gate_url_prefix;

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};

enum ClickLbState{
    Normal = 0,
    Selected = 1
};

struct ServerInfo{
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

struct MsgInfo{
    QString msgFlag;//"text,image,file"
    QString content;//表示文件和图像的url,文本信息
    QPixmap pixmap;//文件和图片的缩略图
};

enum class ChatRole
{

    Self,
    Other
};

//自定义QListWidgetItem的几种类型
enum ListItemType{
    ChatUserItem, //聊天用户
    ContactUserItem, //联系人用户
    SearchUserItem, //搜索到的用户
    AddUserTipItem, //提示添加用户
    InvalidItem,  //不可点击条目
};


#endif // GLOBAL_H
