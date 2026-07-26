# ビルド方法

## 必要環境

- GBDK-2020
- Windowsでは`build.bat`
- macOS/Linuxでは`make`

`GBDK_HOME`にGBDK-2020のインストール先を設定してください。

## Windows

```bat
set GBDK_HOME=C:\gbdk
build.bat
```

## macOS / Linux

```sh
export GBDK_HOME=/opt/gbdk
make
```

## 出力

```text
build/fm_drum_tracker.gb
```

## クリーン

```sh
make clean
```
