#ifndef TFHELPERS_H
#define TFHELPERS_H
struct RGB {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

inline unsigned char TFHb_set_attrs(bool b[8]) {
	unsigned char c = 0;
	for (int i=0; i< 8; i++) {
		if (b[i] == true) { c |= 1 << i; }
	}
	return c;
};

inline unsigned char TFHb_set_attr(unsigned char r, uint8_t index, bool value) {
	unsigned char c = 0;
	for (int i=0; i< 8; i++) {
		if ((index == i && value) || (index != i && ((r & (1<<i)) != 0 ))) {
			c |= 1 << i;
		}
	}
	return c;
};

inline bool TFHb_get_attr(unsigned char c, uint8_t index) {
	return (c & (1<<index)) != 0;
};

inline void TFHb_get_attrs(unsigned char c, bool *r) {
	for (uint8_t i=0; i< 8; i++) {
		if ((c & (1<<i)) != 0) { r[i] = true; }else { r[i] = false; }
	}
};

#endif // TFHELPERS_H

