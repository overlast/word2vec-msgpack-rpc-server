word2vec-msgpack-rpc-server
=======================

MessagePack RPC server of word2vec in C++

## Motive

Reduce the processing time of word2vec application

## Requirements

Following programs are required to build:

- gcc >= 4.1 with C++ support
- [MessagePack for C++](http://msgpack.org/) >= 0.5.2
- [mpio](http://github.com/frsyuki/mpio) >= 0.3.5
- [MessagePack-RPC for C++](https://github.com/msgpack-rpc/msgpack-rpc-cpp/) >= 0.3.1
- [Jansson](https://github.com/akheron/jansson) >= 2.6

And following build tool is required to build:

- waf >= 1.6.11

## Before installation

You should install above shared libraries and waf(https://code.google.com/p/waf/).

### If you use CentOS 6.x

Configure and install in this way:

    $ ./sh/make_centos_env.sh

## Installation

You should be able to build it on most systems with waf(https://code.google.com/p/waf/).

Configure and install in this way:

    $ ./sh/compile.sh

## Example

    $ $/usr/local/bin/word2vec-msgpack-rpc-server -m /path/to/your/word2vec/model/file.bin
    [word2vec] read the model file...
    [word2vec] words in model = 1356952, model window size = 200
    [word2vec] finish to read the model file
    [word2vec] stand by...

### USAGE

    options:
    -m, --model           Absolute path of word2vec model binary file (Must define) (string)
    -i, --ip_address      IP Address of word2vec-msgpack-rpc-server (Dafault = '0.0.0.0') (string [=0.0.0.0])
    -p, --port_num        Port numver of word2vec-msgpack-rpc-server (Dafault = 9090) (int [=9090])
    -n, --instance_num    Number of threads of word2vec-msgpack-rpc-server (Dafault = 4) (int [=4])
    -r, --result_num      Number of max result entry (Dafault = 40) (long long [=40])
    -h, --help            Print help

## License

    Copyright (C) 2014 Toshinori SATO <overlasting _attt_ gmail _dottt_ com>

    Licensed under the Apache License, Version 2.0 (the "License");
    You may not use this file except in compliance with the License.
    You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

See also NOTICE file.
