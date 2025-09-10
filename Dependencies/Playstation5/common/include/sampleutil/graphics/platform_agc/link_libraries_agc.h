/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#pragma comment(lib,"libSceAgcDriver_stub_weak.a")
#pragma comment(lib,"libSceAgc_stub_weak.a")
#ifdef _DEBUG
#pragma comment(lib,"libSceAgc_debug_nosubmission.a")
#pragma comment(lib,"libSceAgcCore_debug_nosubmission.a")
#pragma comment(lib,"libSceAgcGpuAddress_debug_nosubmission.a")
#else
#pragma comment(lib,"libSceAgc.a")
#pragma comment(lib,"libSceAgcCore.a")
#pragma comment(lib,"libSceAgcGpuAddress.a")
#endif


