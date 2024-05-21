#include "resetdialog.h"
#include "ui_resetdialog.h"
#include "httpmgr.h"

#include <QTimer>


ResetDialog::ResetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResetDialog), _countdown(5)
{
    ui->setupUi(this);

    ui->tip_label->clear();

    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });

    connect(ui->pwd_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });


    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this](){
         checkVarifyValid();
    });

    //连接reset相关信号和注册处理回调
    initHttpHandlers();

    connect(&HttpMgr::getSingleton(), &HttpMgr::sig_reset_mod_finish, this, &ResetDialog::slot_reset_mod_finish);

    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });

    connect(ui->pwd_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });

    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this](){
        checkVarifyValid();
    });

    _countdown_timer = new QTimer(this);
    connect(_countdown_timer, &QTimer::timeout, [this](){
        if(_countdown==0){
            _countdown_timer->stop();
            emit switchLogin();
            return;
        }
        _countdown--;
        auto str = QString("重置密码成功，%1 s后返回登录").arg(_countdown);
        ui->tip_lab->setText(str);
    });

    ui->stackedWidget->setCurrentWidget(ui->page);
}

ResetDialog::~ResetDialog()
{
    print("Reset dialog destruct");
    delete ui;
}

void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"), false);
        return;
    }

    //解析json字符串，res.toUtf8()转换为 QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        showTip(tr("json 解析失败"), false);
        return;
    }
    if(!jsonDoc.isObject()){
        showTip(tr("json 解析失败"), false);
        return;
    }

    _handlers[id](jsonDoc.object());
    return;
}

void ResetDialog::showTip(QString s, bool b)
{
    ui->tip_label->setText(s);
    if(b){
        ui->tip_label->setProperty("state", "normal");
    }else{
        ui->tip_label->setProperty("state", "err");
    }
    repolish(ui->tip_label);
}

void ResetDialog::initHttpHandlers()
{
    //注册获取验证码回包逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已发送到邮箱，注意查收"), true);
        qDebug()<< "email is " << email ;
    });

    //注册注册用户回包逻辑
    _handlers.insert(ReqId::ID_RESET_PWD, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("密码重置成功"), true);
        qDebug()<< "email is " << email ;
        qDebug()<< "user uuid is " <<  jsonObj["uuid"].toString();
        ChangeTipPage();
    });
}

bool ResetDialog::checkUserValid()
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

bool ResetDialog::checkEmailValid()
{
    auto email = ui->email_edit->text();
    static QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch();
    if(!match){
        AddErrTips(TipErr::TIP_EMAIL_ERR, "邮箱地址不正确");
        return false;
    }
    if(_err_tips.contains(TipErr::TIP_EMAIL_ERR)){
        DelErrTips(TipErr::TIP_EMAIL_ERR);
    }

    return true;
}

bool ResetDialog::checkPassValid()
{
    auto pass = ui->pwd_edit->text();

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

bool ResetDialog::checkVarifyValid()
{
    QString varify = ui->verify_edit->text();
    if(varify.isEmpty()){
        AddErrTips(TipErr::TIP_VARIFY_ERR, "验证码不能为空");
        return false;
    }
    if(_err_tips.contains(TipErr::TIP_VARIFY_ERR))
        DelErrTips(TipErr::TIP_VARIFY_ERR);
    return true;
}

void ResetDialog::AddErrTips(TipErr te, QString tips)
{
    _err_tips[te] = tips;
    showTip(tips, false);
}

void ResetDialog::DelErrTips(TipErr te)
{
    _err_tips.remove(te);
    if(_err_tips.empty()){
        ui->tip_label->clear();
        return;
    }
    showTip(_err_tips.first(), false);
}

void ResetDialog::ChangeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    // 启动定时器，设置间隔为1000毫秒（1秒）
    _countdown_timer->start(1000);
}

void ResetDialog::on_verify_btn_clicked()
{
    qDebug()<<"reset varify btn clicked ";
    auto email = ui->email_edit->text();
    auto bcheck = checkEmailValid();
    if(!bcheck){
        return;
    }

    //发送http请求获取验证码
    QJsonObject json_obj;
    json_obj["email"] = email;
    HttpMgr::getSingleton().PostHttpReq(QUrl(gate_url_prefix+"/get_varifycode"),
                                        json_obj, ReqId::ID_GET_VARIFY_CODE,Modules::RESET_MOD);
}

void ResetDialog::on_sure_btn_clicked()
{
    if(!checkUserValid())
        return;
    if(!checkEmailValid())
        return;
    if(!checkPassValid())
        return;
    if(!checkVarifyValid())
        return;

    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pwd_edit->text());
    json_obj["varifycode"] = ui->verify_edit->text();

    HttpMgr::getSingleton().PostHttpReq(QUrl(gate_url_prefix + "/reset_pwd"), json_obj, ReqId::ID_RESET_PWD, Modules::RESET_MOD);
    print("确认注册");
}

void ResetDialog::on_back_btn_clicked()
{
    _countdown_timer->stop();
    emit switchLogin();
}

void ResetDialog::on_return_btn_clicked()
{
    _countdown_timer->stop();
    emit switchLogin();
}

