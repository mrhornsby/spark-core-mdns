#include "Record.h"

#include "Label.h"

mdns::Record::Record(uint16_t type, uint16_t cls, uint32_t ttl, bool announce) {
  this->type = type;
  this->cls = cls;
  this->ttl = ttl;
  this->announce = announce;
}

void mdns::Record::setLabel(Label * label) {
  this->label = label;
}

void mdns::Record::announceRecord() {
  if (this->announce) {
    this->answerRecord = true;
  }
}

void mdns::Record::setAnswerRecord() {
  this->answerRecord = true;
}

bool mdns::Record::isAnswerRecord() {
  return answerRecord && !knownRecord;
}

void mdns::Record::setAdditionalRecord() {
  this->additionalRecord = true;
}

bool mdns::Record::isAdditionalRecord() {
  return additionalRecord && !answerRecord && !knownRecord;
}

void mdns::Record::setKnownRecord() {
  this->knownRecord = true;
}

void mdns::Record::write(Buffer * buffer) {
  label->write(buffer);
  buffer->writeUInt16(type);
  buffer->writeUInt16(cls);
  buffer->writeUInt32(ttl);
  writeSpecific(buffer);
}

void mdns::Record::reset() {
  this->answerRecord = false;
  this->additionalRecord = false;
  this->knownRecord = false;
}

mdns::Label * mdns::Record::getLabel() {
  return label;
}

mdns::ARecord::ARecord():Record(A_TYPE, IN_CLASS | CACHE_FLUSH, TTL_2MIN) {
}

void mdns::ARecord::writeSpecific(Buffer * buffer) {
  buffer->writeUInt16(4);
  IPAddress ip = WiFi.localIP();
  for (int i = 0; i < IP_SIZE; i++) {
    buffer->writeUInt8(ip[i]);
  }
}

mdns::NSECRecord::NSECRecord():Record(NSEC_TYPE, IN_CLASS | CACHE_FLUSH, TTL_2MIN) {
}

mdns::HostNSECRecord::HostNSECRecord():NSECRecord() {
}

void mdns::HostNSECRecord::writeSpecific(Buffer * buffer) {
  buffer->writeUInt16(5);
  getLabel()->write(buffer);
  buffer->writeUInt8(0);
  buffer->writeUInt8(1);
  buffer->writeUInt8(0x40);
}

mdns::InstanceNSECRecord::InstanceNSECRecord():NSECRecord() {
}

void mdns::InstanceNSECRecord::writeSpecific(Buffer * buffer) {
  buffer->writeUInt16(9);
  getLabel()->write(buffer);
  buffer->writeUInt8(0);
  buffer->writeUInt8(5);
  buffer->writeUInt8(0);
  buffer->writeUInt8(0);
  buffer->writeUInt8(0x80);
  buffer->writeUInt8(0);
  buffer->writeUInt8(0x40);
}

mdns::PTRRecord::PTRRecord(bool meta):Record(PTR_TYPE, IN_CLASS, TTL_75MIN, !meta) {
}

void mdns::PTRRecord::writeSpecific(Buffer * buffer) {
  buffer->writeUInt16(targetLabel->getWriteSize());
  targetLabel->write(buffer);
}

void mdns::PTRRecord::setTargetLabel(Label * label) {
  targetLabel = label;
}

mdns::SRVRecord::SRVRecord():Record(SRV_TYPE, IN_CLASS | CACHE_FLUSH, TTL_2MIN) {
}

void mdns::SRVRecord::writeSpecific(Buffer * buffer) {
  buffer->writeUInt16(6 + hostLabel->getWriteSize());
  buffer->writeUInt16(0);
  buffer->writeUInt16(0);
  buffer->writeUInt16(port);
  hostLabel->write(buffer);
}

void mdns::SRVRecord::setHostLabel(Label * label) {
  hostLabel = label;
}

void mdns::SRVRecord::setPort(uint16_t port) {
  this->port = port;
}

mdns::TXTRecord::TXTRecord():Record(TXT_TYPE, IN_CLASS | CACHE_FLUSH, TTL_75MIN) {
}

void mdns::TXTRecord::addEntry(String key, String value) {
  String entry = key;

  if (value == NULL || value.length() > 0) {
    entry += '=';
    entry += value;
  }

  data.push_back(entry);
}

void mdns::TXTRecord::writeSpecific(Buffer * buffer) {
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
