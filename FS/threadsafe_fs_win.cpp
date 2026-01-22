#include "threadsafe_fs_win.hpp"
#include <algorithm>
#include <stdexcept>


template<typename Key, typename Value>
bool ThreadSafeMap<Key, Value>::contains(const Key& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return data_.find(key) != data_.end();
}

template<typename Key, typename Value>
Value ThreadSafeMap<Key, Value>::get(const Key& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it != data_.end()) {
        return it->second;
    }
    return Value{};
}

template<typename Key, typename Value>
void ThreadSafeMap<Key, Value>::set(const Key& key, const Value& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_[key] = value;
}

template<typename Key, typename Value>
bool ThreadSafeMap<Key, Value>::remove(const Key& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return data_.erase(key) > 0;
}

template<typename Key, typename Value>
std::vector<Key> ThreadSafeMap<Key, Value>::keys() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<Key> result;
    for (const auto& [key, _] : data_) {
        result.push_back(key);
    }
    return result;
}

template<typename Key, typename Value>
size_t ThreadSafeMap<Key, Value>::size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return data_.size();
}

template<typename Key, typename Value>
void ThreadSafeMap<Key, Value>::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_.clear();
}


Inode::Inode(Type t, mode_t m) 
    : type(t), mode(m), uid(0), gid(0),  // Windows: uid/gid = 0
      size(0), atime(time(nullptr)), 
      mtime(time(nullptr)), ctime(time(nullptr)),
      nlink(t == Type::DIRECTORY ? 2 : 1) {}


ThreadSafeFS_Win::ThreadSafeFS_Win() {

    Inode root(Inode::Type::DIRECTORY, 0755);
    root.entries = {".", ".."};
    root.nlink = 2;
    inodes_.set("/", root);
    
 
    create_file("/hello.txt", 
        "Hello from ThreadSafeFS on Windows!\n"
        "This filesystem runs natively on Windows using WinFsp.\n", 
        0644);
        
    create_file("/readme.txt",
        "Thread-Safe In-Memory Filesystem for Windows\n"
        "Features:\n"
        "1. Multi-threaded access\n"
        "2. Template-based design\n"
        "3. Full Windows support\n"
        "4. POSIX-like API\n", 
        0644);
        
    create_directory("/tmp", 01777);
    create_directory("/Users", 0755);
    create_directory("/Users/Public", 0755);
    create_file("/Users/Public/welcome.txt", 
        "Welcome to the public folder!\n", 
        0644);
        

    create_file("/desktop.ini", "[.ShellClassInfo]\n", 0444);
    create_directory("/ProgramData", 0755);
}

void ThreadSafeFS_Win::update_times(const std::string& path, bool access, bool modify) {
    auto inode = inodes_.get(path);
    time_t now = time(nullptr);
    
    if (access) inode.atime = now;
    if (modify) {
        inode.mtime = now;
        inode.ctime = now;
    }
    
    inodes_.set(path, inode);
}

bool ThreadSafeFS_Win::create_file(const std::string& path, const std::string& content, mode_t mode) {
    std::unique_lock<std::shared_mutex> lock(global_mutex_);
    
  
    if (path.find_first_of(":\\*?\"<>|") != std::string::npos) {
        return false;
    }
    
    if (inodes_.contains(path)) {
        return false;
    }
    
    std::string parent = parent_path(path);
    if (!parent.empty() && !inodes_.contains(parent)) {
        return false;
    }
    
    Inode inode(Inode::Type::FILE, mode | S_IFREG);
    inode.data.assign(content.begin(), content.end());
    inode.size = content.size();
    
    inodes_.set(path, inode);
    
    if (!parent.empty()) {
        auto parent_inode = inodes_.get(parent);
        parent_inode.entries.push_back(base_name(path));
        inodes_.set(parent, parent_inode);
        update_times(parent, false, true);
    }
    
    return true;
}

bool ThreadSafeFS_Win::create_directory(const std::string& path, mode_t mode) {
    std::unique_lock<std::shared_mutex> lock(global_mutex_);
    
    if (inodes_.contains(path)) {
        return false;
    }
    
    std::string parent = parent_path(path);
    if (!parent.empty() && !inodes_.contains(parent)) {
        return false;
    }
    
    Inode inode(Inode::Type::DIRECTORY, mode | S_IFDIR);
    inode.entries = {".", ".."};
    inode.nlink = 2;
    
    inodes_.set(path, inode);
    
    if (!parent.empty()) {
        auto parent_inode = inodes_.get(parent);
        parent_inode.nlink++;
        parent_inode.entries.push_back(base_name(path));
        inodes_.set(parent, parent_inode);
        update_times(parent, false, true);
    }
    
    return true;
}

Inode ThreadSafeFS_Win::getattr(const std::string& path) {
    std::shared_lock<std::shared_mutex> lock(global_mutex_);
    
    if (inodes_.contains(path)) {
        auto inode = inodes_.get(path);
        update_times(path, true, false);
        return inode;
    }
    
    throw std::runtime_error("ENOENT");
}

std::vector<std::string> ThreadSafeFS_Win::readdir(const std::string& path) {
    std::shared_lock<std::shared_mutex> lock(global_mutex_);
    
    if (inodes_.contains(path)) {
        auto inode = inodes_.get(path);
        if (inode.type == Inode::Type::DIRECTORY) {
            update_times(path, true, false);
            return inode.entries;
        }
    }
    
    throw std::runtime_error("ENOTDIR");
}

std::vector<char> ThreadSafeFS_Win::read(const std::string& path, size_t size, off_t offset) {
    auto lock_ptr = file_locks_.get(path);
    if (!lock_ptr) {
        lock_ptr = std::make_shared<std::mutex>();
        file_locks_.set(path, lock_ptr);
    }
    std::lock_guard<std::mutex> lock(*lock_ptr);
    
    std::shared_lock<std::shared_mutex> global_lock(global_mutex_);
    
    if (inodes_.contains(path)) {
        auto inode = inodes_.get(path);
        if (inode.type != Inode::Type::FILE) {
            throw std::runtime_error("EISDIR");
        }
        
        if (offset >= inode.size) {
            return {};
        }
        
        size_t read_size = (size < inode.size - offset) ? size : inode.size - offset;
        std::vector<char> result(
            inode.data.begin() + offset,
            inode.data.begin() + offset + read_size
        );
        
        update_times(path, true, false);
        return result;
    }
    
    throw std::runtime_error("ENOENT");
}

size_t ThreadSafeFS_Win::write(const std::string& path, const char* data, size_t size, off_t offset) {
    auto lock_ptr = file_locks_.get(path);
    if (!lock_ptr) {
        lock_ptr = std::make_shared<std::mutex>();
        file_locks_.set(path, lock_ptr);
    }
    std::lock_guard<std::mutex> lock(*lock_ptr);
    
    std::unique_lock<std::shared_mutex> global_lock(global_mutex_);
    
    if (inodes_.contains(path)) {
        auto inode = inodes_.get(path);
        if (inode.type != Inode::Type::FILE) {
            throw std::runtime_error("EISDIR");
        }
        
        if (offset + size > inode.data.size()) {
            inode.data.resize(offset + size);
        }
        
        std::copy(data, data + size, inode.data.begin() + offset);
        inode.size = inode.data.size();
        
        inodes_.set(path, inode);
        update_times(path, false, true);
        
        return size;
    }
    
    throw std::runtime_error("ENOENT");
}

bool ThreadSafeFS_Win::truncate(const std::string& path, off_t length) {
    std::unique_lock<std::shared_mutex> lock(global_mutex_);
    
    if (inodes_.contains(path)) {
        auto inode = inodes_.get(path);
        if (inode.type != Inode::Type::FILE) {
            return false;
        }
        
        inode.data.resize(length);
        inode.size = length;
        
        inodes_.set(path, inode);
        update_times(path, false, true);
        
        return true;
    }
    
    return false;
}

bool ThreadSafeFS_Win::unlink(const std::string& path) {
    std::unique_lock<std::shared_mutex> lock(global_mutex_);
    
    if (!inodes_.contains(path)) {
        return false;
    }
    
    auto inode = inodes_.get(path);
    if (inode.type != Inode::Type::FILE) {
        return false;
    }
    
    std::string parent = parent_path(path);
    if (!parent.empty() && inodes_.contains(parent)) {
        auto parent_inode = inodes_.get(parent);
        auto& entries = parent_inode.entries;
        std::string filename = base_name(path);
        
        entries.erase(
            std::remove(entries.begin(), entries.end(), filename),
            entries.end()
        );
        
        inodes_.set(parent, parent_inode);
        update_times(parent, false, true);
    }
    
    inodes_.remove(path);
    file_locks_.remove(path);
    
    return true;
}

bool ThreadSafeFS_Win::rmdir(const std::string& path) {
    std::unique_lock<std::shared_mutex> lock(global_mutex_);
    
    if (!inodes_.contains(path)) {
        return false;
    }
    
    auto inode = inodes_.get(path);
    if (inode.type != Inode::Type::DIRECTORY) {
        return false;
    }
    
    if (inode.entries.size() > 2) {
        return false;
    }
    
    std::string parent = parent_path(path);
    if (!parent.empty() && inodes_.contains(parent)) {
        auto parent_inode = inodes_.get(parent);
        parent_inode.nlink--;
        
        auto& entries = parent_inode.entries;
        std::string dirname = base_name(path);
        
        entries.erase(
            std::remove(entries.begin(), entries.end(), dirname),
            entries.end()
        );
        
        inodes_.set(parent, parent_inode);
        update_times(parent, false, true);
    }
    
    inodes_.remove(path);
    
    return true;
}

std::string ThreadSafeFS_Win::parent_path(const std::string& path) {
    if (path == "/" || path.empty()) return "";
    
    size_t pos = path.find_last_of("/\\");
    if (pos == 0) return "/";
    if (pos != std::string::npos) {
        return path.substr(0, pos);
    }
    return "";
}

std::string ThreadSafeFS_Win::base_name(const std::string& path) {
    if (path == "/" || path.empty()) return "";
    
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

bool ThreadSafeFS_Win::create_windows_shortcut(const std::string& path, const std::string& target) {
    std::string content = "[InternetShortcut]\nURL=" + target + "\n";
    return create_file(path + ".url", content, 0644);
}


FUSEAdapter_Win* FUSEAdapter_Win::instance_ = nullptr;

int FUSEAdapter_Win::wrap_exception(std::function<int()> func) {
    try {
        return func();
    } catch (const std::runtime_error& e) {
        if (strcmp(e.what(), "ENOENT") == 0) return -ENOENT;
        if (strcmp(e.what(), "EISDIR") == 0) return -EISDIR;
        if (strcmp(e.what(), "ENOTDIR") == 0) return -ENOTDIR;
        if (strcmp(e.what(), "EACCES") == 0) return -EACCES;
        return -EIO;
    } catch (...) {
        return -EIO;
    }
}

FUSEAdapter_Win::FUSEAdapter_Win(ThreadSafeFS_Win& fs) : fs_(fs) {
    instance_ = this;
}

int FUSEAdapter_Win::fuse_getattr(const char* path, struct fuse_stat* stbuf) {
    return wrap_exception([path, stbuf]() {
        memset(stbuf, 0, sizeof(struct fuse_stat));
        
        auto inode = instance_->fs_.getattr(path);
        
        stbuf->st_mode = inode.mode;
        stbuf->st_nlink = inode.nlink;
        stbuf->st_size = inode.size;
        stbuf->st_uid = inode.uid;
        stbuf->st_gid = inode.gid;
        stbuf->st_atime = inode.atime;
        stbuf->st_mtime = inode.mtime;
        stbuf->st_ctime = inode.ctime;
        
        return 0;
    });
}

int FUSEAdapter_Win::fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                                 fuse_off_t offset, struct fuse_file_info* fi) {
    (void)offset;
    (void)fi;
    
    return wrap_exception([path, buf, filler]() {
        auto entries = instance_->fs_.readdir(path);
        
        filler(buf, ".", NULL, 0, fuse_fill_dir_flags(0));
        filler(buf, "..", NULL, 0, fuse_fill_dir_flags(0));
        
        for (const auto& entry : entries) {
            if (entry != "." && entry != "..") {
                filler(buf, entry.c_str(), NULL, 0, fuse_fill_dir_flags(0));
            }
        }
        
        return 0;
    });
}

int FUSEAdapter_Win::fuse_open(const char* path, struct fuse_file_info* fi) {
    return wrap_exception([path, fi]() {
        auto inode = instance_->fs_.getattr(path);
        
        if ((fi->flags & O_ACCMODE) == O_RDONLY) {
            if (!(inode.mode & S_IRUSR)) return -EACCES;
        }
        if ((fi->flags & O_ACCMODE) == O_WRONLY || 
            (fi->flags & O_ACCMODE) == O_RDWR) {
            if (!(inode.mode & S_IWUSR)) return -EACCES;
        }
        
        return 0;
    });
}

int FUSEAdapter_Win::fuse_read(const char* path, char* buf, size_t size, fuse_off_t offset,
                              struct fuse_file_info* fi) {
    (void)fi;
    
    return wrap_exception([path, buf, size, offset]() {
        auto data = instance_->fs_.read(path, size, offset);
        if (data.size() > size) {
            return -EIO;
        }
        memcpy(buf, data.data(), data.size());
        return (int)data.size();
    });
}

int FUSEAdapter_Win::fuse_write(const char* path, const char* buf, size_t size,
                               fuse_off_t offset, struct fuse_file_info* fi) {
    (void)fi;
    
    return wrap_exception([path, buf, size, offset]() {
        int written = (int)instance_->fs_.write(path, buf, size, offset);
        return written;
    });
}

int FUSEAdapter_Win::fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    return wrap_exception([path, mode, fi]() {
        (void)fi;
        
        if (instance_->fs_.create_file(path, "", mode)) {
            return 0;
        }
        return -EEXIST;
    });
}

int FUSEAdapter_Win::fuse_mkdir(const char* path, mode_t mode) {
    return wrap_exception([path, mode]() {
        if (instance_->fs_.create_directory(path, mode)) {
            return 0;
        }
        return -EEXIST;
    });
}

int FUSEAdapter_Win::fuse_unlink(const char* path) {
    return wrap_exception([path]() {
        if (instance_->fs_.unlink(path)) {
            return 0;
        }
        return -ENOENT;
    });
}

int FUSEAdapter_Win::fuse_rmdir(const char* path) {
    return wrap_exception([path]() {
        if (instance_->fs_.rmdir(path)) {
            return 0;
        }
        return -ENOTEMPTY;
    });
}

int FUSEAdapter_Win::fuse_truncate(const char* path, fuse_off_t length) {
    return wrap_exception([path, length]() {
        if (instance_->fs_.truncate(path, length)) {
            return 0;
        }
        return -ENOENT;
    });
}

int FUSEAdapter_Win::fuse_utimens(const char* path, const struct fuse_timespec ts[2]) {
 
    (void)path;
    (void)ts;
    return 0;
}

int FUSEAdapter_Win::fuse_statfs(const char* path, struct fuse_statvfs* stbuf) {
    (void)path;
    
    memset(stbuf, 0, sizeof(struct fuse_statvfs));
    stbuf->f_bsize = 4096;      // размер блока
    stbuf->f_frsize = 4096;     // размер фрагмента
    stbuf->f_blocks = 1024*1024;// количество блоков
    stbuf->f_bfree = 1024*1024; // свободные блоки
    stbuf->f_bavail = 1024*1024;// доступные блоки
    stbuf->f_files = 1000000;   // макс количество файлов
    stbuf->f_ffree = 999000;    // свободные inodes
    stbuf->f_favail = 999000;   // доступные inodes
    stbuf->f_namemax = 255;     // макс длина имени
    
    return 0;
}

struct fuse_operations FUSEAdapter_Win::get_operations() {
    struct fuse_operations ops = {0};
    
    ops.getattr = fuse_getattr;
    ops.readdir = fuse_readdir;
    ops.open = fuse_open;
    ops.read = fuse_read;
    ops.write = fuse_write;
    ops.create = fuse_create;
    ops.mkdir = fuse_mkdir;
    ops.unlink = fuse_unlink;
    ops.rmdir = fuse_rmdir;
    ops.truncate = fuse_truncate;
    ops.utimens = fuse_utimens;
    ops.statfs = fuse_statfs;
    
    return ops;
}
