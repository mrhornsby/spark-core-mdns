#include "application.h"

#ifndef _INCL_NAME
#define _INCL_NAME

#include "Buffer.h"

#define END_OF_NAME 0x0
#define BACK_REF 0xc0

class Name {
    public:
        bool init(String hostname);

        bool init(Buffer buffer);

        uint8_t getSize();

        bool match(uint8_t c);

        void write(Buffer buffer);

        void reset();

        String toString();

        bool operator==(const Name &other) const;

        bool operator!=(const Name &other) const;

    private:

        uint8_t * data;
        uint16_t size;

        bool matches = true;
        uint8_t matchOffset = 0;
        uint16_t writtenOffset = 0;
};

#endif
