# txt-crypt

A secure, high-performance command-line tool for encrypting and decrypting text files using AES-256-GCM encryption.

## Features

- **AES-256-GCM Encryption**: Military-grade encryption with authenticated encryption (AEAD)
- **Password-Based Key Derivation**: Uses Argon2id KDF with configurable parameters
- **Streaming Architecture**: Efficiently handles files of any size with minimal memory footprint
- **Multi-Threading**: Parallel processing for improved performance on multi-core systems
- **Atomic Operations**: Safe file operations with automatic cleanup and rollback
- **Base64 Encoding**: Encrypted files use Base64 encoding for text-safe storage
- **Progress Tracking**: Real-time progress bar for long-running operations
- **Batch Processing**: Encrypt/decrypt multiple files or entire directories
- **In-Place Editing**: Optional in-place modification with automatic backup creation
- **Cross-Platform**: Works on Linux, macOS, and other Unix-like systems

## Security

- **Algorithm**: AES-256-GCM (256-bit key, 96-bit nonce, 128-bit auth tag)
- **Key Derivation**: Argon2id (memory-hard KDF resistant to GPU/ASIC attacks)
  - Time cost: 3 iterations
  - Memory cost: 64 MB
  - Parallelism: 4 threads
- **Authentication**: GCM provides both confidentiality and integrity
- **No Key Storage**: Passwords are never stored, only used for key derivation
- **Secure Memory**: Sensitive data is cleared from memory after use

## Building from Source

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake build tools
- Git (for cloning with submodules)

### Build Steps

```bash
# Clone the repository with submodules
git clone --recurse-submodules https://github.com/yourusername/txt-crypt.git
cd txt-crypt

# Create build directory
mkdir build && cd build

# Configure with tests enabled
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

# Build
cmake --build .

# (Optional) Install system-wide
sudo cmake --install .
```

### Build Options

- `-DCMAKE_BUILD_TYPE=Release`: Build optimized release version
- `-DCMAKE_BUILD_TYPE=Debug`: Build debug version with symbols
- `-DBUILD_TESTS=ON`: Build test suite
- `-DENABLE_THREADS=ON`: Enable multi-threading (default: ON)

## Usage

### Basic Encryption

```bash
# Encrypt a single file
txt-crypt encrypt myfile.txt -o myfile.txt.enc

# Encrypt to different output directory
txt-crypt encrypt myfile.txt -o encrypted/

# Encrypt in-place (creates backup)
txt-crypt encrypt myfile.txt --in-place --backup
```

### Basic Decryption

```bash
# Decrypt a file
txt-crypt decrypt myfile.txt.enc -o myfile.txt

# Decrypt in-place
txt-crypt decrypt myfile.txt.enc --in-place --backup
```

### Batch Operations

```bash
# Encrypt multiple files
txt-crypt encrypt *.txt -o encrypted/

# Encrypt directory recursively
txt-crypt encrypt documents/ -o encrypted_docs/ --recursive

# Decrypt directory
txt-crypt decrypt encrypted_docs/ -o restored_docs/ --recursive
```

### Advanced Options

```bash
# Disable progress bar (useful for scripting)
txt-crypt encrypt file.txt -o file.enc --no-progress

# Use specific number of threads
txt-crypt encrypt largefile.txt -o largefile.enc --threads 8

# Combine options
txt-crypt encrypt *.txt -o encrypted/ --recursive --threads 4 --no-progress
```

## Output Format

Encrypted files use the following format:

```
TXTCRYPT1
<Base64-encoded-salt>
<Base64-encoded-nonce>
<Base64-encoded-ciphertext-and-auth-tag>
```

- Version identifier: `TXTCRYPT1`
- Salt: 16 bytes (for key derivation)
- Nonce: 12 bytes (unique per encryption)
- Ciphertext + Auth Tag: Variable length

## Examples

### Example 1: Simple Encryption

```bash
echo "Secret message" > secret.txt
txt-crypt encrypt secret.txt -o secret.enc
# (Enter password when prompted)

txt-crypt decrypt secret.enc -o secret_decrypted.txt
# (Enter same password)

cat secret_decrypted.txt
# Output: Secret message
```

### Example 2: Batch Encryption

```bash
# Create test files
echo "File 1" > file1.txt
echo "File 2" > file2.txt
echo "File 3" > file3.txt

# Encrypt all at once
txt-crypt encrypt *.txt -o encrypted/
# (Enter password once)

# List encrypted files
ls encrypted/
# Output: file1.txt.enc file2.txt.enc file3.txt.enc
```

### Example 3: Directory Processing

```bash
# Encrypt entire directory structure
txt-crypt encrypt documents/ -o encrypted_docs/ --recursive

# Decrypt directory
txt-crypt decrypt encrypted_docs/ -o restored_docs/ --recursive

# Verify structure
diff -r documents/ restored_docs/
```

### Example 4: Scripting Usage

```bash
#!/bin/bash
# Non-interactive encryption for scripts

# Use echo to provide password
echo "mypassword" | txt-crypt encrypt input.txt -o output.enc --no-progress

# Decrypt in script
echo "mypassword" | txt-crypt decrypt output.enc -o decrypted.txt --no-progress
```

## Performance

- **Small files** (< 1MB): Near-instant encryption
- **Large files** (> 100MB): Streaming with minimal memory usage
- **Multi-threading**: Up to 4x speedup on multi-core systems
- **Binary size**: ~250KB (statically linked, no external dependencies)

Typical performance on modern hardware (4-core CPU):
- **AES-256-GCM encryption**: ~500 MB/s per thread
- **Argon2id KDF**: ~100ms (one-time cost per operation)

## Testing

The project includes a comprehensive test suite:

```bash
# Build tests
cmake .. -DBUILD_TESTS=ON
cmake --build .

# Run all tests
./bin/txt_crypt_tests

# Run specific test
./bin/txt_crypt_tests "test_name"
```

Test coverage includes:
- Unit tests for crypto operations
- Integration tests for file operations
- Edge case handling (empty files, large files, etc.)
- Error handling and recovery

## Troubleshooting

### Password Prompts in Scripts

The tool prompts for passwords interactively by default. For scripting:

```bash
# Use pipe to provide password
echo "password" | txt-crypt encrypt file.txt -o file.enc --no-progress
```

### File Permissions

Encrypted files inherit permissions from the original file. To change:

```bash
chmod 600 file.enc  # Owner read/write only
```

### Memory Usage

Memory usage is constant regardless of file size:
- Base overhead: ~5 MB
- Per-thread buffer: ~1 MB
- Total: ~5 MB + (threads × 1 MB)

### Large Files

For very large files (> 10GB), consider:
- Reducing thread count to avoid memory pressure
- Using `--no-progress` for minor performance gain
- Ensuring sufficient disk space for temporary files

## Security Best Practices

1. **Strong Passwords**: Use passwords with at least 12 characters, mixed case, numbers, and symbols
2. **Secure Transmission**: Encrypted files are safe to transmit over insecure channels
3. **Backup Originals**: Keep backups until you verify decryption works
4. **Password Management**: Use a password manager to store encryption passwords
5. **Secure Deletion**: securely delete original files after encryption if needed:
   ```bash
   shred -u original.txt  # Linux
   srm -r original.txt    # macOS
   ```

## Limitations

- **Password-Based Only**: No support for public-key cryptography
- **Single Password**: All files in batch operations use the same password
- **No Compression**: Files are encrypted as-is (compress first if needed)
- **Streaming Only**: Random access to encrypted data is not supported

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## Version History

- **v0.1.0** (2025-04-01)
  - Initial release
  - AES-256-GCM encryption
  - Argon2id key derivation
  - Multi-threaded processing
  - Base64 encoding
  - Streaming architecture

## Support

For issues, questions, or contributions:
- GitHub Issues: https://github.com/yourusername/txt-crypt/issues
- Documentation: https://github.com/yourusername/txt-crypt/wiki

## Acknowledgments

- **Mbed TLS**: Cryptographic library (AES, GCM, SHA-256)
- **Argon2**: Password hashing algorithm
- **CMake**: Build system
- **Catch2**: Testing framework
