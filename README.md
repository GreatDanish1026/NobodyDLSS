# NobodyDLSS 1.0.0

ReShade add-on for **Nobody Wants to Die** (Unreal Engine 5.3, DirectX 12) that replaces the
game's TSR upscaler with **NVIDIA DLSS** or **AMD FSR**, and adds **DLSS Frame Generation** or
**FSR Frame Generation**. The game's own DLSS only offers models A-G and has no frame generation;
its FSR frame generation only works with FSR upscaling. It's separate from NobodyTweaks (graphics
settings, hardware ray tracing, HDR), and the two can be installed side by side.

| | Graphics card |
|---|---|
| DLSS upscaling | NVIDIA RTX |
| FSR upscaling | any DirectX 12 card: FSR 4 on Radeon RX 6000 and newer, FSR 3.1 elsewhere |
| DLSS Frame Generation | NVIDIA RTX 40 or 50 (3x, 4x and Dynamic: RTX 50), hardware-accelerated GPU scheduling |
| FSR Frame Generation | any DirectX 12 card: FSR 4 on Radeon RX 9000 under Windows 11, FSR 3.1 elsewhere |

ReShade 6.8 with add-on support. Upscaler and frame generation can be mixed (for example DLSS
upscaling with FSR Frame Generation on an RTX 30 card).

## What it does

- **Upscaler** (F9 to switch on and off): Auto (DLSS on NVIDIA RTX, FSR elsewhere), DLSS or FSR in
  place of TSR. DLSS Super Resolution 310.9.16; FSR through AMD's FSR SDK 2.4.0. Motion vectors
  are computed by the add-on from the depth buffer and the camera, plus the engine's own motion
  of moving objects. The game's separate translucency layer (particles, light beams, fog) is
  blended in before upscaling.
- **Render resolution**: Native/DLAA, Quality, Balanced, Performance or Ultra Performance; "Game's
  Resolution Scale" leaves it to the game's menu.
- **DLSS model**: K (transformer, default), J, L, M or NVIDIA's default. **FSR sharpness**: 0-1.
- **Frame generation**: Off, DLSS 2x / 3x / 4x / Dynamic (NVIDIA Streamline 2.14.16, with Reflex),
  or FSR 2x. Switching it on, or between DLSS and FSR, takes a game restart.

**Keep the game's upscaling menu on TSR**: the add-on replaces TSR's output.

**What it changes in the game's settings.** The game's own DLSS plugin starts NVIDIA's NGX at
launch, and NGX can't be shared with the add-on's DLSS (it crashed); the game's FSR 3 plugin
replaces the swapchain, which frame generation can't share. So when DLSS or frame generation is
switched on, the add-on adds these lines to
`%LOCALAPPDATA%\detnoir\Saved\Config\Windows\Engine.ini` (read at the next launch, hence one
restart):

```
[SystemSettings]
r.NGX.Enable=0
r.FidelityFX.FI.OverrideSwapChainDX12=0
```

The game's own DLSS and FSR frame generation menu options then do nothing; delete the lines to
get them back.

The overlay (ReShade's Add-ons tab) shows a one-line summary at the top, with **Settings**,
**Status** and **Advanced** tabs. Settings are saved in `ReShade.ini` under `[NobodyDLSS]`.

## Install

1. Install ReShade 6.8 or later with full add-on support for `detnoir-Win64-Shipping.exe`, in
   `Nobody Wants to Die\detnoir\Binaries\Win64`.
2. Copy everything from the release zip into that `Win64` folder: `NobodyDLSS.addon64`,
   `nvngx_dlss.dll`, the three `amd_fidelityfx_*_dx12.dll` files and the `streamline` folder.
3. Choose TSR in the game's upscaling menu. Open ReShade (Home key), Add-ons tab, NobodyDLSS:
   switch the upscaler on (or press F9), pick a render resolution and, if you like, frame
   generation. Restart the game once.

Tested with Steam build 14922697 at 3840x2160 with HDR (through NobodyTweaks) on an NVIDIA RTX
card: DLSS upscaling, DLSS Frame Generation 2x, FSR 3.1 upscaling and FSR Frame Generation 2x. FSR 4
on Radeon cards goes through the same code but hasn't been tested on one.

To uninstall, delete those files (and the Engine.ini lines). Logs: `NobodyDLSS.log` next to the
game's exe, Streamline's in `streamline\sl.log`.

## Build

`.\build.ps1` (Visual Studio 2022 C++ tools, Windows SDK for `fxc`). The runtimes aren't in the
repository: unzip `bin\x64` of NVIDIA's Streamline 2.14.16 release into
`third_party\streamline\bin\x64`, and copy `amd_fidelityfx_loader_dx12.dll`,
`amd_fidelityfx_upscaler_dx12.dll` and `amd_fidelityfx_framegeneration_dx12.dll` from
`Kits\FidelityFX\signedbin` of AMD's FSR SDK 2.4.0 into `third_party\fidelityfx\bin`.
`.\build.ps1 -Install` copies the add-on and the runtimes into the game folder (game closed);
`.\build.ps1 -Package` makes the release zip in `bin\release`.

## How it works

Ported from DoubleExposureDLSS (UE 5.2). `docs/FINDINGS.md` has what differs in UE 5.3: TSR's
passes split over the async compute and graphics queues, no ResolveHistory (UpdateHistory writes
the output), the chain linked across queues, the View uniform buffer offsets (UE 5.6's), and the
game's own NGX and FSR 3 swapchain. `docs/FSR.md` covers FSR. The F10 frame capture (Advanced tab)
writes two frames of GPU commands to `NobodyDLSS-frame-N.txt`, for finding TSR's passes again
after a game update.
