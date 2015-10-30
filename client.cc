#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>

#include <iostream>
#include <memory>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include <signal.h>

#include <fstream>

#include <grpc++/grpc++.h>
#include "afs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using grpc::ClientWriter;
using afsgrpc::AfsService;
using afsgrpc::StringMessage;
using afsgrpc::ReadDirMessage;
using afsgrpc::ReadDirReply;
using afsgrpc::GetAttrRequest;
using afsgrpc::GetAttrResponse;
using afsgrpc::MkDirRequest;
using afsgrpc::MkDirResponse;
using afsgrpc::RmDirRequest;
using afsgrpc::RmDirResponse;
using afsgrpc::MkNodRequest;
using afsgrpc::MkNodResponse;
using afsgrpc::ReadRequest;
using afsgrpc::ReadResponse;
using afsgrpc::OpenRequest;
using afsgrpc::OpenResponse;
using afsgrpc::WriteRequest;
using afsgrpc::WriteResponse;
using afsgrpc::AccessRequest;
using afsgrpc::AccessResponse;
using afsgrpc::UTimeRequest;
using afsgrpc::UTimeResponse;
using afsgrpc::UnlinkRequest;
using afsgrpc::UnlinkResponse;
using afsgrpc::GetFileRequest;
using afsgrpc::GetFileResponse;
using afsgrpc::WriteFileRequest;
using afsgrpc::WriteFileResponse;
using afsgrpc::RenameRequest;
using afsgrpc::RenameResponse;

using namespace std;

static int file_copy(string sourcePath, string destPath);

// Defines a stub which you call into to call server functions
class AfsClient {
  public:
    AfsClient(shared_ptr<Channel> channel)
      : stub_(AfsService::NewStub(channel)) {}

    string SendString(const string &message) {
      StringMessage request;
      request.set_stringmessage(message);

      StringMessage reply;
      ClientContext context;
      Status status = stub_->SendString(&context, request, &reply);

      if (status.ok()) {
        return reply.stringmessage();
      } else {
        return "RPC returned FAILURE";
      }  
    }

    ReadDirReply ReadDir(const string &path) {
      ReadDirMessage request;
      request.set_path(path);
      
      ReadDirReply reply;
      ClientContext context;
      Status status = stub_->ReadDir(&context, request, &reply);
      if (status.ok()) {
        return reply;
      }
      //return null;
      // TODO: Do something on failure here
      return reply;
    }

    GetAttrResponse GetAttr(const string &path) {
      GetAttrRequest request;
      request.set_path(path);

      GetAttrResponse response;
      ClientContext context;
      Status status = stub_->GetAttr(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    MkDirResponse MkDir(const string &path, mode_t mode) {
      MkDirRequest request;
      request.set_path(path);
      request.set_mode(mode);

      MkDirResponse response;
      ClientContext context;
      Status status = stub_->MkDir(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

	RmDirResponse RmDir(const string &path) {
      RmDirRequest request;
      request.set_path(path);

      RmDirResponse response;
      ClientContext context;
      Status status = stub_->RmDir(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    MkNodResponse MkNod(const string &path, mode_t mode, dev_t rdev) {
      MkNodRequest request;
      request.set_path(path);
      request.set_mode(mode);
      request.set_rdev(rdev);

      MkNodResponse response;
      ClientContext context;
      Status status = stub_->MkNod(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    ReadResponse ReadFile(const string &path, size_t size, off_t offset) {
      ReadRequest request;
      request.set_path(path);
      request.set_size(size);
      request.set_offset(offset);

      ReadResponse response;
      ClientContext context;
      Status status = stub_->ReadFile(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do someting on failure here
      return response;
    }

    GetFileResponse GetFile(const string &path) {
      GetFileRequest request;
      request.set_path(path);

      GetFileResponse response;
      ClientContext context;
      Status status = stub_->GetFile(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do Something on failure here
      return response;
    }

    WriteFileResponse WriteWholeFile(const string &path, string &buf, size_t size) {
      WriteFileRequest request;
      request.set_path(path);
      request.set_buf(buf);
      request.set_size(size);
 
      WriteFileResponse response;
      ClientContext context;
      Status status = stub_->WriteWholeFile(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    OpenResponse OpenFile(const string &path, int flags) {
      OpenRequest request;
      request.set_path(path);
      request.set_flags(flags);

      OpenResponse response;
      ClientContext context;
      Status status = stub_->OpenFile(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    WriteResponse WriteFile(const string &path, string &buf, size_t size, off_t offset) {
      WriteRequest request;
      request.set_path(path);
      request.set_buf(buf);
      request.set_size(size);
      request.set_offset(offset);

      WriteResponse response;
      ClientContext context;
      Status status = stub_->WriteFile(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    AccessResponse AccessFile(const string &path, int mask) {
      AccessRequest request;
      request.set_path(path);
      request.set_mask(mask);

      AccessResponse response;
      ClientContext context;
      Status status = stub_->AccessFile(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    UTimeResponse UTime(const string &path) {
      UTimeRequest request;
      request.set_path(path);

      UTimeResponse response;
      ClientContext context;
      Status status = stub_->UTime(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    UnlinkResponse Unlink(const string &path) {
      UnlinkRequest request;
      request.set_path(path);

      UnlinkResponse response;
      ClientContext context;
      Status status = stub_->Unlink(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    }

    RenameResponse Rename(const string &from, const string &to) {
      RenameRequest request;
      request.set_toname(to);
      request.set_fromname(from);

      RenameResponse response;
      ClientContext context;
      Status status = stub_->RenameFile(&context, request, &response);
      if (status.ok()) {
        return response;
      }
      // TODO: Do something on failure here
      return response;
    } 

  private:
    unique_ptr<AfsService::Stub> stub_;

};


static AfsClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureCredentials()));
static string clientCacheDirectory;
static string clientTmpDirectory;

static int client_getattr(const char *path, struct stat *stbuf) {
  string stringpath(path);
  GetAttrResponse response = client.GetAttr(path);

  stbuf->st_dev = response.dev();
  stbuf->st_ino = response.ino();
  stbuf->st_mode = response.mode();
  stbuf->st_nlink = response.nlink();
  stbuf->st_uid = response.uid();
  stbuf->st_gid = response.gid();
  stbuf->st_rdev = response.rdev();
  stbuf->st_size = response.size();
  stbuf->st_atime = response.atime();
  stbuf->st_mtime = response.mtime();
  stbuf->st_ctime = response.ctime();
  stbuf->st_blksize = response.blksize();
  stbuf->st_blocks = response.blocks();

  int res = response.res();
  if (res == -1) return -ENOENT;

  return 0;
}

static int open_local_file(string &clientPath, int flags) {
  int fd;
  fd = open(clientPath.c_str(), flags, 0666); 
  return fd;
}

static int read_whole_file(int fd, char *buf, size_t size) {
  int res = pread(fd, buf, size, 0);
  return res;
}

static int write_new_file(int fd, string &buf, size_t size) {
  int res = pwrite(fd, buf.c_str(), size, 0);
  return res;
}

static int client_release(const char *path, struct fuse_file_info *fi) {
  string stringpath(path);
  string clientPath = clientCacheDirectory + stringpath;

  struct stat st;
  if (stat(clientPath.c_str(), &st) == -1) return -errno;
  size_t size = st.st_size;
  GetAttrResponse attrResponse = client.GetAttr(stringpath);
  int res = attrResponse.res();
  if (res == -1) return -errno;
  //if (attrResponse.mtime() < st.st_mtime) {
  client.SendString("LOCAL FILE: " + stringpath + " atime: " + to_string(st.st_atime) + " mtime: " + to_string(st.st_mtime) + " ctime: " + to_string(st.st_ctime));
  client.SendString("SERVER FILE: " + stringpath + " atime: " + to_string(attrResponse.atime()) + " mtime: " + to_string(attrResponse.mtime()) + " ctime: " + to_string(attrResponse.ctime()));
  //client.SendString("FILE: " + stringpath + " write: " + to_string(fi->flush));
  if (st.st_mtime > st.st_atime) {
    int fd = open_local_file(clientPath, O_RDONLY);
    if (fd == -1) return -errno;
  
    char buf[size];
    res = pread(fd, &buf, size, 0); 
    if (res == -1) return -errno;
    close(fd);

    string data = string(buf);
    client.SendString("File: " + stringpath + " Updating file on server.");
    WriteFileResponse response = client.WriteWholeFile(stringpath, data, size);
    res = response.res();
    if (res == -1) return -errno;
  } else {
    client.SendString("File: " + stringpath + " Fsync called, but the timestamp says not to update the file.");
  }
  return 0;
}


static int client_open(const char *path, struct fuse_file_info *fi) {
  string stringpath = string(path);
  string clientPath = clientCacheDirectory + string(path);
  // test if the file is cached
  ifstream infile(clientPath);
  if (infile.good()) {
    // the file is cached
    struct stat statbuf;
    if (stat(clientPath.c_str(), &statbuf) == -1) return -errno;
    GetAttrResponse attrResponse = client.GetAttr(stringpath);
    if (attrResponse.res() == -1) return -errno;
    if (attrResponse.mtime() > statbuf.st_mtime) {
      // need to get server copy
      client.SendString("File: " + string(path) + " UPDATED. Getting new copy.");
      GetFileResponse response = client.GetFile(string(path));
      int res = response.res();
      if (res == -1) return -errno;
      
      int fd = open_local_file(clientPath, O_CREAT | O_RDWR | O_TRUNC);
      if (fd == -1) return -errno;

      string buf = response.buf();
      size_t size = response.size();
      int clientRes = write_new_file(fd, buf, size);
      if (clientRes == -1) return -errno;
      close(fd);
      return 0;
    } else {
      // safe to use cached copy
      int fd = open_local_file(clientPath, O_RDWR);
      if (fd == -1) return -errno;
      close(fd);
      client.SendString("File: " + string(path) + " Using cached copy! PATH: " + clientPath);
      return 0;
    }
  } else {
    // get file from server
    GetFileResponse response = client.GetFile(string(path));
    int res = response.res();
    if (res == -1) return -errno; 

    // open local copy and write to it
    int fd = open_local_file(clientPath, O_CREAT | O_RDWR | O_TRUNC);
    if (fd == -1) return -errno;
    //write new data
    string buf = response.buf();
    size_t size = response.size();
    int clientRes = write_new_file(fd, buf, size);
    if (clientRes == -1) clientRes = -errno;
    close(fd);
    client.SendString("File: " + string(path) + " FIRST. Getting file from server");
    return 0;
  }

  return -1;
}

static int client_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi) {
  int fd;
  int res;

  (void) fi;
  string clientPath = clientCacheDirectory + string(path);
  fd = open(clientPath.c_str(), O_RDONLY);
  if (fd == -1) return -errno;

  res = pread(fd, buf, size, offset);
  if (res == -1) res = -errno;
  close(fd);
  return res;
}

static int client_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi) {
  string stringPath(path);
  ReadDirReply reply = client.ReadDir(stringPath);
  (void) offset;
  (void) fi;


  for (int i = 0; i < reply.inodenumber().size(); i++) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = reply.inodenumber().Get(i);
    st.st_mode = reply.type().Get(i) << 12;
    string name = reply.name().Get(i);
    if (filler(buf, name.c_str(), &st, 0)) break;
  }

  return 0;
}

static int client_mknod(const char *path, mode_t mode, dev_t rdev) {
  string stringpath(path);
  MkNodResponse response = client.MkNod(stringpath, mode, rdev);

  int res = response.res();
  if (res == -1) return -errno;

  return 0; 
}

static int client_mkdir(const char *path, mode_t mode) {
  string stringpath(path);
  MkDirResponse response = client.MkDir(stringpath, mode);

  int res = response.res();
  if (res == -1) return -errno;
  
  return 0;
}

static int client_rmdir(const char *path) {
  string stringpath(path);
  RmDirResponse response = client.RmDir(stringpath);

  int res = response.res();
  if (res == -1) return -errno;
 
  return 0;
}

static int client_write(const char *path, const char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi) {
  // TODO: Write all data to an intermediate file
  // when the file is released, and the data has successfully been written
  // back to the server, atomically rename it to the correct name
  int fd;
  int file_size;
  int res;
  int tmp_fd;
  
  string clientCachePath = clientCacheDirectory + string(path);
  string clientTmpPath = clientTmpDirectory + string(path);

  (void) fi;

  // First, copy the cached source file into the tmp directory
  file_copy(clientCachePath, clientTmpPath);
  
  // Write to the tmp file
  tmp_fd = open(clientTmpPath.c_str(), O_CREAT | O_WRONLY);  
  if (tmp_fd == -1) return -errno;
  res = pwrite(tmp_fd, buf, size, offset);
  if (res == -1) res = -errno;
  close(tmp_fd);
  
  // If the operation succeeds, (atomically) rename the tmp file back to the cache directory
  rename(clientTmpPath.c_str(), clientCachePath.c_str());

  //kill(getpid(), SIGKILL);
  return res;
}

static int client_access(const char *path, int mask) {
  string stringpath(path);
  AccessResponse response = client.AccessFile(stringpath, mask);
  int res = response.res();
  if (res == -1) return -errno;
  return 0; 
}

static int client_utime(const char *path, utimbuf *time) {
  string stringpath(path);
  UTimeResponse response = client.UTime(stringpath);
  
  int res = response.res();
  if (res == -1) return -errno;

  // also utime on the client
  string clientPath = clientPath + stringpath;
  res = utime(clientPath.c_str(), NULL);
  if (res == -1) return -errno;

  return 0;
}

static int client_unlink(const char *path) {
  string stringpath(path);
  UnlinkResponse response = client.Unlink(stringpath);
  
  int res = response.res();
  if (res == -1) return -errno;

  // also unlink it on the client
  string clientPath = clientCacheDirectory + stringpath;
  res = unlink(clientPath.c_str());
  if (res == -1) return -errno;

  return 0;
}

static int client_truncate(const char *path, off_t size) {
  int res;
  string clientPath = clientCacheDirectory + string(path);
  res = truncate(clientPath.c_str(), size);
  if (res == -1) return -errno;
  return 0;
}

static int client_rename(const char *from, const char *to) {
  string frompath(from);
  string topath(to);
  RenameResponse response = client.Rename(frompath, topath);
  int res = response.res();
  if (res == -1) return -errno;

  return 0; 
}

/*
 *  Utility function: copies a file.
 */
static int file_copy(string sourcePath, string destPath) {
  char buf[1];
  size_t fileSize;

  FILE* source = fopen(sourcePath.c_str(), "rb");
  FILE* dest = fopen(destPath.c_str(), "wb");

  while (fileSize = fread(buf, 1, 1, source)) {
    client.SendString("Copying a byte to the temp folder");
    fwrite(buf, 1, fileSize, dest);
  }

  fclose(source);
  fclose(dest);
}


// All these attributes must appear here in this exact order!
static struct fuse_operations client_oper = {
  getattr: client_getattr,
  readlink: NULL,
  getdir: NULL,
  mknod: client_mknod,
  mkdir: client_mkdir,
  unlink: client_unlink,
  rmdir: client_rmdir,
  symlink: NULL,
  rename: client_rename,
  link: NULL,
  chmod: NULL,
  chown: NULL,
  truncate: client_truncate,
  utime: client_utime,
  open: client_open,
  read: client_read,
  write: client_write,
  statfs: NULL,
  flush: NULL,
  release: client_release,
  fsync: NULL,
  setxattr: NULL,
  getxattr: NULL,
  listxattr: NULL,
  removexattr: NULL,
  opendir: NULL,
  readdir: client_readdir,
  releasedir: NULL,
  fsyncdir: NULL,
  init: NULL,
  destroy: NULL,
  access: NULL,
};

int main(int argc, char *argv[]) { 
  if (argc < 3 || argc > 3) {
    cout << "Invalid number of arguments. Quitting..." << endl;
    cout << "Third argument must be an absolute path." << endl;
    exit(0);
  }

  char *args[argc];
  for (int i = 0; i < argc - 1; i++) {
    int length = strlen(argv[i]);
    args[i] = new char[length + 1];
    strncpy(args[i], argv[i], length);
    args[i][length] = '\0';
  }

  //clientCacheDirectory = "/home/justin/cs739/p2/afs/clientDir";
  clientCacheDirectory = argv[argc - 1];
  clientTmpDirectory = "/tmp";

  return fuse_main(argc - 1, args, &client_oper, NULL);
  //return 0;
}
