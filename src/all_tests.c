#include <stdio.h>

#include "CuTest.h"

CuSuite* cu_config_suite();
CuSuite* cu_fetch_openai_suite();
CuSuite* cu_fetch_llamacpp_suite();
CuSuite* cu_support_suite();

void
run_all_tests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, cu_config_suite());
	CuSuiteAddSuite(suite, cu_fetch_openai_suite());
	CuSuiteAddSuite(suite, cu_fetch_llamacpp_suite());
	CuSuiteAddSuite(suite, cu_support_suite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

int main(void)
{
	run_all_tests();
}
