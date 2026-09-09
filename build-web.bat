@echo off
setlocal

where emcmake >nul 2>nul
if errorlevel 1 (
  echo [CTR Web] emcmake was not found in PATH.
  echo [CTR Web] Open an emsdk-enabled command prompt or run emsdk_env.bat first.
  exit /b 1
)

where ninja >nul 2>nul
if errorlevel 1 (
  echo [CTR Web] ninja was not found in PATH.
  exit /b 1
)

echo [CTR Web] Configuring Emscripten Release build...
emcmake cmake -S . -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
if errorlevel 1 exit /b %errorlevel%

echo [CTR Web] Building...
cmake --build build-web
if errorlevel 1 exit /b %errorlevel%

echo.
echo [CTR Web] Build complete: build-web\ctr-native.html
echo [CTR Web] Serve it locally with: python -m http.server 8000 -d build-web
