Package: refureku:x64-windows@1.0.0

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.42.34435.0
-    vcpkg-tool version: 2024-09-30-ab8988503c7cffabfd440b243a383c0a352a023d
    vcpkg-readonly: true
    vcpkg-scripts version: 2960d7d80e8d09c84ae8abf15c12196c2ca7d39a

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Downloading https://github.com/jsoysouvanh/Refureku/archive/master.tar.gz -> jsoysouvanh-Refureku-master.tar.gz...
[DEBUG] To include the environment variables in debug output, pass --debug-env
[DEBUG] Trying to load bundleconfig from C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg\vcpkg-bundle.json
[DEBUG] Bundle config: readonly=true, usegitregistry=true, embeddedsha=2960d7d80e8d09c84ae8abf15c12196c2ca7d39a, deployment=VisualStudio, vsversion=17.0
[DEBUG] VS telemetry opted in at SOFTWARE\WOW6432Node\Microsoft\VSCommon\17.0\SQM\\OptIn
[DEBUG] Metrics enabled.
[DEBUG] Feature flag 'binarycaching' unset
[DEBUG] Feature flag 'compilertracking' unset
[DEBUG] Feature flag 'registries' unset
[DEBUG] Feature flag 'versions' unset
[DEBUG] Feature flag 'dependencygraph' unset
[DEBUG] Trying to hash C:\Users\Theo\AppData\Local\vcpkg\downloads\jsoysouvanh-Refureku-master.tar.gz.2172.part
[DEBUG] C:\Users\Theo\AppData\Local\vcpkg\downloads\jsoysouvanh-Refureku-master.tar.gz.2172.part has hash f2611f6591977e09cddb71c7ed58cc3989e8c170d0901616dd6985c229afe8ea5208a9be29d1c91f8425d3321e4b344e17e243e06f9f88c8ce4d0f43202fc946
error: Missing jsoysouvanh-Refureku-master.tar.gz and downloads are blocked by x-block-origin.
error: File does not have the expected hash:
url: https://github.com/jsoysouvanh/Refureku/archive/master.tar.gz
File: C:\Users\Theo\AppData\Local\vcpkg\downloads\jsoysouvanh-Refureku-master.tar.gz.2172.part
Expected hash: 88427efc92c8f1f893323cb0ad30f85eee1f3b9bcf6d5d7d724dbf0d4bf07edabe683ec340bad5987b0e57e2748eacdc7aa30dbcb545c63eba0fba4ef36867f6
Actual hash: f2611f6591977e09cddb71c7ed58cc3989e8c170d0901616dd6985c229afe8ea5208a9be29d1c91f8425d3321e4b344e17e243e06f9f88c8ce4d0f43202fc946
[DEBUG] D:\a\_work\1\s\src\vcpkg\base\downloads.cpp(1030): 
[DEBUG] Time in subprocesses: 0us
[DEBUG] Time in parsing JSON: 11us
[DEBUG] Time in JSON reader: 0us
[DEBUG] Time in filesystem: 961us
[DEBUG] Time in loading ports: 0us
[DEBUG] Exiting after 1.1 s (1065705us)

CMake Error at scripts/cmake/vcpkg_download_distfile.cmake:32 (message):
      
      Failed to download file with error: 1
      If you are using a proxy, please check your proxy setting. Possible causes are:
      
      1. You are actually using an HTTP proxy, but setting HTTPS_PROXY variable
         to `https://address:port`. This is not correct, because `https://` prefix
         claims the proxy is an HTTPS proxy, while your proxy (v2ray, shadowsocksr
         , etc..) is an HTTP proxy. Try setting `http://address:port` to both
         HTTP_PROXY and HTTPS_PROXY instead.
      
      2. If you are using Windows, vcpkg will automatically use your Windows IE Proxy Settings
         set by your proxy software. See https://github.com/microsoft/vcpkg-tool/pull/77
         The value set by your proxy might be wrong, or have same `https://` prefix issue.
      
      3. Your proxy's remote server is out of service.
      
      If you've tried directly download the link, and believe this is not a temporary
      download server failure, please submit an issue at https://github.com/Microsoft/vcpkg/issues
      to report this upstream download server failure.
      

Call Stack (most recent call first):
  scripts/cmake/vcpkg_download_distfile.cmake:270 (z_vcpkg_download_distfile_show_proxy_and_fail)
  scripts/cmake/vcpkg_from_github.cmake:106 (vcpkg_download_distfile)
  C:/Users/Theo/TossEngine/vcpkg_overlay/ports/refureku/portfile.cmake:1 (vcpkg_from_github)
  scripts/ports.cmake:192 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "tossengine",
  "version-string": "1.0.0",
  "builtin-baseline": "c63619856b89f0af4d77ba2049396039e4985418",
  "dependencies": [
    "refureku",
    "nlohmann-json"
  ]
}

```
</details>
