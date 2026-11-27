# Change Log

## [1.2.0] - 2026-11-27 — Onyx Design System
* **Complete Interface Redesign**: Every surface of the desktop application now follows a single, hand-drawn "Onyx" design language — the floating toolbar, the logo menu, the layout picker, the settings page, the About page, the "About current layout" dialog, the layout viewer, the auto-correct dictionary and the Harper grammar dialog.
* **Vector Icon Set**: All icons are now drawn as resolution independent vector art (`MatraKiIcons`) instead of raster PNGs, so they stay crisp on every display and always follow the active theme colour.
* **Brand Mark**: A new vector brand mark (`MatraKiLogo`) — the rounded brass tile with the ivory keycap and the Bengali "ম" and the signature red status dot — is used in the toolbar, in every dialog header and on the official website.
* **Onyx Menus**: The logo menu and the layout picker were rebuilt with custom rows that support a leading icon, a trailing caption ("Live keyboard") and an inline switch for the Harper grammar engine.
* **Real Switch Controls**: All on/off options in the settings page now use pill switches instead of ON/OFF text buttons, and the page is grouped into onyx cards with a scrollable body.
* **About Layout Dialog**: Rebuilt to show the layout name and version chip, engine statistics (trained words / auto-correct entries), maintainer and description in a single, clean card.
* **Shared Update Checker**: The GitHub / updates.json check was extracted into `MatraKiUpdater` and is now available from both the logo menu and the settings page. It detects the way MatraKi was installed and downloads the correct asset.
* **One-Click In-App Updates**: The update dialog now downloads the new release itself, shows live progress, installs it and restarts MatraKi — no browser hop required. AppImages replace themselves, portable/user-local installs receive the new binary and missing AI data files, and system `.deb`/`.rpm` installs use `pkexec` once. Release notes are de-Markdowned and an offline/failed check gets a clear retry state.
* **Release Feed**: The release workflow now publishes direct `.deb` / `.rpm` / `.AppImage` asset URLs in `updates.json` (and the root `UPDATES.json`) for every release, so the fallback path can update too.
* **Adaptive Theme**: Light and Dark modes were re-tuned to the new palette, with a redesigned rounded, translucent window frame for every dialog.
* **Frameless Dialog Chrome**: Dialogs now hide the native title bar so the Onyx surface runs edge to edge; each header carries its own close button and the top strip doubles as a drag handle.
* **Keycap Artwork**: The layout charts and the application icon were regenerated in the same ivory/brass keycap style, with dark and light variants.
* **Website Re-skin**: The official download centre and live studio now use the same Onyx palette as the application.
* **Developer Preview**: `MATRAKI_UI_SHOTS=/some/dir ./matraki-gui` renders every window of the interface to PNG for quick visual review.

## [1.1.0] - 2026-11-15 (Adaptive Theme & Harper AI Edition)
* **Complete UI Redesign & Tech Aesthetic**: Full redesign across TopBar, SettingsDialog, AboutDialog, LayoutViewer, AutoCorrectDialog, and contextual menus with a modern, professional, minimalist classic tech theme.
* **Adaptive Dark & Light Modes**: Added automatic computer OS theme detection (supporting FreeDesktop Portal, GNOME, Deepin, KDE, and Qt palettes) alongside manual options (`Auto (System Detect)`, `Dark Tech Mode`, `Light Classic Mode`) with instant live theme switching and contrast-adapted dual icons.
* **Integrated Harper Grammar Checker**: Added real-time English and Bengali grammar verification engine with subject-verb concordance rules.
* **Expanded Vocabulary**: Deep-learning trained dictionary expanded to 38,000+ words with zero-latency lookups.
* **Instant Productivity Templates**: Added fast shortcodes for job applications (`!applybn`, `!applyen`), CV / resumes (`!cvbn`, `!cven`), and formal emails (`!emailbn`, `!emailen`).
* **Live In-Keyboard Calculator & Unit Converters**: Real-time math evaluation with `=` and 16 unit / currency conversions.
* **Pure Linux Dedicated Packaging**: Fully streamlined packaging for `.deb` (Ubuntu, Debian, Deepin, Mint), `.AppImage`, `.rpm` (Fedora, openSUSE), and portable tarballs (`.tar.gz`).
* **Comprehensive Modern Screenshot Showcase**: Added full visual suite to official portal with high-DPI desktop, settings, templates, grammar, and layout viewer previews.

## [1.0.0] - 2026-11-09
* First public release of MatraKi Keyboard AI Smart Edition.

* Complete re-implementation of input method functionalities in Rust. ([#107][107]) It is [~1.2x faster in searching][3] dictionary suggestions.
* Dictionary suggestion is now available in Fixed Method Layouts(Provat, National etc.) ([#61][61], [#6][6])
* Phonetic suggestion now includes the typed English word as the last item in the suggestion list. ([#121][121], [#124][124])
* UI Refresh & Polish
    * Uses Material Design Icons. ([#165][165])
    * Merges the About button and it's menu into the Icon button which is also used to move the TopBar. This shrinks the TopBar a little. ([#165][165])
    * Redesign the Layout Viewer Dialog. ([#115][115], [#145][145])
    * Redesign the About Dialog. ([#116][116], [#155][155])
    * Redesign the Settings Dialog. This also merges all the available settings from the settings menu. ([#159][159])
    * Polish the Auto Correct entries Dialog. ([#164][164])
* Introduces a new Fixed Keyboard layout format. ([#143][143])
    * Now all of the keyboard layout we distribute are 58% smaller altogether.
    * Properly supports [other system keyboard layouts][36] instead of just the Qwerty layout.
    * Supports AltGr & ShiftAltGr layer of numerical keys.
    * Automatic conversion of old layout formats.
* Now AltGr layer can be typed with the Right Alt key in Fixed Keyboard methods. ([#152][152])
* **Bug Fix**: Handle the BackSlash key in Fixed Keyboard methods. ([#98][98])
* **Bug Fix**: Crash when using the Arrow keys in phonetic method. ([#123][123])
* **Bug Fix**: Fix writing of `Hasant` + `য` and য-ফলা(`্য`) in Fixed Keyboard methods. ([#117][117], [#163][163])
* **Bug Fix**: Fix Window Manager losing the track of the TopBar issue. ([#153][153], [#171][171])
* Follows XDG Directory Specification for storing user specific data. ([#16][16]) Automatically [migrates user specific files][143] into the new place.
* Changes binary names to be more UNIX like. ([#103][103])
* Huge infrastructural change: We now use Github Actions and Bintray for automating our testing and package deployment. ([#130][130])

[107]: https://github.com/MatraKi/MatraKi-Keyboard/pull/107
[3]: https://github.com/MatraKi/riti/pull/3
[61]: https://github.com/MatraKi/MatraKi-Keyboard/issues/61
[6]: https://github.com/MatraKi/riti/pull/6
[121]: https://github.com/MatraKi/MatraKi-Keyboard/issues/121
[124]: https://github.com/MatraKi/MatraKi-Keyboard/pull/124
[16]: https://github.com/MatraKi/MatraKi-Keyboard/issues/16
[165]: https://github.com/MatraKi/MatraKi-Keyboard/pull/165
[116]: https://github.com/MatraKi/MatraKi-Keyboard/issues/116
[155]: https://github.com/MatraKi/MatraKi-Keyboard/pull/155
[159]: https://github.com/MatraKi/MatraKi-Keyboard/pull/159
[164]: https://github.com/MatraKi/MatraKi-Keyboard/pull/164
[115]: https://github.com/MatraKi/MatraKi-Keyboard/issues/115
[145]: https://github.com/MatraKi/MatraKi-Keyboard/pull/145
[143]: https://github.com/MatraKi/MatraKi-Keyboard/pull/143
[36]: https://github.com/MatraKi/MatraKi-Keyboard/issues/36
[98]: https://github.com/MatraKi/MatraKi-Keyboard/issues/98
[123]: https://github.com/MatraKi/MatraKi-Keyboard/issues/123
[152]: https://github.com/MatraKi/MatraKi-Keyboard/issues/152
[117]: https://github.com/MatraKi/MatraKi-Keyboard/issues/117
[163]: https://github.com/MatraKi/MatraKi-Keyboard/issues/163#issuecomment-689956532
[103]: https://github.com/MatraKi/MatraKi-Keyboard/pull/103
[153]: https://github.com/MatraKi/MatraKi-Keyboard/issues/153
[171]: https://github.com/MatraKi/MatraKi-Keyboard/pull/171
[130]: https://github.com/MatraKi/MatraKi-Keyboard/pull/130
## 1.5.1
* Fixes a dictionary loading issue which was affecting in some scenarios. ([#73](https://github.com/MatraKi/MatraKi-Keyboard/issues/73))
* Polished some UI dialogs. ([#66](https://github.com/MatraKi/MatraKi-Keyboard/pull/66))
## 1.5.0
* AutoCorrect item is shown as the first suggestion item if available when writing using Avro Phonetic method. ([#42](https://github.com/MatraKi/MatraKi-Keyboard/pull/42))
* Performance update for Avro Phonetic suggestion making code. ([#42](https://github.com/MatraKi/MatraKi-Keyboard/pull/42))
* Be less restrictive when saving users selection. ([#41](https://github.com/MatraKi/MatraKi-Keyboard/pull/41)) 
* Now it is possible to write using AltGr and ShiftAltGr layer when using Fixed layout methods. Fixes bug [#40](https://github.com/MatraKi/MatraKi-Keyboard/issues/40).
* Auto-update checking is now configurable from settings dialog. There is now also a menu item in the TopBar for instant update checking. ([#32](https://github.com/MatraKi/MatraKi-Keyboard/issues/32))
* Fixed a crash. ([#52](https://github.com/MatraKi/MatraKi-Keyboard/issues/52))
## 1.4.0
* Fixes a serious bug [#26](https://github.com/MatraKi/MatraKi-Keyboard/issues/26).
* MatraKi will check for updates automatically and notify the user if an update is available.
## 1.3.0
* MatraKi now features a Phonetic AutoCorrect Preference Dialog [#20](https://github.com/MatraKi/MatraKi-Keyboard/pull/20). User now can view/edit/add AutoCorrect entries.
* Better Topbar integration with X11 windowing system and reduce the size of TopBar [#22](https://github.com/MatraKi/MatraKi-Keyboard/pull/22).
* Let users turn off candidate box [#11](https://github.com/MatraKi/MatraKi-Keyboard/issues/11)
* Build system improvement. There is no need to pass `CMAKE_INSTALL_PREFIX` when building.
* Now supports RPM packaging.
* Fixed issue [#17](https://github.com/MatraKi/MatraKi-Keyboard/issues/17). MatraKi was failing to store Layout setting when using Qt 5.10 version.
* Improved Settings dialog UI.
* Rewritten Avro Layout conversion code. MatraKi now handles errors more gracefully and reports them to the user.

## 1.2.0
* Fixed a memory leak.
* Fixed a long standing bug [#8](https://github.com/MatraKi/MatraKi-Keyboard/issues/8).
## 1.1.0
* Improve Phonetic support. Resolves issue [#2](https://github.com/MatraKi/MatraKi-Keyboard/issues/2). Now Phonetic support is mostly complete!
* A new button in TopBar for easily moving it.
* User can select candidate by clicking.
* These new layouts were added: **Munir Optima (uni)**, **Avro Easy**, **National (Jatiya)** and **Probhat**. Thanks to [OmicronLab](https://www.omicronlab.com/) because these layouts were also imported from [Avro Keyboard](https://www.omicronlab.com/avro-keyboard.html)!

## 1.0.0
First release! All things are new!

**Changes after 0.1.0 beta:**
* New feature: Tab to switch between the matches in candidate window
* While typing numbers in phonetic method, pressing : should make it : not ঃ
* Old Reph feature in fixed typing method
* User can select the orientation of candidate window in Settings
* Icons for MatraKi by [@saaiful](https://github.com/saaiful)

**Known issues:**
* [Implement suffix phonetic suggestion](https://github.com/MatraKi/MatraKi-Keyboard/issues/2)

## 0.1.0
Beta release
