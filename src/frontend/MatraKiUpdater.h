/*
 *  MatraKi Keyboard
 *  Update checker — Onyx Design System
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

#ifndef MATRAKIUPDATER_H
#define MATRAKIUPDATER_H

#include <QList>
#include <QString>

class QWidget;

/** One downloadable file attached to a GitHub release. */
struct MatraKiReleaseAsset {
  QString name;
  QString url;
  qint64 size = 0;

  bool isValid() const { return !url.isEmpty(); }
};

using MatraKiReleaseAssets = QList<MatraKiReleaseAsset>;

/**
 * MatraKiUpdater checks GitHub Releases first and falls back to the
 * official updates.json feed, then presents the result with the Onyx
 * dialog styling. The dialog can download the matching package itself
 * and install it without leaving the app.
 */
class MatraKiUpdater {
public:
  /* Run the check. When @silent is true the dialog is only shown if a newer
     release actually exists. */
  static void check(QWidget *parent, bool silent = false);

  /* True when @remoteVer is newer than @localVer. */
  static bool isRemoteVersionNewer(const QString &remoteVer, const QString &localVer);

private:
  static void showDialog(QWidget *parent, bool updateAvailable, const QString &latestVer,
                          const QString &releaseTitle, const QString &changelog,
                          const QString &releaseUrl, const MatraKiReleaseAssets &assets,
                          bool silent, bool checkFailed = false);
};

#endif // MATRAKIUPDATER_H
