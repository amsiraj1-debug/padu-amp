# Padu Amp

Padu Amp is a clean, source-built Windows VST3 guitar saturation/amp effect made with JUCE 7.0.12.

## Controls

- Input
- Gain
- Drive
- Bass
- Mid
- Treble
- Presence
- Mix
- Output

There is **no trial timer, white-noise injection, login page, activation endpoint, or account requirement** in this project.

## Build locally on Windows

Requirements:
- Visual Studio 2022 with Desktop development with C++
- CMake 3.22+
- Git

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target PaduAmp_VST3
```

The VST3 bundle is generated under the build tree in the JUCE artefacts folder.

## Build with GitHub Actions

Push this repository to GitHub. The workflow `.github/workflows/build.yml` runs automatically on `main` and also supports manual runs from **Actions > Build Windows VST3 > Run workflow**.

When the build succeeds, download the artifact named `Padu-Amp-Windows-VST3`.

## Notes

This is a fresh implementation. It does not contain or depend on licensing/authentication code from the previous Slam Amp binary.
