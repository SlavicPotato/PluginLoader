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
			kDirect3DCreate9On12 = 0,
			kDirect3DCreate9On12Ex = 1,
			kDirect3DShaderValidatorCreate9 = 2,
			kPSGPError = 3,
			kPSGPSampleTexture = 4,
			kD3DPERF_BeginEvent = 5,
			kD3DPERF_EndEvent = 6,
			kD3DPERF_GetStatus = 7,
			kD3DPERF_QueryRepeatFrame = 8,
			kD3DPERF_SetMarker = 9,
			kD3DPERF_SetOptions = 10,
			kD3DPERF_SetRegion = 11,
			kDebugSetLevel = 12,
			kDebugSetMute = 13,
			kDirect3D9EnableMaximizedWindowedModeShim = 14,
			kDirect3DCreate9 = 15,
			kDirect3DCreate9Ex = 16
		};
	}
}



namespace Loader {
	namespace Version {
		constexpr uint32_t DXGI = 0x1;
		constexpr uint32_t D3D9 = 0x2;
	}
	
}