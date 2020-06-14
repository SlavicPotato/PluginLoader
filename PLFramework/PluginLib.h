#pragma once

namespace Plugin 
{
	namespace Proxy
	{
		enum class Index : uint32_t {
			kApplyCompatResolutionQuirking = 0,
			kCompatString = 1,
			kCompatValue = 2,
			kDXGIDumpJournal = 3,
			kPIXBeginCapture = 4,
			kPIXEndCapture = 5,
			kPIXGetCaptureState = 6,
			kSetAppCompatStringPointer = 7,
			kUpdateHMDEmulationStatus = 8,
			kCreateDXGIFactory = 9,
			kCreateDXGIFactory1 = 10,
			kCreateDXGIFactory2 = 11,
			kDXGID3D10CreateDevice = 12,
			kDXGID3D10CreateLayeredDevice = 13,
			kDXGID3D10GetLayeredDeviceSize = 14,
			kDXGID3D10RegisterLayers = 15,
			kDXGIDeclareAdapterRemovalSupport = 16,
			kDXGIGetDebugInterface1 = 17,
			kDXGIReportAdapterConfiguration = 18
		} ;
	}

	namespace ProxyD3D9
	{
		enum class Index : uint32_t {
			kOrdinal16 = 0,
			kOrdinal17 = 1,
			kOrdinal18 = 2,
			kOrdinal19= 3,
			kDirect3DCreate9On12 = 4,
			kDirect3DCreate9On12Ex = 5,
			kOrdinal22 = 6,
			kOrdinal23 = 7,
			kDirect3DShaderValidatorCreate9 = 8,
			kPSGPError = 9,
			kPSGPSampleTexture = 10,
			kD3DPERF_BeginEvent = 11,
			kD3DPERF_EndEvent = 12,
			kD3DPERF_GetStatus = 13,
			kD3DPERF_QueryRepeatFrame = 14,
			kD3DPERF_SetMarker = 15,
			kD3DPERF_SetOptions = 16,
			kD3DPERF_SetRegion = 17,
			kDebugSetLevel = 18,
			kDebugSetMute = 19,
			kDirect3D9EnableMaximizedWindowedModeShim = 20,
			kDirect3DCreate9 = 21,
			kDirect3DCreate9Ex = 22
		};
	}
}



namespace Loader {
	namespace Version {
		constexpr uint32_t DXGI = 0x1;
		constexpr uint32_t D3D9 = 0x2;
	}
	
}