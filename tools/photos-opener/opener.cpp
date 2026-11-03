#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <bcrypt.h>
#include <string>
#include <vector>
#include <cstdio>
#include <stdexcept>
#include <algorithm>
#include "wic_heic.h"
#include "release_hash.h"
using Microsoft::WRL::ComPtr;

static void checked(HRESULT hr,const char* action) {
    if(FAILED(hr)){char text[256];sprintf_s(text,"%s failed (0x%08lX)",action,hr);throw std::runtime_error(text);}
}
static void wincheck(bool ok,const char* action){if(!ok){DWORD e=GetLastError();checked(e?HRESULT_FROM_WIN32(e):E_FAIL,action);}}
struct File {
    HANDLE h=INVALID_HANDLE_VALUE;
    explicit File(const std::wstring& p) {h=CreateFileW(p.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);wincheck(h!=INVALID_HANDLE_VALUE,"Read file");}
    ~File(){if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);}
    File(const File&)=delete;File& operator=(const File&)=delete;
};
struct Module {HMODULE h=nullptr;~Module(){if(h)FreeLibrary(h);}};
struct TemporaryFile {std::wstring path;~TemporaryFile(){if(!path.empty())DeleteFileW(path.c_str());}};

static std::string hash_file(File& file) {
    LARGE_INTEGER size={};wincheck(GetFileSizeEx(file.h,&size)!=FALSE,"Read file size");
    if(size.QuadPart<=0 || size.QuadPart>512LL*1024*1024)throw std::runtime_error("File is empty or exceeds the 512 MiB limit.");
    LARGE_INTEGER start={};wincheck(SetFilePointerEx(file.h,start,nullptr,FILE_BEGIN)!=FALSE,"Read file from start");
    BCRYPT_ALG_HANDLE alg=nullptr;BCRYPT_HASH_HANDLE hash=nullptr;DWORD length=0,actual=0;
    bool ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok)ok=BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,reinterpret_cast<BYTE*>(&length),sizeof(length),&actual,0)>=0;
    std::vector<BYTE> object(length),buffer(65536);BYTE digest[32]={};
    if(ok)ok=BCryptCreateHash(alg,&hash,object.data(),length,nullptr,0,0)>=0;
    while(ok){DWORD n=0;if(!ReadFile(file.h,buffer.data(),static_cast<DWORD>(buffer.size()),&n,nullptr)){ok=false;break;}if(!n)break;ok=BCryptHashData(hash,buffer.data(),n,0)>=0;}
    if(ok)ok=BCryptFinishHash(hash,digest,sizeof(digest),0)>=0;
    if(hash)BCryptDestroyHash(hash);if(alg)BCryptCloseAlgorithmProvider(alg,0);
    if(!ok)throw std::runtime_error("SHA-256 calculation failed.");
    const char* hex="0123456789abcdef";std::string text;
    for(BYTE b:digest){text+=hex[b>>4];text+=hex[b&15];}return text;
}
static std::wstring cache_folder() {
    wchar_t temp[32768]={};DWORD n=GetTempPathW(32768,temp);wincheck(n>0 && n<32768,"Find cache folder");
    std::wstring p=std::wstring(temp)+L"WicHeicLocalCache-1.1.17";
    if(!CreateDirectoryW(p.c_str(),nullptr)){
        DWORD err=GetLastError();if(err!=ERROR_ALREADY_EXISTS){SetLastError(err);wincheck(false,"Create image cache");}
        DWORD attributes=GetFileAttributesW(p.c_str());
        if(attributes==INVALID_FILE_ATTRIBUTES || !(attributes&FILE_ATTRIBUTE_DIRECTORY) || (attributes&FILE_ATTRIBUTE_REPARSE_POINT))throw std::runtime_error("Image cache is not a regular directory.");
    }
    return p;
}
static void write_png(IWICImagingFactory* imaging,const std::wstring& name,UINT width,UINT height,const std::vector<BYTE>& pixels) {
    ComPtr<IWICStream> stream;checked(imaging->CreateStream(&stream),"Create PNG stream");
    checked(stream->InitializeFromFilename(name.c_str(),GENERIC_WRITE),"Write PNG file");
    ComPtr<IWICBitmapEncoder> encoder;checked(imaging->CreateEncoder(GUID_ContainerFormatPng,nullptr,&encoder),"Create PNG encoder");
    checked(encoder->Initialize(stream.Get(),WICBitmapEncoderNoCache),"Initialize PNG encoder");
    ComPtr<IWICBitmapFrameEncode> frame;checked(encoder->CreateNewFrame(&frame,nullptr),"Create PNG frame");
    checked(frame->Initialize(nullptr),"Initialize PNG frame");checked(frame->SetSize(width,height),"Set PNG dimensions");
    WICPixelFormatGUID format=GUID_WICPixelFormat24bppRGB;checked(frame->SetPixelFormat(&format),"Set PNG pixel format");
    std::vector<BYTE> reordered;
    BYTE* data=const_cast<BYTE*>(pixels.data());
    if(format==GUID_WICPixelFormat24bppBGR){
        reordered=pixels;for(size_t i=0;i<reordered.size();i+=3)std::swap(reordered[i],reordered[i+2]);data=reordered.data();
    }else if(format!=GUID_WICPixelFormat24bppRGB)throw std::runtime_error("Unexpected PNG pixel format.");
    checked(frame->WritePixels(height,width*3,static_cast<UINT>(pixels.size()),data),"Encode PNG pixels");
    checked(frame->Commit(),"Commit PNG frame");checked(encoder->Commit(),"Commit PNG file");
}
static std::vector<BYTE> read_pixels(IWICImagingFactory* imaging,IWICBitmapDecoder* decoder,UINT& width,UINT& height) {
    ComPtr<IWICBitmapFrameDecode> frame;checked(decoder->GetFrame(0,&frame),"Read HEIC frame");
    checked(frame->GetSize(&width,&height),"Read image dimensions");
    if(!width || !height || uint64_t(width)*height>100000000)throw std::runtime_error("Image dimensions exceed the safety limit.");
    ComPtr<IWICFormatConverter> converted;checked(imaging->CreateFormatConverter(&converted),"Create pixel converter");
    checked(converted->Initialize(frame.Get(),GUID_WICPixelFormat24bppRGB,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom),"Convert image pixels");
    std::vector<BYTE> pixels(size_t(width)*height*3);
    checked(converted->CopyPixels(nullptr,width*3,static_cast<UINT>(pixels.size()),pixels.data()),"Decode image pixels");return pixels;
}
static void open_photos(const std::wstring& png) {
    ComPtr<IShellItem> item;checked(SHCreateItemFromParsingName(png.c_str(),nullptr,IID_PPV_ARGS(&item)),"Prepare image for Photos");
    ComPtr<IShellItemArray> items;checked(SHCreateShellItemArrayFromShellItem(item.Get(),IID_PPV_ARGS(&items)),"Prepare Photos file activation");
    ComPtr<IApplicationActivationManager> activation;
    checked(CoCreateInstance(CLSID_ApplicationActivationManager,nullptr,CLSCTX_LOCAL_SERVER,IID_PPV_ARGS(&activation)),"Connect to Microsoft Photos");
    CoAllowSetForegroundWindow(activation.Get(),nullptr);
    DWORD pid=0;checked(activation->ActivateForFile(L"Microsoft.Windows.Photos_8wekyb3d8bbwe!App",items.Get(),L"open",&pid),"Open image in Microsoft Photos");
    std::printf("Microsoft Photos activated (process %lu).\n",pid);
}
static int self_test(IWICImagingFactory* imaging) {
    const std::wstring folder=cache_folder();wchar_t temp[MAX_PATH]={};
    wincheck(GetTempFileNameW(folder.c_str(),L"wic",0,temp)!=0,"Create test PNG");TemporaryFile file;file.path=temp;
    std::vector<BYTE> expected(4*4*3);for(size_t i=0;i<expected.size();i++)expected[i]=BYTE(i*5);
    write_png(imaging,file.path,4,4,expected);
    ComPtr<IWICBitmapDecoder> png;checked(imaging->CreateDecoderFromFilename(file.path.c_str(),nullptr,GENERIC_READ,WICDecodeMetadataCacheOnDemand,&png),"Read generated PNG");
    UINT width=0,height=0;auto actual=read_pixels(imaging,png.Get(),width,height);
    if(width!=4 || height!=4 || actual!=expected)throw std::runtime_error("PNG pixel round-trip failed.");
    png.Reset();
    File first(file.path),second(file.path);if(hash_file(first)!=hash_file(second))throw std::runtime_error("Cache hash consistency failed.");
    std::puts("PASS: PNG encode/decode preserves all pixels; content hashes are consistent. No Photos window opened.");return 0;
}
static int run(int argc,wchar_t** argv) {
    checked(CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED),"Initialize image opener");
    ComPtr<IWICImagingFactory> imaging;checked(CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&imaging)),"Create Windows imaging factory");
    if(argc==2 && std::wstring(argv[1])==L"--self-test")return self_test(imaging.Get());
    const bool noOpen=argc==3 && std::wstring(argv[1])==L"--no-open";
    if(argc!=2 && !noOpen)throw std::runtime_error("Choose Open with > HEIC in Microsoft Photos, or drag a HEIC photo onto WicHeicOpen.exe.");
    const std::wstring input=argv[noOpen?2:1];
    wchar_t moduleName[32768]={};DWORD n=GetModuleFileNameW(nullptr,moduleName,32768);wincheck(n>0 && n<32768,"Find image opener");
    std::wstring folder(moduleName);folder=folder.substr(0,folder.find_last_of(L"\\/"));
    const std::wstring dllPath=folder+L"\\wic_heic_local.dll";
    File dllFile(dllPath);if(hash_file(dllFile)!=EXPECTED_DLL_SHA256)throw std::runtime_error("The codec DLL does not match this tested build.");
    File source(input);const std::string hash=hash_file(source);
    const std::wstring cache=cache_folder();GUID cacheId={};checked(CoCreateGuid(&cacheId),"Name cached image");
    wchar_t cacheToken[40]={};StringFromGUID2(cacheId,cacheToken,40);
    const std::wstring png=cache+L"\\"+std::wstring(hash.begin(),hash.end())+L"-"+cacheToken+L".png";
    // Regenerate each time to avoid trusting an existing cache file.
    Module module;module.h=LoadLibraryExW(dllPath.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);wincheck(module.h!=nullptr,"Load HEIC decoder");
    auto getFactory=reinterpret_cast<HRESULT(STDAPICALLTYPE*)(REFCLSID,REFIID,void**)>(GetProcAddress(module.h,"DllGetClassObject"));
    if(!getFactory)throw std::runtime_error("The codec has no decoder factory.");
    ComPtr<IClassFactory> factory;checked(getFactory(CLSID_HEICDecoder,IID_PPV_ARGS(&factory)),"Create HEIC factory");
    ComPtr<IWICBitmapDecoder> decoder;checked(factory->CreateInstance(nullptr,IID_PPV_ARGS(&decoder)),"Create HEIC decoder");
    ComPtr<IStream> stream;checked(SHCreateStreamOnFileEx(input.c_str(),STGM_READ|STGM_SHARE_DENY_WRITE,FILE_ATTRIBUTE_NORMAL,FALSE,nullptr,&stream),"Open original HEIC");
    checked(decoder->Initialize(stream.Get(),WICDecodeMetadataCacheOnDemand),"Read original HEIC");
    UINT width=0,height=0;const auto pixels=read_pixels(imaging.Get(),decoder.Get(),width,height);
    wchar_t temp[MAX_PATH]={};wincheck(GetTempFileNameW(cache.c_str(),L"wic",0,temp)!=0,"Create cached image");TemporaryFile temporary;temporary.path=temp;
    write_png(imaging.Get(),temporary.path,width,height,pixels);
    wincheck(MoveFileExW(temporary.path.c_str(),png.c_str(),MOVEFILE_WRITE_THROUGH)!=FALSE,"Save cached image");temporary.path.clear();
    std::wprintf(L"Decoded %u x %u. Local PNG copy: %ls\n",width,height,png.c_str());
    if(!noOpen)open_photos(png);
    return 0;
}
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int) {
    int argc=0;wchar_t** argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(!argv)return 1;
    const bool diagnostic=argc>1 && (std::wstring(argv[1])==L"--no-open" || std::wstring(argv[1])==L"--self-test");
    int result=0;
    try {result=run(argc,argv);}catch(const std::exception& e){std::fprintf(stderr,"HEIC opener: %s\n",e.what());if(!diagnostic)MessageBoxA(nullptr,e.what(),"HEIC in Microsoft Photos",MB_OK|MB_ICONERROR);result=1;}
    LocalFree(argv);return result;
}
