// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WIC_HEIC_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WIC_HEIC_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WIC_HEIC_EXPORTS
#define WIC_API __declspec(dllexport)
#else
#define WIC_API __declspec(dllimport)
#endif

#pragma warning(push)
#pragma warning(disable: 4800)
#define LIBHEIF_STATIC_BUILD
#include <libheif/heif_cxx.h>
#pragma warning(pop)

// CLSID HEIC DECODER
// {6215667c-74ca-4076-a769-3f834c3f25f5}
static const GUID CLSID_HEICDecoder =
{ 0x6215667c, 0x74ca, 0x4076,{ 0xa7, 0x69, 0x3f, 0x83, 0x4c, 0x3f, 0x25, 0xf5 } };


// CLSID HEIC ENCODER
// {0e8ab687-73a5-455b-8c19-b93ca9ac0eeb}
static const GUID CLSID_HEICEncoder =
{ 0x0e8ab687, 0x73a5, 0x455b,{ 0x8c, 0x19, 0xb9, 0x3c, 0xa9, 0xac, 0x0e, 0xeb } };


// GUID HEIC CONTAINER FORMAT
// {98ce31dd-5435-41bd-a1af-29c387c20135}
static const GUID GUID_ContainerFormatHEIC =
{ 0x98ce31dd, 0x5435, 0x41bd,{ 0xa1, 0xaf, 0x29, 0xc3, 0x87, 0xc2, 0x01, 0x35 } };


// GUID VENDOR PRSYAHMI
// {d01102c8-6108-41dd-bd60-1f3a8e7f2447}
static const GUID GUID_VendorPrsyahmi =
{ 0xd01102c8, 0x6108, 0x41dd,{ 0xbd, 0x60, 0x1f, 0x3a, 0x8e, 0x7f, 0x24, 0x47 } };


void Log(const char* szFormat, ...);

#ifdef _DEBUG
#define DbgLog Log
#else
#define DbgLog
#endif