#include "stdafx.h"
#include "PixelCopy.h"
#include "HeifStreamReader.h"
#include "wic_heic.h"
#include <cstdio>
#include <vector>
#include <array>
#include <algorithm>

static int checks = 0;
static void check(bool ok, const char* name) {
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", name); std::exit(1); }
}

class ShortStream : public IStream {
    ULONG refs = 1;
    uint64_t pos = 0;
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** out) override {
        if (!out) return E_POINTER;
        *out = nullptr;
        if (id != IID_IUnknown && id != IID_IStream && id != IID_ISequentialStream) return E_NOINTERFACE;
        *out = this; AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++refs; }
    ULONG STDMETHODCALLTYPE Release() override { ULONG r=--refs; if(!r) delete this; return r; }
    HRESULT STDMETHODCALLTYPE Read(void* out, ULONG n, ULONG* actual) override {
        if(n && out) { *static_cast<BYTE*>(out)=0; pos++; }
        if(actual) *actual=n?1:0;
        return S_OK; // Deliberately claims success despite returning a short read.
    }
    HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER* actual) override {
        if(origin==STREAM_SEEK_SET) pos=move.QuadPart;
        else if(origin==STREAM_SEEK_CUR) pos+=move.QuadPart;
        else return E_INVALIDARG;
        if(actual) actual->QuadPart=pos;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Stat(STATSTG* s, DWORD) override { if(!s)return E_POINTER; *s={};s->cbSize.QuadPart=8;return S_OK; }
    HRESULT STDMETHODCALLTYPE Write(const void*,ULONG,ULONG*) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE CopyTo(IStream*,ULARGE_INTEGER,ULARGE_INTEGER*,ULARGE_INTEGER*) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE Commit(DWORD) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE Revert() override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE Clone(IStream**) override {return E_NOTIMPL;}
};

static void pixel_tests() {
    BYTE input[48]; for(int i=0;i<48;++i)input[i]=BYTE(i);
    std::array<BYTE,64> output; output.fill(0xAB);
    check(SUCCEEDED(CopyValidatedPixels(input,12,4,4,3,nullptr,12,48,output.data()+8)),"full image copy");
    check(std::equal(input,input+48,output.begin()+8),"full image bytes");
    check(output.front()==0xAB && output[7]==0xAB && output[56]==0xAB,"output canaries");
    WICRect crop={1,1,2,2}; output.fill(0xAB);
    check(SUCCEEDED(CopyValidatedPixels(input,12,4,4,3,&crop,8,14,output.data()+8)),"minimal final row with stride padding");
    check(std::equal(input+15,input+21,output.begin()+8) && std::equal(input+27,input+33,output.begin()+16),"cropped pixels");
    check(output[14]==0xAB && output[15]==0xAB && output[22]==0xAB,"padding preserved");
    const WICRect invalid[]={{-1,0,1,1},{0,-1,1,1},{0,0,-1,1},{0,0,1,0},{4,0,1,1},{0,4,1,1},{3,0,2,1},{0,3,1,2},{INT_MAX,0,INT_MAX,1}};
    for(const auto& r:invalid) {
        output.fill(0xAB);
        check(FAILED(CopyValidatedPixels(input,12,4,4,3,&r,12,48,output.data()+8)),"reject invalid rectangle");
        check(std::all_of(output.begin(),output.end(),[](BYTE b){return b==0xAB;}),"invalid rectangle leaves buffer untouched");
    }
    check(FAILED(CopyValidatedPixels(input,12,4,4,3,nullptr,11,48,output.data())),"short output stride");
    check(FAILED(CopyValidatedPixels(input,12,4,4,3,nullptr,12,47,output.data())),"short output buffer");
    check(FAILED(CopyValidatedPixels(input,11,4,4,3,nullptr,12,48,output.data())),"short input stride");
    check(FAILED(CopyValidatedPixels(input,SIZE_MAX,4,4,3,nullptr,12,48,output.data())),"input offset overflow");
    check(CopyValidatedPixels(input,12,4,4,3,nullptr,12,48,nullptr)==E_POINTER,"null output");
}

static void stream_tests() {
    auto* stream = new ShortStream();
    {
        CHeifStreamReader reader(stream); BYTE b[8]={};
        check(reader.read(b,4)!=0,"S_OK short read rejected");
        check(reader.seek(-1)!=0,"negative stream seek rejected");
        check(reader.seek(9)!=0,"seek past EOF rejected");
        check(reader.wait_for_file_size(-1)==heif_reader_grow_status_size_beyond_eof,"negative size rejected");
        check(reader.wait_for_file_size(9)==heif_reader_grow_status_size_beyond_eof,"file growth past EOF rejected");
        check(reader.seek(0)==0,"seek to start");
        check(reader.read(b,size_t(ULONG_MAX)+1)!=0,"ULONG truncation rejected");
    }
    stream->Release();
}

static void decoder_tests(const wchar_t* path) {
    HMODULE dll=LoadLibraryExW(path,nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    check(dll!=nullptr,"load local codec");
    auto getFactory=reinterpret_cast<HRESULT(STDAPICALLTYPE*)(REFCLSID,REFIID,void**)>(GetProcAddress(dll,"DllGetClassObject"));
    auto canUnload=reinterpret_cast<HRESULT(STDAPICALLTYPE*)()>(GetProcAddress(dll,"DllCanUnloadNow"));
    check(getFactory && canUnload,"COM exports present");
    for(int i=0;i<200;++i) {
        IClassFactory* factory=nullptr;
        check(SUCCEEDED(getFactory(CLSID_HEICDecoder,IID_IClassFactory,reinterpret_cast<void**>(&factory))),"get decoder factory");
        IWICBitmapDecoder* decoder=nullptr;
        check(SUCCEEDED(factory->CreateInstance(nullptr,IID_PPV_ARGS(&decoder))),"create decoder");
        factory->Release();
        check(canUnload()==S_FALSE,"live decoder prevents DLL unload");
        std::vector<BYTE> bytes(16+(i%113));
        for(size_t j=0;j<bytes.size();++j)bytes[j]=BYTE((i*73+j*37)&255);
        if(i%2==0) {bytes[0]=0;bytes[1]=0;bytes[2]=0;bytes[3]=24;bytes[4]='f';bytes[5]='t';bytes[6]='y';bytes[7]='p';bytes[8]='h';bytes[9]='e';bytes[10]='i';bytes[11]='c';}
        IStream* stream=SHCreateMemStream(bytes.data(),static_cast<UINT>(bytes.size()));
        check(stream!=nullptr,"create malformed input stream");
        check(FAILED(decoder->Initialize(stream,WICDecodeMetadataCacheOnDemand)),"malformed image rejected");
        stream->Release(); decoder->Release();
        check(canUnload()==S_OK,"decoder releases lifetime lock");
    }
    FreeLibrary(dll);
}

int wmain(int argc,wchar_t** argv) {
    if(argc!=2)return 2;
    HRESULT hr=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
    check(SUCCEEDED(hr),"COM initialize");
    pixel_tests();stream_tests();decoder_tests(argv[1]);
    CoUninitialize();
    std::printf("PASS: %d checks, including 200 malformed HEIC inputs.\n",checks);
    return 0;
}
