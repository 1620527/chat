#include "logindialog.h"
#include "ui_logindialog.h"
#include "httpmgr.h"
#include "tcpmgr.h"

#include <QPainter>
#include <QPainterPath>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    initHead();

    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);

    ui->forget_label->SetState("normal","hover","","selected","selected_hover","");
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);

    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->password_visible->SetState("unvisible", "unvisible_hover", "unvisible_hover", "visible", "visible_hover", "visible_hover");
    connect(ui->password_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->password_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->password_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->password_edit->setEchoMode(QLineEdit::Normal);
        }
        //qDebug() << "Label was clicked!";
    });

    ui->tip_label->clear();

    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });
    connect(ui->password_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });

    //连接登录回包信号
    connect(&HttpMgr::getSingleton(), &HttpMgr::sig_login_mod_finish, this, &LoginDialog::slot_login_mod_finish);
    //注册处理回调
    initHttpHandlers();

    //连接tcp连接请求的信号和槽函数
    connect(this, &LoginDialog::sig_connect_tcp, &TcpMgr::getSingleton(), &TcpMgr::slot_tcp_connect);
    //连接tcp管理者发出的连接成功信号
    connect(&TcpMgr::getSingleton(), &TcpMgr::sig_con_success, this, &LoginDialog::slot_tcp_con_finish);

    //连接tcp管理者发出的登陆失败信号
    connect(&TcpMgr::getSingleton(), &TcpMgr::sig_login_failed, this, &LoginDialog::slot_login_failed);
}

LoginDialog::~LoginDialog()
{
    delete ui;
    qDebug()<<"Login dialog destuct";
}

void LoginDialog::showTip(QString s, bool b)
{
    ui->tip_label->setText(s);
    if(b){
        ui->tip_label->setProperty("state", "normal");
    }else{
        ui->tip_label->setProperty("state", "err");
    }
    repolish(ui->tip_label);
}

bool LoginDialog::checkUserValid()
{
    if(ui->user_edit->text() == ""){
        AddErrTips(TipErr::TIP_USER_ERR, "用户名不能为空");
        return false;
    }
    if(_err_tips.contains(TipErr::TIP_USER_ERR)){
        DelErrTips(TipErr::TIP_USER_ERR);
    }
    return true;
}

bool LoginDialog::checkPassValid()
{
    auto pass = ui->password_edit->text();

    if(pass.length() < 6 || pass.length()>15){
        //提示长度不准确
        AddErrTips(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddErrTips(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;;
    }

    if(_err_tips.contains(TipErr::TIP_PWD_ERR)){
        DelErrTips(TipErr::TIP_PWD_ERR);
    }

    return true;
}

void LoginDialog::AddErrTips(TipErr te, QString tips)
{
    _err_tips[te] = tips;
    showTip(tips, false);
}

void LoginDialog::DelErrTips(TipErr te)
{
    _err_tips.remove(te);
    if(_err_tips.empty()){
        ui->tip_label->clear();
        return;
    }
    showTip(_err_tips.first(), false);
}

void LoginDialog::initHttpHandlers()
{
    //注册获取登录回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto user = jsonObj["user"].toString();
        showTip(tr("验证成功,正在链接服务器..."), true);
        qDebug()<< "user is " << user ;

        //发送信号通知tcpMgr发送长链接
        ServerInfo si;
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();

        _uid = si.Uid;
        _token = si.Token;
        auto email = jsonObj["email"].toString();
        qDebug()<< "email is " << email << " uid is " << si.Uid <<" host is " << si.Host << " Port is " << si.Port << " Token is " << si.Token;
        emit sig_connect_tcp(si);
    });
}

void LoginDialog::initHead()
{
    // 加载图片
    QPixmap originalPixmap(":/res/login_img.png");
        // 设置图片自动缩放
    qDebug()<< originalPixmap.size() << ui->img_label->size();
    originalPixmap = originalPixmap.scaled(ui->img_label->size(),Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个和原始图片相同大小的QPixmap，用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent); // 用透明色填充

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing); // 设置抗锯齿，使圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 使用QPainterPath设置圆角
    QPainterPath path;
    path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10, 10); // 最后两个参数分别是x和y方向的圆角半径
    painter.setClipPath(path);

    // 将原始图片绘制到roundedPixmap上
    painter.drawPixmap(0, 0, originalPixmap);

    // 设置绘制好的圆角图片到QLabel上
    ui->img_label->setPixmap(roundedPixmap);
}

bool LoginDialog::enableBtn(bool enabled)
{
    ui->login_btn->setEnabled(enabled);
    ui->reg_btn->setEnabled(enabled);
    return true;
}

void LoginDialog::slot_forget_pwd()
{
    print("slot_forget_pwd");
    emit switchReset();
}

void LoginDialog::on_login_btn_clicked()
{
    qDebug()<<"login btn clicked";
    if(!checkUserValid())
        return;

    if(!checkPassValid())
        return ;

    enableBtn(false);

    auto user = ui->user_edit->text();
    auto pwd = ui->password_edit->text();
    //发送http请求登录
    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::getSingleton().PostHttpReq(QUrl(gate_url_prefix+"/user_login"), json_obj, ReqId::ID_LOGIN_USER,Modules::LOGIN_MOD);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"),false);
        return;
    }


    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());

    return;
}

void LoginDialog:: slot_tcp_con_finish(bool bsuccess)
{
    if(bsuccess){
        showTip(tr("聊天服务连接成功，正在登录..."),true);
        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;

        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);

        //发送tcp请求给chat server
        emit TcpMgr::getSingleton().sig_send_data(ReqId::ID_CHAT_LOGIN, jsonString);

    }else{
        showTip(tr("网络异常"),false);
        enableBtn(true);
    }
}

void LoginDialog::slot_login_failed(int err)
{
    QString result = QString("登录失败, err is %1").arg(err);
    showTip(result,false);
    enableBtn(true);
}

