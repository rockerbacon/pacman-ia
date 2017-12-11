#ifndef LAB309_RANDOM_H
#define LAB309_RANDOM_H

namespace lab309 {
	void timeSeed (void);
	int randomBetween (int lower, int upper); //retorna inteiro aleatorio no intervalo [lower, upper)
	float random (void); //retorna float aleatorio no intervalo [0, 1]
};

#endif
