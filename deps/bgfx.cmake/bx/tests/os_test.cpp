/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/os.h>
#include <bx/semaphore.h>
#include <bx/timer.h>

TEST_CASE("getProcessMemoryUsed", "")
{
	REQUIRE(0 != bx::getProcessMemoryUsed() );
//	DBG("bx::getProcessMemoryUsed %d", bx::getProcessMemoryUsed() );
}

TEST_CASE("semaphore_timeout", "")
{
	bx::Semaphore sem;

	int64_t start = bx::getHPCounter();
	bool ok = sem.wait(900);
	int64_t elapsed = bx::getHPCounter() - start;
	int64_t frequency = bx::getHPFrequency();
	double ms = double(elapsed) / double(frequency) * 1000;
	printf("%f\n", ms);
	REQUIRE(!ok);
}
