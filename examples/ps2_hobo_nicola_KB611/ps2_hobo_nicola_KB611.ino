/**
 * 
	ps2_hobo_nicola.ino Main sketch of "Hobo-nicola PS/2 adapter" using Hobo-nicola keyboard and adapter library.
	Copyright (c) 2018 Takeshi Higasa

	This file is part of "Hobo-nicola keyboard and adapter".

	"Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	"Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
 
  hoboNicolaLibrary 1.7.6   Mar. 8. 2024
*/

/**koseki(2024.3.21)

	設定モードに入るには 右CTRL + MENU(App) + 100ms -> 'S'キー

	● FMV-KB611キーボードの設定モード; *2 + *S
       ^^^^^^^^^^^^^^^^^
	---------------------------------
	|     無変換     |      空白      |	
	---------------------------------
            | 無変換 |  変換  |
	        ------------------

	・親指シフトキーを長押しすると以下のコードを送出する。
	---------------------------------
	|     タブ      |      変換      |	
	---------------------------------

	-------------------------------------------
	********************************************************
	*** CPU Arduino Leonardo (Arduino AVR Boards)        ***
	********************************************************

	\作業ディレクトリ---+---\ps2_hobo_nicola_KB611---+---ps2_hobo_nicola_KB611.ino
					   |							|
					   |							+---ps2_kbd.cpp
					   |							|
					   |							+---ps2_kbd.h
					   |
	                   +---\libraries---+---\Adafruit_TinyUSB_Arduino-3.6.1
	                                    |
	                                    +---\hoboNicolaLibrary_KB611
*/

#include "hobo_nicola.h"
#include "hobo_board_config.h"
#include "hobo_sleep.h"
#include "ps2_kbd.h"

ps2_kbd* ps2;

// Function keys with Fn-key pressed.
static const uint16_t fn_keys[] PROGMEM = {
	HID_S | WITH_R_CTRL,	FN_SETUP_MODE,
	HID_R | WITH_R_CTRL,	FN_MEMORY_READ_MODE,	// read stored settings
	HID_W | WITH_R_CTRL,	FN_MEMORY_WRITE_MODE,	// store current settings

	HID_M,								FN_MEDIA_MUTE,
	HID_COMMA,						FN_MEDIA_VOL_DOWN,
	HID_PERIOD,						FN_MEDIA_VOL_UP,
	HID_U_ARROW,					FN_MEDIA_VOL_UP,
	HID_D_ARROW,					FN_MEDIA_VOL_DOWN,
	HID_R_ARROW,					FN_MEDIA_SCAN_NEXT,
	HID_L_ARROW,					FN_MEDIA_SCAN_PREV,
	HID_ENTER,						FN_MEDIA_PLAY_PAUSE,
	HID_ESCAPE | WITH_R_CTRL,  FN_SYSTEM_SLEEP,   // Ctrl + App + Esc 
	0, 0
};

class ps2_hobo_nicola : public HoboNicola {
public:
	ps2_hobo_nicola() {}
	const uint16_t* fn_keys_table() { return fn_keys; }
	// if you want to change soft-Fn key, un-comment below. 
	//  const uint8_t fn_key() { return HID_APP; }
};

ps2_hobo_nicola hobo_nicola;

class ps2_event : public keyboard_notify {
	public:
		void raw_key_event(uint8_t key, bool released = false) {
			if (is_usb_suspended()) {
				usb_wakeup();
			//**koseki(2024.12.12)
				//ps2->kbd_reset();
				ps2->reset_KB611();
			//**
				return;
			}
		};
		void key_pressed  (uint8_t key, uint8_t mod) { hobo_nicola.key_event(key, true);  };
		void key_released  (uint8_t key, uint8_t mod) { hobo_nicola.key_event(key, false); };
};

void setup() {
	hobo_device_setup(false);
	HoboNicola::init_hobo_nicola(&hobo_nicola);
	ps2 = ps2_kbd::getInstance();
	if (!ps2->begin(new ps2_event, LED_BUILTIN_RX))
		hobo_nicola.error_blink(400);
//**koseki(2024.12.12)
	ps2->kbd_jis109();    //KB611の親指シフト動作を無効にする
	delay(500);
	hobo_nicola.has_dedicated_oyakeys(false);
	//hobo_nicola.has_dedicated_oyakeys(true);        // 専用の親指キーがある(KB611)
	//hobo_nicola.set_oyayubi_keys(HID_F23, HID_F24); // キーコードを変換済、左親指⇒F23、右親指⇒F24
//**
}

void loop() {
	static uint16_t i = 0;

	if (is_usb_suspended() ) {
		ps2->task(0);
		all_led_off();
		//**koseki(2024.12.12)
		//KB611がハングしないように10分毎にリセットしてみる。
		i++;
		if (i == 1200) {
			i = 0;
			ps2->reset_KB611();
		}
		//**
		enter_sleep(500);
	} else {
		ps2->task(hobo_nicola.get_hid_led_state());
		hobo_nicola.idle();
		enter_sleep();
	}
}
