#include "application.h"
#include <vector>

#ifndef _INCL_TXTDATA
#define _INCL_TXTDATA

#include "Buffer.h"

class TxtData {

public:

  void addEntry(String key, String value = NULL);

  void write(Buffer * buffer);

private:

  std::vector<String> data;
};

#endif
