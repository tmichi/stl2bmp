# stl2bmp
* バイナリSTLファイルのメッシュをボクセル化して、BMP画像群にするプログラムです。
* 3Dプリンタでの利用を想定しており、DPIを埋め込んだ1bit BMPで出力します。
* OpenGLの機能 (zバッファ) を利用しているため、そこそこ早いです。
# ビルド
## 必要な環境・ライブラリ
* C++17をサポートしたコンパイラ(gcc, clang, visual studio 他)
  * C++14では boost::filesystemを使うとうまく行きます。
* OpenGL
* glfw (ver. 3.3以上)
* Eigen (ver. 3.0以上)
* 以上が入れられるOS (MacOS 12.4 (Monterey), Window 10で確認済み）

glfwとかEigenはhomebrew などで入れておくと楽です。
## CMakeを用いたビルド
### Mac
```shell
% mkdir build
% cd build
% cmake ..
% make 
% make install
```
### Windows
* CMakeLists.txtに以下の変数を追加 
  * Eigen3_DIR : Eigen3Config.cmakeがあるところ　（例: SET(Eigen3_DIR "C:/Program Files (x86)/Eigen3/share/eigen3/cmake"))
  * glfw_DIR : glfw3Config.cmakeがあるところ（例 SET(glfw_DIR "C:/glfw-3.3.7/lib/cmake/glfw3"))
* CMakeでVSソリューションファイルを作って、Visual Studioでビルドする
#Linux
## 手動でコンパイル
* OpenGLとGLFWを用いたコンパイル方法を探してコンパイルしてください。stl2bmp_main.cppをコンパイルすればできます。
* Eigenはダウンロードしてインクルードパスを貼ればokです。
# 使い方
```shell
% stl2bmp input.stl {dpi}
```
* 入力
  * input.stl: 入力メッシュ（バイナリSTL限定）。単位はmm。
  * {dpi}: ピクセルの解像度[dpi]（オプション)。 デフォルトは360dpiです。
* 出力
  * 1bit BMP画像群。 input/フォルダに格納されます。フォルダ名は、入力ファイルから拡張子をとったもので自動的に決定されます。
* Windowsですとドラッグ＆ドロップで実行できます。
# FAQ
## ビルド
* CMakeが使えない(を使わない）ので、ビルドできない。
    * stl2bmp_main.cppの 以下の箇所を修正してください。あとは、GLFWのコンパイルのしかたを調べてビルドしてください。ファイルは１個だけなので、CUIでもできるはずです。
    ```c++
    :
    //#include <stl2bmp_version.hpp> //ここをコメント
    #define stl2bmp_VERSION 0.1.0 //（手入力でバージョンを入れる）
    :
   ```
## 入出力
* 画像の出力を他にしたい(TIFFとか)。
    * 画像変換ソフトでのバッチ処理をご利用ください。知っているものですと、IrfanViewとかImagemagickなどが考えられます。
    ```shell
    $ convert -density 360 -units PixelsPerInch sample01.bmp sample01.tif
    ```
* 他の解像度でもDrag&drop できるようにしてほしい.
   * バッチファイルを作ればokです。参考用に720dpi用のバッチファイルをstlbmp720.batとして用意していますので、stl2bmpのパスを通すか、同じディレクトリ（フォルダ）においた上でご利用ください。
* Binary STL以外の入力をサポートしてほしい。
  * コードが複雑になるためサポートの予定はありません。Meshlab等で binary STLに変換してお使いください。もしくはコードを改良してお使いください。
## 実行・結果の品質
* 本来存在しないひげのような前景ピクセルが出てきます。
    * メッシュの品質が悪い、例えば、閉じていない（穴がある）、三角形が針のように尖っていることが考えられます。再メッシュや修復などの前処理をしてください。例えばMeshlabとかPolymenderが有名です。
    * OpenGLの都合という可能性もあります。
    * 品質の悪いメッシュを扱う方法としてWinding number を利用した内外判定法が考えられます。libiglにサンプルがあるので、それを見ながら自分で作ることをお勧めします。
* dpiを大きくすると途中で途切れる(Mac)。
    * glReadPixels()の制約の可能性が考えられますが、原因不明で調査中です。Windowでは同様の現象はあらわれていません。
* ~.dllがないというメッセージが現れます(Windows, バイナリ)
    * VS2019再頒布可能パッケージを入れてください。
* メタデータのdpi表記が微妙にずれている。
  * dpiが1インチ(23.4mm)あたりのピクセル数なのに対して、BMPのピクセル解像度は1mあたりのピクセル数(int)になっています。そのため、変換時にずれが発生しているみたいです。 
## アルゴリズム
* 内部動作について知りたい
    * このプログラムでは、平行投影で固定視点からメッシュをレンダリングしています。 手前のクリッピングプレーンをちょうどサンプリングする平面に設定します。 この時、背景を黒、ポリゴンの表面を黒、裏面を白でレンダリングすると、裏面が見える箇所は白、それ以外は黒で表示され、クリッピング平面でのでのボクセル化された断面画像が得られます。 これをを動かすことで3次元のボクセル化画像が得られます。
      ![原理](images/principle.png "表裏をそれぞれ緑と白でレンダリングし、とある平面でクリッピングした結果")
# お問合せ
   * 道川隆士(理化学研究所光量子工学研究センター画像情報処理研究チーム)
     