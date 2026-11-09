/*
 *  MatraKi Keyboard
 *  Copyright (C) 2012-2012 Yichao Yu
 *  Copyright (C) 2020-2025 CSSlayer <wengxt@gmail.com>
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
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _FCITX5_MATRAKI_MATRAKI_H_
#define _FCITX5_MATRAKI_MATRAKI_H_

#include "riti.h"
#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/option.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/misc.h>
#include <fcitx-utils/trackableobject.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <chrono>
#include <filesystem>

namespace fcitx {

class MatraKiState;

FCITX_CONFIGURATION(MatraKiConfig,
                    ExternalOption config{this, "MatraKiKeyboard",
                                          _("MatraKi"),
                                          BIN_DIR "/matraki-gui"};);

class MatraKiEngine final : public InputMethodEngine {
public:
  MatraKiEngine(Instance *instance);
  ~MatraKiEngine();

  void activate(const fcitx::InputMethodEntry & /*entry*/,
                fcitx::InputContextEvent & /*event*/) override;
  void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
  void reset(const InputMethodEntry &entry, InputContextEvent &event) override;

  auto *factory() { return &factory_; }
  void reloadConfig() override;

  const Configuration *getConfig() const override { return &config_; }

  auto candidateWinHorizontal() const { return candidateWinHorizontal_; }
  auto enterKeyClosesPrevWin() const { return enterKeyClosesPrevWin_; }

  RitiContext *context(InputContext *ic);

  auto *suggestion() const { return suggestion_.get(); }
  void setSuggestion(Suggestion *suggestion) { suggestion_.reset(suggestion); }

private:
  void populateConfig(const RawConfig &config);

  Instance *instance_;
  MatraKiConfig config_;
  UniqueCPtr<Config, riti_config_free> cfg_;
  UniqueCPtr<RitiContext, riti_context_free> ctx_;
  UniqueCPtr<Suggestion, riti_suggestion_free> suggestion_;
  TrackableObjectReference<InputContext> currentIC_;
  FactoryFor<MatraKiState> factory_;
  std::filesystem::file_time_type lastConfigTimestamp_ =
      std::filesystem::file_time_type::min();
  std::chrono::steady_clock::time_point lastConfigCheckTime_ =
      std::chrono::steady_clock::time_point::min();
  bool candidateWinHorizontal_ = true;
  bool enterKeyClosesPrevWin_ = false;
};

class MatraKiFactory : public AddonFactory {
public:
  AddonInstance *create(AddonManager *manager) override {
    return new MatraKiEngine(manager->instance());
  }
};

} // namespace fcitx

#endif // _FCITX5_MATRAKI_MATRAKI_H_
