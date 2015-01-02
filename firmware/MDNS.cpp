#include "MDNS.h"

bool MDNS::setHostname(String hostname) {
    // TODO: further validate hostname
    bool success = hostname.length() < 63;

    if (success) {
        success = names[HOST_NAME].init(hostname + LOCAL_SUFFIX);
    }

    return success;
}

bool MDNS::setService(String service, uint16_t port, String instance) {
    // TODO: further validate service and instance names
    bool success = service != NULL && service.length() > 0 && service.length() < 63 && instance != NULL && instance.length() > 0 && instance.length() < 63;

    if (success) {
        success = names[SERVICE_NAME].init(service + LOCAL_SUFFIX);
    }

    this->port = port;

    if (success) {
        success = names[INSTANCE_NAME].init(instance + "." + service + LOCAL_SUFFIX);
    }

    return success;
}

bool MDNS::addTXTEntry(String key, String value) {
    return txtData.addEntry(key, value);
}

bool MDNS::begin() {
    bool success = true;

    // Wait for WiFi to connect
    while (!WiFi.ready()) {
    }

    ip = WiFi.localIP();
    udp.begin(MDNS_PORT);

    // TODO: Probing + announcing

    return success;
}

int MDNS::processQueries() {
    uint16_t n = udp.parsePacket();

    uint8_t responses = 0;

    if (n > 0) {
        inBuffer.read(udp);

        udp.flush();

        if (inBuffer.available() >= 12) {
            /*uint16_t id = */inBuffer.readUInt16();
            uint16_t flags = inBuffer.readUInt16();
            uint16_t qdcount = inBuffer.readUInt16();
            /*uint16_t ancount = */inBuffer.readUInt16();
            /*uint16_t nscount = */inBuffer.readUInt16();
            /*uint16_t arcount = */inBuffer.readUInt16();

            if ((flags & 0x8000) == 0) {
                while (qdcount-- > 0) {
                    if (matchedName.init(inBuffer)) {
                        int8_t matchedNameIdx = matchName();

                        uint16_t type = inBuffer.readUInt16();
                        uint16_t cls = inBuffer.readUInt16();

                        Spark.publish("mdns/processQueries", "Query " + matchedName.toString() + " " + String(type, HEX) + " " + String(cls, HEX) + " " + matchedNameIdx, 60, PRIVATE);

                        switch (matchedNameIdx) {
                            case HOST_NAME:
                                // TODO: Negative response for AAAA

                                if (type == A_TYPE || type == ANY_TYPE) {
                                    responses |= A_AN_FLAG;
                                }
                                break;

                            case SERVICE_NAME:
                                if (type == PTR_TYPE || type == ANY_TYPE) {
                                    responses |= PTR_AN_FLAG | SRV_AD_FLAG | TXT_AD_FLAG | A_AD_FLAG;
                                }
                                break;

                            case INSTANCE_NAME:
                                if (type == SRV_TYPE) {
                                    responses |= SRV_AN_FLAG | A_AD_FLAG;
                                } else if (type == TXT_TYPE) {
                                    responses |= TXT_AN_FLAG;
                                } else if (type == ANY_TYPE) {
                                    responses |= SRV_AN_FLAG | TXT_AN_FLAG | A_AD_FLAG;
                                }
                                break;

                            default:
                                break;
                        }
                    } else {
                        qdcount = 0;
                    }
                }
            }
        }
    }

    if (responses > 0) {
        responses &= ~(responses << 4);

        uint8_t counts = (responses & 0x55) + ((responses >> 1) & 0x55);
        counts = (counts & 0x33) + ((counts >> 2) & 0x33);

        Spark.publish("mdns/processQueries", "Response " + String(counts & 0x3) + " " + String(counts >> 4) + " " + String(responses, BIN), 60, PRIVATE);

        outBuffer.writeUInt16(0x0);
        outBuffer.writeUInt16(0x8400);
        outBuffer.writeUInt16(0x0);
        outBuffer.writeUInt16(counts & 0x3);
        outBuffer.writeUInt16(0x0);
        outBuffer.writeUInt16(counts >> 4);

        while (responses > 0) {
            if ((responses & A_AN_FLAG) != 0) {
                writeARecord();
            }
            if ((responses & PTR_AN_FLAG) != 0) {
                writePTRRecord();
            }
            if ((responses & SRV_AN_FLAG) != 0) {
                writeSRVRecord();
            }
            if ((responses & TXT_AN_FLAG) != 0) {
                writeTXTRecord();
            }

            responses >>= 4;
        }

        udp.beginPacket(IPAddress(224, 0, 0, 251), MDNS_PORT);

        outBuffer.write(udp);

        udp.endPacket();
    }

    for (uint8_t i = 0; i < NAME_COUNT; i++) {
        names[i].reset();
    }

    return n;
}

int8_t MDNS::matchName() {
    int8_t nameIndex = UNKNOWN_NAME;

    int idx = 0;

    while (idx < NAME_COUNT && nameIndex == UNKNOWN_NAME) {
        if (matchedName == names[idx]) {
            nameIndex = idx;
        }

        idx++;
    }

    return nameIndex;
}

void MDNS::writeARecord() {
    writeRecord(HOST_NAME, A_TYPE, TTL_2MIN);
    outBuffer.writeUInt16(4);
    for (int i = 0; i < IP_SIZE; i++) {
        outBuffer.writeUInt8(ip[i]);
    }
}

void MDNS::writePTRRecord() {
    writeRecord(SERVICE_NAME, PTR_TYPE, TTL_75MIN);
    outBuffer.writeUInt16(names[INSTANCE_NAME].getSize());
    names[INSTANCE_NAME].write(outBuffer);
}

void MDNS::writeSRVRecord() {
    writeRecord(INSTANCE_NAME, SRV_TYPE, TTL_2MIN);
    outBuffer.writeUInt16(6 + names[HOST_NAME].getSize());
    outBuffer.writeUInt16(0);
    outBuffer.writeUInt16(0);
    outBuffer.writeUInt16(port);
    names[HOST_NAME].write(outBuffer);
}

void MDNS::writeTXTRecord() {
    writeRecord(INSTANCE_NAME, TXT_TYPE, TTL_75MIN);
    txtData.write(outBuffer);
}

void MDNS::writeRecord(uint8_t nameIndex, uint16_t type, uint32_t ttl) {
    names[nameIndex].write(outBuffer);
    outBuffer.writeUInt16(type);
    outBuffer.writeUInt16(IN_CLASS);
    outBuffer.writeUInt32(ttl);
}
