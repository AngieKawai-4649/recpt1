# Linux用PT1/PT2/PT3録画プログラム

ベース recpt1 HTTPサーバ版RC4 + α（STZ版）

## 【改造箇所】
1. autoconf廃止
2. ビルド時のワーニング除去
3. Makefile修正  
   b25処理有無をautoconfではなくmake時のオプションで切り替えることにした

## 【ビルド】
- recpt1  
  $ cd recpt1/recpt1  
  $ make [オプション]  
  オプションはMakefileコメントを参照  
  $ sudo make install
- driver  
  $ cd recpt1/driver  
  $ make  
  $ sudo make install



  

