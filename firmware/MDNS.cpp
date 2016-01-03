#include "MDNS.h"

bool MDNS::setHostname(String hostname) {
  bool success = true;

  if (aRecord != NULL) {
    success = false;
    status = "Hostname already set";
  }

  if (success && hostname.length() < MAX_LABEL_SIZE && isAlphaDigitHyphen(hostname)) {
    aRecord = new ARecord();

    HostNSECRecord * hostNSECRecord = new HostNSECRecord();

    records.push_back(aRecord);
    records.push_back(hostNSECRecord);

    Label * label = new HostLabel(aRecord, hostNSECRecord, hostname, LOCAL);

    labels.push_back(label);

    aRecord->setLabel(label);
    hostNSECRecord->setLabel(label);
  } else {
    success = false;
    status = "Invalid hostname";
  }

  if (success) {
    status = "Ok";
  }

  return success;
}

bool MDNS::addService(String protocol, String service, uint16_t port, String instance) {
  bool success = true;

  if (protocol.length() < MAX_LABEL_SIZE - 1 && service.length() < MAX_LABEL_SIZE - 1 &&
  instance.length() < MAX_LABEL_SIZE && isAlphaDigitHyphen(protocol) && isAlphaDigitHyphen(service) && isNetUnicode(instance)) {

    ptrRecord = new PTRRecord();
    srvRecord = new SRVRecord();
    txtRecord = new TXTRecord();
    InstanceNSECRecord * instanceNSECRecord = new InstanceNSECRecord();

    records.push_back(ptrRecord);
    records.push_back(srvRecord);
    records.push_back(txtRecord);
    records.push_back(instanceNSECRecord);

    ServiceLabel * serviceLabel = new ServiceLabel(ptrRecord, srvRecord, txtRecord, aRecord, "_" + service, new Label("_" + protocol, LOCAL));

    labels.push_back(serviceLabel);

    InstanceLabel * instanceLabel = new InstanceLabel(srvRecord, txtRecord, instanceNSECRecord, aRecord, instance, serviceLabel, true);

    labels.push_back(instanceLabel);

    ptrRecord->setLabel(serviceLabel);
    ptrRecord->setInstanceLabel(instanceLabel);
    srvRecord->setLabel(instanceLabel);
    srvRecord->setPort(port);
    srvRecord->setHostLabel(labels.front());
    txtRecord->setLabel(instanceLabel);
    instanceNSECRecord->setLabel(instanceLabel);
  } else {
    success = false;
    status = "Invalid name";
  }

  return success;
}

void MDNS::addTXTEntry(String key, String value) {
  txtRecord->addEntry(key, value);
}

bool MDNS::begin() {
  // Wait for WiFi to connect
  while (!WiFi.ready()) {
  }

  udp->begin(MDNS_PORT);
  udp->joinMulticast(IPAddress(224, 0, 0, 251));

  // TODO: Probing + announcing

  return true;
}

bool MDNS::processQueries() {
  uint16_t n = udp->parsePacket();

  if (n > 0) {
    buffer->read(udp);

    udp->flush();

    getResponses();

    buffer->clear();

    writeResponses();

    if (buffer->available() > 0) {
      udp->beginPacket(IPAddress(224, 0, 0, 251), MDNS_PORT);

      buffer->write(udp);

      udp->endPacket();
    }
  }

  return n > 0;
}

void MDNS::getResponses() {
  QueryHeader header = readHeader(buffer);

  if ((header.flags & 0x8000) == 0 && header.qdcount > 0) {
    uint8_t count = 0;

    while (count++ < header.qdcount && buffer->available() > 0) {
      Label * label = matcher->match(labels, buffer);

      if (label != NULL) {
        if (buffer->available() >= 4) {
          label->matched(buffer->readUInt16(), buffer->readUInt16());
        } else {
          status = "Buffer underflow at index " + buffer->getOffset();
        }
      }
    }
  }
}

MDNS::QueryHeader MDNS::readHeader(Buffer * buffer) {
  QueryHeader header;

  if (buffer->available() >= 12) {
    header.id = buffer->readUInt16();
    header.flags = buffer->readUInt16();
    header.qdcount = buffer->readUInt16();
    header.ancount = buffer->readUInt16();
    header.nscount = buffer->readUInt16();
    header.arcount = buffer->readUInt16();
  }

  return header;
}

void MDNS::writeResponses() {

  uint8_t answerCount = 0;
  uint8_t additionalCount = 0;

  for (std::vector<Record *>::const_iterator i = records.begin(); i != records.end(); ++i) {
    answerCount += (*i)->isAnswerRecord()? 1 : 0;
    additionalCount += (*i)->isAdditionalRecord()? 1 : 0;
  }

  if (answerCount > 0) {
    buffer->writeUInt16(0x0);
    buffer->writeUInt16(0x8400);
    buffer->writeUInt16(0x0);
    buffer->writeUInt16(answerCount);
    buffer->writeUInt16(0x0);
    buffer->writeUInt16(additionalCount);

    for (std::vector<Record *>::const_iterator i = records.begin(); i != records.end(); ++i) {
      if ((*i)->isAnswerRecord()) {
        (*i)->write(buffer);
      }
    }

    for (std::vector<Record *>::const_iterator i = records.begin(); i != records.end(); ++i) {
      if ((*i)->isAdditionalRecord()) {
        (*i)->write(buffer);
      }
    }
  }
  
  for (std::vector<Label *>::const_iterator i = labels.begin(); i != labels.end(); ++i) {
    (*i)->reset();
  }

  for (std::vector<Record *>::const_iterator i = records.begin(); i != records.end(); ++i) {
    (*i)->reset();
  }
}

bool MDNS::isAlphaDigitHyphen(String string) {
  bool result = true;

  uint8_t idx = 0;

  while (result && idx < string.length()) {
    uint8_t c = string.charAt(idx++);

    result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-';
  }

  return result;
}

bool MDNS::isNetUnicode(String string) {
  bool result = true;

  uint8_t idx = 0;

  while (result && idx < string.length()) {
    uint8_t c = string.charAt(idx++);

    result = c >= 0x1f && c != 0x7f;
  }

  return result;
}
