/*
 *  MatraKi Keyboard
 *  Software Update Dialog — Onyx Design System
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

#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QString>

#include "MatraKiUpdater.h"

class QFile;
class QNetworkAccessManager;
class QNetworkReply;

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog {
  Q_OBJECT

public:
  explicit UpdateDialog(QWidget *parent = nullptr);
  ~UpdateDialog() override;

  void setUpdateInfo(bool updateAvailable, const QString &latestVer,
                     const QString &releaseTitle, const QString &changelog,
                     const QString &releaseUrl,
                     const MatraKiReleaseAssets &assets = MatraKiReleaseAssets(),
                     bool checkFailed = false);

protected:
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

private slots:
  void on_btnCloseIcon_clicked();
  void on_btnLater_clicked();
  void on_btnGitHub_clicked();
  void on_btnAction_clicked();

private:
  enum class PackageKind { None, AppImage, Deb, Rpm, PortableTarball, RawBinary };

  Ui::UpdateDialog *ui;
  bool m_updateAvailable = false;
  bool m_checkFailed = false;
  QString m_latestVersion;
  QString m_releaseUrl;
  QString m_downloadPageUrl;
  MatraKiReleaseAssets m_assets;

  /* Download state. */
  QNetworkAccessManager *m_network = nullptr;
  QNetworkReply *m_reply = nullptr;
  QFile *m_downloadFile = nullptr;
  QProcess *m_installProcess = nullptr;
  QString m_downloadPath;
  PackageKind m_packageKind = PackageKind::None;
  qint64 m_expectedSize = 0;
  qint64 m_receivedBytes = 0;
  bool m_busy = false;
  bool m_readyToRestart = false;

  void applyTheme();
  void retranslate();
  void setStatus(const QString &text, bool error = false);
  void setProgressVisible(bool visible);
  void setProgressValue(qint64 received, qint64 total);
  void setBusy(bool busy);

  PackageKind choosePackage(MatraKiReleaseAsset *asset) const;
  void startDownload();
  void onDownloadProgress(qint64 received, qint64 total);
  void onDownloadFinished();
  bool installDownloaded(QString *errorMessage);
  bool installPortable(const QString &archive, QString *errorMessage);
  bool installAppImage(const QString &archive, QString *errorMessage);
  void installSystemPackage(const QString &archive);
  void installFinished(int exitCode, QProcess::ExitStatus status);
  void restartApplication();
};

#endif // UPDATEDIALOG_H
