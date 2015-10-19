#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
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

using namespace std;

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

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

  private:
    unique_ptr<AfsService::Stub> stub_;

};


static AfsClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureCredentials()));

static int client_getattr(const char *path, struct stat *stbuf) {
  //client.SendString("getattr was called!");
  //cout << "getattr was called!" << endl;
  string stringpath(path);
  GetAttrResponse response = client.GetAttr(path);

  /*
  cout << "st_dev: " << response.dev() << endl;
  cout << "st_ino: " << response.ino() << endl;
  cout << "st_mode: " << response.mode() << endl;
  cout << "st_nlink: " << response.nlink() << endl;
  cout << "st_uid: " << response.uid() << endl;
  cout << "st_gid: " << response.gid() << endl;
  cout << "st_rdev: " << response.rdev() << endl;
  cout << "st_size: " << response.size() << endl;
  cout << "st_atime: " << response.atime() << endl;
  cout << "st_mtime: " << response.mtime() << endl;
  cout << "st_ctime: " << response.ctime() << endl;
  cout << "st_blksize: " << response.blksize() << endl;
  cout << "st_blocks: " << response.blocks() << endl;
  */

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
  if (res == -1) return -errno;

  return 0;
}

static int client_open(const char *path, struct fuse_file_info *fi) {
  printf("Open Called!\n");
  if (strcmp(path, hello_path) != 0)
    return -ENOENT;

  if ((fi->flags & 3) != O_RDONLY)
    return -EACCES;

  return 0;
}

static int client_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi) {
  	printf("read called!\n");
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

	return size;

}

static int client_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
  //client.SendString("readdir was called!");
  string stringPath(path);
  ReadDirReply reply = client.ReadDir(stringPath);
  (void) offset;
  (void) fi;


  for (int i = 0; i < reply.inodenumber().size(); i++) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    //ofs << "inode: " << reply.inodenumber().Get(i) << endl;
    //ofs << "type: " << reply.type().Get(i) << endl;
    //ofs << "name: " << reply.name().Get(i) << endl;
    st.st_ino = reply.inodenumber().Get(i);
    st.st_mode = reply.type().Get(i) << 12;
    string name = reply.name().Get(i);
    if (filler(buf, name.c_str(), &st, 0)) break;
  }

  //ofs.close();

  //if (strcmp(path, "/") != 0)
  //  return -ENOENT;

  //filler(buf, ".", NULL, 0);
  //filler(buf, "..", NULL, 0);
  //filler(buf, hello_path + 1, NULL, 0);

  return 0;
}

static struct fuse_operations client_oper = {
  getattr: client_getattr,
  readlink: NULL,
  getdir: NULL,
  mknod: NULL,
  mkdir: NULL,
  unlink: NULL,
  rmdir: NULL,
  symlink: NULL,
  rename: NULL,
  link: NULL,
  chmod: NULL,
  chown: NULL,
  truncate: NULL,
  utime: NULL,
  open: client_open,
  read: client_read,
  write: NULL,
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
};

int main(int argc, char *argv[]) { 
  //string message = client.SendString("Hello, World!\n");
  return fuse_main(argc, argv, &client_oper, NULL);
  //struct stat stbuf;
  //client_getattr("/", &stbuf);
  //return 0;
}
