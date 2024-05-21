#ifndef HTTPMGR_H
#define HTTPMGR_H


#include "singleton.h"
#include "QNetworkAccessManager"
#include <QUrl>
#include <QString>
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include "global.h"


class HttpMgr :public QObject, public Singleton<HttpMgr>
{
   Q_OBJECT
public:
   ~HttpMgr();
    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);

private:
    friend class Singleton<HttpMgr>;
    HttpMgr();

private slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
private:
    QNetworkAccessManager _manager;

signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_login_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
