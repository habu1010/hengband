#pragma once

#include "system/angband.h"
#include <string>

/*!
 * @brief 一時ファイルクラス
 * @details 主に画面に表示する内容を一旦ファイルに書き出す時に使用する。
 * 基本的な使用の流れとしては、以下のようになる
 * 1. open() でファイルを開く
 * 2. fp() を経由してファイルに書き込む
 * 3. close() でファイルを閉じる
 * 4. name() で得られるファイル名から内容を読み出して画面に表示
 * 5. (デストラクタで) ファイルが削除される
 */
class TempFile {
public:
    TempFile() = default;
    ~TempFile();
    TempFile(const TempFile &) = delete;
    TempFile &operator=(const TempFile &) = delete;

    FILE *fp() const;
    bool open();
    void close();
    concptr name() const;

private:
    FILE *fp_ = nullptr;
    std::string filename;
};
