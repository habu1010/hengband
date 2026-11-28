/*!
 * @file cfg-reader.h
 * @brief BGM・効果音の設定ファイル (.cfg) の読み込み
 * @details 設定の値の取り出しは Unix 系と Windows で方法が違うので、read_sections() のうち値を取り出す関数を受け取らない版を環境ごとに定義する
 */

#pragma once

#include "system/angband.h"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <string>
#include <tl/optional.hpp>
#include <unordered_map>
#include <vector>

typedef uint cfg_key;
using cfg_values = std::vector<std::string>;
using cfg_map = std::unordered_map<cfg_key, cfg_values>;

/*!
 * @brief .cfgファイルの読み取り対象とCfgData格納先のキー指定
 * @details
 * "term_xtra()"の第1引数(action-type)、第2引数(action-val)をデータの格納キーとする。
 * action-typeはaction_typeメンバで指定する。
 * key_atにより、action-valと.cfg内の読取対象キーの対応を取る。
 * key_atの引数にaction-valが渡され、対応する読取対象キーを返す。
 * key_at引数のaction-valは0から1,2,3...と順に呼ばれ、key_atがtl::nulloptを返すまで続ける。
 */
struct cfg_section {

    //! The name of the section in cfg file
    std::string section_name;
    //! the "actions" value of "term_xtra()". see:z-term.h TERM_XTRA_xxxxx
    int action_type;
    /*!
     * Returns a reference to the name of the key at a specified action-val* in the section.
     * *action-val : the 2nd parameter of "term_xtra()"
     */
    std::function<tl::optional<std::string>(int)> key_at;
    //! 1つでもデータを読み込めた場合にtrueを設定する。（nullptrの場合を除く）
    bool *has_data = nullptr;
};

class CfgData {
public:
    CfgData() = default;

    tl::optional<std::string> get_rand(int key1_type, int key2_val) const;
    bool has_key(int key1_type, int key2_val) const;
    void insert(int key1_type, int key2_val, cfg_values &&value);

protected:
    cfg_map map{};
};

//! .cfg のセクション名とキー名から値を取り出す関数。値が無ければ空の文字列を返す
using CfgValueGetter = std::function<std::string(const std::string &section, const std::string &key)>;

class CfgReader {
public:
    CfgReader(const std::filesystem::path &dir, std::initializer_list<concptr> files);
    CfgData read_sections(std::initializer_list<cfg_section> sections) const;
    CfgData read_sections(std::initializer_list<cfg_section> sections, const CfgValueGetter &get_value) const;
    const std::filesystem::path &get_cfg_path() const
    {
        return cfg_path;
    }

protected:
    std::filesystem::path dir;
    std::filesystem::path cfg_path;
};
