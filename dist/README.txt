BakkesMod Plugin PraS(Private-match aid Steam)
developed by Kazuryu
# 概要
・ゴール時にOBSの現在のシーンにあるpras_goal(メディアソース)を再生する
・観戦画面時にフォーカスしている、
プレイヤーのスコアを、OBSの現在のシーンにあるpras_score(テキストソース)に反映
            画像をOBS virtualCameraに描画

# 設定方法
## BakkesMod pluginのインストール
- https://note.com/forusian/n/n0d15fde904d3 この記事を参考に、PraS.dllをpluginsに配置する
- plugin.cfgに*plugin load pras*を追記する

## OBS-websocketの導入
- https://github.com/obsproject/obs-websocket/releases/tag/4.9.1 このサイトの
obs-websocket-4.9.1-Windows-Installer.exeをダウンロードし、実行する　その後はダイアログに従いインストールする
- OBSを開き、ツールリボンにWebsocketサーバー設定が追加されていれば導入成功(設定ウィンドウはデフォルトのままで大丈夫です)

## PraS.batの起動
- あらかじめOBSを開いておき、PraS.batを起動する
- 接続成功が表示されれば成功

## OBSの設定
### トランザクション
- OBSでpras_goal名でメディアソースを追加する
- 使用するトランザクション動画を設定する
### スコア
- OBSでpras_score名でテキストソースを追加する
- 任意のフォントや色を設定する 
### アイコン
- OBSで任意の名前で映像キャプチャデバイスからOBS Virtual Cameraを設定する

## プレイヤー画像の設定
- imagesフォルダに使用するプレイヤー画像を配置しておく
- table.txtを開き、ディスプレイ名:imgaeフォルダに配置したファイル名
のフォーマットで対応づけをしていく
- 最上段のTEMPははじめに表示される画像

# 使い方
- OBS -> PraS.bat -> ロケリの順で起動する
- OBSと接続したときには、接続成功と出力される
- ロケリと接続したときには、connected!と出力される
