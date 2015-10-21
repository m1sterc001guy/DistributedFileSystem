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

using namespace std;

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

  private:
    unique_ptr<AfsService::Stub> stub_;

};


static AfsClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureCredentials()));

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

static int client_open(const char *path, struct fuse_file_info *fi) {
  string stringpath(path);
  OpenResponse response = client.OpenFile(stringpath, fi->flags);
  int res = response.res();
  if (res == -1) return -errno;
  return 0;
}

static int client_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi) {
  (void) fi;

  string stringpath(path);
  ReadResponse response = client.ReadFile(stringpath, size, offset);
  int res = response.res();
  // TODO: Do something with res here
  string data = response.buf();
  strcpy(buf, data.c_str());
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
  (void) fi;
  string stringpath(path);
  string stringbuf(buf);
  WriteResponse response = client.WriteFile(stringpath, stringbuf, size, offset);
  int res = response.res();
  if (res == -1) return -errno;
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
 
  return 0;
}

static int client_unlink(const char *path) {
  string stringpath(path);
  UnlinkResponse response = client.Unlink(stringpath);

  int res = response.res();
  if (res == -1) return -errno;
 
  return 0;
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
  rename: NULL,
  link: NULL,
  chmod: NULL,
  chown: NULL,
  truncate: NULL,
  utime: client_utime,
  open: client_open,
  read: client_read,
  write: client_write,
  statfs: NULL,
  flush: NULL,
  release: NULL,
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
  //string message = client.SendString("Hello, World!\n");
  return fuse_main(argc, argv, &client_oper, NULL);
  //struct stat stbuf;
  //client_mkdir("/testdir", 16893);
  //client_utime("/testing.txt", NULL);
  //return 0;
}
