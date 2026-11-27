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

#include "UpdateDialog.h"
#include "ui_UpdateDialog.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStyle>
#include <QSysInfo>
#include <QTimer>
#include <QUrl>

#include <functional>

#include "MatraKiIcons.h"
#include "MatraKiLogo.h"
#include "MatraKiWindowChrome.h"
#include "ThemeManager.h"

static const QString DEFAULT_UPDATE_PAGE =
    QStringLiteral("https://sojibcreates-maker.github.io/MatraKi-Keyboard/#downloads");
static const QString DEFAULT_GITHUB_RELEASES =
    QStringLiteral("https://github.com/sojibcreates-maker/MatraKi-Keyboard/releases/latest");

/* ------------------------------------------------------------------ */
/*  Small helpers                                                     */
/* ------------------------------------------------------------------ */

static QString shellQuote(const QString &value) {
  QString escaped = value;
  escaped.replace('\'', QStringLiteral("'\\''"));
  return QStringLiteral("'%1'").arg(escaped);
}

static QString currentExecutablePath() {
  QFileInfo link(QStringLiteral("/proc/self/exe"));
  const QString target = link.symLinkTarget();
  if (!target.isEmpty()) return QFileInfo(target).absoluteFilePath();
  return QCoreApplication::applicationFilePath();
}

static bool executableDirectoryIsWritable() {
  const QString exe = currentExecutablePath();
  if (exe.isEmpty()) return false;
  QFileInfo info(exe);
  if (!info.exists()) return false;
  return QFileInfo(info.absolutePath()).isWritable();
}

static QString cpuArchToken() {
  const QString cpu = QSysInfo::currentCpuArchitecture().toLower();
  if (cpu.contains(QStringLiteral("arm")) || cpu.contains(QStringLiteral("aarch"))) {
    return cpu.contains(QStringLiteral("64")) ? QStringLiteral("aarch64")
                                             : QStringLiteral("armhf");
  }
  return cpu.contains(QStringLiteral("64")) ? QStringLiteral("x86_64")
                                            : QStringLiteral("i386");
}

static QString debArchToken() {
  const QString cpu = QSysInfo::currentCpuArchitecture().toLower();
  if (cpu.contains(QStringLiteral("arm")) || cpu.contains(QStringLiteral("aarch"))) {
    return cpu.contains(QStringLiteral("64")) ? QStringLiteral("arm64")
                                             : QStringLiteral("armhf");
  }
  return cpu.contains(QStringLiteral("64")) ? QStringLiteral("amd64")
                                            : QStringLiteral("i386");
}

static QString downloadCacheDir() {
  QString root = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  if (root.isEmpty()) root = QDir::tempPath() + QStringLiteral("/matraki-cache");
  const QString dir = root + QStringLiteral("/updates");
  QDir().mkpath(dir);
  return dir;
}

static void makeExecutable(const QString &path) {
  QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                  QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                  QFileDevice::ExeGroup | QFileDevice::ReadOther |
                                  QFileDevice::ExeOther);
}

static QString humanSize(qint64 bytes) {
  if (bytes < 1024) return QStringLiteral("%1 B").arg(bytes);
  if (bytes < 1024 * 1024) return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 0);
  return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
}

/* Release notes arrive as Markdown. The dialog has room for a short
   paragraph, so strip headings, emphasis, links and code fences. */
static QString cleanReleaseNotes(const QString &raw) {
  QString text = raw;
  text.replace(QRegularExpression(QStringLiteral("```[\\s\\S]*?```")), QStringLiteral(" "));
  text.replace(QRegularExpression(QStringLiteral("<[^>]+>")), QStringLiteral(" "));
  text.replace(QRegularExpression(QStringLiteral("!\\[([^\\]]*)\\]\\([^)]*\\)")),
               QStringLiteral("\\1"));
  text.replace(QRegularExpression(QStringLiteral("\\[([^\\]]*)\\]\\([^)]*\\)")),
               QStringLiteral("\\1"));
  text.replace(QRegularExpression(QStringLiteral("(?m)^\\s{0,3}#{1,6}\\s*")), QString());
  text.replace(QRegularExpression(QStringLiteral("(?m)^\\s*[-*+]\\s+")), QStringLiteral("• "));
  text.remove(QLatin1Char('*'));
  text.remove(QLatin1Char('`'));
  text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
  return text.trimmed();
}

/* ------------------------------------------------------------------ */
/*  Construction                                                      */
/* ------------------------------------------------------------------ */

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog) {
  ui->setupUi(this);

  setAttribute(Qt::WA_TranslucentBackground, true);
  setWindowIcon(QIcon(":/images/MatraKi.png"));
  makeOnyxDialog(this);

  ui->btnCloseIcon->setObjectName("iconOnly");
  ui->btnAction->setObjectName("primary");
  ui->progressCard->setVisible(false);
  ui->lblProgress->setWordWrap(true);
  ui->progressBar->setTextVisible(false);
  ui->progressBar->setRange(0, 100);
  ui->progressBar->setValue(0);

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool) {
    applyTheme();
  });

  applyTheme();
}

UpdateDialog::~UpdateDialog() {
  if (m_reply) {
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
  }
  if (m_downloadFile) {
    m_downloadFile->close();
    delete m_downloadFile;
    m_downloadFile = nullptr;
  }
  if (!m_downloadPath.isEmpty()) QFile::remove(m_downloadPath);
  delete ui;
}

void UpdateDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  applyTheme();
}

void UpdateDialog::closeEvent(QCloseEvent *event) {
  /* Never disappear halfway through a privileged install or while the
     freshly installed build is about to restart. */
  if (m_installProcess || m_readyToRestart) {
    event->ignore();
    return;
  }
  if (m_reply) {
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
  }
  QDialog::closeEvent(event);
}

/* ------------------------------------------------------------------ */
/*  Theme + copy                                                      */
/* ------------------------------------------------------------------ */

void UpdateDialog::applyTheme() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  setStyleSheet(ThemeManager::instance().updateDialogStyle());

  ui->logoSlot->setPixmap(MatraKiLogo::render(26, p.brand, p.danger));
  ui->btnCloseIcon->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Close, p.icon, 18)));

  if (m_updateAvailable) {
    ui->iconBadge->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Refresh, p.accent, 24));
    ui->iconBadge->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 19px;").arg(p.accentSoft));
  } else if (m_checkFailed) {
    ui->iconBadge->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Info, p.textMuted, 22));
    ui->iconBadge->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 19px;").arg(p.accentSoft));
  } else {
    ui->iconBadge->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Check, p.brand, 22));
    ui->iconBadge->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 19px;").arg(p.accentSoft));
  }

  /* Re-polish the status label so the error colour follows theme changes. */
  ui->lblProgress->style()->unpolish(ui->lblProgress);
  ui->lblProgress->style()->polish(ui->lblProgress);
}

void UpdateDialog::setStatus(const QString &text, bool error) {
  ui->lblProgress->setText(text);
  ui->lblProgress->setProperty("error", error);
  ui->lblProgress->style()->unpolish(ui->lblProgress);
  ui->lblProgress->style()->polish(ui->lblProgress);
}

void UpdateDialog::setProgressVisible(bool visible) {
  ui->progressCard->setVisible(visible);
}

void UpdateDialog::setProgressValue(qint64 received, qint64 total) {
  if (total <= 0) total = m_expectedSize;
  if (total > 0) {
    const int percent = static_cast<int>(qBound<qint64>(0, received * 100 / total, 100));
    ui->progressBar->setValue(percent);
  } else {
    ui->progressBar->setValue(0);
  }
  m_receivedBytes = received;

  QString detail;
  if (total > 0) {
    detail = QStringLiteral("%1 / %2").arg(humanSize(received), humanSize(total));
  } else {
    detail = humanSize(received);
  }
  setStatus(QStringLiteral("ডাউনলোড হচ্ছে… %1  (%2)").arg(ui->progressBar->value()).arg(detail));
}

void UpdateDialog::setBusy(bool busy) {
  m_busy = busy;
  ui->btnAction->setEnabled(!busy);
  ui->btnLater->setEnabled(!busy);
  ui->btnGitHub->setEnabled(!busy);
  ui->btnCloseIcon->setEnabled(!busy || m_installProcess == nullptr);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                        */
/* ------------------------------------------------------------------ */

void UpdateDialog::setUpdateInfo(bool updateAvailable, const QString &latestVer,
                                 const QString &releaseTitle, const QString &changelog,
                                 const QString &releaseUrl,
                                 const MatraKiReleaseAssets &assets, bool checkFailed) {
  m_checkFailed = checkFailed;
  m_updateAvailable = updateAvailable && !checkFailed;
  m_latestVersion = latestVer;
  m_assets = assets;
  m_readyToRestart = false;
  m_releaseUrl = releaseUrl.isEmpty() ? DEFAULT_GITHUB_RELEASES : releaseUrl;
  m_downloadPageUrl = DEFAULT_UPDATE_PAGE;

  MatraKiReleaseAsset probe;
  m_packageKind = m_updateAvailable ? choosePackage(&probe) : PackageKind::None;

  if (m_updateAvailable) {
    QString packageHint;
    switch (m_packageKind) {
      case PackageKind::AppImage:
        packageHint = QStringLiteral("AppImage");
        break;
      case PackageKind::Deb:
        packageHint = QStringLiteral(".deb");
        break;
      case PackageKind::Rpm:
        packageHint = QStringLiteral(".rpm");
        break;
      case PackageKind::PortableTarball:
        packageHint = QStringLiteral(".tar.gz");
        break;
      case PackageKind::RawBinary:
        packageHint = QStringLiteral("standalone");
        break;
      case PackageKind::None:
        break;
    }

    setWindowTitle("MatraKi · নতুন আপডেট পাওয়া গেছে!");
    ui->lblSub->setText("নতুন আপডেট পাওয়া গেছে");
    ui->lblHeadline->setText(QString("নতুন আপডেট পাওয়া গেছে · v%1").arg(latestVer));
    QString versionLine = QString("ইনস্টলকৃত: v%1  •  সর্বশেষ: v%2")
                              .arg(QStringLiteral(PROJECT_VERSION), latestVer);
    if (!packageHint.isEmpty()) {
      versionLine += QStringLiteral("  •  %1").arg(packageHint);
    }
    ui->lblVersionLine->setText(versionLine);
    ui->lblHighlightsTitle->setText("রিলিজ হাইলাইটস (Release Highlights):");

    QString summary = cleanReleaseNotes(changelog);
    if (summary.isEmpty()) {
      summary = "Next-generation AI-smart Bangla phonetic & fixed-layout suite, engineered natively for Linux.";
    } else if (summary.length() > 240) {
      summary = summary.left(237) + QStringLiteral("…");
    }
    ui->lblHighlightsBody->setText(summary);

    ui->btnLater->setText("পরে");
    ui->btnAction->setText(m_packageKind == PackageKind::None
                               ? "ম্যানুয়াল ডাউনলোড"
                               : "ডাউনলোড ও ইনস্টল");
  } else if (m_checkFailed) {
    setWindowTitle("MatraKi · আপডেট চেক");
    ui->lblSub->setText("আপডেট চেক");
    ui->lblHeadline->setText("আপডেট সার্ভারে পৌঁছানো যায়নি");
    ui->lblVersionLine->setText(
        QString("ইনস্টলকৃত: v%1 — ইন্টারনেট সংযোগ পরীক্ষা করে আবার চেষ্টা করুন।")
            .arg(QStringLiteral(PROJECT_VERSION)));
    ui->lblHighlightsTitle->setText("আপনি যা করতে পারেন:");
    ui->lblHighlightsBody->setText(
        "•  ইন্টারনেট সংযোগ পরীক্ষা করুন\n"
        "•  কিছুক্ষণ পরে আবার চেক করুন\n"
        "•  অথবা সরাসরি ডাউনলোড পেজ থেকে সর্বশেষ সংস্করণ নামান");
    ui->btnLater->setText("বন্ধ করুন");
    ui->btnAction->setText("ম্যানুয়াল ডাউনলোড");
  } else {
    setWindowTitle("MatraKi · সফটওয়্যার আপডেট");
    ui->lblSub->setText("সফটওয়্যার আপডেট");
    ui->lblHeadline->setText(QString("সর্বশেষ সংস্করণ ইনস্টল আছে  •  v%1").arg(QStringLiteral(PROJECT_VERSION)));
    ui->lblVersionLine->setText(QString("ইনস্টলকৃত: v%1 — Onyx AI Edition (সর্বশেষ সংস্করণ)").arg(QStringLiteral(PROJECT_VERSION)));
    ui->lblHighlightsTitle->setText("রিলিজ হাইলাইটস:");
    ui->lblHighlightsBody->setText(
        "• ৩৮,৫৯২+ অভিধান এন্ট্রি ও অটো-কারেক্ট নিয়ম\n"
        "• স্মার্ট সাফিক্স স্টেমার, নেক্সট-ওয়ার্ড AI ও লাইভ ম্যাথ (=)\n"
        "• ১০০+ রেডি স্নিপেট (!salam, !bismillah, ইমেইল ও সিভি)");

    ui->btnLater->setText("বন্ধ করুন");
    ui->btnAction->setText("অফিসিয়াল ওয়েবসাইট");
  }

  setProgressVisible(false);
  setBusy(false);
  applyTheme();
}

/* ------------------------------------------------------------------ */
/*  Package selection                                                 */
/* ------------------------------------------------------------------ */

UpdateDialog::PackageKind UpdateDialog::choosePackage(MatraKiReleaseAsset *asset) const {
  if (asset) *asset = MatraKiReleaseAsset();

  const QString arch = cpuArchToken();
  const QString debArch = debArchToken();
  const QString appImage = qEnvironmentVariable("APPIMAGE");
  const bool exeWritable = executableDirectoryIsWritable();

  auto find = [this](const std::function<bool(const QString &)> &predicate,
                     MatraKiReleaseAsset *out) -> bool {
    for (const MatraKiReleaseAsset &candidate : m_assets) {
      const QString name = candidate.name.toLower();
      if (predicate(name)) {
        if (out) *out = candidate;
        return true;
      }
    }
    return false;
  };

  /* A real AppImage must replace itself, otherwise the launcher keeps
     pointing at the old file. */
  if (!appImage.isEmpty()) {
    if (find([&](const QString &n) {
          return n.endsWith(QStringLiteral(".appimage")) && n.contains(arch);
        }, asset)) {
      return PackageKind::AppImage;
    }
  }

  if (exeWritable) {
    /* Portable install — no password, no package manager, no restart prompt. */
    if (find([&](const QString &n) {
          return n.endsWith(QStringLiteral(".tar.gz")) && n.contains(QStringLiteral("-linux-")) &&
                 n.contains(arch);
        }, asset)) {
      return PackageKind::PortableTarball;
    }
    if (find([&](const QString &n) {
          return n == QStringLiteral("matraki-gui") || n.endsWith(QStringLiteral("/matraki-gui"));
        }, asset)) {
      return PackageKind::RawBinary;
    }
    /* We can still pull the binary out of a package without root. */
    if (find([&](const QString &n) {
          return n.endsWith(QStringLiteral(".deb")) && n.contains(debArch);
        }, asset)) {
      return PackageKind::Deb;
    }
    if (find([&](const QString &n) {
          return n.endsWith(QStringLiteral(".rpm")) && n.contains(arch);
        }, asset)) {
      return PackageKind::Rpm;
    }
    return PackageKind::None;
  }

  /* System-wide installation — ask for administrator rights once. */
  if (!QStandardPaths::findExecutable(QStringLiteral("pkexec")).isEmpty()) {
    if (find([&](const QString &n) {
          return n.endsWith(QStringLiteral(".deb")) && n.contains(debArch);
        }, asset)) {
      return PackageKind::Deb;
    }
    if (find([&](const QString &n) {
          return n.endsWith(QStringLiteral(".rpm")) && n.contains(arch);
        }, asset)) {
      return PackageKind::Rpm;
    }
  }
  return PackageKind::None;
}

/* ------------------------------------------------------------------ */
/*  Download                                                          */
/* ------------------------------------------------------------------ */

void UpdateDialog::startDownload() {
  if (m_busy || !m_updateAvailable) return;

  MatraKiReleaseAsset asset;
  m_packageKind = choosePackage(&asset);
  if (!asset.isValid() || m_packageKind == PackageKind::None) {
    QDesktopServices::openUrl(QUrl(m_downloadPageUrl));
    return;
  }

  const QString fileName = QFileInfo(asset.name).fileName();
  m_downloadPath = downloadCacheDir() + QLatin1Char('/') +
                   (fileName.isEmpty() ? QStringLiteral("matraki-update") : fileName);
  QFile::remove(m_downloadPath);

  m_expectedSize = asset.size;
  m_receivedBytes = 0;
  m_downloadFile = new QFile(m_downloadPath, this);
  if (!m_downloadFile->open(QIODevice::WriteOnly)) {
    setProgressVisible(true);
    setStatus(QStringLiteral("ডাউনলোড ফাইল তৈরি করা যায়নি। ডিস্কের জায়গা পরীক্ষা করুন।"), true);
    delete m_downloadFile;
    m_downloadFile = nullptr;
    return;
  }

  setBusy(true);
  setProgressVisible(true);
  setProgressValue(0, m_expectedSize);
  setStatus(QStringLiteral("সংযোগ তৈরি হচ্ছে…"));

  m_network = new QNetworkAccessManager(this);
  QNetworkRequest request{QUrl(asset.url)};
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QNetworkRequest::NoLessSafeRedirectPolicy);
  request.setRawHeader("User-Agent", "MatraKi-Keyboard-Updater");
  request.setRawHeader("Accept", "application/octet-stream");
  m_reply = m_network->get(request);

  connect(m_reply, &QNetworkReply::readyRead, this, [this]() {
    if (m_downloadFile && m_reply) m_downloadFile->write(m_reply->readAll());
  });
  connect(m_reply, &QNetworkReply::downloadProgress, this,
          [this](qint64 received, qint64 total) { onDownloadProgress(received, total); });
  connect(m_reply, &QNetworkReply::finished, this, &UpdateDialog::onDownloadFinished);
}

void UpdateDialog::onDownloadProgress(qint64 received, qint64 total) {
  if (total > 0) m_expectedSize = total;
  setProgressValue(received, total);
}

void UpdateDialog::onDownloadFinished() {
  if (!m_reply) return;

  const QNetworkReply::NetworkError error = m_reply->error();
  const int httpStatus = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

  if (m_downloadFile) {
    m_downloadFile->write(m_reply->readAll());
    m_downloadFile->flush();
    m_downloadFile->close();
    delete m_downloadFile;
    m_downloadFile = nullptr;
  }

  m_reply->deleteLater();
  m_reply = nullptr;

  if (error != QNetworkReply::NoError || (httpStatus >= 400)) {
    QFile::remove(m_downloadPath);
    setBusy(false);
    QString reason;
    if (httpStatus >= 400) {
      reason = QStringLiteral("HTTP %1").arg(httpStatus);
    } else {
      reason = QStringLiteral("নেটওয়ার্ক ত্রুটি %1").arg(static_cast<int>(error));
    }
    setStatus(QStringLiteral("ডাউনলোড ব্যর্থ হয়েছে (%1)। আবার চেষ্টা করুন।").arg(reason), true);
    return;
  }

  const qint64 size = QFileInfo(m_downloadPath).size();
  if (size < 4096) {
    QFile::remove(m_downloadPath);
    setBusy(false);
    setStatus(QStringLiteral("ডাউনলোড করা ফাইলটি অসম্পূর্ণ। আবার চেষ্টা করুন।"), true);
    return;
  }
  if (m_expectedSize > 0 && size != m_expectedSize) {
    QFile::remove(m_downloadPath);
    setBusy(false);
    setStatus(QStringLiteral("ফাইলের আকার মেলেনি (%1 / %2)। আবার চেষ্টা করুন।")
                  .arg(humanSize(size), humanSize(m_expectedSize)), true);
    return;
  }

  ui->progressBar->setValue(100);
  setStatus(QStringLiteral("ডাউনলোড সম্পন্ন — যাচাই করা হচ্ছে…"));

  QString errorMessage;
  if (!installDownloaded(&errorMessage)) {
    QFile::remove(m_downloadPath);
    setBusy(false);
    setStatus(QStringLiteral("ইনস্টল করা যায়নি: %1").arg(errorMessage), true);
  }
}

/* ------------------------------------------------------------------ */
/*  Install                                                           */
/* ------------------------------------------------------------------ */

bool UpdateDialog::installDownloaded(QString *errorMessage) {
  switch (m_packageKind) {
    case PackageKind::AppImage:
      return installAppImage(m_downloadPath, errorMessage);
    case PackageKind::Deb:
    case PackageKind::Rpm:
      if (executableDirectoryIsWritable()) {
        /* Deb/rpm used as a portable container, not a system package. */
        return installPortable(m_downloadPath, errorMessage);
      }
      installSystemPackage(m_downloadPath);
      return true;
    case PackageKind::PortableTarball:
    case PackageKind::RawBinary:
      return installPortable(m_downloadPath, errorMessage);
    case PackageKind::None:
      break;
  }
  if (errorMessage) *errorMessage = QStringLiteral("কোনো উপযুক্ত প্যাকেজ পাওয়া যায়নি");
  return false;
}

bool UpdateDialog::installPortable(const QString &archive, QString *errorMessage) {
  const QString exe = currentExecutablePath();
  if (exe.isEmpty()) {
    if (errorMessage) *errorMessage = QStringLiteral("চলমান অ্যাপের অবস্থান পাওয়া যায়নি");
    return false;
  }

  setStatus(QStringLiteral("ইনস্টল হচ্ছে…"));

  QString sourceBinary = archive;
  QString extractDir;
  QString dataDir;

  if (m_packageKind != PackageKind::RawBinary) {
    extractDir = downloadCacheDir() + QStringLiteral("/extract");
    QDir(extractDir).removeRecursively();
    if (!QDir().mkpath(extractDir)) {
      if (errorMessage) *errorMessage = QStringLiteral("অস্থায়ী ফোল্ডার তৈরি করা যায়নি");
      return false;
    }

    QProcess extractor;
    if (m_packageKind == PackageKind::PortableTarball) {
      extractor.start(QStringLiteral("tar"),
                      {QStringLiteral("-xzf"), archive, QStringLiteral("-C"), extractDir});
    } else if (m_packageKind == PackageKind::Deb) {
      extractor.start(QStringLiteral("dpkg-deb"),
                      {QStringLiteral("-x"), archive, extractDir});
    } else if (m_packageKind == PackageKind::Rpm) {
      extractor.start(QStringLiteral("/bin/sh"),
                      {QStringLiteral("-c"),
                       QStringLiteral("rpm2cpio %1 | (cd %2 && cpio -idm --quiet)")
                           .arg(shellQuote(archive), shellQuote(extractDir))});
    }
    if (!extractor.waitForStarted(5000) || !extractor.waitForFinished(120000) ||
        extractor.exitStatus() != QProcess::NormalExit || extractor.exitCode() != 0) {
      QDir(extractDir).removeRecursively();
      if (errorMessage) {
        *errorMessage = QStringLiteral("প্যাকেজ খোলা যায়নি (%1)")
                            .arg(QString::fromLocal8Bit(extractor.readAllStandardError()).trimmed());
      }
      return false;
    }

    QDirIterator it(extractDir, QStringList() << QStringLiteral("matraki-gui"),
                    QDir::Files, QDirIterator::Subdirectories);
    if (it.hasNext()) sourceBinary = it.next();

    /* Publish bundled AI data only when the user has no local copy yet. */
    QString root = QFileInfo(sourceBinary).absolutePath();
    for (int i = 0; i < 2; ++i) {
      QDir candidate(root);
      if (candidate.exists(QStringLiteral("data"))) {
        dataDir = candidate.filePath(QStringLiteral("data"));
        break;
      }
      if (!candidate.cdUp()) break;
      root = candidate.absolutePath();
    }

    if (sourceBinary == archive || !QFileInfo(sourceBinary).isFile()) {
      QDir(extractDir).removeRecursively();
      if (errorMessage) *errorMessage = QStringLiteral("প্যাকেজের ভেতরে matraki-gui পাওয়া যায়নি");
      return false;
    }
  } else {
    QFile binary(archive);
    if (!binary.open(QIODevice::ReadOnly)) {
      if (errorMessage) *errorMessage = QStringLiteral("বাইনারি ফাইল পড়া যায়নি");
      return false;
    }
    const QByteArray magic = binary.read(4);
    binary.close();
    if (magic != QByteArray("\x7f" "ELF", 4)) {
      if (errorMessage) *errorMessage = QStringLiteral("ফাইলটি বৈধ Linux বাইনারি নয়");
      return false;
    }
  }

  const QString staged = exe + QStringLiteral(".new");
  QFile::remove(staged);
  if (!QFile::copy(sourceBinary, staged)) {
    if (!extractDir.isEmpty()) QDir(extractDir).removeRecursively();
    if (errorMessage) *errorMessage = QStringLiteral("নতুন বাইনারি লেখা যায়নি");
    return false;
  }
  makeExecutable(staged);

  const QString backup = exe + QStringLiteral(".old");
  QFile::remove(backup);
  if (!QFile::rename(exe, backup)) {
    QFile::remove(staged);
    if (!extractDir.isEmpty()) QDir(extractDir).removeRecursively();
    if (errorMessage) *errorMessage = QStringLiteral("পুরোনো সংস্করণ সরানো যায়নি");
    return false;
  }
  if (!QFile::rename(staged, exe)) {
    QFile::rename(backup, exe);
    QFile::remove(staged);
    if (!extractDir.isEmpty()) QDir(extractDir).removeRecursively();
    if (errorMessage) *errorMessage = QStringLiteral("নতুন সংস্করণ বসানো যায়নি");
    return false;
  }
  QFile::remove(backup);

  /* Refresh the user-level engine data, but never overwrite personal edits. */
  if (!dataDir.isEmpty()) {
    const QString target = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                           QStringLiteral("/matraki-keyboard");
    QDir().mkpath(target);
    const QStringList copyIfMissing = {
        QStringLiteral("transliteration.json"),
        QStringLiteral("phonetic-candidate-selection.json"),
        QStringLiteral("phonetic-next-word.json"),
        QStringLiteral("snippets.json")};
    for (const QString &name : copyIfMissing) {
      const QString src = QDir(dataDir).filePath(name);
      const QString dst = QDir(target).filePath(name);
      if (QFile::exists(src) && !QFile::exists(dst)) QFile::copy(src, dst);
    }
  }

  if (!extractDir.isEmpty()) QDir(extractDir).removeRecursively();
  QFile::remove(archive);

  setStatus(QStringLiteral("✓ আপডেট ইনস্টল হয়েছে। MatraKi রিস্টার্ট হচ্ছে…"));
  m_readyToRestart = true;
  ui->btnCloseIcon->setEnabled(false);
  ui->btnLater->setEnabled(false);
  QTimer::singleShot(1400, this, &UpdateDialog::restartApplication);
  return true;
}

bool UpdateDialog::installAppImage(const QString &archive, QString *errorMessage) {
  const QString appImage = qEnvironmentVariable("APPIMAGE");
  if (appImage.isEmpty()) {
    if (errorMessage) *errorMessage = QStringLiteral("AppImage এর অবস্থান পাওয়া যায়নি");
    return false;
  }

  setStatus(QStringLiteral("ইনস্টল হচ্ছে…"));

  const QString staged = appImage + QStringLiteral(".new");
  QFile::remove(staged);
  if (!QFile::copy(archive, staged)) {
    if (errorMessage) *errorMessage = QStringLiteral("নতুন AppImage লেখা যায়নি");
    return false;
  }
  makeExecutable(staged);

  const QString backup = appImage + QStringLiteral(".old");
  QFile::remove(backup);
  if (!QFile::rename(appImage, backup)) {
    QFile::remove(staged);
    if (errorMessage) *errorMessage = QStringLiteral("পুরোনো AppImage সরানো যায়নি");
    return false;
  }
  if (!QFile::rename(staged, appImage)) {
    QFile::rename(backup, appImage);
    QFile::remove(staged);
    if (errorMessage) *errorMessage = QStringLiteral("নতুন AppImage বসানো যায়নি");
    return false;
  }
  QFile::remove(backup);
  QFile::remove(archive);

  setStatus(QStringLiteral("✓ আপডেট ইনস্টল হয়েছে। MatraKi রিস্টার্ট হচ্ছে…"));
  m_readyToRestart = true;
  ui->btnCloseIcon->setEnabled(false);
  ui->btnLater->setEnabled(false);
  QTimer::singleShot(1400, this, &UpdateDialog::restartApplication);
  return true;
}

void UpdateDialog::installSystemPackage(const QString &archive) {
  setStatus(QStringLiteral("প্রশাসক অনুমতি প্রয়োজন — পাসওয়ার্ড দিন…"));

  QString command;
  if (m_packageKind == PackageKind::Deb) {
    command = QStringLiteral(
                  "DEBIAN_FRONTEND=noninteractive apt-get install -y --allow-downgrades %1")
                  .arg(shellQuote(archive));
  } else {
    if (!QStandardPaths::findExecutable(QStringLiteral("dnf")).isEmpty()) {
      command = QStringLiteral("dnf install -y %1").arg(shellQuote(archive));
    } else {
      command = QStringLiteral("zypper --non-interactive install -y %1").arg(shellQuote(archive));
    }
  }

  m_installProcess = new QProcess(this);
  connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, &UpdateDialog::installFinished);
  setBusy(true); /* keep the close button locked while the installer runs */
  m_installProcess->start(QStringLiteral("pkexec"),
                          {QStringLiteral("/bin/sh"), QStringLiteral("-c"), command});
  if (!m_installProcess->waitForStarted(5000)) {
    delete m_installProcess;
    m_installProcess = nullptr;
    QFile::remove(archive);
    setBusy(false);
    setStatus(QStringLiteral("প্রশাসক অনুমতি নেওয়া যায়নি (pkexec পাওয়া যায়নি)।"), true);
  }
}

void UpdateDialog::installFinished(int exitCode, QProcess::ExitStatus status) {
  if (m_installProcess) {
    const QByteArray stderrText = m_installProcess->readAllStandardError();
    m_installProcess->deleteLater();
    m_installProcess = nullptr;

    if (status != QProcess::NormalExit || exitCode != 0) {
      QFile::remove(m_downloadPath);
      setBusy(false);
      QString detail = QString::fromLocal8Bit(stderrText).trimmed();
      if (detail.size() > 160) detail = detail.left(160) + QStringLiteral("…");
      setStatus(QStringLiteral("প্যাকেজ ইনস্টল ব্যর্থ হয়েছে। %1").arg(detail), true);
      return;
    }
  }

  /* The deb/rpm installs a fresh GUI in /usr/share. Mirror it into the
     user-local launcher when one is in use. */
  const QString systemBinary = QStringLiteral("/usr/share/matraki-keyboard/matraki-gui");
  const QString exe = currentExecutablePath();
  if (exe != systemBinary && QFileInfo(systemBinary).isExecutable() &&
      executableDirectoryIsWritable()) {
    QFile::remove(exe);
    if (QFile::copy(systemBinary, exe)) makeExecutable(exe);
  }

  QFile::remove(m_downloadPath);
  setStatus(QStringLiteral("✓ আপডেট ইনস্টল হয়েছে। MatraKi রিস্টার্ট হচ্ছে…"));
  m_readyToRestart = true;
  ui->btnCloseIcon->setEnabled(false);
  ui->btnLater->setEnabled(false);
  QTimer::singleShot(1400, this, &UpdateDialog::restartApplication);
}

/* ------------------------------------------------------------------ */
/*  Restart + buttons                                                 */
/* ------------------------------------------------------------------ */

void UpdateDialog::restartApplication() {
  /* Inside an AppImage /proc/self/exe points at the read-only mount, so
     relaunch the outer AppImage file that we just replaced. */
  const QString appImage = qEnvironmentVariable("APPIMAGE");
  QString exe = appImage.isEmpty() ? currentExecutablePath() : appImage;
  const QString systemBinary = QStringLiteral("/usr/share/matraki-keyboard/matraki-gui");
  if (!QFileInfo(exe).isExecutable() && QFileInfo(systemBinary).isExecutable()) {
    exe = systemBinary;
  }
  if (exe.isEmpty()) {
    QCoreApplication::quit();
    return;
  }

  const QString command = QStringLiteral("sleep 1; nohup %1 >/dev/null 2>&1 &")
                              .arg(shellQuote(exe));
  QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), command});
  QCoreApplication::quit();
}

void UpdateDialog::on_btnCloseIcon_clicked() {
  if (m_installProcess || m_readyToRestart) return;
  close();
}

void UpdateDialog::on_btnLater_clicked() {
  if (m_installProcess || m_readyToRestart) return;
  close();
}

void UpdateDialog::on_btnGitHub_clicked() {
  QDesktopServices::openUrl(QUrl(m_releaseUrl.isEmpty() ? DEFAULT_GITHUB_RELEASES : m_releaseUrl));
}

void UpdateDialog::on_btnAction_clicked() {
  if (m_checkFailed) {
    QDesktopServices::openUrl(QUrl(m_downloadPageUrl));
    return;
  }
  if (!m_updateAvailable) {
    QDesktopServices::openUrl(QUrl(DEFAULT_UPDATE_PAGE));
    return;
  }
  if (m_busy) return;

  if (m_packageKind == PackageKind::None) {
    QDesktopServices::openUrl(QUrl(m_downloadPageUrl));
    return;
  }
  startDownload();
}
