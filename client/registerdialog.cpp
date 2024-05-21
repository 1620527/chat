#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"


RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog), _countdown(5)
{
    ui->setupUi(this);
    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    ui->tip_label->setProperty("state", "normal");
    repolish(ui->tip_label);

    connect(&HttpMgr::getSingleton(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    initHttpHandlers();

    ui->tip_label->clear();

    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });

    connect(ui->password_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });

    connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this](){
        checkConfirmValid();
    });

    connect(ui->varify_edit, &QLineEdit::editingFinished, this, [this](){
        checkVarifyValid();
    });

    ui->password_visible->SetState("unvisible", "unvisible_hover", "unvisible_hover", "visible", "visible_hover", "visible_hover");
    ui->confirm_visible->SetState("unvisible", "unvisible_hover", "unvisible_hover", "visible", "visible_hover", "visible_hover");

    connect(ui->password_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->password_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->password_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->password_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->confirm_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    //创建返回倒计时定时器
    _countdown_timer = new QTimer(this);
    connect(_countdown_timer, &QTimer::timeout, [this](){
        if(_countdown==0){
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _countdown--;
        auto str = QString("注册成功，%1 s后返回登录").arg(_countdown);
        ui->tip_lab->setText(str);
    });

    ui->stackedWidget->setCurrentWidget(ui->page);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
    qDebug() << "register dialog destruct.";
}

void RegisterDialog::on_getCod_btn_clicked()
{
    bool valid = checkEmailValid();
    if(valid){
        showTip(tr("验证码已发送"), 1);
        QJsonObject json_obj;
        json_obj["email"] = ui->email_edit->text();
        HttpMgr::getSingleton().PostHttpReq(QUrl(gate_url_prefix + "/get_varifycode"), json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::REGISTER_MOD);

        //发送http验证码

    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
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

void RegisterDialog::showTip(QString s, bool b)
{
    ui->tip_label->setText(s);
    if(b){
        ui->tip_label->setProperty("state", "normal");
    }else{
        ui->tip_label->setProperty("state", "err");
    }
    repolish(ui->tip_label);
}

void RegisterDialog::initHttpHandlers()
{
    //注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"), 0);
            return;
        }

        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已发送至邮箱"), 1);
        qDebug() << "email is: " << email;
    });

    //注册用户回包的逻辑
    _handlers.insert(ReqId::ID_REG_USER, [this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"), 0);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"), 1);
        qDebug() << "email is: " << email;
        ChangeTipPage();
    });
}

bool RegisterDialog::checkUserValid()
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

bool RegisterDialog::checkEmailValid()
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

bool RegisterDialog::checkPassValid()
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

bool RegisterDialog::checkConfirmValid()
{
    if(ui->confirm_edit->text() != ui->password_edit->text()){
        AddErrTips(TipErr::TIP_CONFIRM_ERR, "确认密码和密码不相同");
        return false;
    }
    if(_err_tips.contains(TipErr::TIP_CONFIRM_ERR)){
        DelErrTips(TipErr::TIP_CONFIRM_ERR);
    }
    return true;
}

bool RegisterDialog::checkVarifyValid()
{
    QString varify = ui->varify_edit->text();
    if(varify.isEmpty()){
        AddErrTips(TipErr::TIP_VARIFY_ERR, "验证码不能为空");
        return false;
    }
    if(_err_tips.contains(TipErr::TIP_VARIFY_ERR))
        DelErrTips(TipErr::TIP_VARIFY_ERR);
    return true;
}

void RegisterDialog::AddErrTips(TipErr te, QString tips)
{
    _err_tips[te] = tips;
    showTip(tips, false);
}

void RegisterDialog::DelErrTips(TipErr te)
{
    _err_tips.remove(te);
    if(_err_tips.empty()){
        ui->tip_label->clear();
        return;
    }
    showTip(_err_tips.first(), false);
}

void RegisterDialog::ChangeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    // 启动定时器，设置间隔为1000毫秒（1秒）
    _countdown_timer->start(1000);
}


void RegisterDialog::on_sure_btn_clicked()
{
    if(!checkUserValid())
        return;
    if(!checkEmailValid())
        return;
    if(!checkPassValid())
        return;
    if(!checkConfirmValid())
        return;
    if(!checkVarifyValid())
        return;

    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->password_edit->text());
    json_obj["varifycode"] = ui->varify_edit->text();

    HttpMgr::getSingleton().PostHttpReq(QUrl(gate_url_prefix + "/user_register"), json_obj, ReqId::ID_REG_USER, Modules::REGISTER_MOD);
    print("确认注册");
}


void RegisterDialog::on_return_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}


void RegisterDialog::on_cancel_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}

