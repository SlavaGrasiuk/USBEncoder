#pragma once

#include <scmRTOS.h>

using USBCtrlProc = OS::process<OS::pr0, 10'240>;

namespace OS {
	template<> void USBCtrlProc::exec();
}
