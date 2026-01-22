#ifndef THREADSAFE_FS_WIN_HPP
#define THREADSAFE_FS_WIN_HPP

#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <ctime>
#include <cstring>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <fuse.h>  // WinFsp 


#ifdef _WIN32
    #include <io.h>
    #include <direct.h>
    #define S_IFREG _S_IFREG
    #define S_IFDIR _S_IFDIR
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
    #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
    

    #define S_IRUSR 0400
    #define S_IWUSR 0200
    #define S_IXUSR 0100
    #define S_IRGRP 0040
    #define S_IWGRP 0020
    #define S_IXGRP 0010
    #define S_IROTH 0004
    #define S_IWOTH 0002
    #define S_IXOTH 0001
    
    #define O_RDONLY _O_RDONLY
    #define O_WRONLY _O_WRONLY
    #define O_RDWR   _O_RDWR
    #define O_CREAT  _O_CREAT
    #define O_TRUNC  _O_TRUNC
    #define O_APPEND _O_APPEND
#endif


template<typename T>
class Singleton {
public:
    static T& instance() {
        static T instance;
        return instance;
    }
    
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    
protected:
    Singleton() = default;
    virtual ~Singleton() = default;
};

template<typename Key, typename Value>
class ThreadSafeMap {
private:
    std::unordered_map<Key, Value> data_;
    mutable std::shared_mutex mutex_;
    
public:
    bool contains(const Key& key) const;
    Value get(const Key& key) const;
    void set(const Key& key, const Value& value);
    bool remove(const Key& key);
    std::vector<Key> keys() const;
    size_t size() const;
    void clear();
};


struct Inode {
    enum class Type { FILE, DIRECTORY, SYMLINK };
    
    Type type;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    size_t size;
    time_t atime;
    time_t mtime;
    time_t ctime;
    nlink_t nlink;
    
    std::vector<char> data;
    std::vector<std::string> entries;
    
    Inode(Type t = Type::FILE, mode_t m = 0644);
};


class ThreadSafeFS_Win : public Singleton<ThreadSafeFS_Win> {
    friend class Singleton<ThreadSafeFS_Win>;
    
private:
    ThreadSafeMap<std::string, Inode> inodes_;
    std::atomic<uint64_t> next_inode_{2};
    ThreadSafeMap<std::string, std::shared_ptr<std::mutex>> file_locks_;
    mutable std::shared_mutex global_mutex_;
    
    ThreadSafeFS_Win();
    void update_times(const std::string& path, bool access = false, bool modify = false);
    
public:
    bool create_file(const std::string& path, const std::string& content, mode_t mode = 0644);
    bool create_directory(const std::string& path, mode_t mode = 0755);
    Inode getattr(const std::string& path);
    std::vector<std::string> readdir(const std::string& path);
    std::vector<char> read(const std::string& path, size_t size, off_t offset);
    size_t write(const std::string& path, const char* data, size_t size, off_t offset);
    bool truncate(const std::string& path, off_t length);
    bool unlink(const std::string& path);
    bool rmdir(const std::string& path);
    
    static std::string parent_path(const std::string& path);
    static std::string base_name(const std::string& path);
    
    
    bool create_windows_shortcut(const std::string& path, const std::string& target);
};


class FUSEAdapter_Win {
private:
    static FUSEAdapter_Win* instance_;
    ThreadSafeFS_Win& fs_;
    
    static int wrap_exception(std::function<int()> func);
    
public:
    FUSEAdapter_Win(ThreadSafeFS_Win& fs);
    
    static int fuse_getattr(const char* path, struct fuse_stat* stbuf);
    static int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                           fuse_off_t offset, struct fuse_file_info* fi);
    static int fuse_open(const char* path, struct fuse_file_info* fi);
    static int fuse_read(const char* path, char* buf, size_t size, fuse_off_t offset,
                        struct fuse_file_info* fi);
    static int fuse_write(const char* path, const char* buf, size_t size,
                         fuse_off_t offset, struct fuse_file_info* fi);
    static int fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi);
    static int fuse_mkdir(const char* path, mode_t mode);
    static int fuse_unlink(const char* path);
    static int fuse_rmdir(const char* path);
    static int fuse_truncate(const char* path, fuse_off_t length);
    static int fuse_utimens(const char* path, const struct fuse_timespec ts[2]);
    static int fuse_statfs(const char* path, struct fuse_statvfs* stbuf);
    
    static struct fuse_operations get_operations();
};

#endif // THREADSAFE_FS_WIN_HPP
