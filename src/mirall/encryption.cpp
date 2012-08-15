/*
 * Copyright (C) 2012 Bjoern Schiessle <schiessle@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "encryption.h"
#include <QMap>
#include <iostream>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDomDocument>

Encryption::Encryption(QString baseurl, QString username, QString password, QObject *parent) :
    QObject(parent)
{
    setBaseUrl(baseurl);
    setAuthCredentials(username, password);
    _nam = new QNetworkAccessManager(this);

    _returnValues << "statuscode";

    connect( _nam, SIGNAL(finished(QNetworkReply*)),this,SLOT(slotHttpRequestResults(QNetworkReply*)));
    connect( _nam, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(slotHttpAuth(QNetworkReply*,QAuthenticator*)));
}

void Encryption::setBaseUrl(QString baseurl)
{
    _baseurl = baseurl;
}

void Encryption::setAuthCredentials(QString username, QString password) {
    _username = username;
    _password = password;
}

QMap<QString, QString> Encryption::parseXML(QString xml, QList<QString> tags)
{
    QMap<QString, QString> result;

    QDomDocument doc("results");
    doc.setContent(xml);

    for (int i = 0; i < tags.size(); ++i) {
        QString tag = tags.at(i);
        QDomNodeList elements = doc.elementsByTagName(tag);
        if (elements.size() ==  1 ) {
            result[tag] = elements.at(0).toElement().text();
        }
    }
    return result;
}

void Encryption::getUserKeys()
{
    _nam->get(QNetworkRequest(QUrl(_baseurl + "/ocs/v1.php/cloud/userkeys")));
}

void Encryption::generateUserKeys()
{
    QString privatekey("foo-PRIVATE"); // dummy key for the moment
    QString publickey("foo-PUBLIC"); // dummy key for the moment

    QByteArray postData;
    postData.append("privatekey="+privatekey+"&");
    postData.append("publickey="+publickey);

    QNetworkRequest req;
    req.setUrl(QUrl(_baseurl + "/ocs/v1.php/cloud/userkeys"));

    _nam->post(req, postData);
}

void Encryption::setExpectedReturnValues(QList<QString> returnValues)
{
    _returnValues.append(returnValues);
}

void Encryption::slotHttpRequestResults(QNetworkReply *reply)
{
    QMap<QString, QString> result;
    if(reply->error() == QNetworkReply::NoError) {
        QString xml =  reply->readAll();
        result = parseXML(xml, _returnValues);
    } else {
        qDebug() << reply->errorString();
    }
    emit ocsResults(result);
}

void Encryption::slotHttpAuth(QNetworkReply *reply, QAuthenticator *auth)
{
    auth->setUser(_username);
    auth->setPassword(_password);
}
