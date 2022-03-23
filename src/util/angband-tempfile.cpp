#include "util/angband-tempfile.h"
#include "util/angband-files.h"

/*!
 * @brief デストラクタ
 * @details 一時ファイルをファイルを削除する。書き込み用に開いたままであれば閉じてから削除する。
 */
TempFile::~TempFile()
{
    this->close();
    fd_kill(this->filename.c_str());
}

/*!
 * @brief 一時ファイルを書き込み用に開く
 *
 * @return ファイルを開くのに成功したら true、失敗したら false
 */
bool TempFile::open()
{
    char buf[1024];

#ifdef HAVE_MKSTEMP
    strncpy(buf, "/tmp/hengband.XXXXXX", sizeof(buf));
    if (auto fd = mkstemp(buf); fd >= 0) {
        filename = buf;
        this->fp_ = fdopen(fd, "w");
    }
#else /* HAVE_MKSTEMP */
    if (path_temp(buf, sizeof(buf)) == 0) {
        filename = buf;
        this->fp_ = angband_fopen(buf, "w");
    }
#endif /* HAVE_MKSTEMP */

    return this->fp_ != nullptr;
}

/*!
 * @brief 一時ファイル書き込み用のFILEポインタを得る
 *
 * @return 一時ファイル書き込み用のFILEポインタ。書き込み用に開いているファイルがなければ nullptr
 */
FILE *TempFile::fp() const
{
    return this->fp_;
}

/*!
 * @brief 一時ファイルを閉じる
 * @details 一時ファイルに書き込んだ内容を確定させるため、読み出す前に呼び出すこと。
 */
void TempFile::close()
{
    angband_fclose(this->fp_);
    this->fp_ = nullptr;
}

/*!
 * @brief 一時ファイルのパス名を得る
 *
 * @return 一時ファイルのパス名を表す文字列へのポインタ
 */
concptr TempFile::name() const
{
    return this->filename.c_str();
}
