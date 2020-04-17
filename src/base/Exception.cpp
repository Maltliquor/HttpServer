#include "src/base/Exception.h"
#include "src/base/CurrentThread.h"

namespace serverlib
{

Exception::Exception(std::string msg)
  : message_(std::move(msg)),
    stack_(CurrentThread::stackTrace(/*demangle=*/false))
{
}

}