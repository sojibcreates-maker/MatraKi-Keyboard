/*
 *  MatraKi Keyboard
 *  Update checker — Emerald Design System
 *  Copyright (C) 2026 Md Sojib Hossain
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include "MatraKiUpdater.h"
#include "UpdateDialog.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "ThemeManager.h"

static const QString UPDATE_PAGE_URL =
    QStringLiteral("https://sojibcreates-maker.github.io/MatraKi-Keyboard/#downloads");
static const QString DEFS_URL =
    QStringLiteral("https://sojibcreates-maker.github.io/MatraKi-Keyboard/updates.json");
static const QString GITHUB_API_LATEST_URL =
    QStringLiteral("https://api.github.com/repos/sojibcreates-maker/MatraKi-Keyboard/releases/latest");
static const QString GITHUB_RELEASES_URL =
    QStringLiteral("https://github.com/sojibcreates-maker/MatraKi-Keyboard/releases/latest");

bool MatraKiUpdater::isRemoteVersionNewer(const QString &remoteVer, const QString &localVer) {
  QString r = remoteVer.trimmed();
  QString l = localVer.trimmed();
  if (r.startsWith('v') || r.startsWith('V')) r = r.mid(1);
  if (l.startsWith('v') || l.startsWith('V')) l = l.mid(1);
  QStringList rParts = r.split('.');
  QStringList lParts = l.split('.');
  int count = qMax(rParts.size(), lParts.size());
  for (int i = 0; i < count; ++i) {
    int rv = (i < rParts.size()) ? rParts.at(i).toInt() : 0;
    int lv = (i < lParts.size()) ? lParts.at(i).toInt() : 0;
    if (rv > lv) return true;
    if (rv < lv) return false;
  }
  return false;
}

void MatraKiUpdater::check(QWidget *parent, bool silent) {
  QNetworkAccessManager *nam = new QNetworkAccessManager(parent);

  /* Step 1 — GitHub Releases API picks up any freshly published release. */
  QNetworkRequest apiReq{QUrl(GITHUB_API_LATEST_URL)};
  apiReq.setRawHeader("User-Agent", "MatraKi-Keyboard-Updater");
  apiReq.setRawHeader("Accept", "application/vnd.github+json");
  QNetworkReply *apiReply = nam->get(apiReq);

  QObject::connect(apiReply, &QNetworkReply::finished, parent, [=]() {
    apiReply->deleteLater();
    if (apiReply->error() == QNetworkReply::NoError) {
      QJsonDocument doc = QJsonDocument::fromJson(apiReply->readAll());
      if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QString tag = obj.value("tag_name").toString().trimmed();
        if (tag.startsWith('v') || tag.startsWith('V')) tag = tag.mid(1);
        QString title = obj.value("name").toString("MatraKi Release").trimmed();
        QString body = obj.value("body").toString().trimmed();
        QString htmlUrl = obj.value("html_url").toString(GITHUB_RELEASES_URL).trimmed();
        if (!tag.isEmpty()) {
          showDialog(parent, isRemoteVersionNewer(tag, QStringLiteral(PROJECT_VERSION)), tag,
                     title, body, htmlUrl, silent);
          nam->deleteLater();
          return;
        }
      }
    }

    /* Step 2 — fallback to the official updates.json feed. */
    QNetworkRequest pageReq{QUrl(DEFS_URL)};
    pageReq.setRawHeader("User-Agent", "MatraKi-Keyboard-Updater");
    QNetworkReply *pageReply = nam->get(pageReq);
    QObject::connect(pageReply, &QNetworkReply::finished, parent, [=]() {
      pageReply->deleteLater();
      nam->deleteLater();
      if (pageReply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(pageReply->readAll());
        if (doc.isObject()) {
          QJsonObject obj = doc.object();
          QString latestVer = obj.value("latest_version").toString().trimmed();
          if (latestVer.isEmpty()) {
            latestVer = obj.value("updates").toObject().value("linux").toObject()
                            .value("latest-version").toString().trimmed();
          }
          if (latestVer.startsWith('v') || latestVer.startsWith('V')) latestVer = latestVer.mid(1);
          QString title = obj.value("release_title").toString("MatraKi Release");
          QString changelog = obj.value("changelog").toString();
          QString relUrl = obj.value("github_release_url").toString(GITHUB_RELEASES_URL);
          if (!latestVer.isEmpty()) {
            showDialog(parent, isRemoteVersionNewer(latestVer, QStringLiteral(PROJECT_VERSION)),
                       latestVer, title, changelog, relUrl, silent);
            return;
          }
        }
      }
      /* Offline or unreachable — only report when the user asked for it. */
      showDialog(parent, false, QStringLiteral(PROJECT_VERSION),
                 QStringLiteral("MatraKi v%1 — Harper AI Edition").arg(QStringLiteral(PROJECT_VERSION)),
                 QString(), GITHUB_RELEASES_URL, silent);
    });
  });
}

void MatraKiUpdater::showDialog(QWidget *parent, bool updateAvailable, const QString &latestVer,
                                 const QString &releaseTitle, const QString &changelog,
                                 const QString &releaseUrl, bool silent) {
  if (!updateAvailable && silent) return;

  UpdateDialog dlg(parent);
  dlg.setUpdateInfo(updateAvailable, latestVer, releaseTitle, changelog, releaseUrl);
  dlg.exec();
}
