/**
 * 
	nicola_state.cpp   NICOLA-state-machine implements simultaneous typing method of "Hobo-nicola keyboard and adapter library".
	Copyright (c) 2018 Takeshi Higasa, okiraku-camera.tokyo

	This file is part of "Hobo-nicola keyboard and adapter library".

	"Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by the Free Software Foundation, 
	either version 3 of the License, or (at your option) any later version.

	"Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

		Included in hoboNicola 1.5.0.		July. 31. 2021.
			based on nicola_keyboard.cpp
		hoboNicola 1.7.0. March.3. 2023
			Separating nicola tables to nicola_table.cpp
*/

//**koseki(2024.12.12)
//	- deleted the Release_Wait_State and Repeat_State.
//	- added output_oya_long_press() for long time pressing oyayubi.
//  - please see <https://note.com/ja7rhk/n/n38d0e93c19ad?magazine_key=me7734127c997>
//**

#include "hobo_nicola.h"
#include "hobo_settings.h"

void HoboNicola::timer_tick(unsigned long now) {
	if (event_time != 0 && (now > event_time)) {
		event_time = 0;
		nicola_state(Time_out, 0);
	}
}

void HoboNicola::moji_set(uint16_t param) {
	moji = param;
	moji_time = millis();
	event_time = moji_time + e_charTime;
}

void HoboNicola::oyayubi_set(uint16_t param) {
	oyayubi = param;
	oyayubi_time = millis();
	event_time = oyayubi_time + e_oyaTime;
}

void HoboNicola::nicola_state(nicola_event_t e, uint16_t param) {
	unsigned long now = millis();
	unsigned long t1 = 0;
	unsigned long t2 = 0;

	switch(state) {
		case S1_Initial_State:
			event_time = moji_time = oyayubi_time = 0;
			oyayubi = moji = 0;
			switch(e) {
				case Moji_pressed:
					moji_set(param);
					state = S2_Character_State;
					break;
				case Oyayubi_pressed:
					oyayubi_set(param);
					state = S3_Oyayubi_State;
					break;
				default:
					break;
			}
			break;
		case S2_Character_State:
			switch(e) {
			case Init:
				state = S1_Initial_State;
				break;
			case All_off:
				output();
				state = S1_Initial_State;
				break;
			case Moji_pressed:
				output();
				moji_set(param);
				break;
			case Oyayubi_pressed:
				oyayubi_set(param);
				state = S4_Char_Oya_State;
				break;
			case Key_released:
				if (param == moji) {	// 関係のない文字のオフは無視する。
					output();
					state = S1_Initial_State;
				}
				break;
			case Time_out:
				output();
				state = S1_Initial_State;
				break;
			default:
				break;			
			}
			break;
		case S3_Oyayubi_State:
			switch(e) {
			case Init:
				state = S1_Initial_State;
				break;
			case All_off:
				output();
				state = S1_Initial_State;
				break;
			case Moji_pressed:
				moji_set(param);
				state = S5_Oya_Char_State;
				break;
			case Oyayubi_pressed:
				output();
				oyayubi_set(param);
				break;
			case Key_released:
				if (param == oyayubi) {
					output();
					state = S1_Initial_State;
				}
				break;
			case Time_out:
				if (!dedicated_oyakeys) {
					output_oya_long_press();
					state = S1_Initial_State;
				}
				break;
			default:
				break;			
			}
			break;
		case S4_Char_Oya_State:
			switch(e) {
			case Init:
				state = S1_Initial_State;
				break;
			case All_off:
				output();
				state = S1_Initial_State;
				break;
			case Oyayubi_pressed:
				output();
				oyayubi_set(param);
				state = S3_Oyayubi_State;
				break;
			case Moji_pressed:
				// M ON --> O ON --> M OFF
				t1 = oyayubi_time - moji_time;
				t2 = now - oyayubi_time;
				if (t1 < t2) {
					output();   // 先行の文字と親指
					moji_set(param);
					state = S2_Character_State;
				} else {
					uint16_t oya = oyayubi;
					oyayubi = 0;
					output();   // 先行の文字だけ出す
					oyayubi = oya;
					moji_set(param);
					state = S5_Oya_Char_State;
				}
				break;
			case Key_released:  // 文字キーが離されたタイミングに応じて単独打鍵とする。
				// M ON --> O ON --> M OFF
				t1 = oyayubi_time - moji_time;
				t2 = now - oyayubi_time;
				if ((moji == param) && (t2 < e_nicolaTime) && (t1 >= t2)) {
					uint16_t oya = oyayubi;
					oyayubi = 0;
					output();	// 先行の文字だけ出す
					moji = 0;
					oyayubi = oya;
					state = S3_Oyayubi_State;
					break;
				}
				output();
				state = S1_Initial_State;
				break;
			case Time_out:
				output();
				state = S1_Initial_State;
				break;
			default:
				break;		
			}
			break;
		case S5_Oya_Char_State:
			switch(e) {
				case Init:
					state = S1_Initial_State;
					break;
				case All_off:
					output();
					state = S1_Initial_State;
					break;
				case Moji_pressed:
					output();   	// 先行の文字と親指
					moji_set(param);
					state = S2_Character_State;
					break;
				case Oyayubi_pressed:
					t1 = moji_time - oyayubi_time;
					t2 = now - moji_time;
					if (t1 < t2) {
						output();   // 先行の親指と文字
						oyayubi_set(param);
						state = S3_Oyayubi_State;
					} else {
						uint16_t mo = moji;
						moji = 0;
						output();   // 先行の親指だけ出す
						moji = mo;
						oyayubi_set(param);
						state = S4_Char_Oya_State;
					}
					break;
				case Key_released:  // 文字キーが離されたタイミングに応じて単独打鍵とする。
					// O ON --> M ON --> O OFF
					t1 = moji_time - oyayubi_time;
					t2 = now - moji_time;
					if ((oyayubi == param) && (t2 < e_nicolaTime) && (t1 >= t2)) {
					// O ON --> M ON --> O OFF (O is output, but M is still open to combo)
						uint16_t mo = moji;
						moji = 0;
						output();   // 先行の親指だけ出す  
						moji = mo;
						state = S2_Character_State;
						break;
					}
					output();
					state = S1_Initial_State;
					break;
				case Time_out:
					output();
					state = S1_Initial_State;
					break;
				default:
					break;		
			}
			break;
		default:
			state = S1_Initial_State;
			break;
	}
}

#define isShiftPressed() (bool) ((modifiers & HID_SHIFT_MASK) != 0)

void HoboNicola::output() {
	if (moji == 0 && oyayubi) {
		stroke(HIGHBYTE(oyayubi), modifiers);
		oyayubi = 0;
		return;
	}
	if (setup_mode) {
		setup_options(LOWBYTE(moji));
		moji = 0;
		oyayubi = 0;
		return;
	}

	if (isShiftPressed() && oyayubi == 0)
		oyayubi = NID_SHIFT_KEY;

	const uint8_t* p = get_output_data(moji, oyayubi);
	oyayubi = 0;
	moji = 0;
	if (p) {
		send_PGM_string(p);
	}
}

//**koseki(2024.12.12)
void HoboNicola::output_oya_long_press() {
	// USキーボードのときだけ親指キーの長押しが有効
	//if (_US_LAYOUT(global_setting)) {
		if(LOWBYTE(oyayubi) == NID_LEFT_OYAYUBI) {
			stroke(HID_TAB, modifiers);         // タイムアウト時はTAB(変換候補選択)キー
		}
		else if(LOWBYTE(oyayubi) == NID_RIGHT_OYAYUBI) {
			stroke(HID_F15, modifiers);         // タイムアウト時はF15(変換)キー
		}
	//}
	moji = 0;
	oyayubi = 0;
}
//**
