#include <iostream>
#include <memory>
#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <algorithm>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <grpc++/grpc++.h>

#include "afs.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using grpc::ServerReader;
using afsgrpc::StringMessage;
using afsgrpc::ReadDirMessage;
using afsgrpc::ReadDirReply;
using afsgrpc::AfsService;
using afsgrpc::GetAttrRequest;
using afsgrpc::GetAttrResponse;
using afsgrpc::MkDirRequest;
using afsgrpc::MkDirResponse;
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

using namespace std;

class AfsServiceImpl final : public AfsService::Service {

  Status SendString(ServerContext *context, const StringMessage *request,
                    StringMessage *reply) override {
    cout << request->stringmessage() << endl;
    string stringToReturn("Your message was: " + request->stringmessage());
    reply->set_stringmessage(stringToReturn);
    return Status::OK;
  }

  Status ReadDir(ServerContext *context, const ReadDirMessage *request,
                 ReadDirReply *reply) override {
    string path = serverpath + request->path();

    DIR *dp;
    struct dirent *de;
    
    dp = opendir(path.c_str());
    if (dp == NULL) return Status::CANCELLED;

    while ((de = readdir(dp)) != NULL) {
      reply->add_inodenumber(de->d_ino);  
      reply->add_type(de->d_type);
      reply->add_name(de->d_name);
    }
    closedir(dp);
    return Status::OK;
  }

  Status GetAttr(ServerContext *context, const GetAttrRequest *request,
                 GetAttrResponse *response) override {
    string path = serverpath + request->path();
    struct stat stbuf;
    int res = lstat(path.c_str(), &stbuf);

    response->set_dev(stbuf.st_dev);
    response->set_ino(stbuf.st_ino);
    response->set_mode(stbuf.st_mode);
    response->set_nlink(stbuf.st_nlink);
    response->set_uid(stbuf.st_uid);
    response->set_gid(stbuf.st_gid);
    response->set_rdev(stbuf.st_rdev);
    response->set_size(stbuf.st_size);
    response->set_atime(stbuf.st_atime);
    response->set_mtime(stbuf.st_mtime);
    response->set_ctime(stbuf.st_ctime);
    response->set_blksize(stbuf.st_blksize);
    response->set_blocks(stbuf.st_blocks);
    response->set_res(res);

    return Status::OK;
  }

  // not working yet
  Status MkDir(ServerContext *context, const MkDirRequest *request,
               MkDirResponse *response) override {
    string path = serverpath + request->path();
    cout << "MKDIR PATH: " << path << endl;
    int res = mkdir(path.c_str(), request->mode());
    response->set_res(res);
    cout << "MKDIR RES: " << res << endl;
    return Status::OK;
  }

  // not working yet
  Status MkNod(ServerContext *context, const MkNodRequest *request,
               MkNodResponse *response) override {
    string path = serverpath + request->path();
    cout << "MKNOD PATH: " << path << endl;
    int res;
    mode_t mode = request->mode();
    dev_t rdev = request->rdev();

    if (S_ISREG(mode)) {
      res = open(path.c_str(), O_CREAT | O_EXCL | O_WRONLY, mode);
      if (res >= 0) res = close(res);
    } else if (S_ISFIFO(mode)) {
      res = mkfifo(path.c_str(), mode);
    } else {
      res = mknod(path.c_str(), mode, rdev);
    }

    response->set_res(res);
    return Status::OK;
  }

  Status ReadFile(ServerContext *context, const ReadRequest *request,
                  ReadResponse *response) override {
    int fd;
    int res;
    string path = serverpath + request->path();

    fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) return Status::CANCELLED;
 
    size_t size = request->size();
    off_t offset = request->offset();
    char buf[size];

    res = pread(fd, &buf, size, offset);
    if (res == -1) return Status::CANCELLED;
    close(fd);

    string returnBuf(buf);
    response->set_buf(returnBuf);
    response->set_res(res);

    return Status::OK;
  }

  Status OpenFile(ServerContext *context, const OpenRequest *request,
                  OpenResponse *response) override {
    int res;
    string path = serverpath + request->path();
    res = open(path.c_str(), request->flags());
    response->set_res(res);
    close(res);
    return Status::OK;
  }

  Status WriteFile(ServerContext *context, const WriteRequest *request,
                   WriteResponse *response) override {
    int fd;
    int res;
    string path = serverpath + request->path();
    fd = open(path.c_str(), O_WRONLY);
    if (fd == -1) return Status::CANCELLED;
 
    string buf = request->buf();
    size_t size = request->size();
    off_t offset = request->offset();
    res = pwrite(fd, buf.c_str(), size, offset);
    close(fd);
    
    response->set_res(res);
    return Status::OK;
  }

  Status AccessFile(ServerContext *context, const AccessRequest *request,
                    AccessResponse *response) override {
    int res;
    string path = serverpath + request->path();
    int mask = request->mask();
    res = access(path.c_str(), mask);
    response->set_res(res);
    return Status::OK;
  }

  public:
    AfsServiceImpl(string path) {
      serverpath = path; 
    }

  private:
    string serverpath;
  
};

void RunServer() {
  string server_address("0.0.0.0:50051");
  AfsServiceImpl service("/home/justin/cs739/p2/afs/serverdir");

  ServerBuilder builder;

  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  builder.RegisterService(&service);

  unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << server_address << endl;
  server->Wait();
}

int main(int argc, char** argv) {
  cout << "Server Running..." << endl;
  RunServer();
  return 0;
}
