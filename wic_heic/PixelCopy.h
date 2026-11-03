#pragma once
#include <windows.h>
#include <wincodec.h>
#include <cstdint>
#include <cstring>

inline HRESULT CopyValidatedPixels(const BYTE* plane, size_t sourceStride,
    int width, int height, int bytesPerPixel, const WICRect* requested,
    UINT outputStride, UINT outputSize, BYTE* output)
{
    if (!output) return E_POINTER;
    if (!plane || width <= 0 || height <= 0 || bytesPerPixel <= 0) return WINCODEC_ERR_BADIMAGE;
    const WICRect full = {0, 0, width, height};
    const WICRect& r = requested ? *requested : full;
    if (r.X < 0 || r.Y < 0 || r.Width <= 0 || r.Height <= 0 ||
        r.X > width || r.Y > height || r.Width > width-r.X || r.Height > height-r.Y)
        return E_INVALIDARG;
    const uint64_t fullRow = uint64_t(width) * bytesPerPixel;
    const uint64_t rowBytes = uint64_t(r.Width) * bytesPerPixel;
    const uint64_t required = uint64_t(r.Height-1) * outputStride + rowBytes;
    if (fullRow > sourceStride || rowBytes > outputStride || required > outputSize)
        return E_INVALIDARG;
    if (sourceStride > SIZE_MAX / static_cast<size_t>(height)) return E_INVALIDARG;
    const BYTE* input = plane + sourceStride * static_cast<size_t>(r.Y) + size_t(r.X) * bytesPerPixel;
    for (int row=0; row<r.Height; ++row) {
        std::memcpy(output + size_t(row)*outputStride, input, static_cast<size_t>(rowBytes));
        if (row+1 < r.Height) input += sourceStride;
    }
    return S_OK;
}
