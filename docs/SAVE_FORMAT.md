# 保存データ仕様

## カートリッジ

- MBC5 + RAM + Battery
- SRAM 32KB想定

## ヘッダー

Offset 0から16バイトを使用します。

| Offset | 内容 |
| --- | --- |
| 0-3 | Magic `GFMD` |
| 4 | Version |
| 5 | Last Pattern Index |
| 6 | 固定値 `0x55` |
| 7 | Header XOR checksum |
| 8-15 | 予約 |

## パターンスロット

Offset 16以降に32スロットを配置します。

```text
slot_offset = 16 + slot_index * (sizeof(PatternData) + 2)
```

各スロット:

| 内容 | サイズ |
| --- | --- |
| PatternData | コンパイル時の構造体サイズ |
| Checksum | 1 |
| Inverted checksum | 1 |

チェックサム不一致時は該当パターンのみ初期化します。パターン01はデモ、それ以外は初期値パターンになります。
