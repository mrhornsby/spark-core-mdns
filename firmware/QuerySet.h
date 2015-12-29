#include "application.h"

#ifndef _INCL_QUERY_SET
#define _INCL_QUERY_SET

#include "Buffer.h"

class QuerySet {
    public:

        bool readHeader(Buffer * buffer);

        bool isQuery();

        uint16_t getId();
        uint16_t getFlags();
        uint16_t getQueryCount();
        uint16_t getAnswerCount();
        uint16_t getAuthorityCount();
        uint16_t getAdditionalCount();

        uint8_t getEntryCount();

        void setResponses(uint16_t responses);

        uint16_t getResponses();

        void setStatus(String status);

        String getStatus();

    private:

        uint16_t id;
        uint16_t flags;
        uint16_t qdcount;
        uint16_t ancount;
        uint16_t nscount;
        uint16_t arcount;

        uint8_t entryCount;

        uint16_t responses;
        String status = "ok";
};

#endif
