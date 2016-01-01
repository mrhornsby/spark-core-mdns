#include "application.h"

#ifndef _INCL_MDNS
#define _INCL_MDNS

#include "Buffer.h"
#include "Label.h"
#include "Record.h"
#include <vector>

#define MDNS_PORT 5353

#define BUFFER_SIZE 512

class MDNS {
public:

  bool setHostname(String hostname);

  bool addService(String protocol, String service, uint16_t port, String instance);

  void addTXTEntry(String key, String value = NULL);

  bool begin();

  bool processQueries();

private:

  struct QueryHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
  };

  UDP * udp = new UDP();
  Buffer * buffer = new Buffer(BUFFER_SIZE);

  Label * ROOT = new Label("");
  Label * LOCAL = new Label("local", ROOT);
  Label::Matcher * matcher = new Label::Matcher();

  ARecord * aRecord;
  PTRRecord * ptrRecord;
  SRVRecord * srvRecord;
  TXTRecord * txtRecord;

  std::vector<Label *> labels;
  std::vector<Record *> records;

  String status = "Ok";

  QueryHeader readHeader(Buffer * buffer);
  void getResponses();
  void writeResponses();
  bool isAlphaDigitHyphen(String string);
  bool isNetUnicode(String string);

  struct Query {
    int8_t matchedName;
    uint16_t type;
    uint16_t cls;
  };
};

#endif
