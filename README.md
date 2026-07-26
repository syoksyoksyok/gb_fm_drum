# GB FM Drum Tracker

初代ゲームボーイ互換モード向けの、2ボイス擬似FMドラム音源付き16ステップトラッカーです。Pulse Channel 1を左、Pulse Channel 2を右へハードパンし、左右独立のパターン長・再生方向・ステップパラメータを編集できます。

```text
P01 120B L16F R16F R050S
 TRG L TRG R TRG
  01 [01 ]  00
  02  00    00
LR03  00   [01]
...
```

## 対応機種

- Game Boy DMG / Pocket / Light
- Game Boy Color、Game Boy Advance、Game Boy Advance SPのGB互換モード

カラー専用機能は使用していません。

## 主な機能

- 16ステップ、左右独立2トラック
- 各ステップに13種類のパラメータを保持
- BPM 40-300
- Forward / Reverse / Pendulum / Random
- 32パターンをSRAMへ保存
- 音楽的ランダマイズ、完全ランダマイズ、1回Undo
- パターン01にデモパターンを自動作成

## 操作

詳しくは[docs/MANUAL_ja.md](docs/MANUAL_ja.md)を参照してください。

- START: 再生/停止
- 上下: ステップ移動
- 左右: L/R切替
- A+方向: 値編集
- B: 現在セルのTrigger切替
- SELECT+左右: 表示パラメータ切替
- SELECT+上: ヘッダー編集
- SELECT+A: 音楽的ランダマイズ
- SELECT+A長押し: 完全ランダマイズ
- SELECT+B: Undo
- SELECT+START: SRAM保存

## ビルド

GBDK-2020をインストールし、`GBDK_HOME`を設定してください。

Windows:

```bat
build.bat
```

macOS / Linux:

```sh
make
```

生成ROM:

```text
build/fm_drum_tracker.gb
```

## SRAM保存

カートリッジタイプはMBC5 + RAM + Battery、SRAMは32KB想定です。保存領域にはマジック、バージョン、簡易チェックサムを持たせています。破損検出時は該当パターンを初期化し、起動不能にならないようにしています。

## 擬似FM方式

ゲームボーイAPUには真のFM合成はありません。本ソフトのFMは、Pulse Channelの周波数をソフトウェアで周期的に揺らし、ピッチエンベロープと音量エンベロープを組み合わせる擬似2オペレーターFM風ドラムです。真の位相変調・周波数変調シンセサイザーではありません。

## 制約

- 画面はGBDK標準コンソールフォントを使います。
- UIは軽量化のため1フレーム単位の再描画を行いますが、音源・シーケンサー更新を先に処理します。
- Amp AttackはPulse音量レジスタ更新による段階的な疑似Attackです。
- 実機でのパン分離、クリックノイズ量、長時間テンポ精度は最終確認が必要です。

## エミュレーター確認

BGB、SameBoy、Emuliciousなどで`build/fm_drum_tracker.gb`を開きます。SRAM対応を有効にして、保存後に再起動してロードできることを確認してください。

## 実機確認

EverDrive GB、GBxCart RW、FlashGBX対応フラッシュカートなどにROMを書き込みます。MBC5+RAM+Batteryに対応したカートでSRAM保持を確認してください。

## ライセンス

MIT License。詳細は[LICENSE](LICENSE)を参照してください。
