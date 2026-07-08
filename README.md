# M5StickC Plus2 Pomodoro

M5StickC Plus2用のポモドーロタイマーです。

半ニートになってから集中して物事に取り組むために作りました。
作業中と休憩中を色付きゲージで表示して直観的に確認できるようにしています。

## Hardware

- M5StickC Plus2

## Environment

- VSCode
- PlatformIO

## UI Concept

色とゲージで状況を確認できます。

| Color | Meaning |
|---|---|
| Blue | 準備完了/休憩終わり |
| Orange | 作業 |
| Red | 作業中断 |
| Green | 休憩 |

## Controls

| Button | Action |
|---|---|
| A | スタート、ストップ、選択 |
| B | メニューを開く、次の項目 |

## Menu

Bボタンでメニューを起動して選択、Aボタンで切り替えです。
なにも押さず3秒放置すればメニューが閉じます。

メニューには3つの項目があり、モード、プリセット、サウンドの設定が切替可能です。

- MODE
  - Switch between WORK and BREAK
- PRESET
  - Rotate timer presets
- SOUND
  - Toggle sound ON/OFF


## Notes
This project is a beginner-friendly embedded development experiment using M5StickC Plus2.

The UI and behavior are intentionally simple so that the device can be used like a small desk companion.

## License
MIT