#ifndef HELENA_PLATFORM_PROCESSOR_HPP
#define HELENA_PLATFORM_PROCESSOR_HPP

/* ----------- [Processor detect] ----------- */
#if defined(__x86_64__) || defined(_M_AMD64) || defined(__amd64__) || defined(__amd64)
	#define HELENA_PROCESSOR_NAME	"Intel x86-64"
	#define HELENA_PROCESSOR_BITS	64
	#define HELENA_PROCESSOR_AMD64
#elif defined(__i386__) || defined(_M_IX86)
	#define HELENA_PROCESSOR_NAME	"Intel x86"
	#define HELENA_PROCESSOR_BITS	32
	#define HELENA_PROCESSOR_X86
#elif defined (__sparc__)
	#define HELENA_PROCESSOR_NAME	"Sparc"
	#define HELENA_PROCESSOR_SPARC
	#if defined(__arch64__)
		#define HELENA_PROCESSOR_BITS	64
	#else
		#error Sparc 32bit is not supported
	#endif
#elif defined(__ia64__) || defined(_M_IA64)
	#define HELENA_PROCESSOR_NAME	"Intel IA64"
	#define HELENA_PROCESSOR_BITS	64
	#define HELENA_PROCESSOR_IA64
#elif defined(_ARCH_PPC64) || defined(_M_PPC)
	#define HELENA_PROCESSOR_NAME	"IBM PowerPC64"
	#define HELENA_PROCESSOR_BITS	64
	#define HELENA_PROCESSOR_PPC64
#elif defined(__arm__)  || defined(_M_ARM)
	#define HELENA_PROCESSOR_NAME	"ARM"
	#if defined(__aarch64__)
		#define HELENA_PROCESSOR_BITS	64
	#else
		#define HELENA_PROCESSOR_BITS	32
	#endif
	#define HELENA_PROCESSOR_ARM
#endif

#endif  // HELENA_PLATFORM_PROCESSOR_HPP
