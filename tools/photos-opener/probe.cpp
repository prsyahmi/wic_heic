#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincodec.h>
#include <shlwapi.h>
#include <wrl/client.h>
#include <cstdio>
#include <vector>
#include "wic_heic.h"
using Microsoft::WRL::ComPtr;
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) {std::fprintf(stderr,"Usage: probe.exe image.heic codec.dll [registered]\n");return 2;}
    HRESULT hr=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
    if(FAILED(hr))return 1;
    ComPtr<IWICImagingFactory> imaging;
    hr=CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&imaging));
    if(FAILED(hr)) {std::printf("WIC factory: 0x%08lX\n",hr);return 1;}
    ComPtr<IWICBitmapDecoder> decoder;
    HMODULE module=nullptr;
    if(argc==4) {
        hr=imaging->CreateDecoderFromFilename(argv[1],nullptr,GENERIC_READ,WICDecodeMetadataCacheOnDemand,&decoder);
    } else {
        module=LoadLibraryExW(argv[2],nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(!module) {std::printf("DLL load error: %lu\n",GetLastError());return 1;}
        auto get=reinterpret_cast<HRESULT(STDAPICALLTYPE*)(REFCLSID,REFIID,void**)>(GetProcAddress(module,"DllGetClassObject"));
        ComPtr<IClassFactory> factory;
        hr=get?get(CLSID_HEICDecoder,IID_PPV_ARGS(&factory)):E_FAIL;
        if(SUCCEEDED(hr))hr=factory->CreateInstance(nullptr,IID_PPV_ARGS(&decoder));
        ComPtr<IStream> stream;
        if(SUCCEEDED(hr))hr=SHCreateStreamOnFileEx(argv[1],STGM_READ|STGM_SHARE_DENY_WRITE,FILE_ATTRIBUTE_NORMAL,FALSE,nullptr,&stream);
        if(SUCCEEDED(hr))hr=decoder->Initialize(stream.Get(),WICDecodeMetadataCacheOnDemand);
    }
    if(FAILED(hr)) {std::printf("Decode initialization: 0x%08lX\n",hr);return 1;}
    UINT count=0;
    hr=decoder->GetFrameCount(&count);
    ComPtr<IWICBitmapFrameDecode> frame;
    if(SUCCEEDED(hr))hr=decoder->GetFrame(0,&frame);
    UINT width=0,height=0;
    if(SUCCEEDED(hr))hr=frame->GetSize(&width,&height);
    ComPtr<IWICFormatConverter> converter;
    if(SUCCEEDED(hr))hr=imaging->CreateFormatConverter(&converter);
    if(SUCCEEDED(hr))hr=converter->Initialize(frame.Get(),GUID_WICPixelFormat24bppRGB,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom);
    if(FAILED(hr)||!width||!height||uint64_t(width)*height>100000000) {std::printf("Frame setup: 0x%08lX\n",hr);return 1;}
    std::vector<BYTE> pixels(size_t(width)*height*3);
    hr=converter->CopyPixels(nullptr,width*3,static_cast<UINT>(pixels.size()),pixels.data());
    if(FAILED(hr)){std::printf("Pixel decode: 0x%08lX\n",hr);return 1;}
    unsigned long long fingerprint=1469598103934665603ULL;
    for(BYTE b:pixels){fingerprint^=b;fingerprint*=1099511628211ULL;}
    std::printf("PASS: %u x %u, %u frames, %zu RGB bytes, fingerprint %016llX\n",width,height,count,pixels.size(),fingerprint);
    converter.Reset();frame.Reset();decoder.Reset();imaging.Reset();
    if(module)FreeLibrary(module);
    CoUninitialize();return 0;
}
