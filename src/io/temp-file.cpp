#include "io/temp-file.h"
#include "locale/language-switcher.h"
#include <cstdlib>
#include <fmt/format.h>
#include <system_error>
#ifdef _WIN32
#define NOMINMAX
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

TempFile::TempFile()
{
    std::error_code ec;
    const auto temp_dir = std::filesystem::temp_directory_path(ec);
    if (ec) {
        constexpr auto fmt = _("一時ファイル格納先フォルダの取得に失敗しました: {}", "Failed to get temporary directory path: {}");
        this->error_message = fmt::format(fmt, ec.message());
        return;
    }

#ifdef _WIN32
    wchar_t temp_file_path[MAX_PATH]{};
    if (GetTempFileNameW(temp_dir.wstring().data(), L"tmp", 0, temp_file_path) == 0) {
        this->error_message = _("一時ファイルの作成に失敗しました", "Failed to create temporary file name");
        return;
    }

    this->path = temp_file_path;

    // 書いた一時ファイルは narrow のパスで読む (FileDisplayer など) ので、内部の文字コードにできないパスは使わない
    try {
        (void)this->path.string();
    } catch (const std::system_error &) {
        this->error_message = _("一時ファイルのパスを内部の文字コードにできません", "Failed to convert the temporary file path to the internal encoding");
        return;
    }
#else
    auto temp_file_path = (temp_dir / "tempfile_XXXXXX").string();
    const auto fd = mkstemp(&temp_file_path[0]);
    if (fd == -1) {
        constexpr auto fmt = _("一時ファイルの作成に失敗しました: {}", "Failed to create temporary file name: {}");
        this->error_message = fmt::format(fmt, temp_file_path);
        return;
    }

    fchmod(fd, S_IRUSR | S_IWUSR);
    ::close(fd); // メンバー関数の std::ofstream::close() ではなく、ファイル記述子を閉じる
    this->path = temp_file_path;
#endif

    this->open(this->path);
    if (!this->is_open()) {
        constexpr auto fmt = _("一時ファイルを開けませんでした: {}", "Failed to open the temporary file: {}");
        this->error_message = fmt::format(fmt, this->path.string());
    }
}

/*!
 * @details Windows では開いたままのファイルを消せないので、先に閉じる
 */
TempFile::~TempFile()
{
    this->close();
    if (!this->path.empty()) {
        std::error_code ec;
        std::filesystem::remove(this->path, ec);
    }
}

const std::filesystem::path &TempFile::get_path() const
{
    return this->path;
}

const tl::optional<std::string> &TempFile::get_error_message() const
{
    return this->error_message;
}
