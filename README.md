# Linux用PT1/PT2/PT3用チューナーアプリケーション

ベース recpt1 HTTPサーバ版RC4 + α（STZ版）

## 【改造箇所】
1. autoconf廃止
2. ビルド時のワーニング除去
3. Makefile修正  
   b25処理有無をautoconfではなくmake時のオプションで切り替えることにした
4. プログラムコンスタントに所持していたチャンネル情報を削除しchannelconfから取得するように修正

## 【ビルド】
- recpt1  
  b25デコードをする場合、事前に https://github.com/AngieKawai-4649/libarib25 を導入する  
  事前に https://github.com/AngieKawai-4649/channelconf を導入する（必須）  
  git clone https://github.com/AngieKawai-4649/recpt1.git  
  $ cd recpt1/recpt1  
  $ make [オプション]  
  オプションはMakefileコメントを参照  
  $ sudo make install  
- driver  
  $ cd recpt1/driver  
  $ make  
  以降は https://github.com/AngieKawai-4649/pt3 ドライバーインストールを参照  
  pt3_drv を pt1_drv に読み替えれば同様の手順である  

##  【使い方】
- Mirakurunから起動するチューナーアプリケーションとして  
  Mirakurun tuners.yml にrecpt1を起動登録する  
### 例：  
    - name: PT2-S1
      types:
        - BS
        - CS
      command: recpt1 --dev /dev/pt1video0 --b25 <channel> - -
      decoder:
      isDisabled: false
    - name: PT2-S2
      types:
        - BS
        - CS
      command: recpt1 --dev /dev/pt1video1 --b25 <channel> - -
      decoder:
      isDisabled: false
    - name: PT2-T1
      types:
        - GR
      command: recpt1 --dev /dev/pt1video2 --b25 <channel> - -
      decoder:
      isDisabled: false
    - name: PT2-T2
      types:
        - GR
      command: recpt1 --dev /dev/pt1video3 --b25 <channel> - -
      decoder:
      isDisabled: false
  - HTTP サーバーとして起動する
### 例：
    PT3を実装しているPCで以下を起動（デーモン起動される）
    PT3 S0
    $ recpt1 --b25 --http 4646 --device /dev/pt3video0
    PT3 S1
    $ recpt1 --b25 --http 4647 --device /dev/pt3video1
    PT3 T0
    $ recpt1 --b25 --http 4648 --device /dev/pt3video2
    PT3 T1
    $ recpt1 --b25 --http 4649 --device /dev/pt3video3

    他PC等のメデアプレイヤー（VLC、SMPlayer等）で以下のURLを指定することで視聴できる
    http://192.168.1.5:4648/UHF_27/1024
    ※ PT3を実装しているPCIPアドレスを指定
       recpt1を起動時に指定したポート番号を指定
       チャンネルとSIDを指定
   
    


