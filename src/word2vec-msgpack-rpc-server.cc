//  Copyright (C) 2014 Toshinori SATO <overlasting _attt_ gmail _dottt_ com>
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  You may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <msgpack/rpc/server.h>

#include "cmdline.h"
#include "word2vec.h"

using namespace std;

class Word2vecMsgpackRPCServer : public msgpack::rpc::server::base {
 public:
  word2vec_model_t *model;

  Word2vecMsgpackRPCServer(string file_path, long long result_num = 40) {
    model = get_word2vec_model((char*)file_path.c_str());
    model->N = result_num;
  }
  /*
  void reload_model(msgpack::rpc::request req, string file_path, long long result_num = 40) {
    destroy_word2vec_model(model);
    model = get_word2vec_model((char*)file_path.c_str());
    model->N = result_num;
  }
  */

  void get_distance(msgpack::rpc::request req, std::string query) {
    string json = distance(model, (char*)query.c_str());
    req.result(json);
  }

  /*
  void analogy(msgpack::rpc::request req, int a1, int a2) {
  }
  */

 public:
  void dispatch(msgpack::rpc::request req)
      try {
        std::string method;
        req.method().convert(&method);
        if(method == "distance") {
          msgpack::type::tuple<std::string> params;
          req.params().convert(&params);
          get_distance(req, params.get<0>());
        } else {
          req.error(msgpack::rpc::NO_METHOD_ERROR);
        }
      } catch (msgpack::type_error& e) {
        req.error(msgpack::rpc::ARGUMENT_ERROR);
        return;
      } catch (std::exception& e) {
        req.error(std::string(e.what()));
        return;
      }
};

int main(int argc, char *argv[]) {
  string file_path;
  string ip_address = "";
  int port_num = -1;
  int instance_num = -1;
  long long result_num = -1;

  cmdline::parser cmdp;

  cmdp.add<string>("model", 'm', "Absolute path of word2vec model binary file (Must define)");
  cmdp.add<string>("ip_address", 'i', "IP Address of word2vec-msgpack-rpc-server (Dafault = '0.0.0.0')", false, "0.0.0.0");
  cmdp.add<int>("port_num", 'p', "Port numver of word2vec-msgpack-rpc-server (Dafault = 9090)", false, 9090);
  cmdp.add<int>("instance_num", 'n', "Number of threads of word2vec-msgpack-rpc-server (Dafault = 4)", false, 4);
  cmdp.add<long long>("result_num", 'r', "Number of max result entry (Dafault = 40)", false, 40);
  cmdp.add("help", 'h', "Print help");
  if (!cmdp.parse(argc, argv) || cmdp.exist("help") || !cmdp.exist("model")){
    std::cout << cmdp.error_full() << cmdp.usage();
    return 0;
  }

  file_path = cmdp.get<string>("model");
  ip_address = cmdp.get<string>("ip_address");
  port_num = cmdp.get<int>("port_num");
  instance_num = cmdp.get<int>("instance_num");
  result_num = cmdp.get<long long>("result_num");

  Word2vecMsgpackRPCServer srv(file_path, result_num);
  srv.instance.listen(ip_address, port_num);
  srv.instance.run(instance_num);
  // run 4 threads
}
