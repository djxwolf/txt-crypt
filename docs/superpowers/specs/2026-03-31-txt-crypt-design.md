# txt-crypt 设计文档

**日期:** 2026-03-31
**版本:** 1.0
**状态:** 设计阶段

---

## 1. 项目概述

**txt-crypt** - 一个高效的命令行文本文件加密工具，专门优化大文本文件处理。

### 1.1 核心功能

- 使用 AES-256-GCM 加密文本内容
- 通过密码短语派生密钥（Argon2id KDF）
- 流式处理，内存占用恒定
- 输出 Base64 编码的加密文本
- 显示实时进度条
- 支持批量处理（通配符、目录递归）
- 支持原地修改（--in-place）

### 1.2 技术栈

- C++17
- mbedtls 3.x（静态链接）
- 标准库-only 依赖

---

## 2. 命令行接口

### 2.1 基本用法

```bash
# 加密到新文件
txt-crypt encrypt input.txt -o encrypted.txt

# 原地加密（直接覆盖原文件）
txt-crypt encrypt input.txt --in-place
txt-crypt encrypt -i input.txt

# 解密
txt-crypt decrypt encrypted.txt -o decrypted.txt
txt-crypt decrypt encrypted.txt --in-place

# 批量处理
txt-crypt encrypt *.txt --in-place
txt-crypt encrypt /path/to/files/ -r -o encrypted/

# 选项
txt-crypt encrypt input.txt -i --no-progress
txt-crypt encrypt input.txt -i --threads 4
txt-crypt encrypt input.txt -i --backup
```

### 2.2 命令行参数

| 参数 | 说明 |
|-----|------|
| `encrypt`/`decrypt` | 操作模式 |
| `-i, --in-place` | 原地修改文件 |
| `-o, --output PATH` | 输出文件/目录 |
| `-r, --recursive` | 递归处理目录 |
| `--no-progress` | 禁用进度条 |
| `--threads N` | 指定线程数 |
| `--backup` | 原地修改时备份原文件 |

### 2.3 原地修改安全机制

1. 先写入临时文件（`.tmp` 后缀）
2. 完成后原子替换原文件
3. 可选 `--backup` 保留原文件备份（`.bak` 后缀）

---

## 3. 架构设计

### 3.1 目录结构

```
txt-crypt
├── src/
│   ├── main.cpp              # 命令行入口
│   ├── cli/
│   │   ├── cli_parser.cpp     # 参数解析
│   │   └── progress.cpp       # 进度条显示
│   ├── core/
│   │   ├── crypto_engine.cpp  # 加密/解密引擎
│   │   ├── kdf.cpp            # 密钥派生 (Argon2id)
│   │   └── stream_processor.cpp # 流式处理
│   ├── io/
│   │   ├── file_reader.cpp    # 文件读取
│   │   ├── file_writer.cpp    # 文件写入
│   │   └── base64.cpp         # Base64 编码/解码
│   └── utils/
│       ├── thread_pool.cpp    # 线程池
│       └── temp_file.cpp      # 临时文件处理
├── include/                   # 头文件
├── tests/                     # 单元测试
└── third_party/mbedtls/       # 静态链接的加密库
```

### 3.2 模块职责

| 模块 | 职责 |
|------|------|
| `cli_parser` | 解析命令行参数，验证输入 |
| `crypto_engine` | 封装 AES-256-GCM 加密操作 |
| `kdf` | 使用 Argon2id 从密码派生密钥 |
| `stream_processor` | 协调读取→加密→编码→写入流程 |
| `file_reader/writer` | 高效文件 I/O，支持大文件 |
| `base64` | 流式 Base64 编码/解码 |
| `thread_pool` | 管理加密工作线程 |
| `temp_file` | 原地修改的临时文件处理 |
| `progress` | 实时进度条显示 |

---

## 4. 数据流设计

### 4.1 加密流程

```
输入文件 ──→ [File Reader] ──→ [Argon2id KDF] ──→ [AES-256-GCM]
                                                         │
                                              生成 Salt + IV + 密文
                                                         │
                                                         ↓
                                                   [Base64 编码]
                                                         │
                                                         ↓
                   [进度更新] ←──────────────────────────────────┘
                                                         │
                                                         ↓
                                              输出文件 / 原地替换
```

### 4.2 输出文件格式

加密后的文件包含以下信息（Base64 编码）：

```
Header (16 bytes):
+--------+--------+----------+---------+----------+
| MAGIC  | VERSION| SALT_LEN | IV_LEN  | RESERVED |
| 8 bytes| 1 byte | 1 byte   | 1 byte  | 5 bytes  |
+--------+--------+----------+---------+----------+

Encrypted Data:
+--------+--------+----------+-----------------------+
| SALT   | IV     | TAG      | CIPHERTEXT...         |
| 16B    | 12B    | 16B      | 剩余部分              |
+--------+--------+----------+-----------------------+
```

- **MAGIC**: `"TXTCRYPT"` (文件类型标识)
- **VERSION**: `1` (格式版本)
- **SALT**: Argon2id 盐值
- **IV**: GCM 初始化向量
- **TAG**: GCM 认证标签（完整性校验）
- **CIPHERTEXT**: 实际加密内容

### 4.3 流式处理架构

```
┌─────────────────────────────────────────────────────────────┐
│                    主线程                                    │
│  ┌──────────┐    ┌────────────┐    ┌──────────┐            │
│  │ File     │───▶│ Buffer     │───▶│ Base64   │            │
│  │ Reader   │    │ Queue      │    │ Encoder  │            │
│  └──────────┘    └────────────┘    └──────────┘            │
│       │                                                  │
│       │ 更新进度                                          │
│       ▼                                                  │
│  ┌──────────┐                                            │
│  │ Progress │                                            │
│  │ Display  │                                            │
│  └──────────┘                                            │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                    加密线程池                                │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Thread 1: Buffer → AES Encrypt → Encrypted Queue    │  │
│  │ Thread 2: Buffer → AES Encrypt → Encrypted Queue    │  │
│  │ Thread N: Buffer → AES Encrypt → Encrypted Queue    │  │
│  └──────────────────────────────────────────────────────┘  │
│                           │                                  │
│                           ▼                                  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Encrypted Queue → Base64 Encode → Write Queue       │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

**关键参数：**
- 缓冲区大小：256 KB - 1 MB（可配置）
- 线程数：默认 `std::thread::hardware_concurrency()`

---

## 5. 错误处理

### 5.1 错误类型

| 错误类型 | 处理方式 | 退出码 |
|---------|---------|--------|
| 文件不存在 | 清晰错误消息 | 1 |
| 权限不足 | 提示用户 | 2 |
| 密码错误（解密时） | 提示"密码错误或文件已损坏" | 3 |
| 磁盘空间不足 | 检查后提前提示 | 4 |
| 中断信号 (Ctrl+C) | 清理临时文件，优雅退出 | 130 |
| 加密验证失败 | 完整性校验错误 | 5 |
| 无效参数 | 用法提示 | 6 |

### 5.2 原地修改的安全保证

```
1. 创建临时文件: input.txt.tmp
2. 所有加密内容写入 .tmp
3. 验证写入完整性
4. 原子替换: rename(.tmp, input.txt)
   (如果任何步骤失败，原文件不受影响)
```

### 5.3 退出码定义

```cpp
enum ExitCode {
    SUCCESS = 0,
    ERROR_FILE_NOT_FOUND = 1,
    ERROR_PERMISSION = 2,
    ERROR_DECRYPTION_FAILED = 3,
    ERROR_DISK_SPACE = 4,
    ERROR_INTEGRITY = 5,
    ERROR_INVALID_ARG = 6,
    ERROR_UNKNOWN = 255
};
```

---

## 6. 依赖和构建

### 6.1 外部依赖

| 依赖 | 版本 | 用途 | 链接方式 |
|-----|------|------|---------|
| mbedtls | 3.6+ | AES-256-GCM, Argon2id | 静态链接 |

### 6.2 构建系统

使用 CMake（跨平台标准）

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### 6.3 平台支持

- Linux (x86_64, ARM64)
- macOS (x86_64, ARM64)
- Windows (MSVC, MinGW)

### 6.4 单文件部署

```
Linux:   txt-crypt (约 2-3 MB)
macOS:   txt-crypt (约 2-3 MB)
Windows: txt-crypt.exe (约 2-3 MB)
```

---

## 7. 测试策略

### 7.1 单元测试

| 模块 | 测试内容 |
|-----|---------|
| `crypto_engine` | 加密/解密正确性、不同数据大小 |
| `kdf` | 密钥派生正确性、相同密码+盐产生相同密钥 |
| `base64` | 编码/解码正确性、边界条件 |
| `stream_processor` | 流式处理、内存泄漏检查 |
| `temp_file` | 原地替换、失败回滚 |

### 7.2 集成测试

```
test_fixtures/
├── small.txt           # 1 KB
├── medium.txt          # 1 MB
├── large.txt           # 100 MB
├── unicode.txt         # UTF-8, emoji
├── multiline.txt       # 多种换行符
└── special_chars.txt   # 特殊字符
```

测试场景：
- 加密 → 解密 → 验证内容一致
- 原地修改 → 验证文件正确替换
- 错误密码 → 解密失败
- 批量处理 → 所有文件正确处理
- Ctrl+C 中断 → 临时文件清理

### 7.3 大文件测试

**真实大文件测试（本地）：**
```bash
dd if=/dev/zero of=1GB.txt bs=1M count=1024
dd if=/dev/zero of=5GB.txt bs=1M count=5120
```

**模拟大文件测试（CI）：**
```cpp
class VirtualLargeFileStream : public std::streambuf {
    // 虚拟流，不占用磁盘，按需生成数据
};
```

**大文件测试项：**

| 测试项 | 验证内容 | 预期结果 |
|-------|---------|---------|
| 内存占用 | 处理 10GB 文件 | 内存 < 100 MB |
| 速度 | 1GB, 5GB, 10GB | 线性增长，无退化 |
| 中断恢复 | 50% 时 Ctrl+C | 临时文件清理 |
| 磁盘空间检测 | 空间不足时 | 提前检测失败 |

### 7.4 性能基准

```
文件大小    | 目标速度 (加密)
-----------|-------------
1 MB       | < 10 ms
100 MB     | < 1 s
1 GB       | < 10 s
10 GB      | < 100 s
```

### 7.5 测试框架

使用 Catch2 或 Google Test

---

## 8. 安全考虑

### 8.1 密码处理

```
输入密码 → 立即派生密钥 → 清零密码内存
(不在日志、内存转储中残留)
```

### 8.2 密钥派生参数（Argon2id）

| 参数 | 值 | 说明 |
|-----|---|------|
| salt | 16 字节随机 | 每次加密不同 |
| t_cost | 3 | 迭代次数 |
| m_cost | 64 MiB | 内存成本（防 GPU 破解） |
| parallelism | 4 | 并行度 |
| output_len | 32 字节 | AES-256 密钥长度 |

### 8.3 加密安全

- AES-256-GCM：认证加密，防篡改
- 每次 IV 随机（12 字节）
- GCM Tag 验证完整性（16 字节）
- 密文泄露 ≠ 密钥泄露（KDF 单向）

### 8.4 文件格式安全

```
Header 明文存储（MAGIC, VERSION, salt_len, iv_len）
├─ Salt 明文存储（公开，无安全风险）
├─ IV 明文存储（公开，无安全风险）
└─ 密文加密存储（Tag + Ciphertext）
```

### 8.5 侧信道防护

- 密码比较使用恒定时间
- 避免分支依赖于密钥数据
- 敏感数据及时清零

---

## 9. 未来扩展（可选）

| 功能 | 优先级 |
|-----|-------|
| 多密码加密（双重加密） | P3 |
| 云存储集成 | P3 |
| GUI 版本 | P2 |
| 密码管理器集成 | P2 |

---

## 变更历史

| 日期 | 版本 | 变更内容 |
|------|------|---------|
| 2026-03-31 | 1.0 | 初始设计 |
