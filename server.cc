#include <iostream>
#include <memory>
#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <algorithm>

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
    //this path isnt correct, needs to be appended to root of server directory
    string path = serverpath + request->path();
    cout << "READDIR PATH: " << path << endl;
    //string path = "/home/justin/cs739/p2/afs/serverdir";

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
    cout << "GetAttr called!!" << endl;
    string path = serverpath + request->path();
    cout << "GETATTR PATH: " << path << endl;
    struct stat stbuf;
    int res = lstat(path.c_str(), &stbuf);
    
    cout << "st_dev: " << stbuf.st_dev << endl;
    cout << "st_ino: " << stbuf.st_ino << endl;
    cout << "st_mode: " << stbuf.st_mode << endl;
    cout << "st_nlink: " << stbuf.st_nlink << endl;
    cout << "st_uid: " << stbuf.st_uid << endl;
    cout << "st_gid: " << stbuf.st_gid << endl;
    cout << "st_rdev: " << stbuf.st_rdev << endl;
    cout << "st_size: " << stbuf.st_size << endl;
    cout << "st_atime: " << stbuf.st_atime << endl;
    cout << "st_mtime: " << stbuf.st_mtime << endl;
    cout << "st_ctime: " << stbuf.st_ctime << endl;
    cout << "st_blksize: " << stbuf.st_blksize << endl;
    cout << "st_blocks: " << stbuf.st_blocks << endl;

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
