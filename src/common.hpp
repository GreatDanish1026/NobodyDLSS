// Shared between the capture tool (main.cpp), the TSR hook (upscaler.cpp) and the NGX wrapper
// (dlss.cpp).
#pragma once

#ifndef ImTextureID
#define ImTextureID ImU64
#endif
#include <imgui.h>
#include <reshade.hpp>

#include <cstdint>
#include <string>

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace rd {

// NobodyDLSS.log next to the game exe.
void Log(const char* fmt, ...);

// The folder of the game exe, with a trailing backslash.
const std::wstring& ExeDir();

// FNV-1a of a compute pipeline's shader bytecode (0 if unknown): stable across runs.
uint32_t PipeId(reshade::api::pipeline pipe);

// TSR's compute shaders in this game (Nobody Wants to Die build 14922697, UE 5.3). Not mapped
// yet: UE 5.3's passes may differ from 5.2's, so these are placeholders until a capture with TSR
// chosen in the game's menu; kTsrMapped=false keeps DLSS and frame generation off.
constexpr bool kTsrMapped = false;
// Frame generation's inputs go to their frame when the game submits them (false; UE 5.2 submits
// and presents on the same thread) or when they're recorded (true, ReunionDLSS: UE 5.6 submits on
// a thread of its own). To check for UE 5.3. framegen.cpp is shared with ReunionDLSS.
constexpr bool kTagAtRecord = false;
constexpr uint32_t kTsrClearPrevTextures = 0xfffffff0;
constexpr uint32_t kTsrForwardScatterDepth = 0xfffffff1;
constexpr uint32_t kTsrDilateVelocity = 0xfffffff2;     // t0 depth, t1 velocity -> u0 dilated velocity
constexpr uint32_t kTsrDecimateHistory = 0xfffffff3;    // t0 = DilateVelocity's u0 (links the chain)
constexpr uint32_t kTsrRejectShading = 0xfffffff4;
constexpr uint32_t kTsrSpatialAntiAliasing = 0xfffffff5;
constexpr uint32_t kTsrFilterAntiAliasing = 0xfffffff6;
constexpr uint32_t kTsrUpdateHistory = 0xfffffff7;      // t0 scene colour, t1 separate translucency
constexpr uint32_t kTsrResolveHistory = 0xfffffff8;     // t0 history -> u0 output

// upscaler.cpp
void RegisterUpscaler();
void UnregisterUpscaler();
// Overlay: the Settings tab and the Status tab.
void DrawUpscalerSettings();
void DrawUpscalerStatus();
void DrawUpscalerAdvanced();
// One line for the top of the overlay; *warn when it's a problem.
std::string UpscalerHeadline(bool* warn);

// Overlay helpers (main.cpp), in the Tweaks add-ons' style.
float ControlWidth();
void Help(const char* text);  // a "(?)" after the last item, with <text> as its tooltip
extern const ImVec4 kWarn;
// Puts the game's pipeline, root signatures, descriptor heaps and bindings back on <cl> after
// the add-on recorded work of its own on it.
void RestoreListState(reshade::api::command_list* cl);

// framegen.cpp: DLSS Frame Generation through Streamline.
void RegisterFrameGen();
void UnregisterFrameGen();
void DrawFrameGenSettings();
void DrawFrameGenStatus();
std::string FrameGenHeadline(bool* warn);
// The View uniform buffer (first 16 KB) of this frame's main view, with View.ClipToPrevClip's offset.
void FrameGenSetCamera(const float* view, int clipToPrevClipOffset);
// True if a TSR output of this size is the picture on screen (the back buffer's size).
bool FrameGenIsScreen(unsigned width, unsigned height);
std::string FrameGenDiag();  // diagnostics for the upscaler's diag line
// This frame's depth (UE's, state depth read + shader resource) and our motion vectors (render
// pixels, previous minus current; state non-pixel shader resource), tagged on <list>.
void FrameGenTag(ID3D12GraphicsCommandList* list, ID3D12Resource* depth, ID3D12Resource* motion, unsigned renderWidth,
                 unsigned renderHeight, float jitterX, float jitterY, bool reset);

}  // namespace rd
