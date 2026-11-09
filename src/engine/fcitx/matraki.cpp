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
#include "matraki.h"
#include "keycode.h"
#include "riti.h"
#include "smart_features.h"
#include "language_engine.h"
#include <chrono>
#include <cstdint>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/fs.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/misc.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/stringutils.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/addoninstance.h>
#include <fcitx/candidatelist.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/inputpanel.h>
#include <fcitx/text.h>
#include <fcitx/userinterface.h>
#include <fcntl.h>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

FCITX_DEFINE_LOG_CATEGORY(matraki, "matraki");
#define FCITX_MATRAKI_DEBUG() FCITX_LOGC(matraki, Debug)

namespace fcitx {

class MatraKiState : public InputContextProperty {
public:
  MatraKiState(MatraKiEngine *engine, InputContext &ic)
      : engine_(engine), ic_(&ic) {}

  ~MatraKiState() {}

  class MatraKiCandidate : public CandidateWord {
  public:
    MatraKiCandidate(MatraKiEngine *engine, std::string text, int index, std::string directCommit = "")
        : CandidateWord(Text(std::move(text))), engine_(engine), index_(index), directCommit_(std::move(directCommit)) {
    }

    void select(InputContext *inputContext) const override {
      auto *state = inputContext->propertyFor(engine_->factory());
      state->selectCandidate(directCommit_.empty() ? text().toString() : directCommit_, index_);
    }

    int index() const { return index_; }
    const std::string& directCommit() const { return directCommit_; }

  private:
    MatraKiEngine *engine_;
    int index_;
    std::string directCommit_;
  };

  class MatraKiCandidateList : public CommonCandidateList {
  public:
    void setGlobalCursorIndex(int index) {
      int items = this->pageSize();
      int page = index / items;
      setPage(page);
      CommonCandidateList::setGlobalCursorIndex(index);
    }
  };

  void selectCandidate(const std::string &commitText, int index) {
    auto *ctx = engine_->context(ic_);
    auto *suggestion = engine_->suggestion();

    if (index < 0) {
      if (!commitText.empty()) {
        ic_->inputPanel().reset();
        ic_->updatePreedit();
        ic_->commitString(commitText);
        if (commitText == "।" || commitText == "?" || commitText == "!" || commitText == ",") {
          lastCommittedWord_.clear();
        } else {
          lastCommittedWord_ = commitText;
        }
      }
      rawInputBuffer_.clear();
      reset();
      return;
    }

    if (!suggestion) {
      rawInputBuffer_.clear();
      reset();
      return;
    }

    char *txt = riti_suggestion_get_pre_edit_text(suggestion, index);
    if (txt) {
      std::string commitStr(txt);
      riti_string_free(txt);
      if (!commitStr.empty()) {
        ic_->inputPanel().reset();
        ic_->updatePreedit();
        ic_->commitString(commitStr);
        if (commitStr == "।" || commitStr == "?" || commitStr == "!" || commitStr == ",") {
          lastCommittedWord_.clear();
        } else {
          lastCommittedWord_ = commitStr;
        }
        riti_context_candidate_committed(ctx, index);
      }
    }
    rawInputBuffer_.clear();
    reset();
  }

  void updatePreedit() {
    auto candidateList = std::dynamic_pointer_cast<CommonCandidateList>(
        ic_->inputPanel().candidateList());
    if (candidateList && candidateList->size() > 0) {
      auto idx = candidateList->globalCursorIndex();
      if (idx >= 0 && idx < candidateList->totalSize()) {
        const auto *candidate = dynamic_cast<const MatraKiCandidate*>(&candidateList->candidate(idx));
        if (candidate && candidate->index() < 0) {
          std::string text = candidate->directCommit();
          Text preedit(std::move(text));
          preedit.setCursor(preedit.textLength());
          if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
            ic_->inputPanel().setClientPreedit(preedit);
            ic_->updatePreedit();
          }
          return;
        }
      }
    }

    auto *suggestion = engine_->suggestion();
    if (!suggestion) {
      return;
    }

    std::string text;
    auto index = 0;

    if (!riti_suggestion_is_lonely(suggestion)) {
      if (candidateList) {
        auto idx = candidateList->globalCursorIndex();
        if (idx >= 0 && idx < candidateList->totalSize()) {
          const auto *candidate = dynamic_cast<const MatraKiCandidate*>(&candidateList->candidate(idx));
          if (candidate && candidate->index() >= 0) {
            index = candidate->index();
          }
        }
      }
    }

    auto *txt = riti_suggestion_get_pre_edit_text(suggestion, index);
    if (txt) {
      text = txt;
      riti_string_free(txt);
    }

    Text preedit(std::move(text));
    preedit.setCursor(preedit.textLength());
    if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
      ic_->inputPanel().setClientPreedit(preedit);
      ic_->updatePreedit();
    }
  }

  void commit() {
    int index = 0;
    std::string textToCommit;

    auto candidateList = ic_->inputPanel().candidateList();
    if (candidateList && candidateList->size() > 0) {
      auto idx = candidateList->cursorIndex();
      if (idx >= 0 && idx < candidateList->size()) {
        index = idx;
      }
      const auto *candidate = dynamic_cast<const MatraKiCandidate*>(&candidateList->candidate(index));
      if (candidate) {
        index = candidate->index();
        textToCommit = candidate->directCommit();
        if (textToCommit.empty()) {
          textToCommit = candidate->text().toString();
        }
      }
    }

    selectCandidate(textToCommit, index);
  }

  void updateUI() {
    ic_->inputPanel().reset();
    do {
      auto *suggestion = engine_->suggestion();
      if (!suggestion) {
        break;
      }

      auto smartCandidates = SmartFeatures::processInput(rawInputBuffer_);
      bool isExclusive = (!smartCandidates.empty() && smartCandidates[0].isExclusive);

      if (isExclusive) {
        auto candidateList = std::make_unique<MatraKiCandidateList>();
        for (const auto &sc : smartCandidates) {
          candidateList->append<MatraKiCandidate>(engine_, sc.display, -1, sc.commitText);
        }
        candidateList->setLayoutHint(engine_->candidateWinHorizontal()
                                         ? CandidateLayoutHint::Horizontal
                                         : CandidateLayoutHint::Vertical);
        candidateList->setGlobalCursorIndex(0);
        ic_->inputPanel().setCandidateList(std::move(candidateList));
        updatePreedit();
        break;
      }

      if (!riti_suggestion_is_lonely(suggestion) || !smartCandidates.empty()) {
        char *aux = riti_suggestion_get_auxiliary_text(suggestion);
        if (aux && aux[0]) {
          ic_->inputPanel().setAuxUp(Text(std::string(aux)));
        }
        riti_string_free(aux);

        auto candidateList = std::make_unique<MatraKiCandidateList>();
        std::unordered_set<std::string> seenCommits;

        auto len = riti_suggestion_get_length(suggestion);
        std::vector<std::pair<std::string, int>> ritiItems;
        ritiItems.reserve(len);
        for (decltype(len) i = 0; i < len; i++) {
          char *text = riti_suggestion_get_pre_edit_text(suggestion, i);
          std::string s = text ? text : "";
          riti_string_free(text);
          if (!s.empty()) {
            // Apply grammar rule fix if available and not already in list
            std::string gfix = LanguageEngine::getGrammarRuleFix(s);
            if (!gfix.empty() && seenCommits.insert(gfix).second) {
              candidateList->append<MatraKiCandidate>(engine_, gfix + " ✓", -1, gfix);
            }
            ritiItems.push_back({s, static_cast<int>(i)});
          }
        }

        // Apply subject-verb concordance ranking if previous word was a subject pronoun
        Person subjPerson = LanguageEngine::getSubjectPerson(lastCommittedWord_);
        if (subjPerson != Person::None && ritiItems.size() > 1) {
          int bestScore = -999;
          size_t bestIdx = 0;
          for (size_t i = 0; i < ritiItems.size(); ++i) {
            int score = LanguageEngine::getVerbConcordanceScore(subjPerson, ritiItems[i].first);
            if (score > bestScore) {
              bestScore = score;
              bestIdx = i;
            }
          }
          if (bestScore > 0 && bestIdx > 0) {
            auto bestItem = ritiItems[bestIdx];
            ritiItems.erase(ritiItems.begin() + bestIdx);
            ritiItems.insert(ritiItems.begin(), bestItem);
          }
        }

        // Prepend smart candidates (only if not already identical to the top riti candidate)
        for (const auto &sc : smartCandidates) {
          if (!ritiItems.empty() && ritiItems[0].first == sc.commitText) {
            continue;
          }
          if (seenCommits.insert(sc.commitText).second) {
            candidateList->append<MatraKiCandidate>(engine_, sc.display, -1, sc.commitText);
          }
        }

        int prevRitiIdx = static_cast<int>(riti_suggestion_previously_selected_index(suggestion));
        int targetCursorIdx = 0;
        for (const auto &ri : ritiItems) {
          if (seenCommits.insert(ri.first).second) {
            if (ri.second == prevRitiIdx && smartCandidates.empty()) {
              targetCursorIdx = candidateList->totalSize();
            }
            candidateList->append<MatraKiCandidate>(engine_, ri.first, ri.second);
          }
        }

        candidateList->setLayoutHint(engine_->candidateWinHorizontal()
                                         ? CandidateLayoutHint::Horizontal
                                         : CandidateLayoutHint::Vertical);
        if (targetCursorIdx < 0 || targetCursorIdx >= candidateList->totalSize()) {
          targetCursorIdx = 0;
        }
        candidateList->setGlobalCursorIndex(targetCursorIdx);
        ic_->inputPanel().setCandidateList(std::move(candidateList));
      }
      updatePreedit();
    } while (0);
    ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
  }

  void reset() {
    altGrPressed_ = false;
    rawInputBuffer_.clear();
    engine_->setSuggestion(nullptr);
    auto *ctx = engine_->context(ic_);
    if (riti_context_ongoing_input_session(ctx)) {
      riti_context_finish_input_session(ctx);
    }
    ic_->inputPanel().reset();
    ic_->updatePreedit();
    ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
  }

  void keyEvent(KeyEvent &keyEvent) {
    auto key = keyEvent.rawKey();
    if (keyEvent.isRelease()) {
      if (key.sym() == FcitxKey_Alt_R ||
          key.sym() == FcitxKey_ISO_Level3_Shift) {
        altGrPressed_ = false;
      }
      return;
    }

    auto *ctx = engine_->context(ic_);
    if (!riti_context_ongoing_input_session(ctx)) {
      engine_->reloadConfig();
    }
    auto candidateList = ic_->inputPanel().candidateList();
    // At first, handle the special keys.
    switch (key.sym()) {
    case FcitxKey_Escape:
      lastCharWasSpace_ = false;
      rawInputBuffer_.clear();
      if (riti_context_ongoing_input_session(ctx) || (candidateList && candidateList->size() > 0)) {
        reset();
        keyEvent.filterAndAccept();
        return;
      }
      return;
    case FcitxKey_BackSpace:
      lastCharWasSpace_ = false;
      if (riti_context_ongoing_input_session(ctx)) {
        auto ctrlKey = key.states().test(KeyState::Ctrl);
        if (ctrlKey) {
          rawInputBuffer_.clear();
        } else if (!rawInputBuffer_.empty()) {
          rawInputBuffer_.pop_back();
        }
        engine_->setSuggestion(riti_context_backspace_event(ctx, ctrlKey));

        if (engine_->suggestion() &&
            !riti_suggestion_is_empty(engine_->suggestion())) {
          updateUI();
        } else {
          reset();
        }

        keyEvent.filterAndAccept();
        return;
      }
      reset();
      return;
    case FcitxKey_Return:
      lastCharWasSpace_ = false;
      if (riti_context_ongoing_input_session(ctx)) {
        commit();
        if (engine_->enterKeyClosesPrevWin()) {
          keyEvent.filterAndAccept();
        }
      } else if (candidateList && candidateList->size() > 0) {
        reset();
      }
      return;
    case FcitxKey_space:
    case FcitxKey_KP_Enter:
      if (riti_context_ongoing_input_session(ctx)) {
        commit();
        lastCharWasSpace_ = true;
        lastSpaceTime_ = std::chrono::steady_clock::now();

        // Next Word Prediction
        if (!lastCommittedWord_.empty()) {
          auto predictions = LanguageEngine::getNextWordPredictions(lastCommittedWord_);
          if (!predictions.empty()) {
            auto candidateList = std::make_unique<MatraKiCandidateList>();
            for (const auto &pred : predictions) {
              candidateList->append<MatraKiCandidate>(engine_, pred + " ➜", -1, pred);
            }
            candidateList->setLayoutHint(engine_->candidateWinHorizontal()
                                             ? CandidateLayoutHint::Horizontal
                                             : CandidateLayoutHint::Vertical);
            candidateList->setGlobalCursorIndex(0);
            ic_->inputPanel().setCandidateList(std::move(candidateList));
            ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
          }
        }
      } else {
        if (candidateList && candidateList->size() > 0) {
          reset();
        }
        auto now = std::chrono::steady_clock::now();
        if (key.sym() == FcitxKey_space && lastCharWasSpace_ &&
            (now - lastSpaceTime_) < std::chrono::milliseconds(800)) {
          // Double space -> Bengali Dari
          if (ic_->capabilityFlags().test(CapabilityFlag::SurroundingText)) {
            ic_->deleteSurroundingText(-1, 1);
          } else {
            ic_->forwardKey(Key(FcitxKey_BackSpace));
          }
          ic_->commitString("। ");
          lastCharWasSpace_ = false;
          keyEvent.filterAndAccept();
          return;
        } else {
          lastCharWasSpace_ = (key.sym() == FcitxKey_space);
          lastSpaceTime_ = now;
        }
      }
      return;
    case FcitxKey_Page_Up:
    case FcitxKey_KP_Page_Up:
      lastCharWasSpace_ = false;
      if (riti_context_ongoing_input_session(ctx)) {
        if (candidateList && candidateList->toPageable() &&
            candidateList->toPageable()->hasPrev()) {
          candidateList->toPageable()->prev();
          updatePreedit();
          ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
          keyEvent.filterAndAccept();
          return;
        }
      }
      return;
    case FcitxKey_Page_Down:
    case FcitxKey_KP_Page_Down:
      lastCharWasSpace_ = false;
      if (riti_context_ongoing_input_session(ctx)) {
        if (candidateList && candidateList->toPageable() &&
            candidateList->toPageable()->hasNext()) {
          candidateList->toPageable()->next();
          updatePreedit();
          ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
          keyEvent.filterAndAccept();
          return;
        }
      }
      return;
    /** Arrow and Tab keys.
     * We use the arrow keys and the tab key to move the selection
     * in the preview window. So we have to ensure the preview
     * window is shown by checking if the current suggestion is
     * not a lonely one. Otherwise we don't handle it.
     **/
    case FcitxKey_Right:
    case FcitxKey_Left:
    case FcitxKey_Up:
    case FcitxKey_Down:
    case FcitxKey_Tab:
    case FcitxKey_ISO_Left_Tab:
      lastCharWasSpace_ = false;
      if (!riti_context_ongoing_input_session(ctx)) {
        // When Next Word Prediction popup is active after Space:
        // Tab commits the selected prediction; Shift+Tab cycles; Arrow keys dismiss popup and move cursor naturally
        if (candidateList && candidateList->size() > 0) {
          if (key.sym() == FcitxKey_Tab) {
            commit();
            keyEvent.filterAndAccept();
            return;
          } else if (key.sym() == FcitxKey_ISO_Left_Tab && candidateList->toCursorMovable()) {
            candidateList->toCursorMovable()->nextCandidate();
            ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
            keyEvent.filterAndAccept();
            return;
          }
          reset();
        }
        return;
      }
      if (candidateList && candidateList->toCursorMovable() && candidateList->size() > 1) {
        if (key.sym() == FcitxKey_Right || key.sym() == FcitxKey_Tab ||
            key.sym() == FcitxKey_Down) {
          candidateList->toCursorMovable()->nextCandidate();
        } else {
          candidateList->toCursorMovable()->prevCandidate();
        }
        updatePreedit();

        ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
        keyEvent.filterAndAccept();
        return;
      } else if (riti_context_ongoing_input_session(ctx)) {
        commit();
      }
      return;
    /** Modifier keys **/
    case FcitxKey_Alt_R:
    case FcitxKey_ISO_Level3_Shift:
      // Keep track of the right Alt key (also known as the AltGr key)
      altGrPressed_ = true;
      [[fallthrough]];
    case FcitxKey_Shift_L:
    case FcitxKey_Shift_R:
    case FcitxKey_Control_L:
    case FcitxKey_Control_R:
    case FcitxKey_Alt_L:
    case FcitxKey_Meta_L:
    case FcitxKey_Meta_R:
      if (riti_context_ongoing_input_session(ctx)) {
        keyEvent.filterAndAccept();
      }
      return;
    default:
      break;
    }

    lastCharWasSpace_ = false;

    uint8_t modifier = 0;
    // Set modifiers
    if (key.states().test(KeyState::Shift)) {
      modifier |= MODIFIER_SHIFT;
    }

    auto ctrlKey = key.states().test(KeyState::Ctrl);
    auto altKey = key.states().test(KeyState::Alt);

    // Convert the key value into riti's key value.
    auto ritiKey = keySymToRitiKey(key.sym());

    // Reject the key which has only Ctrl or Alt (not the right one) combination
    // and riti doesn't handle.
    if ((ctrlKey && !altKey) || (!ctrlKey && altKey && !altGrPressed_) ||
        ritiKey == VC_UNKNOWN) {
      if (riti_context_ongoing_input_session(ctx)) {
        commit();
      }
      return;
    }

    // Direct candidate selection with digits 1-9 for exclusive modes (!snippets, :emojis)
    if (!rawInputBuffer_.empty() && (rawInputBuffer_[0] == '!' || rawInputBuffer_[0] == ':')) {
      if (key.sym() >= FcitxKey_1 && key.sym() <= FcitxKey_9 && !ctrlKey && !altKey) {
        int targetIdx = key.sym() - FcitxKey_1;
        auto cl = ic_->inputPanel().candidateList();
        if (cl && targetIdx < cl->size()) {
          const auto *candidate = dynamic_cast<const MatraKiCandidate*>(&cl->candidate(targetIdx));
          if (candidate) {
            std::string toCommit = candidate->directCommit().empty() ? candidate->text().toString() : candidate->directCommit();
            selectCandidate(toCommit, -1);
            keyEvent.filterAndAccept();
            return;
          }
        }
      }
    }

    // If we have Ctrl and Alt combination or the right Alt, set it as the AltGr
    // modifier.
    if ((ctrlKey && altKey) || altGrPressed_) {
      modifier |= MODIFIER_ALT_GR;
    }

    if (!riti_context_ongoing_input_session(ctx)) {
      rawInputBuffer_.clear();
    }
    if (key.sym() >= 0x20 && key.sym() <= 0x7E) {
      rawInputBuffer_ += static_cast<char>(key.sym());
    }

    auto index = 0;

    if (riti_context_ongoing_input_session(ctx) && engine_->suggestion() &&
        !riti_suggestion_is_lonely(engine_->suggestion())) {
      auto candidateList = std::dynamic_pointer_cast<CommonCandidateList>(
          ic_->inputPanel().candidateList());
      if (candidateList) {
        auto idx = candidateList->globalCursorIndex();
        if (idx >= 0 && idx < candidateList->totalSize()) {
          const auto *candidate = dynamic_cast<const MatraKiCandidate*>(&candidateList->candidate(idx));
          if (candidate && candidate->index() >= 0) {
            index = candidate->index();
          }
        }
      }
    }

    engine_->setSuggestion(
        riti_get_suggestion_for_key(ctx, ritiKey, modifier, index));

    if (engine_->suggestion() &&
        !riti_suggestion_is_empty(engine_->suggestion())) {
      updateUI();
      keyEvent.filterAndAccept();
      return;
    }

    // Corner case: When old style kar typing is enabled, a lonely suggestion
    // and an empty suggestion is not distinguishable. So we check if a input
    // session is ongoing to accept the key event.
    if (riti_context_ongoing_input_session(ctx)) {
      keyEvent.filterAndAccept();
    }
  }

private:
  MatraKiEngine *engine_;
  InputContext *ic_;
  /* Unfortunately, we have to keep track of the right Alt Key. */
  bool altGrPressed_ = false;
  std::string rawInputBuffer_;
  bool lastCharWasSpace_ = false;
  std::chrono::steady_clock::time_point lastSpaceTime_;
  std::string lastCommittedWord_;
};

MatraKiEngine::MatraKiEngine(Instance *instance)
    : instance_(instance), cfg_(riti_config_new()),
      factory_(
          [this](InputContext &ic) { return new MatraKiState(this, ic); }) {
  if (!fs::makePath(stringutils::joinPath(
          StandardPath::global().userDirectory(StandardPath::Type::Data),
          "matraki-keyboard"))) {
    throw std::runtime_error("Failed to create user directory");
  }
  reloadConfig();
  instance->inputContextManager().registerProperty("matrakiState",
                                                   &factory_);
}

MatraKiEngine::~MatraKiEngine() {}

void MatraKiEngine::activate(const InputMethodEntry & /*entry*/,
                             InputContextEvent & /*event*/) {
  reloadConfig();
}

void MatraKiEngine::keyEvent(const InputMethodEntry & /*entry*/,
                             KeyEvent &keyEvent) {
  auto *ic = keyEvent.inputContext();
  auto *state = ic->propertyFor(&factory_);
  state->keyEvent(keyEvent);
}

void MatraKiEngine::reset(const InputMethodEntry & /*entry*/,
                          InputContextEvent &event) {
  auto *state = event.inputContext()->propertyFor(&factory_);
  state->reset();
}

RitiContext *MatraKiEngine::context(InputContext *ic) {
  // Check if the last input context is same as the current one.
  // Since RitiContext is shared, reset any existing state if needed.
  if (!currentIC_.isValid()) {
    currentIC_ = ic->watch();
  } else if (ic != currentIC_.get()) {
    suggestion_.reset();
    if (ctx_ && riti_context_ongoing_input_session(ctx_.get())) {
      riti_context_finish_input_session(ctx_.get());
    }
    auto *oldIC = currentIC_.get();
    if (oldIC) {
      oldIC->inputPanel().reset();
      oldIC->updatePreedit();
      oldIC->updateUserInterface(UserInterfaceComponent::InputPanel);
    }
    currentIC_ = ic->watch();
  }
  return ctx_.get();
}

bool booleanValue(const RawConfig &config, const std::string &path,
                  bool defaultValue) {
  bool value = defaultValue;
  if (const auto *option = config.valueByPath(path)) {
    value = *option == "true";
  }
  return value;
}

void MatraKiEngine::populateConfig(const RawConfig &config) {
  // Keep sync with Settings.cpp
  std::string layoutPath = "avro_phonetic";
  if (const auto *path = config.valueByPath("layout/path")) {
    layoutPath = *path;
    if (layoutPath.find("avrophonetic") != std::string::npos ||
        layoutPath.find("avro_phonetic") != std::string::npos) {
      layoutPath = "avro_phonetic";
    }
  }
  const bool showCWPhonetic =
      booleanValue(config, "settings/CandidateWin\\Phonetic", true);
  const bool includeEnglish =
      booleanValue(config, "settings/PreviewWin\\IncludeEnglish", true);
  const bool showPrevWinFixed =
      booleanValue(config, "settings/FixedLayout\\ShowPrevWin", true);
  const bool autoVowelFormFixed =
      booleanValue(config, "settings/FixedLayout\\AutoVowelForm", true);
  const bool autoChandraPosFixed =
      booleanValue(config, "settings/FixedLayout\\AutoChandraPos", true);
  const bool traditionalKarFixed =
      booleanValue(config, "settings/FixedLayout\\TraditionalKar", false);
  const bool oldKarOrder =
      booleanValue(config, "settings/FixedLayout\\OldKarOrder", false);
  const bool oldReph =
      booleanValue(config, "settings/FixedLayout\\OldReph", true);
  const bool numberPadFixed =
      booleanValue(config, "settings/FixedLayout\\NumberPad", true);
  const bool ansiOutput = booleanValue(config, "settings/ANSI", false);
  const bool smartQuoting = booleanValue(config, "settings/SmartQuoting", true);

  if (!riti_config_set_layout_file(cfg_.get(), layoutPath.data())) {
    FCITX_MATRAKI_DEBUG() << "Failed to set layout file: " << layoutPath;
  }

  riti_config_set_suggestion_include_english(cfg_.get(), includeEnglish);
  riti_config_set_phonetic_suggestion(cfg_.get(), showCWPhonetic);

  if (!riti_config_set_database_dir(cfg_.get(), PROJECT_DATADIR "/data")) {
    FCITX_MATRAKI_DEBUG()
        << "Failed to set database directory: " << PROJECT_DATADIR << "/data";
  }

  riti_config_set_fixed_suggestion(cfg_.get(), showPrevWinFixed);
  riti_config_set_fixed_auto_vowel(cfg_.get(), autoVowelFormFixed);
  riti_config_set_fixed_auto_chandra(cfg_.get(), autoChandraPosFixed);
  riti_config_set_fixed_traditional_kar(cfg_.get(), traditionalKarFixed);
  riti_config_set_fixed_old_kar_order(cfg_.get(), oldKarOrder);
  riti_config_set_fixed_old_reph(cfg_.get(), oldReph);
  riti_config_set_fixed_numpad(cfg_.get(), numberPadFixed);
  riti_config_set_ansi_encoding(cfg_.get(), ansiOutput);
  riti_config_set_smart_quote(cfg_.get(), smartQuoting);

  candidateWinHorizontal_ =
      booleanValue(config, "settings/CandidateWin\\Horizontal", true);
  enterKeyClosesPrevWin_ =
      booleanValue(config, "settings/EnterKeyClosesPrevWin", false);
}

void MatraKiEngine::reloadConfig() {
  auto now = std::chrono::steady_clock::now();
  if (ctx_ && (now - lastConfigCheckTime_) < std::chrono::seconds(3)) {
    return;
  }
  lastConfigCheckTime_ = now;
  auto path = StandardPath::global().locate(StandardPath::Type::Config,
                                            "MatraKi/Keyboard.conf");
  auto time = std::filesystem::file_time_type::min();
  if (!path.empty()) {
    time = std::filesystem::last_write_time(path);
  }
  if (time <= lastConfigTimestamp_) {
    return;
  }

  RawConfig config;
  auto configFile = StandardPath::global().open(
      StandardPath::Type::Config, "MatraKi/Keyboard.conf", O_RDONLY);
  if (configFile.isValid()) {
    FCITX_MATRAKI_DEBUG() << "Reload matraki configuration";
    lastConfigTimestamp_ = time;
    readFromIni(config, configFile.fd());
  }
  populateConfig(config);
  suggestion_.reset();
  if (ctx_) {
    riti_context_update_engine(ctx_.get(), cfg_.get());
  } else {
    ctx_.reset(riti_context_new_with_config(cfg_.get()));
  }
}

FCITX_ADDON_FACTORY(MatraKiFactory);
} // namespace fcitx
