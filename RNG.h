#ifndef TIN_RNG
#define TIN_RNG

class RNG {
public:
	int generate(unsigned char *buf, size_t size);
	RNG();
	~RNG();
};
#endif //TIN_RNG
