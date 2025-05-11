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
  

