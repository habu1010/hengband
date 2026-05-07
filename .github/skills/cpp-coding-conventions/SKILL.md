---
name: cpp-coding-conventions
description: 'Use when: implementing, refactoring, or reviewing Hengband C++ code and you need the repository''s coding conventions for formatting, string handling, modernization, and validation. Hengband の C++ コーディング規約を参照したいときに使う。'
---

# Hengband C++ Coding Conventions / Hengband C++ コーディング規約

## Use This Skill When / この Skill を使う場面

- You are editing C++ code under src/ and want repository-specific rules rather than generic C++ advice.
- src/ 配下の C++ コードを編集するときに、一般論ではなく Hengband 固有のルールを参照したい。
- You are reviewing a PR for style, modernization, or safety issues.
- PR レビューで、スタイル、モダナイズ、安全性の観点を確認したい。
- You are replacing legacy C-style string handling with std::string, std::string_view, std::vector<std::string>, or related utilities.
- 旧来の C スタイル文字列処理を std::string、std::string_view、std::vector<std::string> などへ置き換えたい。

## Goals / 目的

- Keep changes consistent with the existing Hengband codebase.
- 変更内容を既存の Hengband コードベースと整合させる。
- Prefer safe, incremental modernization over broad rewrites.
- 大規模な書き換えよりも、安全で段階的なモダナイズを優先する。
- Preserve behavior across Japanese and non-Japanese builds.
- 日本語版ビルドと英語版ビルドの両方で挙動を維持する。
- Validate formatting and compilation before concluding work.
- 作業を終える前に、整形とコンパイルの検証を行う。

## Repository-Specific Rules / リポジトリ固有のルール

### 1. Formatting / 整形

- Follow the repository clang-format rules from .clang-format.
- .clang-format の設定に従う。
- Use 4 spaces for indentation. Do not use tabs.
- インデントは 4 スペースを使い、タブは使わない。
- Keep include ordering compatible with the existing formatter and CI checks.
- include の並び順は既存の formatter と CI チェックに合わせる。
- Do not reformat unrelated code just because you touched the file.
- 触ったついでに無関係なコードまで整形しない。

### 2. Scope Of Changes / 変更範囲

- Fix the requested problem at the root cause, but keep edits narrowly scoped.
- 要求された問題は根本原因から直すが、変更範囲は狭く保つ。
- Avoid drive-by refactors in unrelated modules.
- 無関係なモジュールでのついでリファクタは避ける。
- Preserve existing naming, file layout, and public APIs unless the task requires a change.
- タスク上必要でない限り、既存の命名、ファイル構成、公開 API を維持する。

### 3. String And Container Modernization / 文字列とコンテナのモダナイズ

- Prefer std::string for owned text.
- 所有する文字列には std::string を優先する。
- Prefer std::string_view for read-only string parameters when the callee does not need ownership.
- 呼び出し先が所有権を必要としない読み取り専用引数には std::string_view を優先する。
- Prefer std::vector<std::string> for split or line-based text collections.
- 分割結果や行単位の文字列集合には std::vector<std::string> を優先する。
- Prefer existing helpers in src/util/string-processor.cpp before inventing new ad hoc parsing logic.
- 独自の場当たり的なパース処理を増やす前に、src/util/string-processor.cpp の既存ヘルパーを優先する。
- Prefer std::filesystem::path for file path handling where the surrounding code already uses it.
- 周辺コードが使っている箇所では、ファイルパス処理に std::filesystem::path を優先する。

### 4. C String Boundaries / C 文字列境界

- Do not replace char * or fixed-size buffers blindly when the interface is constrained by platform APIs, savefile formats, or legacy engine boundaries.
- プラットフォーム API、セーブデータ形式、旧来エンジン境界に縛られる箇所では、char * や固定長バッファを安易に置き換えない。
- When a C string boundary must remain, prefer routing logic through existing wrappers such as angband_strcpy, angband_strcat, and related utilities.
- C 文字列境界を残す必要がある場合は、angband_strcpy、angband_strcat など既存ラッパー経由の実装を優先する。
- Be careful around JP-specific behavior. Some wrappers contain multibyte-aware handling that must not be bypassed casually.
- JP 固有の挙動に注意する。既存ラッパーにはマルチバイト対応が含まれており、安易に迂回してはいけない。

### 5. Existing Project Idioms / 既存プロジェクトの流儀

- Prefer tl::optional where that is already the project convention.
- 既存コードがそうなっている箇所では tl::optional を優先する。
- Match nearby code style for namespace usage, helper placement, and local type aliases.
- namespace の使い方、ヘルパーの配置、ローカルな型別名は近傍コードに合わせる。
- Reuse existing utility functions and types before introducing new abstractions.
- 新しい抽象化を導入する前に、既存のユーティリティ関数や型を再利用する。
- Keep comments sparse and only add them where the control flow or compatibility constraint is not obvious.
- コメントは最小限にし、制御フローや互換性制約が読み取りにくい箇所にだけ付ける。

### 6. Reviews And Refactors / レビューとリファクタ

- For review tasks, prioritize behavioral regressions, unsafe conversions, lifetime issues, and encoding or path handling bugs.
- レビュー時は、挙動退行、危険な変換、寿命問題、エンコーディングやパス処理の不具合を優先して見る。
- When replacing C-style string code, check ownership, null termination, truncation behavior, and lifetime of string_view.
- C スタイル文字列コードを置き換えるときは、所有権、終端 NUL、切り詰め挙動、string_view の寿命を確認する。
- Prefer incremental refactors that can be validated by build and formatter checks.
- formatter とビルドで検証できる、段階的なリファクタを優先する。

## Suggested Workflow / 推奨ワークフロー

1. Read the surrounding file and related utilities before editing.
1. 編集前に周辺ファイルと関連ユーティリティを読む。
2. Identify whether the code is at a true C boundary or can be modernized safely.
2. そのコードが本当に C 境界にあるのか、安全にモダナイズできるのかを見極める。
3. Make the smallest coherent change.
3. 最小限で一貫した変更に留める。
4. Run formatting checks if the change affects C++ source layout.
4. C++ ソースのレイアウトに影響したら整形チェックを行う。
5. Run a build or the relevant target to catch regressions.
5. ビルドまたは関連ターゲットを実行して退行を確認する。

## Validation Commands / 検証コマンド

- Format check: sh ./.github/scripts/check-cpp-format.sh
- 整形チェック: sh ./.github/scripts/check-cpp-format.sh
- Full build: use the workspace build task or run make -j $(nproc)
- 全体ビルド: ワークスペースの build タスク、または make -j $(nproc)

## Review Checklist / レビューチェックリスト

- Is the change consistent with .clang-format output?
- 変更は .clang-format の出力と整合しているか。
- Did the change preserve existing behavior for both JP and non-JP code paths?
- JP と非 JP の両コードパスで既存挙動を保てているか。
- Are std::string_view uses free from dangling references?
- std::string_view にダングリング参照はないか。
- Were existing string/path helpers reused where appropriate?
- 既存の文字列処理・パス処理ヘルパーを適切に再利用しているか。
- Did the change avoid unrelated cleanup?
- 無関係なクリーンアップを混ぜていないか。
- Was the code built or otherwise validated after the edit?
- 編集後にビルドまたは相当の検証を実施したか。
