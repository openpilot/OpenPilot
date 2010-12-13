#ifndef COMMON_H
#define COMMON_H

enum decodeState_ {
    decode_len1_e = 0,
    decode_seqNo_e,
    decode_data_e,
    decode_crc1_e,
    decode_crc2_e,
    decode_idle_e
};
enum ReceiveState {
    state_escaped_e = 0,
    state_unescaped_e
};


#endif // COMMON_H
