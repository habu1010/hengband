import os
import hashlib
import json
import sys


def calculate_sha256(file_path):
    """
    ファイルのSHA-256ハッシュを計算する関数
    """
    hash_sha256 = hashlib.sha256()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_sha256.update(chunk)
    return hash_sha256.hexdigest()


def create_file_hash_data(directory):
    """
    指定したディレクトリ内のファイルとそのMD5ハッシュの一覧をJSONデータとして作成する関数
    """
    file_hash_data = []
    for root, dirs, files in os.walk(directory):
        for file_name in files:
            file_path = os.path.join(root, file_name)
            sha256_hash = calculate_sha256(file_path)
            file_info = {"file_path": file_path, "sha256_digest": sha256_hash}
            file_hash_data.append(file_info)

    return file_hash_data


# コマンドライン引数からディレクトリパスを取得
if len(sys.argv) < 2:
    print("ディレクトリのパスを指定してください。")
    sys.exit(1)

directory_paths = sys.argv[1:]

# 各ディレクトリに対してファイルとMD5ハッシュの一覧を作成する
all_file_hash_data = []
for directory_path in directory_paths:
    file_hash_data = create_file_hash_data(directory_path)
    all_file_hash_data.extend(file_hash_data)

# JSONデータを作成する
json_data = json.dumps(all_file_hash_data, indent=4)

# JSONデータを表示する
print(json_data)
