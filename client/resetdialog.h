#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include <global.h>

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();

signals:
    void switchLogin();

private slots:
    void on_back_btn_clicked();

    void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);

    void on_sure_btn_clicked();

    void on_verify_btn_clicked();

    void on_return_btn_clicked();

private:
    void showTip(QString s, bool b = 1);

    void initHttpHandlers();

    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkVarifyValid();

    void AddErrTips(TipErr te, QString tips);
    void DelErrTips(TipErr te);

    void ChangeTipPage();

private:
    Ui::ResetDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QMap<TipErr, QString> _err_tips;
    QTimer* _countdown_timer;
    int _countdown;
};

#endif // RESETDIALOG_H
