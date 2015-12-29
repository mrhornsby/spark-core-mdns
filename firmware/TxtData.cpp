#include "TxtData.h"

void TxtData::addEntry(String key, String value) {
  String entry = key;

  if (value != NULL) {
    entry += '=';
    entry += value;
  }

  data.push_back(entry);
}

void TxtData::write(Buffer * buffer) {
  uint16_t size = 0;

  std::vector<String>::const_iterator i;

  for(i = data.begin(); i != data.end(); ++i) {
    size += i->length() + 1;
  }

  buffer->writeUInt16(size);

  for(i = data.begin(); i != data.end(); ++i) {
    uint8_t length = i->length();

    buffer->writeUInt8(length);

    for (uint8_t idx = 0; idx < length; idx++) {
      buffer->writeUInt8(i->charAt(idx));
    }
  }
}
