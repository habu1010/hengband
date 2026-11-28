/*!
 * @file temp-file.h
 * @brief 消えるときにファイルも消える一時ファイル
 */

#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <tl/optional.hpp>

/*!
 * @brief 一時ファイルの書き込み用のファイルストリーム
 * @details
 * 作ったときに一時フォルダにファイルを作り、書き込み用に開く。std::ofstream として書き込める。
 * 消えるときに閉じてからファイルを消す。作れなかったときは get_error_message() が理由を返し、
 * ストリームは開いていない (書き込みは失敗の状態になるだけで何も書かない)。
 */
class TempFile : public std::ofstream {
public:
    TempFile();
    TempFile(const TempFile &) = delete;
    TempFile(TempFile &&) = delete;
    TempFile &operator=(const TempFile &) = delete;
    TempFile &operator=(TempFile &&) = delete;
    ~TempFile() override;

    const std::filesystem::path &get_path() const;
    const tl::optional<std::string> &get_error_message() const;

private:
    std::filesystem::path path;
    tl::optional<std::string> error_message;
};
