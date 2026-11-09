/*
 *  MatraKi Keyboard
 *  Copyright (C) 2015-2020 Muhammad Mominul Huque <mominul2082@gmail.com>
 *  Copyright (C) 2021 CSSlayer <wengxt@gmail.com>
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

#ifndef KEYCODE_H
#define KEYCODE_H

#include "riti.h"
#include <fcitx-utils/keysym.h>
#include <cstdint>

/* Here we map Fcitx keycodes with riti's using a fast jump table. */
constexpr uint16_t VC_UNKNOWN = 0x0046;

inline uint16_t keySymToRitiKey(FcitxKeySym k) {
  switch (k) {
  // Begin Alphanumeric Zone
  case FcitxKey_grave:
  case FcitxKey_dead_grave:
    return VC_GRAVE;
  case FcitxKey_asciitilde:
    return VC_TILDE;

  case FcitxKey_0: return VC_0;
  case FcitxKey_1: return VC_1;
  case FcitxKey_2: return VC_2;
  case FcitxKey_3: return VC_3;
  case FcitxKey_4: return VC_4;
  case FcitxKey_5: return VC_5;
  case FcitxKey_6: return VC_6;
  case FcitxKey_7: return VC_7;
  case FcitxKey_8: return VC_8;
  case FcitxKey_9: return VC_9;

  case FcitxKey_parenright: return VC_PAREN_RIGHT;
  case FcitxKey_exclam: return VC_EXCLAIM;
  case FcitxKey_at: return VC_AT;
  case FcitxKey_numbersign: return VC_HASH;
  case FcitxKey_dollar: return VC_DOLLAR;
  case FcitxKey_percent: return VC_PERCENT;
  case FcitxKey_asciicircum: return VC_CIRCUM;
  case FcitxKey_ampersand: return VC_AMPERSAND;
  case FcitxKey_asterisk: return VC_ASTERISK;
  case FcitxKey_parenleft: return VC_PAREN_LEFT;

  case FcitxKey_minus: return VC_MINUS;
  case FcitxKey_underscore: return VC_UNDERSCORE;

  case FcitxKey_equal: return VC_EQUALS;
  case FcitxKey_plus: return VC_PLUS;

  case FcitxKey_A: return VC_A_SHIFT;
  case FcitxKey_B: return VC_B_SHIFT;
  case FcitxKey_C: return VC_C_SHIFT;
  case FcitxKey_D: return VC_D_SHIFT;
  case FcitxKey_E: return VC_E_SHIFT;
  case FcitxKey_F: return VC_F_SHIFT;
  case FcitxKey_G: return VC_G_SHIFT;
  case FcitxKey_H: return VC_H_SHIFT;
  case FcitxKey_I: return VC_I_SHIFT;
  case FcitxKey_J: return VC_J_SHIFT;
  case FcitxKey_K: return VC_K_SHIFT;
  case FcitxKey_L: return VC_L_SHIFT;
  case FcitxKey_M: return VC_M_SHIFT;
  case FcitxKey_N: return VC_N_SHIFT;
  case FcitxKey_O: return VC_O_SHIFT;
  case FcitxKey_P: return VC_P_SHIFT;
  case FcitxKey_Q: return VC_Q_SHIFT;
  case FcitxKey_R: return VC_R_SHIFT;
  case FcitxKey_S: return VC_S_SHIFT;
  case FcitxKey_T: return VC_T_SHIFT;
  case FcitxKey_U: return VC_U_SHIFT;
  case FcitxKey_V: return VC_V_SHIFT;
  case FcitxKey_W: return VC_W_SHIFT;
  case FcitxKey_X: return VC_X_SHIFT;
  case FcitxKey_Y: return VC_Y_SHIFT;
  case FcitxKey_Z: return VC_Z_SHIFT;

  case FcitxKey_a: return VC_A;
  case FcitxKey_b: return VC_B;
  case FcitxKey_c: return VC_C;
  case FcitxKey_d: return VC_D;
  case FcitxKey_e: return VC_E;
  case FcitxKey_f: return VC_F;
  case FcitxKey_g: return VC_G;
  case FcitxKey_h: return VC_H;
  case FcitxKey_i: return VC_I;
  case FcitxKey_j: return VC_J;
  case FcitxKey_k: return VC_K;
  case FcitxKey_l: return VC_L;
  case FcitxKey_m: return VC_M;
  case FcitxKey_n: return VC_N;
  case FcitxKey_o: return VC_O;
  case FcitxKey_p: return VC_P;
  case FcitxKey_q: return VC_Q;
  case FcitxKey_r: return VC_R;
  case FcitxKey_s: return VC_S;
  case FcitxKey_t: return VC_T;
  case FcitxKey_u: return VC_U;
  case FcitxKey_v: return VC_V;
  case FcitxKey_w: return VC_W;
  case FcitxKey_x: return VC_X;
  case FcitxKey_y: return VC_Y;
  case FcitxKey_z: return VC_Z;

  case FcitxKey_bracketleft: return VC_BRACKET_LEFT;
  case FcitxKey_braceleft: return VC_BRACE_LEFT;

  case FcitxKey_bracketright: return VC_BRACKET_RIGHT;
  case FcitxKey_braceright: return VC_BRACE_RIGHT;

  case FcitxKey_backslash: return VC_BACK_SLASH;
  case FcitxKey_bar: return VC_BAR;

  case FcitxKey_slash: return VC_SLASH;
  case FcitxKey_question: return VC_QUESTION;

  case FcitxKey_semicolon: return VC_SEMICOLON;
  case FcitxKey_colon: return VC_COLON;

  case FcitxKey_comma: return VC_COMMA;
  case FcitxKey_less: return VC_LESS;

  case FcitxKey_period: return VC_PERIOD;
  case FcitxKey_greater: return VC_GREATER;

  case FcitxKey_apostrophe: return VC_APOSTROPHE;
  case FcitxKey_quotedbl: return VC_QUOTE;

  // Begin Numeric Zone
  case FcitxKey_KP_Divide: return VC_KP_DIVIDE;
  case FcitxKey_KP_Multiply: return VC_KP_MULTIPLY;
  case FcitxKey_KP_Subtract: return VC_KP_SUBTRACT;
  case FcitxKey_KP_Equal: return VC_KP_EQUALS;
  case FcitxKey_KP_Add: return VC_KP_ADD;
  case FcitxKey_KP_Enter: return VC_KP_ENTER;
  case FcitxKey_KP_Decimal: return VC_KP_DECIMAL;

  case FcitxKey_KP_1: return VC_KP_1;
  case FcitxKey_KP_2: return VC_KP_2;
  case FcitxKey_KP_3: return VC_KP_3;
  case FcitxKey_KP_4: return VC_KP_4;
  case FcitxKey_KP_5: return VC_KP_5;
  case FcitxKey_KP_6: return VC_KP_6;
  case FcitxKey_KP_7: return VC_KP_7;
  case FcitxKey_KP_8: return VC_KP_8;
  case FcitxKey_KP_9: return VC_KP_9;
  case FcitxKey_KP_0: return VC_KP_0;
  // End Numeric Zone

  default:
    return VC_UNKNOWN;
  }
}

#endif /* end of include guard: KEYCODE_H */
