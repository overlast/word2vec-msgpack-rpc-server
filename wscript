#!/usr/bin/env python
#-*- coding:utf-8 -*-

APPNAME = 'word2vec-msgpack-rpc-server'
VERSION = '0.0.1'

def options(opt):
  opt.load('compiler_cc')
  opt.load('compiler_cxx')

def configure(conf):
  conf.env.CXXFLAGS += ['-O3', '-Wall', '-g']
  conf.load('compiler_cc')
  conf.load('compiler_cxx')

  conf.check_cxx(lib = 'msgpack-rpc')
  conf.check_cxx(lib = 'mpio')
  conf.check_cxx(lib = 'msgpack')
  conf.check_cxx(lib = 'jansson')

  conf.env.INSTALL_DIR = "/usr/local/bin"

  conf.env.append_value('LINKFLAGS', '-lmsgpack-rpc')
  conf.env.append_value('LINKFLAGS', '-lmpio')
  conf.env.append_value('LINKFLAGS', '-lmsgpack')
  conf.env.append_value('LINKFLAGS', '-ljansson')

def build(bld):
  bld.recurse('src')
