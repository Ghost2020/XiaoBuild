/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "Protobuf/xiao_msg.pb.h"

#define SIZEOF_ARRAY(X) ((int)(sizeof(X)/sizeof(X[0])))

// 核心状态的锁
static FCriticalSection SCriticalSection;