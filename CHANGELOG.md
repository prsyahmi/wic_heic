## v1.1.6 - 2026-07-21
- Fix issue reported by static code analysis 
- log: Fix DbgLog leaves behind argument list on release build 

## v1.1.5 - 2026-07-21
- Fix MSI version

## v1.1.4 - 2026-07-20
- Fix decoding bpp
- Fix encoding WritePixels back to source buffer

## v1.0.8 - 2026-04-19
- Fix palette and thumbnail return value
- Implement GetDecoderInfo / GetIncoderInfo
- Decode image during CopyPixels
- Remove GetPreview implementation
- installer: Add UI

## v1.0.7 - 2025-08-14
- Fix opening on wine due to non-zero initial seek offset

## v1.0.6 - 2025-08-14
- Register .heic, .heif as file image type for explorer and thumbnail cache
- Fix memory leaks

## v1.0.5 - 2025-08-13
- Add installer

## v1.0.4 - 2025-08-13
- Replace x265 encoder with libkvazaar
- Update libde265 to 1.0.16, libheif to 1.21.0
- Decode in 8-bit only, >8 bit will be converted
- Add more heif file format

## v1.0.3 - 2022-09-26
- x86: Fix missing exports for registering/unregistering dll

## v1.0.2 - 2022-09-21
- Fix crash when reading thumbnails from heic file without thumbnails

## v1.0.1 - 2022-09-21
- Switch to static build
- Add encoding support (8 bit only)
- Add thumbnail/preview support

## v1.0.0 - 2022-09-19
- Initial Release
