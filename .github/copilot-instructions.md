# Hengband Copilot Instructions / Hengband Copilot 指示

These instructions apply to routine work in this repository.
この指示は、このリポジトリでの日常的な作業全般に適用する。

## General / 全般

- Keep changes narrowly scoped to the requested task.
- 変更は依頼されたタスクに対して必要最小限に留める。
- Fix the root cause when practical, but avoid unrelated refactors.
- 可能なら根本原因を直すが、無関係なリファクタは避ける。
- Preserve existing naming, file structure, and public APIs unless the task requires changing them.
- タスク上必要でない限り、既存の命名、ファイル構成、公開 API を維持する。

## C++ Style / C++ スタイル

- Follow the repository formatting rules from .clang-format.
- .clang-format の整形ルールに従う。
- Use 4 spaces for indentation and do not use tabs.
- インデントは 4 スペースとし、タブは使わない。
- Do not reformat unrelated lines just because a file was touched.
- ファイルを触ったついでに、無関係な行まで整形しない。

## Modernization / モダナイズ

- Prefer std::string for owned text.
- 所有する文字列には std::string を優先する。
- Prefer std::string_view for read-only string parameters when ownership is unnecessary.
- 所有権が不要な読み取り専用文字列引数には std::string_view を優先する。
- Prefer std::vector<std::string> for split or line-based text collections.
- 分割結果や行単位の文字列集合には std::vector<std::string> を優先する。
- Reuse existing helpers before introducing new parsing or string utility code.
- 新しいパース処理や文字列ユーティリティを増やす前に、既存ヘルパーを再利用する。

## Legacy Boundaries / 旧来境界

- Do not replace char * or fixed-size buffers blindly at platform, savefile, or legacy engine boundaries.
- プラットフォーム API、セーブデータ形式、旧来エンジン境界では、char * や固定長バッファを安易に置き換えない。
- When a C string boundary must remain, prefer existing wrappers such as angband_strcpy and angband_strcat.
- C 文字列境界を残す必要がある場合は、angband_strcpy や angband_strcat など既存ラッパーを優先する。
- Be careful not to break JP-specific multibyte handling.
- JP 固有のマルチバイト処理を壊さないよう注意する。

## Project Idioms / プロジェクトの流儀

- Prefer tl::optional where that matches surrounding code.
- 周辺コードに合わせて tl::optional を優先する。
- Match nearby code for namespace usage, helper placement, and local aliases.
- namespace の使い方、ヘルパー配置、ローカルな型別名は近傍コードに合わせる。
- Keep comments sparse and only add them where compatibility constraints or control flow are hard to infer.
- コメントは最小限にし、互換性制約や制御フローが読み取りにくい箇所だけに付ける。

## Validation / 検証

- When editing C++ files, run the relevant formatting or build checks when practical.
- C++ ファイルを編集したら、可能な範囲で整形チェックやビルド確認を行う。
- Prefer the workspace build task or make -j $(nproc) for full verification.
- 全体確認にはワークスペースの build タスク、または make -j $(nproc) を優先する。

For deeper guidance specific to C++ implementation and review work, use the skill at .github/skills/cpp-coding-conventions/SKILL.md.
より詳しい C++ 実装・レビュー指針は .github/skills/cpp-coding-conventions/SKILL.md を参照する。
