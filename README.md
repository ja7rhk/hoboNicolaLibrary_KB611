# hoboNicola library for FMV-KB611

![](assets/images/PS2_adapter.jpg)

本ライブラリはオリジナルのhoboNicola libraryを特定のキーボード向けアダプター用に一部改変したものです。Arduinoのスケッチ部分は \hoboNicolaLibrary_KB611\examples にあります。

アダプターやキーボードの [【実際の写真】](./assets/hobonicola_gallery.md)

## 富士通FMV-KB611
	● 対象アダプター
	ps2_hobo_nicola_KB611  : SparkFun Pro Micro相当品(+5V, 16MHz版)

	設定モードに入るには 右CTRL + MENU(App) + 100ms -> 'S'キー
	● FMV-KB611キーボードの設定モード; *3 + *6

	---------------------------------
	|      F23      |      F24      |	親指シフトキーは内部で使うだけ。
	---------------------------------
            | 無変換 |  変換  |
	        ------------------

	********************************************************
	*** CPU Arduino Leonardo (Arduino AVR Boards)        ***
	********************************************************

	\作業ディレクトリ---+---\ps2_hobo_nicola_KB611---+---ps2_hobo_nicola_KB611.ino
	                   |                            |
	                   |                            +---ps2_kbd.cpp
	                   |                            |
	                   |                            +---ps2_kbd.h
	                   |
	                   +---\libralies---+---\Adafruit_TinyUSB_Library
	                                    |
	                                    +---\hoboNicolaLibrary_KB611

## observe_imeとMS-IMEの設定

アダプターを使用する場合、observe_imeを利用すると所謂モードずれがないので便利です。FMV-KB611は10キーがあるのでScrLockでモード切替えをします。

![](assets/images/observe_ime.png)

以下はMS-IMEの設定例です(以前のバージョンを使用)。

![](assets/images/ms_ime1.png)

カナ変換はF7キーで出来ますが、カタカナ/ひらがなキーにカタカナ変換を割り当てることも出来ます。

![](assets/images/ms_ime2.png)

## 情報元
* http://usa-tarou.la.coocan.jp/oyayubi/adapter_KB611.html ホームページの解説記事。 
* https://github.com/okiraku-camera/hoboNicolaLibrary オリジナルのhoboNicola library。 
* https://github.com/okiraku-camera/observe_ime hoboNicolaで使用しているobserve_imeの情報です。

