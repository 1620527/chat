#include "mainwindow.h"
#include "tcpmgr.h"
#include "ui_mainwindow.h"
#include "global.h"
//#include "httpmgr.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog();
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchRegister);
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);

    connect(&TcpMgr::getSingleton(), &TcpMgr::sigSwitchChat, this, &MainWindow::SlotSwitchChat);

    this->setCentralWidget(_login_dlg);


}

MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::SlotSwitchRegister()
{
    _register_dlg = new RegisterDialog();
    connect(_register_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);
    this->setCentralWidget(_register_dlg);
}

void MainWindow::SlotSwitchLogin()
{
    _login_dlg = new LoginDialog();
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchRegister);
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    this->setCentralWidget(_login_dlg);
}

void MainWindow::SlotSwitchReset()
{
    _reset_dlg = new ResetDialog();
     connect(_reset_dlg, &ResetDialog::switchLogin, this, &MainWindow::SlotSwitchLogin);
    this->setCentralWidget(_reset_dlg);
}

void MainWindow::SlotSwitchChat()
{
    _chat_dlg = new ChatDialog();
    this->setCentralWidget(_chat_dlg);

    this->setMinimumSize(1200, 900);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}
