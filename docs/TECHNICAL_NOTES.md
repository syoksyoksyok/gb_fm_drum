# 技術メモ

## 音源

左はPulse Channel 1、右はPulse Channel 2を使用します。NR51はChannel 1を左、Channel 2を右へ割り当てる設定です。Wave ChannelとNoise Channelは使用していません。

擬似FMはフレームごとの周波数更新で行います。Carrier PitchはLUTでAPU周波数値に変換し、Modulator Ratioで進む簡易位相、FM Depth、Pitch Envelope Amount/Decayを加算して周波数を揺らします。

## タイミング

VBlank 60Hz基準です。1ステップは16分音符です。毎フレーム`BPM * 4`を加算し、3600以上になった時点で1ステップ進める誤差蓄積方式を使います。浮動小数点は使用していません。

## 安全性

- Pattern Lengthは1-16へクランプ
- BPMは40-300へクランプ
- Directionは4種類へクランプ
- Pitch/FrequencyはAPU有効範囲へクランプ
- PendulumはLength 1/2を個別に破綻しないよう処理

## 簡略化

テンポはVBlank基準のため、エミュレーターや実機のフレーム周期に依存します。オーディオ専用タイマー割り込み方式ではありません。
