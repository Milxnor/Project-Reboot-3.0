#pragma once

#include "Controller.h"

class AGameSession : public AActor
{
public:
	static inline void (*KickPlayerOriginal)(AGameSession*, AController*);

	static void KickPlayerHook(AGameSession*, AController*) { return; }

};