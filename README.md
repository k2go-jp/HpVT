# HpVT (High-performance Video Transmission)
HpVTは、カメラから映像を取込み、Raspberry Piのハードウェアエンコーディング処理（H.264圧縮変換処理）、画像解析処理用の画像変換処理、メタ情報の取得、パケット分割処理、FECによる訂正処理を行い、UDPベースのIPネットワーク映像・メタ情報（解析処理結果を含む）伝送を行う技術です。
※HpVT映像伝送プロトコルは、NICT（国立研究開発法人情報通信研究機構）との共同研究成果、または、共同研究の成果を活用したものです。

## 動作環境
- OS
  - Raspberry Pi OS with desktop (Bullseye, 32-bit/64-bit, released on 2025-05-06)
- ハードウェア
  - 動作要件
    - H.264ハードウェアエンコーダ・デコーダ搭載
  - モデル
    - Raspberry Pi 3 Model B
    - Raspberry Pi 3 Model B+
    - Raspberry Pi 4 Model B
    - Raspberry Pi Zero 2 W
  - カメラ
    - Raspberry Pi Camera V2

## ビルド方法

### 必要なパッケージのインストール

Raspberry Pi OS上で以下のコマンドを実行してください。

```sh
sudo apt update
sudo apt install libcamera-dev libdrm-dev libepoxy-dev libx11-dev libv4l-dev
```

### ビルドスクリプトの実行
ビルドスクリプトbuild.shを実行してください。

```sh
sh build.sh
```

ビルドが成功すると、binディレクトリに実行ファイルhpvtが生成されます。

### ビルド済みファイルの削除
ビルド済みファイルを削除するには、以下の様にスクリプトを実行してください。

```sh
sh build.sh clean
```

## 実行方法

### ファイルの配置

以下のファイルを送信側ホストおよび受信側ホストにコピーしてください。

- ビルド済み実行ファイル hpvt (binディレクトリ)
- 設定ファイル hpvt.conf (confディレクトリ)

### 設定ファイルの編集

送信側ホストの設定ファイルをエディタで開き、以下の様に変更してください。

- 送信/受信の変更
```
変更前
#Type=transmitter
Type=receiver
変更後
Type=transmitter
#Type=receiver
```

- 接続先IPアドレスの変更  
受信側ホストのIPアドレス(例:192.168.0.123)に変更
```
変更前
#ConnectionIP=127.0.0.1
変更後
ConnectionIP=192.168.0.123
```

### プログラムの実行

受信側および送信側ホストでデスクトップ画面からTerminalを起動し、以下の様にプログラムを実行してください。

```sh
./hpvt -c hpvt.conf
```

送信側ホストから受信側ホストに接続が成功すると、映像伝送が開始されます。  
受信側ホストのデスクトップ画面にカメラ映像が表示されます。
