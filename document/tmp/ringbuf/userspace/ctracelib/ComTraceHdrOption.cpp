#include "ComTraceHdrOption.h"

ComTraceHdrOption operator|(const ComTraceHdrOption a, const ComTraceHdrOption b)
{
  return (ComTraceHdrOption)((int)a|b);
}

ComTraceHdrOption operator&(const ComTraceHdrOption a, const ComTraceHdrOption b)
{
  return (ComTraceHdrOption)((int)a&b);
}

ComTraceHdrOption operator~(const ComTraceHdrOption a)
{
  return (ComTraceHdrOption)(~(int)a);
}

