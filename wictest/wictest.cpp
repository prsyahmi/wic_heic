// wictest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\wic_heic\wic_heic.h"

#pragma comment (lib, "windowscodecs.lib")

void ConvertToHeic(const std::wstring& srcFile, const std::wstring& destFile)
{
	CComPtr<IWICImagingFactory> pFactory;
	CComPtr<IWICBitmapDecoder> pDecoder;
	CComPtr<IWICBitmapEncoder> pEncoder;
	CComPtr<IWICFormatConverter> pConvertedFrame;
	CComPtr<IWICStream> pStream;
	UINT nFrame;

	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		(LPVOID*)&pFactory);

	if (FAILED(hr)) {
		throw std::exception("Failed to get imaging factory");
	}

	hr = pFactory->CreateEncoder(GUID_ContainerFormatHEIC, &GUID_VendorPrsyahmi, &pEncoder);
	if (FAILED(hr)) {
		throw std::exception("Failed to CreateEncoder");
	}

	hr = pFactory->CreateStream(&pStream);
	if (FAILED(hr)) {
		throw std::exception("Failed to CreateStream");
	}

	hr = pFactory->CreateDecoderFromFilename(
		srcFile.c_str(),
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&pDecoder);

	if (FAILED(hr)) {
		throw std::exception("Failed to CreateDecoderFromFilename");
	}

	hr = pStream->InitializeFromFilename(destFile.c_str(), GENERIC_WRITE);
	if (FAILED(hr)) {
		throw std::exception("Failed to Create Stream file");
	}

	hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
	if (FAILED(hr)) {
		throw std::exception("Failed to Initialize encoder");
	}

	// Copy decoded frame

	hr = pDecoder->GetFrameCount(&nFrame);
	if (FAILED(hr)) {
		throw std::exception("Failed to GetFrameCount");
	}

	for (UINT i = 0; i < nFrame; i++)
	{
		CComPtr<IWICBitmapFrameDecode> pFrameDecode;
		CComPtr<IWICBitmapFrameEncode> pFrameEncode;
		CComPtr<IWICMetadataQueryReader> pFrameQReader;
		CComPtr<IWICMetadataQueryWriter> pFrameQWriter;
		UINT width, height;
		double dpiX, dpiY;
		WICPixelFormatGUID pixFormat;

		hr = pDecoder->GetFrame(i, &pFrameDecode);
		if (FAILED(hr)) {
			throw std::exception("Failed to GetFrame");
		}

		hr = pFrameDecode->GetSize(&width, &height);
		if (FAILED(hr)) {
			throw std::exception("Failed to get image size");
		}

		hr = pFrameDecode->GetResolution(&dpiX, &dpiY);
		if (FAILED(hr)) {
			throw std::exception("Failed to GetResolution");
		}

		hr = pFrameDecode->GetPixelFormat(&pixFormat);
		if (FAILED(hr)) {
			throw std::exception("Failed to GetPixelFormat");
		}

		hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr);
		if (FAILED(hr)) {
			throw std::exception("Failed to CreateNewFrame");
		}

		hr = pFrameEncode->Initialize(nullptr);
		if (FAILED(hr)) {
			throw std::exception("Failed to frame encode");
		}

		hr = pFrameEncode->SetSize(width, height);
		if (FAILED(hr)) {
			throw std::exception("Failed to set image size");
		}

		hr = pFrameEncode->SetResolution(dpiX, dpiY);
		if (FAILED(hr)) {
			throw std::exception("Failed to SetResolution");
		}

		hr = pFrameEncode->SetPixelFormat(&pixFormat);
		if (FAILED(hr)) {
			throw std::exception("Failed to SetPixelFormat");
		}

		hr = pFrameEncode->WriteSource(static_cast<IWICBitmapSource*>(pFrameDecode), NULL);
		if (FAILED(hr)) {
			throw std::exception("Failed to WriteSource");
		}

		hr = pFrameEncode->Commit();
		if (FAILED(hr)) {
			throw std::exception("Failed to Commit frame");
		}
	}

	hr = pEncoder->Commit();
	if (FAILED(hr)) {
		throw std::exception("Failed to Commit encoder");
	}

	printf("File converted successfully\n");
}

int main()
{
	int ret = 0;
	CoInitialize(nullptr);

	try
	{
		ConvertToHeic(L"samples\\01.png", L"samples\\out-01.heic");
		ConvertToHeic(L"samples\\02.png", L"samples\\out-02.heic");
		ConvertToHeic(L"samples\\03.heic", L"samples\\out-03.heic");
		ConvertToHeic(L"samples\\04.heic", L"samples\\out-04.heic");
		ConvertToHeic(L"samples\\05.heic", L"samples\\out-05.heic");
		ConvertToHeic(L"samples\\06.heic", L"samples\\out-06.heic");
		ConvertToHeic(L"samples\\07.heic", L"samples\\out-07.heic");
	}
	catch (const std::exception& ex)
	{
		printf("Exception: %s\n", ex.what());
		ret = 1;
	}

	CoUninitialize();
    return ret;
}

