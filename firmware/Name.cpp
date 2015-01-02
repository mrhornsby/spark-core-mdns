#include "Name.h"

bool Name::init(String name) {
    size = name.length() + 1;
    
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
        
uint8_t Name::getSize() {
    return size;
}

bool Name::match(uint8_t c) {
    matches &= data[matchOffset++] = c;
    
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
