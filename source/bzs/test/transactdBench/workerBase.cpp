#include "workerBase.h"

boost::mutex bzs::test::worker::workerBase::m_mutex;

int bzs::test::worker::g_bench_signal = 0;
