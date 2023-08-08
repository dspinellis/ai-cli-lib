#include "CuTest.h"

char json[] = "{'id': 'chatcmpl-7kiJx4kLB3AFaUr4fSyFdkWaK6Op4', 'object': 'chat.completion', 'created': 1691367813, 'model': 'gpt-3.5-turbo-0613', 'choices': [{'index': 0, 'message': {'role': 'assistant', 'content': 'display a[i]'}, 'finish_reason': 'stop'}], 'usage': {'prompt_tokens': 106, 'completion_tokens': 4, 'total_tokens': 110 }}";

void test_json_parse(CuTest* tc)
{
	CuAssertStrEquals(tc, "display a[i]", "display a[i]");
}

CuSuite* cu_json_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_json_parse);

	return suite;
}


