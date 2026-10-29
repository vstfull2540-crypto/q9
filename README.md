# LA-2A — อาร์ม หนองม่วง × มือเปิ่มสตูดิโอ

Windows VST3/Standalone compressor with a PNG-only faceplate and Neural Amp Modeler Core integration.

## Controls and PNG alignment

The interface is drawn from `Assets/LA2A_Interface.png` at its original 1536×1024 coordinate system. Interactive hit areas are positioned in that same coordinate system and scaled with the window. Rotary controls only draw their moving pointer over the printed knob artwork. The VU meter needle is drawn live over the retouched PNG meter face and responds to the selected input, output, or gain-reduction mode.

## NAM model loading

Click the top model field (where the sample image shows `LA-2A Classic - Medium.nam`) and select a legally obtained `.nam` file. The project fetches and compiles the upstream [NeuralAmpModelerCore (pinned to v0.5.0; dependencies fetched separately to avoid recursive Git submodule failures)](https://github.com/sdatkinson/NeuralAmpModelerCore (pinned to v0.5.0; dependencies fetched separately to avoid recursive Git submodule failures)) sources. Mono-in/mono-out NAM models are instantiated independently for left and right stereo channels. A model must be supplied by the user; it is not bundled in this repository.

The `.nam` load path uses the upstream `nam::get_dsp()` API. Loading happens from the file chooser callback, not inside the audio callback. The model path is saved with the plugin state and reloaded when that path remains available. The gear menu can unload the model and return to the built-in compressor.

## Build on GitHub

1. Upload all repository files while preserving `.github/workflows/build-windows.yml`.
2. Push to `main` or `master`, or select **Actions → Build LA-2A VST3 (Windows) → Run workflow**.
3. Download the `LA2A-Windows-VST3` artifact after a successful run.

## Local build

Use Visual Studio 2022 C++ build tools and CMake 3.22+:

```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

## PNG-only artwork

All interface artwork is contained in the single `Assets/LA2A_Interface.png` PNG. The source code and build workflow remain text files because they are required for compiling the plugin.

## Validation status

The project source and workflow have been updated, but this environment has not executed the GitHub-hosted Windows build. A successful Actions run is still required to verify the exact current JUCE and NAM Core dependency revisions together.


## CI troubleshooting

The Windows workflow fetches NAM Core without recursively initializing its Git submodules, then fetches Eigen and AudioDSPTools explicitly. This avoids the failure mode where CMake FetchContent fails while updating the upstream Eigen submodule. The workflow has been updated, but a successful Windows GitHub Actions run is still required to confirm the full toolchain build.


## FIXED5 build note

This revision updates NeuralAmpModelerCore to **v0.5.4** and pins Eigen to
**5.0.1**. The previous build log showed `Eigen::placeholders::lastN` missing
while compiling `NAM/lstm.cpp`; that indicates the headers actually selected by
the compiler are too old for the NAM LSTM source. This update aligns the NAM
core and Eigen API generation. GitHub Actions must be rerun to confirm the
Windows build end-to-end; a successful ZIP creation is not proof of a successful
Windows compile.


## FIXED6 build note

This revision replaces the deprecated C++20 `std::atomic_load` /
`std::atomic_store` free functions for `std::shared_ptr` with
`std::atomic<std::shared_ptr<NamModelPair>>::load/store`. The screenshot from
the latest CI run shows MSVC C4996/STL4029 deprecation diagnostics in
`PluginProcessor.cpp`; using the C++20 atomic shared-pointer specialization
removes that deprecated API usage. The screenshot does not expose the full
first compiler error, so the workflow must be rerun to confirm the result.


## FIXED7 build note

This revision makes the saved NAM path conversion explicit:
`juce::String(parameters.state.getProperty("namPath"))` rather than chaining
`.toString()` onto the property expression. The latest screenshot only exposes
the tail of the compiler diagnostic, not the full first error line, so this is
a defensive source-compatibility correction and still needs a fresh Windows CI
run to verify.


## FIXED8 build note

The latest MSVC diagnostic identifies the expression at `PluginProcessor.cpp`
around line 198: `juce::String` cannot be constructed directly from a
`juce::var`. FIXED8 corrects this by calling the JUCE-supported
`juce::var::toString()` conversion:
`parameters.state.getProperty("namPath").toString()`.
This directly addresses the visible `String(bool)` / `const juce::var`
overload mismatch. A fresh Windows CI run is still required to verify the
full build.


## FIXED9 build note

The latest screenshot indicates that the expression at `PluginProcessor.cpp`
fails because `toString()` is being looked up on a `juce::String` result.
FIXED9 first stores the property in an explicit `juce::var`, then calls
`juce::var::toString()`. This avoids invoking `toString()` on the original
property expression. Run Windows Actions again to validate the complete build;
a successful build has not been claimed until CI confirms it.


## FIXED10: Windows path conversion

The latest Actions screenshot reports MSVC C2039 because `juce::String` does
not provide `toStdWString()`. FIXED10 replaces that unsupported call with
`std::filesystem::u8path(file.getFullPathName().toStdString())`, using the
JUCE `String::toStdString()` API and filesystem UTF-8 path conversion.
This is a source-level fix for the exact visible diagnostic; a fresh Windows
Actions run is still needed to verify the entire build and catch any later
compiler diagnostics.


## FIXED11 note

Replaced the deprecated `std::filesystem::u8path(std::string)` path conversion
with the Windows wide-character `std::filesystem::path` constructor. The
workflow now captures compiler output and prints matching compiler/configuration
errors with context when a build fails. This improves diagnosis but cannot
guarantee that an unseen compiler error is fixed until GitHub Actions is run.


## FIXED12 — transparent outer canvas

`Assets/LA2A_Interface.png` keeps its original 1536×1024 canvas and all control
coordinates. The white area outside the plugin chassis is now transparent;
the dark outer/inner chassis edge and all controls remain in place. This avoids
moving JUCE hit targets. Check the PNG in an alpha-aware viewer and confirm the
appearance in the target DAW after the next GitHub Actions build.

## FIXED15 — Physical moving lever switches

The three faceplate switches are rendered as interactive two-position lever controls over the supplied PNG. Their visible lever angle/position follows the current state:

- COMPRESS (down) / LIMIT (up)
- OFF (down) / ON (up)
- GAIN REDUCTION (down) / OUTPUT (up)

The background PNG remains the visual base; the switch artwork is painted dynamically so it can move when clicked. Build verification must still be performed by the repository's GitHub Actions workflow.

## FIXED16 — PNG-style lever switches

The three two-position lever controls (COMPRESS/LIMIT, POWER ON/OFF, and VU meter OUTPUT/GAIN REDUCTION) now use compact chrome lever artwork styled to match the switch hardware in `Assets/LA2A_Interface.png`. The control hitboxes were tightened around the original switch locations. Toggle state is reflected visually: UP is LIMIT/ON/OUTPUT and DOWN is COMPRESS/OFF/GAIN REDUCTION.

The faceplate remains the PNG asset. The switch overlays are drawn by JUCE so their levers can move interactively; the top-left toolbar power control remains a push button because that control is a push button in the supplied PNG, not a lever switch.


## FIXED18 artwork alignment

The UI now uses `Assets/LA2A_Interface.png` as its coordinate reference (1536x1024). The checkerboard outside the generated faceplate is transparent; rotary and lever hit areas are aligned to the new PNG. Attack/release parameters remain available for automation/state but are not shown because the selected artwork does not contain those controls.


## Latest interface artwork
The UI uses `Assets/LA2A_Interface.png` as its primary faceplate artwork. The PNG is tightly cropped to the plugin outer edge (checkerboard/outer canvas removed); interactive JUCE controls are positioned as overlays on this artwork. The VU needle is rendered dynamically by the editor.


## Updated faceplate
The plugin uses `Assets/LA2A_Interface.png` as the sole faceplate artwork. Interactive controls and the animated VU needle are drawn by JUCE over this image.


## PNG artwork policy (FIXED22)

- `Assets/LA2A_Interface.png` is the single primary faceplate used by the plugin.
- `Assets/PNG_Parts/` contains the PNG artwork supplied with the project, including the full faceplate and separated top, main, bottom, and logo assets.
- The editor does not draw replacement lever-switch graphics. Switch hit zones are transparent and preserve the switch artwork in the PNG. They still respond to clicks and update the plugin's parameters/meter mode.
- The VU needle remains a dynamic JUCE overlay, because the needle must move with the live audio meter.


## Default control values (restored project)

The initial Gain and Peak Reduction parameter values are both set to 25%. The editor double-click return values and the Default menu action also reset both controls to 25%. This package is based on the previous FIXED24 AsyncDialog project, not the newer InteractiveLevers variant.

## Rotary knob implementation (FIXED29)

- GAIN and PEAK REDUCTION are real `juce::Slider` controls using `RotaryHorizontalVerticalDrag`.
- Each slider is attached to the `gain` / `peak` APVTS parameter, so gestures notify the host and support VST3 automation.
- The live numeric value is displayed below each knob.
- Parameter ranges, defaults and steps are defined centrally in `makeParameters()` in `Source/PluginProcessor.cpp` (GAIN and PEAK REDUCTION: 0–100, default 25, step 0.1).
- The custom rotary LookAndFeel draws a black machined ring, silver metallic face, and a value-linked pointer notch.
- The processor uses the parameter values in the audio path; APVTS state is serialized/restored by `getStateInformation()` / `setStateInformation()`.

A successful ZIP integrity check does not prove that the Windows build succeeds. Run GitHub Actions to compile the VST3 and verify it in Nuendo 15.
