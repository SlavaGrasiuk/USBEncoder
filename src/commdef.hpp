#pragma once

#define MANGOFF		extern "C"

#ifdef DEBUG
static constexpr bool g_debug = true;
#elif defined(NDEBUG)
static constexpr bool g_debug = false;
#endif

#define barrier()		__asm volatile("": : :"memory")		//Compiller memory barrier

#define likely(x)		__builtin_expect((x),1)
#define unlikely(x)		__builtin_expect((x),0)


#define EXCEPT_HNDL()			\
	if constexpr (g_debug) {	\
		__asm("bkpt #0");		\
	} else {					\
		while (true);			\
	}

