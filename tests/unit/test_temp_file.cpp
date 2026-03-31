#include "utils/temp_file.h"
#include <fstream>
#include <cstdio>
#include <cassert>

using namespace txtcrypt;

void test_temp_unique_paths() {
    std::string base = "test_file.txt";
    auto temp1 = TempFile::create(base);
    auto temp2 = TempFile::create(base);

    assert(temp1.path() != temp2.path());
    auto pos = temp1.path().find(".tmp");
    assert(pos != std::string::npos);
    printf("test_temp_unique_paths passed\n");
}

void test_temp_commits_to_original() {
    std::string base = "test_commit.txt";

    {
        auto temp = TempFile::create(base);

        std::ofstream out(temp.path());
        out << "Hello, World!";
        out.close();

        assert(std::filesystem::exists(temp.path()));
        assert(!std::filesystem::exists(base));

        temp.commit();

        assert(!std::filesystem::exists(temp.path()));
        assert(std::filesystem::exists(base));
    }

    std::ifstream in(base);
    std::string content;
    std::getline(in, content);
    assert(content == "Hello, World!");

    std::remove(base.c_str());
    printf("test_temp_commits_to_original passed\n");
}

void test_temp_rollback_removes_temp() {
    std::string base = "test_rollback.txt";

    {
        auto temp = TempFile::create(base);

        std::ofstream out(temp.path());
        out << "Temporary data";
        out.close();

        temp.rollback();

        assert(!std::filesystem::exists(temp.path()));
        assert(!std::filesystem::exists(base));
    }
    printf("test_temp_rollback_removes_temp passed\n");
}

void test_temp_auto_rollback() {
    std::string base = "test_auto_rollback.txt";

    {
        auto temp = TempFile::create(base);

        std::ofstream out(temp.path());
        out << "Auto rollback test";
        out.close();
    }

    assert(!std::filesystem::exists("test_auto_rollback.txt.tmp"));

    // Cleanup any leftover
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (entry.path().string().find("test_auto_rollback") != std::string::npos) {
            std::filesystem::remove(entry.path());
        }
    }
    printf("test_temp_auto_rollback passed\n");
}

void test_temp_backup_creates_copy() {
    std::string base = "test_backup.txt";

    {
        std::ofstream out(base);
        out << "Original content";
    }

    auto backup = TempFile::create_backup(base);

    auto pos = backup.path().find(".bak");
    assert(pos != std::string::npos);

    std::ifstream in(backup.path());
    std::string content;
    std::getline(in, content);
    assert(content == "Original content");

    std::remove(base.c_str());
    std::filesystem::remove(backup.path());
    printf("test_temp_backup_creates_copy passed\n");
}

int main() {
    test_temp_unique_paths();
    test_temp_commits_to_original();
    test_temp_rollback_removes_temp();
    test_temp_auto_rollback();
    test_temp_backup_creates_copy();
    printf("All TempFile tests passed!\n");
    return 0;
}
