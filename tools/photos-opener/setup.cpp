#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include "release_hash.h"

static const std::wstring CLSID_LOCAL=L"{a1c9d2a7-72b6-4ac4-94f8-8a7f07c63e21}";
static const std::wstring CODEC=L"Software\\Classes\\CLSID\\"+CLSID_LOCAL;
static const std::wstring CATEGORY=L"Software\\Classes\\CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\"+CLSID_LOCAL;

static void require(bool ok,const char* action) {
    if(!ok){std::fprintf(stderr,"%s (Windows error %lu)\n",action,GetLastError());throw std::runtime_error(action);}
}
static std::string sha256(const std::wstring& path) {
    HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    require(file!=INVALID_HANDLE_VALUE,"Open codec for hash verification");
    BCRYPT_ALG_HANDLE alg=nullptr;BCRYPT_HASH_HANDLE hash=nullptr;
    DWORD length=0,actual=0;
    bool ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok)ok=BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,reinterpret_cast<BYTE*>(&length),sizeof(length),&actual,0)>=0;
    std::vector<BYTE> object(length),buffer(65536);BYTE digest[32]={};
    if(ok)ok=BCryptCreateHash(alg,&hash,object.data(),length,nullptr,0,0)>=0;
    while(ok){DWORD n=0;if(!ReadFile(file,buffer.data(),static_cast<DWORD>(buffer.size()),&n,nullptr)){ok=false;break;}if(!n)break;ok=BCryptHashData(hash,buffer.data(),n,0)>=0;}
    if(ok)ok=BCryptFinishHash(hash,digest,sizeof(digest),0)>=0;
    if(hash)BCryptDestroyHash(hash);if(alg)BCryptCloseAlgorithmProvider(alg,0);CloseHandle(file);
    require(ok,"SHA-256 verification");
    const char* hex="0123456789abcdef";std::string out;
    for(BYTE b:digest){out+=hex[b>>4];out+=hex[b&15];}return out;
}
static void value(const std::wstring& path,const wchar_t* name,DWORD kind,const void* bytes,DWORD size) {
    HKEY key=nullptr;
    LSTATUS status=RegCreateKeyExW(HKEY_CURRENT_USER,path.c_str(),0,nullptr,0,KEY_SET_VALUE,nullptr,&key,nullptr);
    if(status!=ERROR_SUCCESS){SetLastError(status);require(false,"Create current-user codec registry key");}
    status=RegSetValueExW(key,name,0,kind,static_cast<const BYTE*>(bytes),size);RegCloseKey(key);
    if(status!=ERROR_SUCCESS){SetLastError(status);require(false,"Set current-user codec registry value");}
}
static void string_value(const std::wstring& p,const wchar_t* name,const std::wstring& text){value(p,name,REG_SZ,text.c_str(),static_cast<DWORD>((text.size()+1)*sizeof(wchar_t)));}
static void number_value(const std::wstring& p,const wchar_t* name,DWORD n){value(p,name,REG_DWORD,&n,sizeof(n));}
static void unregister_codec(){
    RegDeleteTreeW(HKEY_CURRENT_USER,CATEGORY.c_str());RegDeleteTreeW(HKEY_CURRENT_USER,CODEC.c_str());
    RegDeleteTreeW(HKEY_CURRENT_USER,L"Software\\Classes\\WicHeicLocal.Image");
    RegDeleteTreeW(HKEY_CURRENT_USER,L"Software\\Classes\\Applications\\WicHeicOpen.exe");
    for(const wchar_t* extension:{L".heic",L".heif"}){
        const std::wstring p=std::wstring(L"Software\\Classes\\")+extension+L"\\OpenWithProgids";
        HKEY key=nullptr;if(RegOpenKeyExW(HKEY_CURRENT_USER,p.c_str(),0,KEY_SET_VALUE,&key)==ERROR_SUCCESS){RegDeleteValueW(key,L"WicHeicLocal.Image");RegCloseKey(key);}
    }
}
static void directory(const std::wstring& path) {
    if(!CreateDirectoryW(path.c_str(),nullptr)) {
        const DWORD err=GetLastError();
        if(err==ERROR_ALREADY_EXISTS && (GetFileAttributesW(path.c_str())&FILE_ATTRIBUTE_DIRECTORY))return;
        SetLastError(err);require(false,"Create permanent installation folder");
    }
}
int wmain(int argc,wchar_t** argv) {
    const bool quiet=argc>1 && std::wstring(argv[1])==L"--quiet";
    int result=0;
    try {
        if(argc>1 && std::wstring(argv[1])==L"--uninstall") {
            unregister_codec();std::puts("WicHeic Local unregistered. Close Photos before removing its DLL.");return 0;
        }
        wchar_t module[32768]={},appdata[32768]={};
        DWORD n=GetModuleFileNameW(nullptr,module,32768);require(n && n<32768,"Find setup folder");
        std::wstring folder(module);folder=folder.substr(0,folder.find_last_of(L"\\/"));
        const std::wstring source=folder+L"\\wic_heic_local.dll";
        require(sha256(source)==EXPECTED_DLL_SHA256,"Source codec hash does not match this tested installer");
        n=GetEnvironmentVariableW(L"LOCALAPPDATA",appdata,32768);require(n && n<32768,"Find local app data folder");
        const std::wstring app=std::wstring(appdata)+L"\\WicHeicLocal";
        const std::wstring destination=app+L"\\1.1.17.0";
        directory(app);directory(destination);
        const std::wstring installed=destination+L"\\wic_heic_local.dll";
        const bool sameCodec=GetFileAttributesW(installed.c_str())!=INVALID_FILE_ATTRIBUTES && sha256(installed)==EXPECTED_DLL_SHA256;
        if(!sameCodec)require(CopyFileW(source.c_str(),installed.c_str(),FALSE)!=0,"Copy codec into permanent install folder");
        require(sha256(installed)==EXPECTED_DLL_SHA256,"Installed codec hash does not match tested build");
        const std::wstring opener=destination+L"\\WicHeicOpen.exe";
        const std::wstring openerSource=folder+L"\\WicHeicOpen.exe";
        require(sha256(openerSource)==EXPECTED_OPENER_SHA256,"HEIC opener hash does not match this tested build");
        require(CopyFileW(openerSource.c_str(),opener.c_str(),FALSE)!=0,"Install HEIC opener");
        require(sha256(opener)==EXPECTED_OPENER_SHA256,"Installed HEIC opener hash verification");
        HKEY existing=nullptr;
        const bool hadRegistration=RegOpenKeyExW(HKEY_CURRENT_USER,CODEC.c_str(),0,KEY_READ,&existing)==ERROR_SUCCESS;
        if(existing)RegCloseKey(existing);
        try {
            string_value(CODEC,L"FriendlyName",L"WicHeic Local 1.1.17 Decoder");
            string_value(CODEC,L"Description",L"WicHeic Local 1.1.17 Decoder");
            string_value(CODEC,L"Author",L"Local build from prsyahmi/wic_heic");
            string_value(CODEC,L"ContainerFormat",L"{98ce31dd-5435-41bd-a1af-29c387c20135}");
            string_value(CODEC,L"FileExtensions",L".heic,.heif,.heics,.heifs");
            string_value(CODEC,L"MimeTypes",L"image/heic,image/heif");
            string_value(CODEC,L"Vendor",L"{d01102c8-6108-41dd-bd60-1f3a8e7f2447}");
            string_value(CODEC,L"Version",L"1.1.17.0");string_value(CODEC,L"SpecVersion",L"1.1.17.0");
            number_value(CODEC,L"ArbitrationPriority",100);number_value(CODEC,L"SupportAnimation",0);
            number_value(CODEC,L"SupportChromaKey",0);number_value(CODEC,L"SupportLossless",0);number_value(CODEC,L"SupportMultiframe",1);
            string_value(CODEC+L"\\InprocServer32",L"",installed);string_value(CODEC+L"\\InprocServer32",L"ThreadingModel",L"Both");
            // A value on the format subkey is harmless; existence advertises the format.
            string_value(CODEC+L"\\Formats\\{6fddc324-4e03-4bfe-b185-3d77768dc90d}",L"",L"");
            const char* brands[]={"heic","heix","hevc","heim","heis","hevm","hevs","mif1","msf1"};
            const BYTE mask[]={255,255,255,255,255,255,255,255};
            for(int i=0;i<9;++i) {
                const std::wstring p=CODEC+L"\\Patterns\\"+std::to_wstring(i);
                const std::string pattern=std::string("ftyp")+brands[i];
                value(p,L"Pattern",REG_BINARY,pattern.data(),8);value(p,L"Mask",REG_BINARY,mask,8);
                number_value(p,L"Length",8);number_value(p,L"Position",4);
            }
            string_value(CATEGORY,L"CLSID",CLSID_LOCAL);string_value(CATEGORY,L"FriendlyName",L"WicHeic Local 1.1.17 Decoder");
            const std::wstring command=L"\""+opener+L"\" \"%1\"";
            const std::wstring application=L"Software\\Classes\\Applications\\WicHeicOpen.exe";
            const std::wstring progid=L"Software\\Classes\\WicHeicLocal.Image";
            string_value(progid,L"",L"HEIC in Microsoft Photos");
            string_value(progid+L"\\shell\\open\\command",L"",command);
            string_value(application,L"FriendlyAppName",L"HEIC in Microsoft Photos");
            string_value(application+L"\\shell\\open\\command",L"",command);
            for(const wchar_t* extension:{L".heic",L".heif"}){
                string_value(application+L"\\SupportedTypes",extension,L"");
                const std::wstring p=std::wstring(L"Software\\Classes\\")+extension+L"\\OpenWithProgids";
                value(p,L"WicHeicLocal.Image",REG_NONE,nullptr,0);
            }
        } catch(...) {if(!hadRegistration)unregister_codec();throw;}
        // Preserve the installer to provide its --uninstall command later.
        CopyFileW(module,(destination+L"\\WicHeicLocalSetup.exe").c_str(),FALSE);
        CopyFileW((folder+L"\\release-manifest.json").c_str(),(destination+L"\\release-manifest.json").c_str(),FALSE);
        std::wprintf(L"Installed for this Windows user: %ls\nRight-click a HEIC image, choose Open with > HEIC in Microsoft Photos, and select Always.\n",installed.c_str());
    }catch(const std::exception& e){std::fprintf(stderr,"Installation incomplete: %s\n",e.what());result=1;}
    if(!quiet){std::puts("Press Enter to close.");std::getchar();}return result;
}
