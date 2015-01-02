#include "Name.h"

bool Name::init(String name) {
    size = name.length() + 1;

    if (data) {
        free(data);
    }

    data = (uint8_t *) malloc(size);

    if (data != NULL) {
        uint8_t lastDot = size;

        for (uint8_t i = size - 1; i > 0; i--) {
            uint8_t c = name.charAt(i - 1);

            if (c == '.') {
                data[i] = lastDot - i - 1;
                lastDot = i;
            } else {
                data[i] = c;
            }
        }

        data[0] = lastDot - 1;
    }

    return data != NULL;
}

bool Name::init(Buffer buffer) {
    bool success = buffer.available();

    if (success) {
        if (data) {
            free(data);
        }

        data = (uint8_t *) malloc(255);

        success = data;
    }

    uint8_t idx = 0;

    if (success) {
        uint8_t c = buffer.readUInt8();

        while (c != END_OF_NAME) {
            while ((c & BACK_REF) == BACK_REF) {
                success = buffer.available() > 0;

                if (success) {
                    uint16_t backReferenceOffset = (c & ~BACK_REF) << 8 | buffer.readUInt8();

                    success = backReferenceOffset < buffer.getOffset();

                    if (success) {
                        buffer.mark();
                        buffer.setOffset(backReferenceOffset);
                        c = buffer.readUInt8();
                    }
                }
            }

            if (success && buffer.available() > c) {
                data[idx++] = c;

                for (int i = 0; i < c; i++) {
                    data[idx++] = buffer.readUInt8();
                }

                c = buffer.readUInt8();
            }
        }

        data[idx++] = c;

        buffer.reset();
    }

    if (!success && data) {
        free(data);
    }

    size = idx;
}

uint8_t Name::getSize() {
    return size;
}

bool Name::match(uint8_t c) {
    matches = matches && matchOffset < size && data[matchOffset++] == c;

    return matches;
}

void Name::write(Buffer buffer) {
    if (writtenOffset == 0) {
        writtenOffset = buffer.getOffset();

        for (int i = 0; i < size; i++) {
            buffer.writeUInt8(data[i]);
        }
    } else {
        buffer.writeUInt16((BACK_REF << 8) | writtenOffset);
    }
}

void Name::reset() {
    matches = true;
    matchOffset = 0;
    writtenOffset = 0;
}

String Name::toString() {
    String s = "";

    uint8_t idx = 0;

    while (data[idx] != END_OF_NAME) {
        uint8_t len = data[idx++];

        for (int i = 0; i < len; i++) {
            s += (char) data[idx++];
        }
    }

    s += '.';

    return s;
}

bool Name::operator==(const Name &other) const {
    bool equal = size == other.size;

    if (equal) {
        int idx = 0;

        while (equal && idx < size) {
            equal &= data[idx] == other.data[idx];
        }
    }

    return equal;
}

bool Name::operator!=(const Name &other) const {
    return !(*this == other);
}
