#include <algorithm>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

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

using namespace std;

class AfsServiceImpl final : public AfsService::Service {


// Names are same as proto file definitions
// All take a ServerContext: object defined by gRPC
//	* First param is input to RPC, second param is output

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

  // These functions need to put the stuff into stbuf
  // Then we need to ship stuff back to the client
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

  Status MkDir(ServerContext *context, const MkDirRequest *request,
               MkDirResponse *response) override {
    string path = serverpath + request->path();
    int res = mkdir(path.c_str(), request->mode());
    response->set_res(res);
    cout << "MKDIR: " << path << endl;
    return Status::OK;
  }

  Status RmDir(ServerContext *context, const RmDirRequest *request,
               RmDirResponse *response) override {
    string path = serverpath + request->path();
    int res = rmdir(path.c_str());
    response->set_res(res);
    cout << "RMDIR: " << path << endl;
    return Status::OK;
  }

  Status MkNod(ServerContext *context, const MkNodRequest *request,
               MkNodResponse *response) override {
    string path = serverpath + request->path();
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
    cout << "MKNOD: " << path << endl;
    return Status::OK;
  }

  // shouldnt be called anymore
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

  Status GetFile(ServerContext *context, const GetFileRequest *request,
                 GetFileResponse *response) override {
    int fd;
    int res;

    string path = serverpath + request->path();
    fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) return Status::CANCELLED;

    // get the size of the file
    struct stat st;
    stat(path.c_str(), &st);
    size_t size = st.st_size;

    char buf[size];
    // read the entire file
    res = pread(fd, &buf, size, 0); 
    if (res == -1) return Status::CANCELLED;
    close(fd);

    string returnBuf(buf);
    response->set_buf(returnBuf);
    response->set_res(res);
    response->set_size(size);

    return Status::OK;
  }

  // shouldnt be called anymore
  Status OpenFile(ServerContext *context, const OpenRequest *request,
                  OpenResponse *response) override {
    int res;
    string path = serverpath + request->path();
    res = open(path.c_str(), request->flags());
    response->set_res(res);
    close(res);
    return Status::OK;
  }

  // shouldnt be called anymore
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

  Status WriteWholeFile(ServerContext *context, const WriteFileRequest *request,
                        WriteFileResponse *response) override {
    int fd;
    int res;
    string path = serverpath + request->path();
    cout << "File: " + path + " Writing the WHOLE file back to the server. Data: " << request->buf() << endl;
    fd = open(path.c_str(), O_CREAT | O_RDWR | O_TRUNC);
    if (fd == -1) return Status::CANCELLED;

    string buf = request->buf();
    size_t size = request->size();
    res = pwrite(fd, buf.c_str(), size, 0);
    close(fd);

    response->set_res(res);
    return Status::OK;
  }

  // not being called
  Status AccessFile(ServerContext *context, const AccessRequest *request,
                    AccessResponse *response) override {
    int res;
    string path = serverpath + request->path();
    int mask = request->mask();
    res = access(path.c_str(), mask);
    response->set_res(res);
    return Status::OK;
  }

  Status UTime(ServerContext *context, const UTimeRequest *request,
               UTimeResponse *response) override {
    string path = serverpath + request->path();
    int res = utime(path.c_str(), NULL);
    response->set_res(res);
    cout << "UTIME: " << path << endl;
    return Status::OK;
  }

  Status Unlink(ServerContext *context, const UnlinkRequest *request,
               UnlinkResponse *response) override {
    string path = serverpath + request->path();
    int res = unlink(path.c_str());
    response->set_res(res);
    cout << "UNLINK: " << path << endl;
    return Status::OK;
  }

  public:
    AfsServiceImpl(string path) {
      serverpath = path; 
    }

  private:
    string serverpath;
  
};

void RunServer(string &directory) {
  string server_address("0.0.0.0:50051");
  // This is where the files live on the server.
  AfsServiceImpl service(directory);

  ServerBuilder builder;

  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  builder.RegisterService(&service);

  unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << server_address << endl;
  server->Wait();
}

int main(int argc, char** argv) {
  if (argc < 2 || argc > 2) {
    cout << "Invalid number of arguments. Quitting..." << endl;
    exit(0);
  }
  string directory(argv[1]);
  cout << "Directory: " << directory << endl;
  cout << "Server Running..." << endl;
  RunServer(directory);
  return 0;
}
