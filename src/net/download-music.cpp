#include "external-lib/json.hpp"
#include "io/files-util.h"
#include "io/input-key-acceptor.h"
#include "net/http-client.h"
#include "term/screen-processor.h"
#include "term/term-color-types.h"
#include "term/z-term.h"
#include "util/angband-files.h"
#include "util/finalizer.h"
#include "util/int-char-converter.h"
#include "util/sha256.h"
#include <filesystem>
#include <iostream>

namespace {

const std::string DOWNLOAD_BASE_URL = "https://github.com/habu1010/hengband.xtra/blob/feature/add-music-sound-file-list-json/";

constexpr auto ROW_FETCH_FILELIST = 3;
constexpr auto ROW_CHECK_FILE = 5;
constexpr auto ROW_DOWNLOAD_FILE = 7;

/*!
 * @brief ファイルリストをダウンロードしてファイル名とSHA-256ハッシュ値のペアのリストを返す
 *
 * @param file_list_url ファイルリストのURL
 * @return ファイル名とSHA-256ハッシュ値のペアのリスト
 */
std::vector<std::pair<std::string, std::string>> fetch_file_list(const std::string &file_list_url)
{
    prt("プリセットファイルリストを取得しています... ", ROW_FETCH_FILELIST, 1);
    term_fresh();

    http::Client client;
    const auto res = client.get(file_list_url);
    if (!res || (res->status != 200)) {
        prt(_("  ダウンロードエラーが発生しました。", "  Download error occurred."), ROW_FETCH_FILELIST + 1, 1);
        inkey();
        return {};
    }

    try {
        std::vector<std::pair<std::string, std::string>> file_list;
        for (const auto &elem : nlohmann::json::parse(res->body)) {
            file_list.emplace_back(elem["filename"].get<std::string>(), elem["sha256_digest"].get<std::string>());
        }

        prt(_("  完了", "  Finished."), ROW_FETCH_FILELIST + 1, 1);
        return file_list;
    } catch (const nlohmann::json::exception &e) {
        const auto msg = format(_("  ファイルリスト解析エラー: %s", "  File list parsing error: %s"), e.what());
        prt(msg, ROW_FETCH_FILELIST + 1, 1);
        inkey();
        return {};
    }
}

/*!
 * @brief file_list に含まれるファイルのうち、SHA-256ハッシュ値が異なるファイルのリストを返す
 *
 * @param directory_path ファイルを検索するディレクトリのパス
 * @param file_list ファイル名とハッシュ値のペアのリスト
 * @return ハッシュ値が異なるファイルのリスト
 */
std::vector<std::filesystem::path> extract_updated_files(
    const std::filesystem::path &directory_path,
    const std::vector<std::pair<std::string, std::string>> &file_list)
{
    prt(_("ファイルチェック中... ", "Checking files..."), ROW_CHECK_FILE, 1);

    std::vector<std::filesystem::path> updated_files;
    for (const auto &[filename, digest] : file_list) {
        inkey_scan = true;
        if (inkey() == ESCAPE) {
            prt(_("中断しました。", "Terminated."), ROW_CHECK_FILE + 1, 1);
            inkey();
            return {};
        }
        auto filepath = path_build(directory_path, filename);

        prt(format("  %s", filename.data()), 6, 1);
        term_fresh();
        const auto file_digest = util::SHA256::compute_filehash(filepath);
        if (file_digest && (util::to_string(*file_digest) == digest)) {
            continue;
        }

        updated_files.push_back(std::move(filepath));
    }

    if (updated_files.empty()) {
        prt(_("  すべて最新です。", "  All files are up to date."), ROW_CHECK_FILE + 1, 1);
        inkey();
        return {};
    }

    prt(_("  完了", "  Finished."), ROW_CHECK_FILE + 1, 1);
    return updated_files;
}

void display_download_progress(const http::Progress &progress, const std::string &filename)
{
    constexpr auto progress_bar_length = 20;
    const auto now_length = (progress.total > 0) ? progress_bar_length * progress.now / progress.total : 0;
    if (now_length == 0) {
        prt(format("  [                    ] %s", filename.data()), ROW_DOWNLOAD_FILE + 1, 1);
        term_fresh();
        return;
    }

    std::stringstream progress_line;
    progress_line << "  [" << std::string(now_length, '#') << std::string(progress_bar_length - now_length, ' ') << "] "
                  << filename;
    prt(progress_line.str(), ROW_DOWNLOAD_FILE + 1, 1);
    term_fresh();
}

bool download_file(const std::string &url, const std::filesystem::path &filepath)
{
    auto is_terminated = false;
    auto progress_handler = [&is_terminated, filename = filepath.filename().string()](const http::Progress &progress) {
        inkey_scan = true;
        if (inkey() == ESCAPE) {
            is_terminated = true;
            return false;
        }

        display_download_progress(progress, filename);
        return true;
    };

    http::Client client;
    auto res = client.get(url, filepath, progress_handler);
    if (is_terminated) {
        prt(_("  中断しました。", "  Terminated."), ROW_DOWNLOAD_FILE + 1, 1);
        inkey();
        return false;
    }
    if (!res || (res->status != 200)) {
        prt(_("  ダウンロードエラーが発生しました。", "  Download error occurred."), ROW_DOWNLOAD_FILE + 1, 1);
        inkey();
        return false;
    }

    return true;
}

void download_files(const std::string &base_url, const std::vector<std::filesystem::path> &download_list)
{
    prt(_("ダウンロード中...", "Downloading... "), ROW_DOWNLOAD_FILE, 1);
    term_fresh();

    for (const auto &file : download_list) {
        const auto url = base_url + "/" + file.filename().string() + "?raw=true";
        if (!download_file(url, file)) {
            return;
        }
        term_xtra(TERM_XTRA_DELAY, 200);
    }

    prt(_("  完了", "  Finished."), ROW_DOWNLOAD_FILE + 1, 1);
    inkey();
}

void download_xtra(std::string_view title, std::string_view directory)
{
    screen_save();
    const auto finalizer = util::make_finalizer([] { screen_load(); });

    term_clear();
    prt(title, 1, 1);
    prt(_("ESCキーを押すと中断します。", "Hit ESC key to terminate."), 2, 1);

    const auto base_url = DOWNLOAD_BASE_URL + std::string(directory);

    const auto file_list = fetch_file_list(base_url + "/preset.json?raw=true");
    if (file_list.empty()) {
        return;
    }

    const auto download_list = extract_updated_files(ANGBAND_DIR_XTRA / directory, file_list);
    if (download_list.empty()) {
        return;
    }

    download_files(base_url, download_list);
}

} // namespace

bool download_music(PlayerType *)
{
    download_xtra(_("BGMプリセットをダウンロードします。", "Download music files."), "music");
    return true;
}

bool download_sound(PlayerType *)
{
    download_xtra(_("効果音プリセットをダウンロードします。", "Download sound files."), "sound");
    return true;
}
